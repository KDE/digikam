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

static const QString KEY_ASPECTRATIO("AspectRatio");
static const QString KEY_AUDIOBITRATE("AudioBitRate");
static const QString KEY_AUDIOCHANNELTYPE("AudioChannelType");
static const QString KEY_AUDIOCOMPRESSOR("AudioCompressor");
static const QString KEY_DURATION("Duration");
static const QString KEY_FRAMERATE("FrameRate");
static const QString KEY_VIDEOCODEC("VideoCodec");
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
    addId(KEY_AUDIOCOMPRESSOR,              i18n("Audio Compressor (Audio Codec)"));
    addId(KEY_DURATION,                     i18n("Duration of File"));
    addId(KEY_FRAMERATE,                    i18n("Frame Rate of Video"));
    addId(KEY_VIDEOCODEC,                   i18n("Video Codec"));
}

QString MetadataKeys::getDbValue(const QString& key, ParseSettings& settings)
{
    ImageInfo info = ImageInfo::fromUrl(settings.fileUrl);
    ImageMetadataContainer container = info.imageMetadataContainer();
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
    else if (key == KEY_AUDIOCOMPRESSOR)
    {
        result = videoContainer.audioCompressor;
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

    return result;
}

} // namespace Digikam
