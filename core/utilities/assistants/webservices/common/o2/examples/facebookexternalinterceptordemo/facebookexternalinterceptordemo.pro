QT       += core gui webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

DEFINES += O0_EXPORT=
include(../../src/src.pri)

TARGET = facebookdemo
TEMPLATE = app

SOURCES += main.cpp \
    fbdemo.cpp \
    webenginepage.cpp \
    webwindow.cpp

HEADERS += \
    fbdemo.h \
    webenginepage.h \
    webwindow.h

FORMS += webwindow.ui
