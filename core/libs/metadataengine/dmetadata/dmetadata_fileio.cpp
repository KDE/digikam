/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - file I/O helpers.
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
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QUuid>
#include <QMimeDatabase>

// Local includes

#include "filereadwritelock.h"
#include "metaenginesettings.h"
#include "drawinfo.h"
#include "drawdecoder.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::load(const QString& filePath)
{
    FileReadLocker lock(filePath);

    QMimeDatabase mimeDB;

    if (!mimeDB.mimeTypeForFile(filePath).name().startsWith(QLatin1String("video/")) &&
        !mimeDB.mimeTypeForFile(filePath).name().startsWith(QLatin1String("audio/"))
       )
    {
        // Non video or audio file, process with Exiv2 backend or libraw if faild with RAW files
        // Never process video file Exiv2, the backend is very unstable.
        if (!MetaEngine::load(filePath))
        {
            if (!loadUsingRawEngine(filePath))
            {
                return false;
            }
        }
    }
    else
    {
        // No image file, process with ffmpeg backend.
        if (!loadUsingFFmpeg(filePath))
        {
            return false;
        }
    }

    return true;
}

bool DMetadata::save(const QString& filePath, bool setVersion) const
{
    FileWriteLocker lock(filePath);
    return MetaEngine::save(filePath, setVersion);
}

bool DMetadata::applyChanges(bool setVersion) const
{
    FileWriteLocker lock(getFilePath());
    return MetaEngine::applyChanges(setVersion);
}

bool DMetadata::loadUsingRawEngine(const QString& filePath)
{
    DRawInfo identify;

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
            setItemDimensions(identify.imageSize);
        }

        // A RAW image is always uncalibrated. */
        setItemColorWorkSpace(WORKSPACE_UNCALIBRATED);

        return true;
    }

    return false;
}

} // namespace Digikam
