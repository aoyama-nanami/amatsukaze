#include <QDebug>
#include <QNetworkProxy>
#include <QTimer>

#include "proxyserver.h"
#include "proxysession.h"

ProxyServer::ProxyServer(QObject *parent) : QTcpServer(parent) {
  setProxy(QNetworkProxy::NoProxy);
  connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout,
          [this]() {
            qDebug() << "Timer: " << children().size();
            for (auto obj: children()) {
              auto session = dynamic_cast<ProxySession*>(obj);
              if (session) {
                session->Debug();
              }
            }
          });
  timer->start(5000);
}

void ProxyServer::onNewConnection() {
  qDebug() << __func__;
  new ProxySession(this, nextPendingConnection());
}
