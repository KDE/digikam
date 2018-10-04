/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : a command line tool to detect image over exposure level
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

#include "dimg.h"
#include "previewloadthread.h"
#include "imagequalitycontainer.h"
#include "imagequalityparser.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "detectblur - Parse image data to detect over exposure level";
        qDebug() << "Usage: <image file>";
        return -1;
    }

    QApplication app(argc, argv);

    QString path = QString::fromUtf8(argv[1]);
    DImg dimg    = PreviewLoadThread::loadFastSynchronously(path, 1024);

    if (dimg.isNull())
    {
        qDebug() << path << "cannot be loaded...";
        return -1;
    }

    ImageQualityContainer settings;
    settings.enableSorter       = true;
    settings.detectBlur         = true;
    settings.detectNoise        = false;
    settings.detectCompression  = false;
    settings.detectOverexposure = false;
    settings.lowQRejected       = true;
    settings.mediumQPending     = true;
    settings.highQAccepted      = true;
    settings.rejectedThreshold  = 10;
    settings.pendingThreshold   = 40;
    settings.acceptedThreshold  = 60;
    settings.blurWeight         = 100;
    settings.noiseWeight        = 100;
    settings.compressionWeight  = 100;
    settings.speed              = 1;

    PickLabel pick;
    ImageQualityParser parser (dimg, settings, &pick);
    parser.startAnalyse();

    qDebug() << "Over exposure quality result is" << pick << "(0:None, 1:Rejected, 2:Pending, 3:Accepted)";

    return 0;
}
