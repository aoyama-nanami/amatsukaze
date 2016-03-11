#include <utility>

#include <QDebug>
#include <QNetworkProxy>

#include "proxysession.h"

ProxySession::ProxySession(QObject *parent, QTcpSocket *clientSocket)
    : QObject(parent), clientSocket_(clientSocket) {
  clientSocket_->setParent(this);
  connect(clientSocket, &QTcpSocket::disconnected, this,
          &ProxySession::deleteLater);
  resetToStartingState_();
}

void ProxySession::Debug() {
  auto state = "HEADER";
  if (state_ == State::BODY) state = "BODY";
  else if (state_ == State::END) state = "END";

  if (requestMethod_ == "POST") {
    qDebug() << this << " " << state << " " << requestMethod_ << " " << requestUri_
             << " (" << remainingBytes_ << " remaining)";
    qDebug() << serverSocket_->state();
  }
}

void ProxySession::clientReadyReadSlot_() {
  if (state_ == State::HEADER) {
    readOneLine_();
    tryProcessHeader_();
  } else if (state_ == State::BODY) {
    auto bytesAvailable = std::min(clientSocket_->bytesAvailable(),
                                   remainingBytes_);
    if (bytesAvailable > 0) {
      auto buf = clientSocket_->read(bytesAvailable);
      remainingBytes_ -= buf.size();
      serverSocket_->write(buf);
      // qDebug() << "Read Body:" << buf.size();
    }
    if (remainingBytes_ == 0) {
      state_ = State::END;
    }
  } else {
    disconnectClientRead_();
  }
  if (clientSocket_->bytesAvailable() > 0) {
    emit clientSocket_->readyRead();
  }
}

void ProxySession::serverConnectedSlot_() {
  qDebug() << __func__;
  if (requestMethod_ == "CONNECT") {
    state_ = State::BODY;
    remainingBytes_ = std::numeric_limits<qint64>::max();

    clientSocket_->write("HTTP/1.1 200 OK\r\n\r\n");
  } else {
    // send request line
    serverSocket_->write(requestMethod_);
    serverSocket_->write(" ");

    if (requestUri_.startsWith("http://")) {
      requestUri_ = requestUri_.mid(7);

    } else if (requestUri_.startsWith("https://")) {
      requestUri_ = requestUri_.mid(8);
    }
    serverSocket_->write(requestUri_.mid(requestUri_.indexOf('/')));

    serverSocket_->write(" HTTP/1.1\r\n");

    // send headers
    requestHeader_.pop_back();
    for (auto& line: requestHeader_) {
      if (line.startsWith("Proxy") || line.startsWith("Connection:")) {
        // drop all Proxy-* and Connection headers
        continue;
      }
      serverSocket_->write(line);
    }
    serverSocket_->write("Connection: Close\r\n\r\n");
  }

  connectClientRead_();
}

void ProxySession::serverDisconnectedSlot_() {
  qDebug() << __func__;
  if (requestMethod_ == "CONNECT") {
    clientSocket_->disconnectFromHost();
  } else {
    resetToStartingState_();
  }
}

void ProxySession::serverErrorSlot_(QTcpSocket::SocketError error) {
  if (error == QTcpSocket::RemoteHostClosedError) {
    // ignore
    return;
  }
  qDebug() << __func__ << error;
  clientSocket_->write("HTTP/1.1 502 Bad Gateway\r\n");
  clientSocket_->write("Connection: Close\r\n");
  clientSocket_->write("Content-Length: 0\r\n");
  clientSocket_->write("\r\n");
  clientSocket_->disconnectFromHost();
}

void ProxySession::serverReadyReadSlot_() {
  auto buf = serverSocket_->readAll();
  clientSocket_->write(buf);
}

void ProxySession::connectClientRead_() {
  connect(clientSocket_, &QTcpSocket::readyRead, this,
          &ProxySession::clientReadyReadSlot_);
  if (clientSocket_->bytesAvailable() > 0) {
    emit clientSocket_->readyRead();
  }
}

void ProxySession::disconnectClientRead_() {
    disconnect(clientSocket_, &QTcpSocket::readyRead, this,
               &ProxySession::clientReadyReadSlot_);
}

bool ProxySession::tryProcessHeader_() {
  if (requestHeader_.size() == 0) {
    return true;
  }
  if (requestHeader_.back().size() > 2) {
    return true;
  }

  /*
  for (auto& line: requestHeader_) {
    qDebug() << "Header: " << line;
  }
  */

  auto requestLine = requestHeader_[0].trimmed().split(' ');
  if (requestLine.size() != 3) {
    qFatal("Invalid HTTP header line");
    return false;
  }
  requestMethod_ = requestLine[0];
  requestUri_ = requestLine[1];
  requestHeader_.pop_front();

  QUrl fullUrl;
  if (requestMethod_ == "CONNECT") {
    requestLine[1].push_front("https://");
    fullUrl = QUrl(requestLine[1]);
  } else {
    fullUrl = QUrl(requestLine[1]);
  }

  if (fullUrl.isRelative()) {
    qFatal("Relative uri not implemented yet");
    return false;
  }

  qDebug() << "method: " << requestMethod_;
  qDebug() << "uri: " << requestUri_;

  remainingBytes_ = 0;
  if (requestMethod_ == "POST") {
    bool contentLengthFound = false;
    for (auto& line: requestHeader_) {
      if (line.startsWith("Content-Length:")) {
        auto tokens = line.split(':');
        remainingBytes_ = tokens[1].trimmed().toInt();
        contentLengthFound = true;
      }
    }

    if (!contentLengthFound) {
      qFatal("POST requires Content-Length header");
    }
  }
  state_ = State::BODY;

  if (serverSocket_ != nullptr) {
    qFatal("Server socket not cleared!");
    return false;
  }

  serverSocket_ = new QTcpSocket(this);
  serverSocket_->setProxy(QNetworkProxy::NoProxy);
  connect(serverSocket_, &QTcpSocket::connected, this,
          &ProxySession::serverConnectedSlot_);
  connect(serverSocket_, &QTcpSocket::disconnected, this,
          &ProxySession::serverDisconnectedSlot_);
  connect(serverSocket_,
          (void (QTcpSocket::*)(QTcpSocket::SocketError))&QTcpSocket::error,
          this, &ProxySession::serverErrorSlot_);
  connect(serverSocket_, &QTcpSocket::readyRead, this,
          &ProxySession::serverReadyReadSlot_);
  qDebug() << "connect to " << fullUrl.host() << ":" << fullUrl.port(80);
  serverSocket_->connectToHost(fullUrl.host(), fullUrl.port(80));

  disconnectClientRead_();

  return true;
}

void ProxySession::readOneLine_() {
  char c;
  while (clientSocket_->getChar(&c)) {
    if (c == '\n' && incompleteLine_.endsWith('\r')) {
      incompleteLine_.append(c);
      requestHeader_.append(incompleteLine_);
      incompleteLine_.clear();
    } else {
      incompleteLine_.append(c);
    }
  }
}

void ProxySession::resetToStartingState_() {
  requestHeader_.clear();
  incompleteLine_.clear();
  state_ = State::HEADER;
  remainingBytes_ = 0;
  if (serverSocket_ != nullptr) {
      serverSocket_->deleteLater();
  }
  serverSocket_ = nullptr;
  disconnectClientRead_();
  connectClientRead_();
}
