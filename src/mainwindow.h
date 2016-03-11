#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>

class MainWindow : public QMainWindow
{
public:
  explicit MainWindow(QWidget *parent = 0);

  QWebEngineView* GetWebEngineView() const;

private:
  QWebEngineView* web_engine_view_ = nullptr;
};

#endif // MAINWINDOW_H
