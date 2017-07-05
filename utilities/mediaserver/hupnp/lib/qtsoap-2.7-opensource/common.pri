infile(config.pri, SOLUTIONS_LIBRARY, yes): CONFIG += qtsoap-uselib
TEMPLATE += fakelib
QTSOAP_LIBNAME = $$qtLibraryTarget(QtSolutions_SOAP)
VERSION = 2.7
TEMPLATE -= fakelib
QTSOAP_LIBDIR = ../lib
unix:qtsoap-uselib:!qtsoap-buildlib:QMAKE_RPATHDIR += .
