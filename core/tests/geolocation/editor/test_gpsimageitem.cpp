/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-28
 * Description : Test loading and saving of data in GPSImageItem.
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "test_gpsimageitem.h"

// Qt includes

#include <QDateTime>
#include <QScopedPointer>
#include <QUrl>

// local includes

#include "dmetadata.h"
#include "gpsdatacontainer.h"
#include "gpsimageitem.h"


using namespace Digikam;

void TestGPSImageItem::initTestCase()
{
    // initialize exiv2 before doing any multitasking
    DMetadata::initializeExiv2();
}

void TestGPSImageItem::cleanupTestCase()
{
    // clean up the exiv2 memory:
    DMetadata::cleanupExiv2();
}

/**
 * @brief Return the path of the directory containing the test data
 */
QString GetTestDataDirectory()
{
    return QString(QFINDTESTDATA("data/"));
}

GPSImageItem* ItemFromFile(const QUrl& url)
{
    QScopedPointer<GPSImageItem> imageItem(new GPSImageItem(url));

    if (imageItem->loadImageData())
    {
        return imageItem.take();
    }

    return 0;
}

/**
 * @brief Dummy test that does nothing
 */
void TestGPSImageItem::testNoOp()
{
}

void TestGPSImageItem::testBasicLoading()
{
    {
        // test failure on not-existing file
        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("not-existing"));
        QScopedPointer<GPSImageItem> imageItem(ItemFromFile(testDataDir));
        QVERIFY(!imageItem);
    }

    {
        // load a file without GPS info
        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("exiftest-nogps.png"));
        QScopedPointer<GPSImageItem> imageItem(ItemFromFile(testDataDir));
        QVERIFY(imageItem);

        const GPSDataContainer container = imageItem->gpsData();
        QVERIFY(!container.hasCoordinates());
        QVERIFY(!container.hasAltitude());
        QVERIFY(!container.hasNSatellites());
        QVERIFY(!container.hasDop());
        QVERIFY(!container.hasFixType());
    }

    {
        // load a file with geo:5,15,25
        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("exiftest-5_15_25.jpg"));
        QScopedPointer<GPSImageItem> imageItem(ItemFromFile(testDataDir));
        QVERIFY(imageItem);

        const GPSDataContainer container = imageItem->gpsData();
        QVERIFY(container.hasCoordinates());
        QVERIFY(container.hasAltitude());
        QVERIFY(container.getCoordinates().lat() == 5.0);
        QVERIFY(container.getCoordinates().lon() == 15.0);
        QVERIFY(container.getCoordinates().alt() == 25.0);
        QVERIFY(!container.hasNSatellites());
        QVERIFY(!container.hasDop());
        QVERIFY(!container.hasFixType());
    }
}

QTEST_GUILESS_MAIN(TestGPSImageItem)
