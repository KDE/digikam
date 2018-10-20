/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface - history helpers
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

#include <QUuid>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::setImageHistory(QString& imageHistoryXml) const
{
    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.ImageHistory", imageHistoryXml))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

QString DMetadata::getImageHistory() const
{
    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.ImageHistory", false);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Loading image history " << value;
        return value;
    }

    return QString();
}

bool DMetadata::hasImageHistoryTag() const
{
    if (hasXmp())
    {
        if (QString(getXmpTagString("Xmp.digiKam.ImageHistory", false)).length() > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

QString DMetadata::getImageUniqueId() const
{
    QString exifUid;

    if (hasXmp())
    {
        QString uuid = getXmpTagString("Xmp.digiKam.ImageUniqueID");

        if (!uuid.isEmpty())
        {
            return uuid;
        }

        exifUid = getXmpTagString("Xmp.exif.ImageUniqueId");
    }

    if (exifUid.isEmpty())
    {
        exifUid = getExifTagString("Exif.Photo.ImageUniqueID");
    }

    // same makers may choose to use a "click counter" to generate the id,
    // which is then weak and not a universally unique id
    // The Exif ImageUniqueID is 128bit, or 32 hex digits.
    // If the first 20 are zero, it's probably a counter,
    // the left 12 are sufficient for more then 10^14 clicks.
    if (!exifUid.isEmpty() && !exifUid.startsWith(QLatin1String("00000000000000000000")))
    {
        if (getExifTagString("Exif.Image.Make").contains(QLatin1String("SAMSUNG"), Qt::CaseInsensitive))
        {
            // Generate for Samsung a new random 32 hex digits unique ID.
            QString imageUniqueID(QUuid::createUuid().toString());
            imageUniqueID.remove(QLatin1Char('-'));
            imageUniqueID.remove(0, 1).chop(1);

            return imageUniqueID;
        }

        return exifUid;
    }

    // Exif.Image.ImageID can also be a pathname, so it's not sufficiently unique

    QString dngUid = getExifTagString("Exif.Image.RawDataUniqueID");

    if (!dngUid.isEmpty())
    {
        return dngUid;
    }

    return QString();
}

bool DMetadata::setImageUniqueId(const QString& uuid) const
{
    if (supportXmp())
    {
        return setXmpTagString("Xmp.digiKam.ImageUniqueID", uuid);
    }

    return false;
}

} // namespace Digikam
