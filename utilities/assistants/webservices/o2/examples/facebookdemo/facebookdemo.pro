QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

DEFINES += O0_EXPORT=
include(../../src/src.pri)

TARGET = facebookdemo
TEMPLATE = app

SOURCES += main.cpp \
    fbdemo.cpp

HEADERS += \
    fbdemo.h
