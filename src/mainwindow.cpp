﻿#include <QHBoxLayout>
#include <QTabWidget>

#include "mainwindow.h"
#include "slotitempanel.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  auto central_widget = new QWidget;
  setCentralWidget(central_widget);

  QHBoxLayout* layout = new QHBoxLayout;
  central_widget->setLayout(layout);

  web_engine_view_ = new QWebEngineView;
  web_engine_view_->setMinimumWidth(996);
  layout->addWidget(web_engine_view_);

  auto tab_widget = new QTabWidget;
  layout->addWidget(tab_widget);

  tab_widget->addTab(new SlotItemPanel, "slots");
  tab_widget->addTab(new QWidget, "ships");  // TODO: incomplete
}

QWebEngineView* MainWindow::GetWebEngineView() const {
  return web_engine_view_;
}
