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

#include <QString>
#include <QFileInfo>

// Local includes

#include "rawinfo.h"
#include "drawdecoder.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::loadUsingRawEngine(const QString& filePath)
{
    RawInfo identify;

    if (DRawDecoder::rawFileIdentify(identify, filePath))
    {
        long int num = 1;
        long int den = 1;

        if (!identify.model.isNull())
        {
            setExifTagString("Exif.Image.Model", identify.model);
        }

        if (!identify.make.isNull())
        {
            setExifTagString("Exif.Image.Make", identify.make);
        }

        if (!identify.owner.isNull())
        {
            setExifTagString("Exif.Image.Artist", identify.owner);
        }

        if (identify.sensitivity != -1)
        {
            setExifTagLong("Exif.Photo.ISOSpeedRatings", lroundf(identify.sensitivity));
        }

        if (identify.dateTime.isValid())
        {
            setImageDateTime(identify.dateTime, false);
        }

        if (identify.exposureTime != -1.0)
        {
            convertToRationalSmallDenominator(identify.exposureTime, &num, &den);
            setExifTagRational("Exif.Photo.ExposureTime", num, den);
        }

        if (identify.aperture != -1.0)
        {
            convertToRational(identify.aperture, &num, &den, 8);
            setExifTagRational("Exif.Photo.ApertureValue", num, den);
        }

        if (identify.focalLength != -1.0)
        {
            convertToRational(identify.focalLength, &num, &den, 8);
            setExifTagRational("Exif.Photo.FocalLength", num, den);
        }

        if (identify.imageSize.isValid())
        {
            setImageDimensions(identify.imageSize);
        }

        // A RAW image is always uncalibrated. */
        setImageColorWorkSpace(WORKSPACE_UNCALIBRATED);

        return true;
    }

    return false;
}

} // namespace Digikam
