/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : Video thumbnailer CLI test tool
 *
 * Copyright (C) 2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "loadvideothumb.h"

// Qt includes

#include <QApplication>
#include <QDebug>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "loadvideothumb - Load video file to extract thumbnail";
        qDebug() << "Usage: <videofile>";
        return -1;
    }

    QApplication app(argc, argv);
    VideoThumbnailer* const vthumb = new VideoThumbnailer(&app);
    vthumb->setThumbnailSize(256);
    
    QObject::connect(vthumb, SIGNAL(signalVideoThumbDone()),
                     &app, SLOT(quit()));

    if (!vthumb->getThumbnail(QString::fromUtf8(argv[1])))
        return -1;

    return app.exec();
}
