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

#include <klocalizedstring.h>

// Local includes

#include "coredbinfocontainers.h"
#include "imageinfo.h"

namespace
{
static const QString KEY_MAKE(QLatin1String("CameraMake"));
static const QString KEY_MODEL(QLatin1String("CameraModel"));
static const QString KEY_LENS(QLatin1String("CameraLens"));
static const QString KEY_APERTURE(QLatin1String("Aperture"));
static const QString KEY_FOCALLENGTH(QLatin1String("FocalLength"));
static const QString KEY_FOCALLENGTH35(QLatin1String("FocalLength35"));
static const QString KEY_EXPOSURETIME(QLatin1String("ExposureTime"));
static const QString KEY_EXPOSUREPROGRAM(QLatin1String("ExposureProgram"));
static const QString KEY_EXPOSUREMODE(QLatin1String("ExposureMode"));
static const QString KEY_SENSITIVITY(QLatin1String("Sensitivity"));
static const QString KEY_FLASHMODE(QLatin1String("FlashMode"));
static const QString KEY_WHITEBALANCE(QLatin1String("WhiteBalance"));
static const QString KEY_WHITEBALANCECOLORTEMPERATURE(QLatin1String("WhiteBalanceColorTemp"));
static const QString KEY_METERINGMODE(QLatin1String("MeteringMode"));
static const QString KEY_SUBJECTDISTANCE(QLatin1String("SubjectDistance"));
static const QString KEY_SUBJECTDISTANCECATEGORY(QLatin1String("SubjectDistanceCategory"));

static const QString KEY_ASPECTRATIO(QLatin1String("AspectRatio"));
static const QString KEY_AUDIOBITRATE(QLatin1String("AudioBitRate"));
static const QString KEY_AUDIOCHANNELTYPE(QLatin1String("AudioChannelType"));
static const QString KEY_AUDIOCodec(QLatin1String("AudioCodec"));
static const QString KEY_DURATION(QLatin1String("Duration"));
static const QString KEY_FRAMERATE(QLatin1String("FrameRate"));
static const QString KEY_VIDEOCODEC(QLatin1String("VideoCodec"));
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

    addId(KEY_ASPECTRATIO,                  i18n("Display Aspect Ratio"));
    addId(KEY_AUDIOBITRATE,                 i18n("Audio Bit Rate"));
    addId(KEY_AUDIOCHANNELTYPE,             i18n("Audio Channel Type"));
    addId(KEY_AUDIOCodec,              i18n("Audio Codec (Audio Codec)"));
    addId(KEY_DURATION,                     i18n("Duration of File"));
    addId(KEY_FRAMERATE,                    i18n("Frame Rate of Video"));
    addId(KEY_VIDEOCODEC,                   i18n("Video Codec"));
}

QString MetadataKeys::getDbValue(const QString& key, ParseSettings& settings)
{
    ImageInfo info                        = ImageInfo::fromUrl(settings.fileUrl);
    ImageMetadataContainer container      = info.imageMetadataContainer();
    VideoMetadataContainer videoContainer = info.videoMetadataContainer();
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
    else if (key == KEY_ASPECTRATIO)
    {
        result = videoContainer.aspectRatio;
    }
    else if (key == KEY_AUDIOBITRATE)
    {
        result = videoContainer.audioBitRate;
    }
    else if (key == KEY_AUDIOCHANNELTYPE)
    {
        result = videoContainer.audioChannelType;
    }
    else if (key == KEY_AUDIOCodec)
    {
        result = videoContainer.audioCodec;
    }
    else if (key == KEY_DURATION)
    {
        result = videoContainer.duration;
    }
    else if (key == KEY_FRAMERATE)
    {
        result = videoContainer.frameRate;
    }
    else if (key == KEY_VIDEOCODEC)
    {
        result = videoContainer.videoCodec;
    }

    result.replace(QLatin1Char('/'), QLatin1Char('|'));

    return result;
}

} // namespace Digikam
