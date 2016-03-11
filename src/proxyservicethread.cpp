#include <memory>

#include <QDebug>

#include "proxyservicethread.h"
#include "proxysessionthread.h"

namespace {
  namespace asio = boost::asio;
  using tcp = asio::ip::tcp;
}

void ProxyServiceThread::run() {
  try {
    tcp::acceptor acceptor(service_,
                           // {asio::ip::address_v4::loopback(), service_port_});
                           {asio::ip::address_v4::any(), service_port_});
    acceptor.listen();

    while (true) {
      auto socket = std::make_unique<tcp::socket>(service_);

      acceptor.accept(*socket);
      auto session = new ProxySessionThread(std::move(socket));
      session->start();
    }
  } catch (boost::system::system_error& ec) {
    qFatal(ec.what());
  }
}

quint16 ProxyServiceThread::ServerPort() const {
  return service_port_;
}
