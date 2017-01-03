/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : Video thumbnailer CLI test tool
 *
 * Copyright (C) 2016-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "videothumbnailerjob.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        qDebug() << "videothumbtest - Load video files to extract thumbnails";
        qDebug() << "Usage: <videofiles>";
        return -1;
    }

    QApplication app(argc, argv);
    QStringList files;

    for (int i = 1 ; i < argc ; i++)
        files.append(QString::fromLocal8Bit(argv[i]));

    VideoThumbnailerJob* const vthumbs = new VideoThumbnailerJob(&app);
    vthumbs->setThumbnailSize(256);
    vthumbs->setCreateStrip(true);

    QObject::connect(vthumbs, SIGNAL(signalThumbnailJobFinished()),
                     &app, SLOT(quit()));

    qDebug() << "Video files to process : " << files;

    vthumbs->addItems(files);
    int ret = app.exec();

    return ret;
}
