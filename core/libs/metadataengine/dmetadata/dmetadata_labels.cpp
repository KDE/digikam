/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
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
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

int DMetadata::getImagePickLabel() const
{
    if (getFilePath().isEmpty())
    {
        return -1;
    }

    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.PickLabel", false);

        if (!value.isEmpty())
        {
            bool ok     = false;
            long pickId = value.toLong(&ok);

            if (ok && pickId >= NoPickLabel && pickId <= AcceptedLabel)
            {
                return pickId;
            }
        }
    }

    return -1;
}

int DMetadata::getImageColorLabel() const
{
    if (getFilePath().isEmpty())
    {
        return -1;
    }

    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.ColorLabel", false);

        if (value.isEmpty())
        {
            // Nikon NX use this XMP tags to store Color Labels
            value = getXmpTagString("Xmp.photoshop.Urgency", false);
        }

        if (!value.isEmpty())
        {
            bool ok      = false;
            long colorId = value.toLong(&ok);

            if (ok && colorId >= NoColorLabel && colorId <= WhiteLabel)
            {
                return colorId;
            }
        }

        // LightRoom use this tag to store color name as string.
        // Values are limited : see bug #358193.

        value = getXmpTagString("Xmp.xmp.Label", false);

        if (value == QLatin1String("Blue"))
        {
            return BlueLabel;
        }
        else if (value == QLatin1String("Green"))
        {
            return GreenLabel;
        }
        else if (value == QLatin1String("Red"))
        {
            return RedLabel;
        }
        else if (value == QLatin1String("Yellow"))
        {
            return YellowLabel;
        }
        else if (value == QLatin1String("Purple"))
        {
            return MagentaLabel;
        }
    }

    return -1;
}

int DMetadata::getImageRating(const DMetadataSettingsContainer& settings) const
{
    if (getFilePath().isEmpty())
    {
        return -1;
    }

    long rating        = -1;
    bool xmpSupported  = hasXmp();
    bool iptcSupported = hasIptc();
    bool exivSupported = hasExif();

    for (NamespaceEntry entry : settings.getReadMapping(QString::fromUtf8(DM_RATING_CONTAINER)))
    {
        if (entry.isDisabled)
            continue;

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();
        QString value;

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                if (xmpSupported)
                    value = getXmpTagString(nameSpace, false);
                break;
            case NamespaceEntry::IPTC:
                if (iptcSupported)
                    value = QString::fromUtf8(getIptcTagData(nameSpace));
                break;
            case NamespaceEntry::EXIF:
                if (exivSupported)
                    getExifTagLong(nameSpace, rating);
                break;
            default:
                break;
        }

        if (!value.isEmpty())
        {
            bool ok = false;
            rating  = value.toLong(&ok);

            if (!ok)
            {
                return -1;
            }

        }
        int index = entry.convertRatio.indexOf(rating);

        // Exact value was not found,but rating is in range,
        // so we try to approximate it
        if ((index == -1)                         &&
            (rating > entry.convertRatio.first()) &&
            (rating < entry.convertRatio.last()))
        {
            for (int i = 0 ; i < entry.convertRatio.size() ; i++)
            {
                if (rating > entry.convertRatio.at(i))
                {
                    index = i;
                }
            }
        }

        if (index != -1)
        {
            return index;
        }
    }

    return -1;
}

bool DMetadata::setImagePickLabel(int pickId) const
{
    if (pickId < NoPickLabel || pickId > AcceptedLabel)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Pick Label value to write is out of range!";
        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Pick Label: " << pickId;

    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.PickLabel", QString::number(pickId)))
        {
            return false;
        }
    }

    return true;
}

bool DMetadata::setImageColorLabel(int colorId) const
{
    if (colorId < NoColorLabel || colorId > WhiteLabel)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Color Label value to write is out of range!";
        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Color Label: " << colorId;

    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.ColorLabel", QString::number(colorId)))
        {
            return false;
        }

        // Nikon NX use this XMP tags to store Color Labels
        if (!setXmpTagString("Xmp.photoshop.Urgency", QString::number(colorId)))
        {
            return false;
        }

        // LightRoom use this XMP tags to store Color Labels name
        // Values are limited : see bug #358193.

        QString LRLabel;

        switch(colorId)
        {
            case BlueLabel:
                LRLabel = QLatin1String("Blue");
                break;
            case GreenLabel:
                LRLabel = QLatin1String("Green");
                break;
            case RedLabel:
                LRLabel = QLatin1String("Red");
                break;
            case YellowLabel:
                LRLabel = QLatin1String("Yellow");
                break;
            case MagentaLabel:
                LRLabel = QLatin1String("Purple");
                break;
        }

        if (!LRLabel.isEmpty())
        {
            if (!setXmpTagString("Xmp.xmp.Label", LRLabel))
            {
                return false;
            }
        }
    }

    return true;
}

bool DMetadata::setImageRating(int rating, const DMetadataSettingsContainer& settings) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Urgency to store Rating.
    // Now this way is obsolete, and we use standard XMP rating tag instead.

    if (rating < RatingMin || rating > RatingMax)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Rating value to write is out of range!";
        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Rating:" << rating;

    QList<NamespaceEntry> toWrite = settings.getReadMapping(QString::fromUtf8(DM_RATING_CONTAINER));

    if (!settings.unifyReadWrite())
        toWrite = settings.getWriteMapping(QString::fromUtf8(DM_RATING_CONTAINER));

    for (NamespaceEntry entry : toWrite)
    {
        if (entry.isDisabled)
            continue;

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                if (!setXmpTagString(nameSpace, QString::number(entry.convertRatio.at(rating))))
                {
                    qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting rating failed" << nameSpace;
                    return false;
                }
                break;
            case NamespaceEntry::EXIF:
                if (!setExifTagLong(nameSpace, rating))
                {
                    return false;
                }
                break;
            case NamespaceEntry::IPTC: // IPTC rating deprecated
            default:
                break;
        }
    }

    // Set Exif rating tag used by Windows Vista.

    if (!setExifTagLong("Exif.Image.0x4746", rating))
    {
        return false;
    }

    // Wrapper around rating percents managed by Windows Vista.
    int ratePercents = 0;

    switch (rating)
    {
        case 0:
            ratePercents = 0;
            break;
        case 1:
            ratePercents = 1;
            break;
        case 2:
            ratePercents = 25;
            break;
        case 3:
            ratePercents = 50;
            break;
        case 4:
            ratePercents = 75;
            break;
        case 5:
            ratePercents = 99;
            break;
    }

    if (!setExifTagLong("Exif.Image.0x4749", ratePercents))
    {
        return false;
    }

    return true;
}

} // namespace Digikam
