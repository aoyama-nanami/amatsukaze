#-------------------------------------------------
#
# Project created by QtCreator 2016-02-06T12:30:51
#
#-------------------------------------------------

QT       += core gui webenginewidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AmatsukazeToolbox
TEMPLATE = app

BOOST_ROOT = "F:/Program Files/boost_1_60_0/"

INCLUDEPATH += $${BOOST_ROOT}

LIBS += -L$${BOOST_ROOT}/lib64-msvc-14.0

SOURCES += src/main.cpp\
    src/util.cpp \
    src/proxyservicethread.cpp \
    src/proxysessionthread.cpp \
    src/relaythread.cpp \
    src/kancolledatabase.cpp \
    src/mainwindow.cpp \
    src/kcsapi.cpp

HEADERS  += \
    src/util.h \
    src/proxyservicethread.h \
    src/proxysessionthread.h \
    src/relaythread.h \
    src/kancolledatabase.h \
    src/mainwindow.h \
    src/kcsapi.h
