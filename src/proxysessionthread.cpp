#include <QDebug>
#include <QUrl>

#include <boost/algorithm/string.hpp>

#include "kancolledatabase.h"
#include "kcsapi.h"
#include "proxysessionthread.h"
#include "relaythread.h"

#define QDEBUG_THIS (qDebug() << this)

namespace {
  namespace asio = boost::asio;

  const std::string MESSAGE_200 = "HTTP/1.1 200 Connection Established\r\n\r\n";
  const std::string MESSAGE_504 = "HTTP/1.1 504 Gateway Timeout\r\n\r\n";

  KcsApi GetKcsApiName(const std::string& url_string) {
    QUrl url(QString::fromUtf8(url_string.c_str()));
    if (url.path() == "/kcsapi/api_start2") {
      return KcsApi::START2;
    } else if (url.path() == "/kcsapi/api_get_member/slot_item") {
      return KcsApi::SLOT_ITEM;
    }
    return KcsApi::NONE;
  }
}

ProxySessionThread::ProxySessionThread(std::unique_ptr<tcp::socket>&& client)
    : client_(std::move(client)) {
  connect(this, &ProxySessionThread::finished,
          this, &ProxySessionThread::deleteLater);
}

void ProxySessionThread::run() {
  try {
    while (true) {
      auto request_line = ReadNextLine_();
      QDEBUG_THIS << "METHOD: " << request_line.c_str();

      std::vector<std::string> tokens;
      boost::algorithm::split(tokens, request_line,
                              boost::algorithm::is_space(),
                              boost::algorithm::token_compress_on);
      Q_ASSERT(tokens.size() == 3);

      if (tokens[0] == "CONNECT") {
        HandleConnect_(request_line);
        break;
      } else {
        HandleGetOrPost_(request_line);
      }
    }
  } catch (boost::system::system_error& ec) {
    if (ec.code() != asio::error::eof) {
      QDEBUG_THIS << "ERROR " << ec.what();
    }
  }
}

void ProxySessionThread::HandleConnect_(const std::string& request_line) {
  while (true) {
    auto header = ReadNextLine_();
    if (header == "\r\n") {
      break;
    }
  }

  auto server = TryConnect_(request_line);
  if (!server) {
    asio::write(*client_, asio::buffer(MESSAGE_504));
    return;
  }

  asio::write(*client_, asio::buffer(MESSAGE_200));

  RelayThread t1(*client_, *server), t2(*server, *client_);
  t1.start();
  t2.start();
  t1.wait();
  t2.wait();
}

void ProxySessionThread::HandleGetOrPost_(const std::string& request_line) {
  auto server = TryConnect_(request_line);
  if (!server) {
    asio::write(*client_, asio::buffer(MESSAGE_504));
    while (true) {
      auto header = ReadNextLine_();
      if (header == "\r\n") {
        break;
      }
    }
    return;
  }

  std::vector<std::string> tokens;
  boost::algorithm::split(tokens, request_line,
                          boost::algorithm::is_space(),
                          boost::algorithm::token_compress_on);
  if (!boost::algorithm::starts_with(tokens[1], "http://")) {
    QDEBUG_THIS << "ERROR: Invalid url: " << tokens[1].c_str();
  }
  asio::write(*server, asio::buffer(tokens[0]));
  asio::write(*server, asio::buffer(" ", 1));
  auto index = tokens[1].find('/', 7);
  asio::write(*server, asio::buffer(tokens[1].substr(index)));
  asio::write(*server, asio::buffer(" ", 1));
  asio::write(*server, asio::buffer(tokens[2]));
  asio::write(*server, asio::buffer("\r\n", 2));

  size_t content_length = 0;
  while (true) {
    auto header = ReadNextLine_();
    if (boost::algorithm::starts_with(header, "Connection:")) {
      asio::write(*server, asio::buffer("Connection: close\r\n"));
      continue;
    } else if (boost::algorithm::starts_with(header, "Proxy")) {
      continue;
    }

    if (boost::algorithm::starts_with(header, "Content-Length:")) {
      auto qstr = QString::fromUtf8(header.c_str());
      auto tokens = qstr.split(":");
      content_length = tokens[1].trimmed().toInt();
    }

    asio::write(*server, asio::buffer(header));
    if (header == "\r\n") {
      break;
    }
  }

  std::array<char, 4096> buf;
  while (content_length > 0) {
    auto max_read_size = std::min(buf.size(), content_length);
    auto nread = client_->read_some(asio::buffer(buf, max_read_size));
    content_length -= nread;
    asio::write(*server, asio::buffer(buf, nread));
  }

  auto api_name = GetKcsApiName(tokens[1]);
  std::string response;

  while (true) {
    boost::system::error_code ec;
    auto nread = server->read_some(asio::buffer(buf), ec);
    if (ec) break;
    asio::write(*client_, asio::buffer(buf, nread));
    if (api_name != KcsApi::NONE) response.append(buf.data(), nread);
  }

  if (api_name != KcsApi::NONE) {
    KanColleDatabase::GetInstance().ProcessData(api_name, response);
  }
}

auto ProxySessionThread::TryConnect_(
    const std::string& request_line) -> std::unique_ptr<tcp::socket> {
  auto& service = client_->get_io_service();

  std::vector<std::string> tokens;
  boost::algorithm::split(tokens, request_line,
                          boost::algorithm::is_space(),
                          boost::algorithm::token_compress_on);

  auto url_string = tokens[1];
  if (!boost::algorithm::contains(url_string, "://")) {
    url_string = "https://" + url_string;
  }
  QUrl url(QString::fromUtf8(url_string.c_str()));
  tcp::resolver resolver(service);
  auto server = std::make_unique<tcp::socket>(service);
  for (auto it = resolver.resolve({url.host().toStdString(), ""});
       it != tcp::resolver::iterator(); ++it) {
    auto endpoint = it->endpoint();
    endpoint.port(url.port(80));
    boost::system::error_code ec;
    server->connect(endpoint, ec);
    if (!ec) {
      return server;
    }
  }

  return nullptr;
}

std::string ProxySessionThread::ReadNextLine_() {
  std::string line;

  while (true) {
    char c;
    client_->read_some(asio::buffer(&c, sizeof(c)));
    line += c;
    if (c == '\n') break;
  }

  return line;
}
