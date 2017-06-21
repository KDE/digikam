/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a stand alone tool to brower a web page.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QUrl>
#include <QDebug>

// Local includes

#include "webbrowserdlg.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    if (argc == 1)
    {
        qDebug() << "browser - web page url to show";
        qDebug() << "Usage: url top open";
        return -1;
    }

    WebBrowserDlg browser(QUrl(QString::fromUtf8(argv[1])));
    browser.show();

    return a.exec();
}
