/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : camera name helper class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

// KDE includes

#include <klocale.h>
#include <kdebug.h>

namespace Digikam
{

class CameraNameHelperPriv
{
public:

    CameraNameHelperPriv()
    {
    }

    bool    autodetect;
    QString originalCameraName;
    QString mode;
    QString vendor;
    QString product;
};

CameraNameHelper::CameraNameHelper(const QString& cameraName)
                : d(new CameraNameHelperPriv)
{
    d->originalCameraName = cameraName;
    d->autodetect         = false;
}

CameraNameHelper::~CameraNameHelper()
{
}

QString CameraNameHelper::createName(const QString& vendor, const QString& product,
                                     const QString& mode,   bool autoDetected)
{
    if (vendor.isEmpty() || product.isEmpty())
        return QString();

    QString tmp;
    QString vVendor       = validateStringAndCopy(vendor);
    QString vProduct      = validateStringAndCopy(product);
    QString vMode         = validateStringAndCopy(mode);

    tmp = QString("%1 %2").arg(vVendor)
                          .arg(vProduct);

    if (!mode.isEmpty())
    {
        tmp.append(" (");
        tmp.append(vMode);
        tmp.append(autoDetected ? QString(", %1)").arg(autoDetectedString())
                                : QString(')'));
    }
    else if (autoDetected)
    {
        tmp.append(QString(" (%1)").arg(autoDetectedString()));
    }

    return tmp;
}

QString CameraNameHelper::validateStringAndCopy(const QString& string)
{
    // remove leading and trailing whitespaces;
    QString tmp = string.trimmed();

    // remove parantheses
    QRegExp noParentheses("[()\\[\\]{}]");
    int pos = 0;
    while (pos > -1)
    {
        pos = noParentheses.indexIn(tmp, pos);
        if (pos > -1)
            tmp.remove(pos, noParentheses.matchedLength());
    }

    return tmp;
}

void CameraNameHelper::validateString(QString& string)
{
    string = validateStringAndCopy(string);
}

bool CameraNameHelper::isAutoDetected(const QString& name)
{
    QString vName   = validateStringAndCopy(name).toLower();
    QString autoTmp = autoDetectedString().toLower();

    if (vName.endsWith(autoTmp))
        return true;
    return false;
}

QString CameraNameHelper::autoDetectedString()
{
    return i18n("auto-detected");
}

void CameraNameHelper::setAutoDetected(bool v)
{
    d->autodetect = v;
}

QString CameraNameHelper::cameraName() const
{
    QString tmp;

    // We should check if vendor and product have been set manually, if not, use the originalCameraName
    // and try to extract the mode
    if (d->vendor.isEmpty() || d->product.isEmpty())
    {
        QRegExp nameRegEx("(.*)\\(.*\\)\\s*$");
        if (nameRegEx.isValid() && nameRegEx.indexIn(d->originalCameraName) != -1)
        {
            tmp               = nameRegEx.cap(1);
            QStringList words = tmp.split((' '));
            tmp.clear();

            foreach (const QString& word, words)
            {
                tmp.append(word.trimmed());
                tmp.append(' ');
            }
            validateString(tmp);
        }
        else if (!d->originalCameraName.isEmpty())
        {
            tmp = validateStringAndCopy(d->originalCameraName);
        }
    }
    else
    {
        tmp = createName(d->vendor, d->product);
    }

    return validateStringAndCopy(tmp);
}

QString CameraNameHelper::fullCameraName() const
{
    return createName(d->vendor, d->product, d->mode, d->autodetect);
}

void CameraNameHelper::setMode(const QString& mode)
{
    d->mode = mode;
}

void CameraNameHelper::setVendor(const QString& vendor)
{
    d->vendor = vendor;
}

void CameraNameHelper::setProduct(const QString& product)
{
    d->product = product;
}

} // namespace Digikam
