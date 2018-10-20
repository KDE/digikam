/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : video information container
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

#include "videoinfocontainer.h"

// Qt includes

#include <QDataStream>

namespace Digikam
{

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
    bool b7  = audioCodec         == t.audioCodec;

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
