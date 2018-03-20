/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : main photograph information container
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "infocontainer.h"

// Qt includes

#include <QDataStream>

namespace Digikam
{

PhotoInfoContainer::PhotoInfoContainer()
{
    hasCoordinates = false;
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
    bool b14 = hasCoordinates  == t.hasCoordinates;

    return (b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13 && b14);
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
        !dateTime.isValid()       &&
        !hasCoordinates)
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
    ds << info.hasCoordinates;

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
    ds >> info.hasCoordinates;

    return ds;
}

QDebug operator<<(QDebug dbg, const PhotoInfoContainer& t)
{
    dbg.nospace() << "PhotoInfoContainer::make: "
                  << t.make << ", ";
    dbg.nospace() << "PhotoInfoContainer::model: "
                  << t.model << ", ";
    dbg.nospace() << "PhotoInfoContainer::lens: "
                  << t.lens << ", ";
    dbg.nospace() << "PhotoInfoContainer::exposureTime: "
                  << t.exposureTime << ", ";
    dbg.nospace() << "PhotoInfoContainer::exposureMode: "
                  << t.exposureMode << ", ";
    dbg.nospace() << "PhotoInfoContainer::exposureProgram: "
                  << t.exposureProgram << ", ";
    dbg.nospace() << "PhotoInfoContainer::aperture: "
                  << t.aperture << ", ";
    dbg.nospace() << "PhotoInfoContainer::focalLength: "
                  << t.focalLength << ", ";
    dbg.nospace() << "PhotoInfoContainer::focalLength35mm: "
                  << t.focalLength35mm << ", ";
    dbg.nospace() << "PhotoInfoContainer::sensitivity: "
                  << t.sensitivity;
    dbg.nospace() << "PhotoInfoContainer::flash: "
                  << t.flash;
    dbg.nospace() << "PhotoInfoContainer::whiteBalance: "
                  << t.whiteBalance;
    dbg.nospace() << "PhotoInfoContainer::dateTime: "
                  << t.dateTime;
    dbg.nospace() << "PhotoInfoContainer::hasCoordinates: "
                  << t.hasCoordinates;
    return dbg.space();
}

// --------------------------------------------------------------------------------------------

VideoInfoContainer::VideoInfoContainer()
{
}

VideoInfoContainer::~VideoInfoContainer()
{
}

bool VideoInfoContainer::operator==(const VideoInfoContainer& t) const
{
    bool b1  = aspectRatio        == t.aspectRatio;
    bool b2  = duration           == t.duration;
    bool b3  = frameRate          == t.frameRate;
    bool b4  = videoCodec         == t.videoCodec;
    bool b5  = audioBitRate       == t.audioBitRate;
    bool b6  = audioChannelType   == t.audioChannelType;
    bool b7  = audioCodec    == t.audioCodec;

    return b1 && b2 && b3 && b4 && b5 && b6 && b7;
}

bool VideoInfoContainer::isEmpty() const
{
    if (aspectRatio.isEmpty()            &&
        duration.isEmpty()               &&
        frameRate.isEmpty()              &&
        videoCodec.isEmpty()             &&
        audioBitRate.isEmpty()           &&
        audioChannelType.isEmpty()       &&
        audioCodec.isEmpty())
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
           duration.isEmpty()               &&
           frameRate.isEmpty()              &&
           videoCodec.isEmpty()             &&
           audioBitRate.isEmpty()           &&
           audioChannelType.isEmpty()       &&
           audioCodec.isEmpty());
}

QDataStream& operator<<(QDataStream& ds, const VideoInfoContainer& info)
{
    ds << info.aspectRatio;
    ds << info.duration;
    ds << info.frameRate;
    ds << info.videoCodec;
    ds << info.audioBitRate;
    ds << info.audioChannelType;
    ds << info.audioCodec;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, VideoInfoContainer& info)
{
    ds >> info.aspectRatio;
    ds >> info.duration;
    ds >> info.frameRate;
    ds >> info.videoCodec;
    ds >> info.audioBitRate;
    ds >> info.audioChannelType;
    ds >> info.audioCodec;

    return ds;
}

QDebug operator<<(QDebug dbg, const VideoInfoContainer& t)
{
    dbg.nospace() << "PhotoInfoContainer::aspectRatio: "
                  << t.aspectRatio << ", ";
    dbg.nospace() << "PhotoInfoContainer::duration: "
                  << t.duration << ", ";
    dbg.nospace() << "PhotoInfoContainer::frameRate: "
                  << t.frameRate << ", ";
    dbg.nospace() << "PhotoInfoContainer::videoCodec: "
                  << t.videoCodec << ", ";
    dbg.nospace() << "PhotoInfoContainer::audioBitRate: "
                  << t.audioBitRate << ", ";
    dbg.nospace() << "PhotoInfoContainer::audioChannelType: "
                  << t.audioChannelType << ", ";
    dbg.nospace() << "PhotoInfoContainer::audioCodec: "
                  << t.audioCodec << ", ";
    return dbg.space();
}

} // namespace Digikam
