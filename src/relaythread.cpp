#include "relaythread.h"

#include <QDebug>

namespace {
  namespace asio = boost::asio;
}

RelayThread::RelayThread(tcp::socket& from, tcp::socket& to)
    : from_(from), to_(to) {
}

void RelayThread::run() {
  std::array<char, 4096> buf;
  boost::system::error_code ec;
  while (true) {
    auto nread = from_.read_some(asio::buffer(buf), ec);
    if (ec) {
      if (ec == boost::asio::error::eof) {
        to_.shutdown(tcp::socket::shutdown_send, ec);
      } else {
        to_.shutdown(tcp::socket::shutdown_both, ec);
      }
      break;
    }

    transfer_count_ += asio::write(to_, asio::buffer(buf, nread), ec);
    if (ec) {
      if (ec == boost::asio::error::eof) {
        from_.shutdown(tcp::socket::shutdown_receive, ec);
      } else {
        from_.shutdown(tcp::socket::shutdown_both, ec);
      }
      break;
    }
  }
}

size_t RelayThread::TransferCount() const {
  return transfer_count_;
}
