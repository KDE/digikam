#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################
# Fix QtWebkit includes under MXE where camel case header files sound like not generated at install.

LC_CTYPE=C find . -name "*.cpp" -or -name "*.h" -type f -exec sed -i '' -e 's/#include <QWebView>/#include <qwebview.h>/g' {} \;
LC_CTYPE=C find . -name "*.cpp" -or -name "*.h" -type f -exec sed -i '' -e 's/#include <QWebPage>/#include <qwebpage.h>/g' {} \;
LC_CTYPE=C find . -name "*.cpp" -or -name "*.h" -type f -exec sed -i '' -e 's/#include <QWebFrame>/#include <qwebframe.h>/g' {} \;
