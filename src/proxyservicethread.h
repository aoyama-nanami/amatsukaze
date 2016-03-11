#ifndef PROXYSERVICETHREAD_H
#define PROXYSERVICETHREAD_H

#include <QThread>

#include <boost/asio.hpp>

class ProxyServiceThread : public QThread {
public:
  ProxyServiceThread() = default;

  virtual void run() override;

  quint16 ServerPort() const;

private:
  const quint16 service_port_ = 22222;

  boost::asio::io_service service_;
};

#endif // PROXYSERVICETHREAD_H
