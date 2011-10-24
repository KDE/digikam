/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : camera name helper class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "cameranamehelper.h"

// Qt includes

#include <QAction>

// KDE includes

#include <klocale.h>

namespace Digikam
{

QString CameraNameHelper::createCameraName(const QString& vendor, const QString& product,
                                           const QString& mode,   bool autoDetected)
{
    if (vendor.isEmpty())
    {
        return QString();
    }

    QString tmp;
    QString _vendor  = vendor.simplified();
    QString _product = product.simplified();
    QString _mode    = mode.simplified().remove('(').remove(')');

    tmp = QString("%1 %2").arg(_vendor)
          .arg(_product)
          .simplified();

    if (!mode.isEmpty())
    {
        tmp.append(" (");
        tmp.append(_mode);
        tmp.append(autoDetected ? QString(", %1)").arg(autoDetectedString())
                   : QString(')'));
    }
    else if (autoDetected)
    {
        tmp.append(QString(" (%1)").arg(autoDetectedString()));
    }

    return tmp.simplified();
}

QString CameraNameHelper::formattedCameraName(const QString& name, bool autoDetected)
{
    return parseAndFormatCameraName(name, false, autoDetected);
}

QString CameraNameHelper::formattedFullCameraName(const QString& name, bool autoDetected)
{
    return parseAndFormatCameraName(name, true, autoDetected);
}

QString CameraNameHelper::parseAndFormatCameraName(const QString& cameraName,
                                                   bool parseMode, bool autoDetected)
{
    QString tmp;

    QString vendorAndProduct = extractCameraNameToken(cameraName, VendorAndProduct);
    QString mode             = parseMode ? extractCameraNameToken(cameraName, Mode) : QString();

    if (vendorAndProduct.isEmpty())
    {
        return QString();
    }

    // we split vendorAndProduct once, it doesn't really matter if the variables are correctly filled,
    // it is only important that both are not empty.
    QStringList words = vendorAndProduct.split(' ');
    QString     vendor;
    QString     product;

    if (words.count() > 1)
    {
        vendor  = words.takeFirst();
        product = words.join(" ");
    }

    tmp = createCameraName(vendor, product, mode, autoDetected);
    return (tmp.isEmpty()) ? cameraName : tmp;
}

QString CameraNameHelper::autoDetectedString()
{
    return i18n("auto-detected");
}

QString CameraNameHelper::extractCameraNameToken(const QString& cameraName, int tokenID)
{
    QStringList capturedTexts;
    QString     tmp;

    capturedTexts = cameraName.split(" (");

    // TODO: Right now we just assume that a camera name has no parentheses in it
    //       There is a testcase (CameraNameHelperTest::testCameraNameFromGPCamera) that
    //       checks all camera names delivered by gphoto2. At the moment all seems to be fine.
    if (!capturedTexts.isEmpty())
    {
        QString mode;
        QString vendorAndProduct;

        if (capturedTexts.count() == 1)     // camera name only
        {
            vendorAndProduct = capturedTexts.takeFirst();
        }
        else
        {
            mode             = capturedTexts.takeLast().simplified();
            vendorAndProduct = capturedTexts.join((" ")).simplified();
        }

        if (tokenID == VendorAndProduct)
        {
            tmp = vendorAndProduct;
        }
        else if (tokenID == Mode)
        {
            tmp = mode;
        }

    }

    // clean up the string
    QStringList words = tmp.split((' '));
    tmp.clear();

    foreach (const QString& word, words)
    {
        tmp.append(word.simplified());
        tmp.append(' ');
    }

    return (tmp.isEmpty()) ? cameraName.simplified() : tmp.simplified();
}

bool CameraNameHelper::sameDevices(const QString& deviceA, const QString& deviceB)
{
    if (deviceA.isEmpty() || deviceB.isEmpty())
    {
        return false;
    }

    if (deviceA == deviceB)
    {
        return true;
    }

    // We need to parse the names a little bit. First check if the vendor and name match
    QString vendorAndProductA = extractCameraNameToken(deviceA, VendorAndProduct);
    QString vendorAndProductB = extractCameraNameToken(deviceB, VendorAndProduct);
    QString cameraNameA       = createCameraName(vendorAndProductA);
    QString cameraNameB       = createCameraName(vendorAndProductB);

    // try to clean up the string, if not possible, return false
    if (cameraNameA != cameraNameB)
    {
        QString tmpA = prepareStringForDeviceComparison(cameraNameA, VendorAndProduct);
        QString tmpB = prepareStringForDeviceComparison(cameraNameB, VendorAndProduct);

        if (tmpA != tmpB)
        {
            return false;
        }
    }

    // now check if the mode is the same
    QString modeA             = extractCameraNameToken(deviceA, Mode);
    QString modeB             = extractCameraNameToken(deviceB, Mode);

    // remove the 'mode' token for comparison
    QString strippedModeA     = prepareStringForDeviceComparison(modeA, Mode);
    QString strippedModeB     = prepareStringForDeviceComparison(modeB, Mode);

    if (strippedModeA == strippedModeB)
    {
        return true;
    }

    return false;
}

QString CameraNameHelper::prepareStringForDeviceComparison(const QString& string, int tokenID)
{
    QString tmp = string.toLower().remove('(').remove(')').remove(autoDetectedString()).simplified();

    if (tokenID == Mode)
    {
        tmp = tmp.remove("mode").remove(',');
    }

    return tmp.simplified();
}

} // namespace Digikam
