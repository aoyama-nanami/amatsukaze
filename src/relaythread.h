#ifndef RELAYTHREAD_H
#define RELAYTHREAD_H

#include <QThread>

#include <boost/asio.hpp>

class RelayThread : public QThread
{
private:
  using tcp = boost::asio::ip::tcp;

public:
  RelayThread(tcp::socket& from, tcp::socket& to);

  virtual void run() override;

  size_t TransferCount() const;

private:
  tcp::socket& from_;
  tcp::socket& to_;
  size_t transfer_count_ = 0;
};

#endif // RELAYTHREAD_H
