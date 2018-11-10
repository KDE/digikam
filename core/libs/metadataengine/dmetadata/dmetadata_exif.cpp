/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface - Exif helpers.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "iccprofile.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

int DMetadata::getMSecsInfo() const
{
    int ms  = 0;
    bool ok = mSecTimeStamp("Exif.Photo.SubSecTime", ms);
    if (ok) return ms;

    ok      = mSecTimeStamp("Exif.Photo.SubSecTimeOriginal", ms);
    if (ok) return ms;

    ok      = mSecTimeStamp("Exif.Photo.SubSecTimeDigitized", ms);
    if (ok) return ms;

    return 0;
}

bool DMetadata::mSecTimeStamp(const char* const exifTagName, int& ms) const
{
    bool ok     = false;
    QString val = getExifTagString(exifTagName);

    if (!val.isEmpty())
    {
        int sub = val.toUInt(&ok);

        if (ok)
        {
            int _ms = (int)(QString::fromLatin1("0.%1").arg(sub).toFloat(&ok) * 1000.0);

            if (ok)
            {
                ms = _ms;
                qCDebug(DIGIKAM_METAENGINE_LOG) << "msec timestamp: " << ms;
            }
        }
    }

    return ok;
}

IccProfile DMetadata::getIccProfile() const
{
    // Check if Exif data contains an ICC color profile.
    QByteArray data = getExifTagData("Exif.Image.InterColorProfile");

    if (!data.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Found an ICC profile in Exif metadata";
        return IccProfile(data);
    }

    // Else check the Exif color-space tag and use default profiles that we ship
    switch (getItemColorWorkSpace())
    {
        case DMetadata::WORKSPACE_SRGB:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif color-space tag is sRGB. Using default sRGB ICC profile.";
            return IccProfile::sRGB();
        }

        case DMetadata::WORKSPACE_ADOBERGB:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif color-space tag is AdobeRGB. Using default AdobeRGB ICC profile.";
            return IccProfile::adobeRGB();
        }

        default:
            break;
    }

    return IccProfile();
}

bool DMetadata::setIccProfile(const IccProfile& profile)
{
    if (profile.isNull())
    {
        removeExifTag("Exif.Image.InterColorProfile");
    }
    else
    {
        QByteArray data = IccProfile(profile).data();

        if (!setExifTagData("Exif.Image.InterColorProfile", data))
        {
            return false;
        }
    }

    removeExifColorSpace();

    return true;
}

bool DMetadata::removeExifColorSpace() const
{
    bool ret =  true;
    ret     &= removeExifTag("Exif.Photo.ColorSpace");
    ret     &= removeXmpTag("Xmp.exif.ColorSpace");

    return ret;
}

QString DMetadata::getExifTagStringFromTagsList(const QStringList& tagsList) const
{
    QString val;

    foreach(const QString& tag, tagsList)
    {
        val = getExifTagString(tag.toLatin1().constData());

        if (!val.isEmpty())
            return val;
    }

    return QString();
}

bool DMetadata::removeExifTags(const QStringList& tagFilters)
{
    MetaDataMap m = getExifTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeExifTag(it.key().toLatin1().constData());
    }

    return true;
}

} // namespace Digikam
