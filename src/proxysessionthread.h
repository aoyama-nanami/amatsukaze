#ifndef PROXYSESSIONTHREAD_H
#define PROXYSESSIONTHREAD_H

#include <memory>

#include <QThread>

#include <boost/asio.hpp>

class ProxySessionThread : public QThread
{
private:
  using tcp = boost::asio::ip::tcp;

public:
  ProxySessionThread(std::unique_ptr<tcp::socket>&& client);

  virtual void run() override;

private:
  std::unique_ptr<tcp::socket> client_;

  void HandleGetOrPost_(const std::string& request_line);
  void HandleConnect_(const std::string& request_line);
  std::unique_ptr<tcp::socket> TryConnect_(const std::string& request_line);
  std::string ReadNextLine_();
};

#endif // PROXYSESSIONTHREAD_H
