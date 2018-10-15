/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to detect image quality level - shared code
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imgqsorttest_shared.h"

// Qt includes

#include <QStringList>
#include <QDebug>

// Local includes

#include "dimg.h"
#include "previewloadthread.h"
#include "imagequalityparser.h"

using namespace Digikam;

QMultiMap<int, QString> ImgQSortTest_ParseTestImages(DetectionType type, const QFileInfoList& list)
{
    QString               tname;
    ImageQualityContainer settings;

    switch (type)
    {
        case DetectNoise:
            tname = QLatin1String("Noise");
            settings.enableSorter       = true;
            settings.detectBlur         = false;
            settings.detectNoise        = true;
            settings.detectCompression  = false;
            settings.detectExposure     = false;
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
            break;
        case DetectCompression:
            tname = QLatin1String("Compression");
            settings.enableSorter       = true;
            settings.detectBlur         = false;
            settings.detectNoise        = false;
            settings.detectCompression  = true;
            settings.detectExposure     = false;
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
            break;
        case DetectExposure:
            tname = QLatin1String("Exposure");
            settings.enableSorter       = true;
            settings.detectBlur         = false;
            settings.detectNoise        = false;
            settings.detectCompression  = false;
            settings.detectExposure     = true;
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
            break;
        default:
            tname = QLatin1String("Blur");
            settings.enableSorter       = true;
            settings.detectBlur         = true;
            settings.detectNoise        = false;
            settings.detectCompression  = false;
            settings.detectExposure     = false;
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
            break;
    }

    qDebug() << "Quality Detection Settings:" << settings;
    qDebug() << "Process images for" << tname << "detection (" << list.size() << ")";

    QMultiMap<int, QString> results;

    foreach (const QFileInfo& inf, list)
    {
        QString path = inf.filePath();
        qDebug() << path;

        DImg dimg    = PreviewLoadThread::loadFastSynchronously(path, 1024);

        if (dimg.isNull())
        {
            qDebug() << path << "File cannot be loaded...";
        }

        PickLabel pick;
        ImageQualityParser parser(dimg, settings, &pick);
        parser.startAnalyse();

        qDebug() << "==>" << tname << "quality result is" << pick;
        results.insert(pick, path);
    }

    qInfo() << tname << "Quality Results (0:None, 1:Rejected, 2:Pending, 3:Accepted):";

    for (QMap<int, QString>::const_iterator it = results.constBegin() ; it != results.constEnd() ; ++it)
    {
        qInfo() << "==>" << it.value() << ":" << it.key();
    }

    return results;
}
