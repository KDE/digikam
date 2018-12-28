#!/bin/bash

# Copyright (c) 2013-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################
# Fix QtWebkit includes under MXE where camel case header files sound like not generated at install.

find . -regex '.*\.\(cpp\|h\)' -type f -print0 | xargs -r0 sed -e 's/#include <QWebView>/#include <qwebview.h>/g' -i
find . -regex '.*\.\(cpp\|h\)' -type f -print0 | xargs -r0 sed -e 's/#include <QWebPage>/#include <qwebpage.h>/g' -i
find . -regex '.*\.\(cpp\|h\)' -type f -print0 | xargs -r0 sed -e 's/#include <QWebFrame>/#include <qwebframe.h>/g' -i
find . -regex '.*\.\(cpp\|h\)' -type f -print0 | xargs -r0 sed -e 's/#include <QWebElement>/#include <qwebelement.h>/g' -i
find . -regex '.*\.\(cpp\|h\)' -type f -print0 | xargs -r0 sed -e 's/#include <QWebHistory>/#include <qwebhistory.h>/g' -i
