#include <QApplication>
#include <QHostAddress>
#include <QNetworkProxy>
#include <QUrl>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include "mainwindow.h"
#include "proxyservicethread.h"
#include "util.h"

auto KANCOLLE_URL =
    "http://www.dmm.com/netgame/social/-/gadgets/=/app_id=854854/";

auto EMAIL = "a265658@trbvn.com";
auto PASSWORD = "aaaa1234";

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  ProxyServiceThread proxyServer;
  proxyServer.start();

  QNetworkProxy::setApplicationProxy({QNetworkProxy::HttpProxy, "localhost",
                                      proxyServer.ServerPort()});
  QWebEngineSettings::globalSettings()->setAttribute(
      QWebEngineSettings::PluginsEnabled, true);

  MainWindow main_window;
  auto view = main_window.GetWebEngineView();
  view->page()->profile()->setHttpUserAgent("Chrome/45.0.2431.0");
  auto cookieStore = view->page()->profile()->cookieStore();
  for (auto cookie: GenerateKanColleCookies()) {
    cookieStore->setCookie(cookie);
  }

  view->load(QUrl(KANCOLLE_URL));

  main_window.show();

  return app.exec();
}
