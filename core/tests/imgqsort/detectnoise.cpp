/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : a command line tool to detect image noise level
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QDebug>

// Local includes

#include "imgqsorttest_shared.h"
#include "digikam_globals.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "detectnoise - Parse image data to detect noise level";
        qDebug() << "Usage: <image file>";
        return -1;
    }

    QApplication app(argc, argv);

    QString path                = QString::fromUtf8(argv[1]);
    QFileInfoList list          = QFileInfoList() << QFileInfo(path);
    QMultiMap<int, QString> map = ImgQSortTest_ParseTestImages(DetectCompression, list);
    PickLabel pick              = (PickLabel)map.key(path);

    qDebug() << "Noise quality result is" << pick << "(0:None, 1:Rejected, 2:Pending, 3:Accepted)";

    return 0;
}
