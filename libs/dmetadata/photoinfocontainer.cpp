/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : main photograph information container
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "photoinfocontainer.h"

// Qt includes

#include <QtCore/QDataStream>

namespace Digikam
{

PhotoInfoContainer::PhotoInfoContainer()
{
}

PhotoInfoContainer::~PhotoInfoContainer()
{
}

bool PhotoInfoContainer::operator==(const PhotoInfoContainer& t) const
{
    bool b1  = make            == t.make;
    bool b2  = model           == t.model;
    bool b3  = lens            == t.lens;
    bool b4  = exposureTime    == t.exposureTime;
    bool b5  = exposureMode    == t.exposureMode;
    bool b6  = exposureProgram == t.exposureProgram;
    bool b7  = aperture        == t.aperture;
    bool b8  = focalLength     == t.focalLength;
    bool b9  = focalLength35mm == t.focalLength35mm;
    bool b10 = sensitivity     == t.sensitivity;
    bool b11 = flash           == t.flash;
    bool b12 = whiteBalance    == t.whiteBalance;
    bool b13 = dateTime        == t.dateTime;

    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13;
}

bool PhotoInfoContainer::isEmpty() const
{
    if (make.isEmpty()            &&
        model.isEmpty()           &&
        lens.isEmpty()            &&
        exposureTime.isEmpty()    &&
        exposureMode.isEmpty()    &&
        exposureProgram.isEmpty() &&
        aperture.isEmpty()        &&
        focalLength.isEmpty()     &&
        focalLength35mm.isEmpty() &&
        sensitivity.isEmpty()     &&
        flash.isEmpty()           &&
        whiteBalance.isEmpty()    &&
        !dateTime.isValid() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PhotoInfoContainer::isNull() const
{
    return(make.isNull()            &&
           model.isNull()           &&
           lens.isNull()            &&
           exposureTime.isNull()    &&
           exposureMode.isNull()    &&
           exposureProgram.isNull() &&
           aperture.isNull()        &&
           focalLength.isNull()     &&
           focalLength35mm.isNull() &&
           sensitivity.isNull()     &&
           flash.isNull()           &&
           whiteBalance.isNull()    &&
           dateTime.isNull());
}

QDataStream& operator<<(QDataStream& ds, const PhotoInfoContainer& info)
{
    ds << info.make;
    ds << info.model;
    ds << info.lens;
    ds << info.exposureTime;
    ds << info.exposureMode;
    ds << info.exposureProgram;
    ds << info.aperture;
    ds << info.focalLength;
    ds << info.focalLength35mm;
    ds << info.sensitivity;
    ds << info.flash;
    ds << info.whiteBalance;
    ds << info.dateTime;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, PhotoInfoContainer& info)
{
    ds >> info.make;
    ds >> info.model;
    ds >> info.lens;
    ds >> info.exposureTime;
    ds >> info.exposureMode;
    ds >> info.exposureProgram;
    ds >> info.aperture;
    ds >> info.focalLength;
    ds >> info.focalLength35mm;
    ds >> info.sensitivity;
    ds >> info.flash;
    ds >> info.whiteBalance;
    ds >> info.dateTime;

    return ds;
}

VideoInfoContainer::VideoInfoContainer()
{
}

VideoInfoContainer::~VideoInfoContainer()
{
}

bool VideoInfoContainer::operator==(const VideoInfoContainer& t) const
{
    bool b1  = aspectRatio        == t.aspectRatio;
    bool b2  = audioBitRate       == t.audioBitRate;
    bool b3  = audioChannelType   == t.audioChannelType;
    bool b4  = audioCompressor    == t.audioCompressor;
    bool b5  = duration           == t.duration;
    bool b6  = frameRate          == t.frameRate;
    bool b7  = resolution         == t.resolution;
    bool b8  = videoCodec         == t.videoCodec;

    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8;
}

bool VideoInfoContainer::isEmpty() const
{
    if (aspectRatio.isEmpty()            &&
        audioBitRate.isEmpty()           &&
        audioChannelType.isEmpty()       &&
        audioCompressor.isEmpty()        &&
        duration.isEmpty()               &&
        frameRate.isEmpty()              &&
        resolution.isEmpty()             &&
        videoCodec.isEmpty() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VideoInfoContainer::isNull() const
{
    return(aspectRatio.isEmpty()            &&
           audioBitRate.isEmpty()           &&
           audioChannelType.isEmpty()       &&
           audioCompressor.isEmpty()        &&
           duration.isEmpty()               &&
           frameRate.isEmpty()              &&
           resolution.isEmpty()             &&
           videoCodec.isEmpty() );
}

QDataStream& operator<<(QDataStream& ds, const VideoInfoContainer& info)
{
    ds << info.aspectRatio;
    ds << info.audioBitRate;
    ds << info.audioChannelType;
    ds << info.audioCompressor;
    ds << info.duration;
    ds << info.frameRate;
    ds << info.resolution;
    ds << info.videoCodec;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, VideoInfoContainer& info)
{
    ds >> info.aspectRatio;
    ds >> info.audioBitRate;
    ds >> info.audioChannelType;
    ds >> info.audioCompressor;
    ds >> info.duration;
    ds >> info.frameRate;
    ds >> info.resolution;
    ds >> info.videoCodec;

    return ds;
}

} // namespace Digikam
