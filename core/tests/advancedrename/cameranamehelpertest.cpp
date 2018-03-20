/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : a test for the CameraNameHelper
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "cameranamehelpertest.h"

// Qt includes

#include <QTest>

// Local includes

#include "cameranamehelper.h"
#include "digikam_config.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(CameraNameHelperTest)

void CameraNameHelperTest::testCreateCameraName_data()
{
    QTest::addColumn<QString>("vendor");
    QTest::addColumn<QString>("product");
    QTest::addColumn<QString>("mode");
    QTest::addColumn<bool>("autoDetect");
    QTest::addColumn<QString>("result");

    QString autoString = QLatin1String("auto-detected");

    QTest::newRow("01") << "Nikon"
                        << "D50"
                        << "PTP Mode"
                        << true
                        << QString::fromUtf8("Nikon D50 (PTP Mode, %1)").arg(autoString);

    QTest::newRow("02") << "  Canon   "
                        << "Powershot A80"
                        << ""
                        << false
                        << QString::fromUtf8("Canon Powershot A80");

    QTest::newRow("03") << "  Canon   "
                        << "Powershot A80"
                        << ""
                        << true
                        << QString::fromUtf8("Canon Powershot A80 (%1)").arg(autoString);

    QTest::newRow("04") << "  Canon   "
                        << ""
                        << "PTP"
                        << true
                        << QString::fromUtf8("Canon (PTP, auto-detected)");

    QTest::newRow("05") << ""
                        << "D50"
                        << "PTP Mode"
                        << true
                        << QString::fromUtf8("");

    QTest::newRow("06") << "Nikon"
                        << "D50"
                        << "(PTP Mode)"
                        << true
                        << QString::fromUtf8("Nikon D50 (PTP Mode, %1)").arg(autoString);
}

void CameraNameHelperTest::testCreateCameraName()
{
    QFETCH(QString, vendor);
    QFETCH(QString, product);
    QFETCH(QString, mode);
    QFETCH(bool,    autoDetect);
    QFETCH(QString, result);

    QCOMPARE(CameraNameHelper::createCameraName(vendor, product, mode, autoDetect), result);
}

void CameraNameHelperTest::testSameDevices_data()
{
    QTest::addColumn<QString>("deviceA");
    QTest::addColumn<QString>("deviceB");
    QTest::addColumn<bool>("result");

    QTest::newRow("01") << "Nikon D50 (ptp, auto-detected)" << "Nikon D50 (PTP Mode)" << true;
    QTest::newRow("02") << "Nikon D50 (ptp mode)"           << "Nikon D50 (PTP)"      << true;
    QTest::newRow("03") << "Nikon D50 (auto-detected)"      << "Nikon D50"            << true;
    QTest::newRow("04") << "Nikon D50 (ptp mode)"           << "Nikon D50 (mtp mode)" << false;
    QTest::newRow("05") << "Nikon D50 (ptp mode)"           << "Nikon D50"            << false;
}

void CameraNameHelperTest::testCameraName_data()
{
    QTest::addColumn<QString>("device");
    QTest::addColumn<QString>("result");

    QTest::newRow("01") << "Nikon D50 (ptp, auto-detected)" << "Nikon D50";
    QTest::newRow("02") << "Nikon D50 (auto-detected)"      << "Nikon D50";
    QTest::newRow("03") << "Nikon D50 (ptp)"                << "Nikon D50";
    QTest::newRow("04") << "Nikon D50 (something else)"     << "Nikon D50 (something else)";
    QTest::newRow("05") << "Nikon D50 (huhu) blubber"       << "Nikon D50 (huhu) blubber";
}

void CameraNameHelperTest::testCameraName()
{
    QFETCH(QString, device);
    QFETCH(QString, result);

    QCOMPARE(CameraNameHelper::cameraName(device), result);
}

void CameraNameHelperTest::testCameraNameAutoDetected_data()
{
    QTest::addColumn<QString>("device");
    QTest::addColumn<QString>("result");

    QTest::newRow("01") << "Nikon D50 (ptp, auto-detected)" << "Nikon D50 (ptp, auto-detected)";
    QTest::newRow("02") << "Nikon D50 (auto-detected)"      << "Nikon D50 (auto-detected)";
    QTest::newRow("03") << "Nikon D50 (ptp)"                << "Nikon D50 (ptp, auto-detected)";
    QTest::newRow("04") << "Nikon D50 (something else)"     << "Nikon D50 (something else) (auto-detected)";
    QTest::newRow("05") << "Nikon D50 (huhu) blubber"       << "Nikon D50 (huhu) blubber (auto-detected)";
}

void CameraNameHelperTest::testCameraNameAutoDetected()
{
    QFETCH(QString, device);
    QFETCH(QString, result);

    QCOMPARE(CameraNameHelper::cameraNameAutoDetected(device), result);
}

void CameraNameHelperTest::testSameDevices()
{
    QFETCH(QString, deviceA);
    QFETCH(QString, deviceB);
    QFETCH(bool,    result);

    QCOMPARE(CameraNameHelper::sameDevices(deviceA, deviceB), result);
}
