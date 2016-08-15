/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : camera name helper class
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include <QRegExp>

namespace
{
static const QString STR_AUTO_DETECTED(QLatin1String("auto-detected"));

static QRegExp REGEXP_CAMERA_NAME(QLatin1String("^(.*)\\s*\\((.*)\\)\\s*$"), Qt::CaseInsensitive);
static QRegExp REGEXP_MODES(QLatin1String("^(ptp|normal|mtp)(\\s+mode)?$"), Qt::CaseInsensitive);
static QRegExp REGEXP_AUTODETECTED(QString::fromUtf8("(%1|, %1)").arg(STR_AUTO_DETECTED));
}

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
    QString _mode    = mode.simplified().remove(QLatin1Char('(')).remove(QLatin1Char(')'));
    tmp              = QString::fromUtf8("%1 %2").arg(_vendor).arg(_product);

    if (!mode.isEmpty() && mode != STR_AUTO_DETECTED)
    {
        tmp.append(QLatin1String(" ("));
        tmp.append(_mode);
        tmp.append(autoDetected ? QString::fromUtf8(", %1)").arg(STR_AUTO_DETECTED)
                                : QLatin1String(")"));
    }
    else if (autoDetected)
    {
        tmp.append(QString::fromUtf8(" (%1)").arg(STR_AUTO_DETECTED));
    }

    return tmp.simplified();
}

QString CameraNameHelper::cameraName(const QString& name)
{
    return parseAndFormatCameraName(name, false, false);
}

QString CameraNameHelper::cameraNameAutoDetected(const QString& name)
{
    return parseAndFormatCameraName(name, true, true);
}

QString CameraNameHelper::parseAndFormatCameraName(const QString& cameraName,
                                                   bool parseMode, bool autoDetected)
{
    QString vendorAndProduct = extractCameraNameToken(cameraName, VendorAndProduct);

    if (vendorAndProduct.isEmpty())
    {
        return QString();
    }

    QString mode = parseMode ? extractCameraNameToken(cameraName, Mode)
                             : QString();

    QString tmp = createCameraName(vendorAndProduct, QString(), mode, autoDetected);

    return tmp.isEmpty() ? cameraName.simplified()
                         : tmp;
}

QString CameraNameHelper::extractCameraNameToken(const QString& cameraName, Token tokenID)
{
    REGEXP_CAMERA_NAME.setMinimal(true);
    REGEXP_MODES.setMinimal(true);
    REGEXP_AUTODETECTED.setMinimal(true);

    if (REGEXP_CAMERA_NAME.exactMatch(cameraName.simplified()))
    {
        QString vendorProduct  = REGEXP_CAMERA_NAME.cap(1).simplified();
        QString tmpMode        = REGEXP_CAMERA_NAME.cap(2).simplified();
        QString clearedTmpMode = tmpMode;
        QString mode;
        clearedTmpMode.remove(REGEXP_AUTODETECTED);

        if (!tmpMode.isEmpty() && clearedTmpMode.isEmpty())
        {
            mode = tmpMode;
        }
        else
        {
            mode = REGEXP_MODES.exactMatch(clearedTmpMode) ? clearedTmpMode
                                                           : QLatin1String("");
        }

        if (tokenID == VendorAndProduct)
        {
            return mode.isEmpty() ? cameraName.simplified()
                                  : vendorProduct;
        }
        else
        {
            return mode;
        }
    }
    return (tokenID == VendorAndProduct) ? cameraName.simplified()
                                         : QLatin1String("");
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
        return false;
    }

    // is the extracted mode known and equal?
    QString modeA       = extractCameraNameToken(deviceA, Mode);
    QString modeB       = extractCameraNameToken(deviceB, Mode);
    bool isModeAValid   = REGEXP_MODES.exactMatch(modeA);
    modeA               = isModeAValid ? REGEXP_MODES.cap(1).simplified().toLower() : QLatin1String("");
    bool isModeBValid   = REGEXP_MODES.exactMatch(modeB);
    modeB               = isModeBValid ? REGEXP_MODES.cap(1).simplified().toLower() : QLatin1String("");

    if ((isModeAValid != isModeBValid) || (modeA != modeB))
    {
        return false;
    }

    return true;
}

} // namespace Digikam
