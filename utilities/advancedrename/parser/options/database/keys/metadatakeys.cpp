/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : metadata information keys
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

#include "metadatakeys.h"

// KDE includes

#include <klocale.h>

// local includes

#include "databaseinfocontainers.h"
#include "imageinfo.h"

namespace
{
static const QString KEY_MAKE("CameraMake");
static const QString KEY_MODEL("CameraModel");
static const QString KEY_LENS("CameraLens");
static const QString KEY_APERTURE("Aperture");
static const QString KEY_FOCALLENGTH("FocalLength");
static const QString KEY_FOCALLENGTH35("FocalLength35");
static const QString KEY_EXPOSURETIME("ExposureTime");
static const QString KEY_EXPOSUREPROGRAM("ExposureProgram");
static const QString KEY_EXPOSUREMODE("ExposureMode");
static const QString KEY_SENSITIVITY("Sensitivity");
static const QString KEY_FLASHMODE("FlashMode");
static const QString KEY_WHITEBALANCE("WhiteBalance");
static const QString KEY_WHITEBALANCECOLORTEMPERATURE("WhiteBalanceColorTemp");
static const QString KEY_METERINGMODE("MeteringMode");
static const QString KEY_SUBJECTDISTANCE("SubjectDistance");
static const QString KEY_SUBJECTDISTANCECATEGORY("SubjectDistanceCategory");
}

namespace Digikam
{

MetadataKeys::MetadataKeys()
    : DbKeysCollection(i18n("Metadata Information"))
{
    addId(KEY_MAKE,                         i18n("Make of the camera"));
    addId(KEY_MODEL,                        i18n("Model of the camera"));
    addId(KEY_LENS,                         i18n("Lens of the camera"));
    addId(KEY_APERTURE,                     i18n("Aperture"));
    addId(KEY_FOCALLENGTH,                  i18n("Focal length"));
    addId(KEY_FOCALLENGTH35,                i18n("Focal length (35mm equivalent)"));
    addId(KEY_EXPOSURETIME,                 i18n("Exposure time"));
    addId(KEY_EXPOSUREPROGRAM,              i18n("Exposure program"));
    addId(KEY_EXPOSUREMODE,                 i18n("Exposure mode"));
    addId(KEY_SENSITIVITY,                  i18n("Sensitivity"));
    addId(KEY_FLASHMODE,                    i18n("Flash mode"));
    addId(KEY_WHITEBALANCE,                 i18n("White balance"));
    addId(KEY_WHITEBALANCECOLORTEMPERATURE, i18n("White balance (color temperature)"));
    addId(KEY_METERINGMODE,                 i18n("Metering mode"));
    addId(KEY_SUBJECTDISTANCE,              i18n("Subject distance"));
    addId(KEY_SUBJECTDISTANCECATEGORY,      i18n("Subject distance (category)"));
}

QString MetadataKeys::getDbValue(const QString& key, ParseSettings& settings)
{
    ImageInfo info(settings.fileUrl);
    ImageMetadataContainer container = info.imageMetadataContainer();
    QString result;

    if (key == KEY_MAKE)
    {
        result = container.make;
    }
    else if (key == KEY_MODEL)
    {
        result = container.model;
    }
    else if (key == KEY_LENS)
    {
        result = container.lens;
    }
    else if (key == KEY_APERTURE)
    {
        result = container.aperture;
    }
    else if (key == KEY_FOCALLENGTH)
    {
        result = container.focalLength;
    }
    else if (key == KEY_FOCALLENGTH35)
    {
        result = container.focalLength35;
    }
    else if (key == KEY_EXPOSURETIME)
    {
        result = container.exposureTime;
    }
    else if (key == KEY_EXPOSUREPROGRAM)
    {
        result = container.exposureProgram;
    }
    else if (key == KEY_EXPOSUREMODE)
    {
        result = container.exposureMode;
    }
    else if (key == KEY_SENSITIVITY)
    {
        result = container.sensitivity;
    }
    else if (key == KEY_FLASHMODE)
    {
        result = container.flashMode;
    }
    else if (key == KEY_WHITEBALANCE)
    {
        result = container.whiteBalance;
    }
    else if (key == KEY_WHITEBALANCECOLORTEMPERATURE)
    {
        result = container.whiteBalanceColorTemperature;
    }
    else if (key == KEY_METERINGMODE)
    {
        result = container.meteringMode;
    }
    else if (key == KEY_SUBJECTDISTANCE)
    {
        result = container.subjectDistance;
    }
    else if (key == KEY_SUBJECTDISTANCECATEGORY)
    {
        result = container.subjectDistanceCategory;
    }

    return result;
}

} // namespace Digikam
