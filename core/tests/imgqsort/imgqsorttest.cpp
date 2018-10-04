/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to detect image quality level
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

#include "imgqsorttest.h"

// Qt includes

#include <QtTest>
#include <QStringList>
#include <QFileInfoList>
#include <QDebug>

// Local includes

#include "dimg.h"
#include "previewloadthread.h"
#include "imagequalityparser.h"

QTEST_MAIN(ImgQSortTest)

void ImgQSortTest::initTestCase()
{
}

void ImgQSortTest::cleanupTestCase()
{
}

QDir ImgQSortTest::imageDir() const
{
    QDir dir(QFINDTESTDATA("data/"));
//    qDebug() << "Images Directory:" << dir;
    return dir;
}

QMultiMap<int, QString> ImgQSortTest::parseTestImages(const QString& tname,
                                                      const QFileInfoList& list,
                                                      const ImageQualityContainer& settings) const
{
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

void ImgQSortTest::testParseTestImagesForBlurDetection()
{
    QFileInfoList list = imageDir().entryInfoList(QStringList() << QLatin1String("test_blurred*.jpg"),
                                                  QDir::Files, QDir::Name);
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

    QMultiMap<int, QString> results = parseTestImages(QLatin1String("Blur"), list, settings);

    QVERIFY(results.count(NoPickLabel)   == 0);
    QVERIFY(results.count(RejectedLabel) == 1);
    QVERIFY(results.count(PendingLabel)  == 8);
    QVERIFY(results.count(AcceptedLabel) == 0);
}

void ImgQSortTest::testParseTestImagesForNoiseDetection()
{
    QFileInfoList list = imageDir().entryInfoList(QStringList() << QLatin1String("test_noised*.jpg"),
                                                  QDir::Files, QDir::Name);

    ImageQualityContainer settings;
    settings.enableSorter       = true;
    settings.detectBlur         = false;
    settings.detectNoise        = true;
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

    QMultiMap<int, QString> results = parseTestImages(QLatin1String("Noise"), list, settings);

    QVERIFY(results.count(NoPickLabel)   == 0);
    QVERIFY(results.count(RejectedLabel) == 1);
    QVERIFY(results.count(PendingLabel)  == 8);
    QVERIFY(results.count(AcceptedLabel) == 0);
}

void ImgQSortTest::testParseTestImagesForCompressionDetection()
{
    QFileInfoList list = imageDir().entryInfoList(QStringList() << QLatin1String("test_compressed*.jpg"),
                                                  QDir::Files, QDir::Name);

    ImageQualityContainer settings;
    settings.enableSorter       = true;
    settings.detectBlur         = false;
    settings.detectNoise        = false;
    settings.detectCompression  = true;
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

    QMultiMap<int, QString> results = parseTestImages(QLatin1String("Compression"), list, settings);

    QVERIFY(results.count(NoPickLabel)   == 9);
    QVERIFY(results.count(RejectedLabel) == 0);
    QVERIFY(results.count(PendingLabel)  == 0);
    QVERIFY(results.count(AcceptedLabel) == 0);
}

void ImgQSortTest::testParseTestImagesForOverExpoDetection()
{
    QFileInfoList list = imageDir().entryInfoList(QStringList() << QLatin1String("test_overexposed*.jpg"),
                                                  QDir::Files, QDir::Name);

    ImageQualityContainer settings;
    settings.enableSorter       = true;
    settings.detectBlur         = false;
    settings.detectNoise        = false;
    settings.detectCompression  = false;
    settings.detectOverexposure = true;
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

    QMultiMap<int, QString> results = parseTestImages(QLatin1String("Over-Exposed"), list, settings);

    QVERIFY(results.count(NoPickLabel)   == 9);
    QVERIFY(results.count(RejectedLabel) == 0);
    QVERIFY(results.count(PendingLabel)  == 0);
    QVERIFY(results.count(AcceptedLabel) == 0);
}
