#ifndef PROXYSESSION_H
#define PROXYSESSION_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QTcpSocket>
#include <QUrl>

class ProxySession : public QObject {
  Q_OBJECT
public:
  explicit ProxySession(QObject *parent, QTcpSocket *clientSocket);
  void Debug();

private slots:
  void clientReadyReadSlot_();
  void serverConnectedSlot_();
  void serverDisconnectedSlot_();
  void serverErrorSlot_(QTcpSocket::SocketError error);
  void serverReadyReadSlot_();

private:
  QTcpSocket *clientSocket_;
  QList<QByteArray> requestHeader_;
  QByteArray incompleteLine_;
  QByteArray requestMethod_;
  QByteArray requestUri_;
  enum class State {HEADER, BODY, END} state_ = State::HEADER;
  qint64 remainingBytes_ = 0;

  QTcpSocket *serverSocket_ = nullptr;

  void connectClientRead_();
  void disconnectClientRead_();
  bool tryProcessHeader_();
  void readOneLine_();
  void resetToStartingState_();
};

#endif // PROXYSESSION_H
