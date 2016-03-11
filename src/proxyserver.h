#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <QTcpServer>

class ProxyServer: public QTcpServer {
  Q_OBJECT
public:
  explicit ProxyServer(QObject *parent = nullptr);

private slots:
  void onNewConnection();
};

#endif // PROXYSERVER_H
