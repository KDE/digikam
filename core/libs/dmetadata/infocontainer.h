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

#ifndef INFO_CONTAINER_H
#define INFO_CONTAINER_H

// Qt includes

#include <QString>
#include <QDateTime>
#include <QDebug>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PhotoInfoContainer
{

public:

    explicit PhotoInfoContainer();
    ~PhotoInfoContainer();

    bool isEmpty() const;
    bool isNull() const;

    bool operator==(const PhotoInfoContainer& t) const;

public:

    QString   make;
    QString   model;
    QString   lens;
    QString   exposureTime;
    QString   exposureMode;
    QString   exposureProgram;
    QString   aperture;
    QString   focalLength;
    QString   focalLength35mm;
    QString   sensitivity;
    QString   flash;
    QString   whiteBalance;

    QDateTime dateTime;

    bool      hasCoordinates;  // GPS info are present
};

DIGIKAM_EXPORT QDataStream& operator<<(QDataStream& ds, const PhotoInfoContainer& info);
DIGIKAM_EXPORT QDataStream& operator>>(QDataStream& ds, PhotoInfoContainer& info);

// --------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT VideoInfoContainer
{

public:

    explicit VideoInfoContainer();
    ~VideoInfoContainer();

    bool isEmpty() const;
    bool isNull() const;

    bool operator==(const VideoInfoContainer& t) const;

public:

    QString aspectRatio;
    QString duration;
    QString frameRate;
    QString videoCodec;
    QString audioBitRate;
    QString audioChannelType;
    QString audioCodec;
};

DIGIKAM_EXPORT QDataStream& operator<<(QDataStream& ds, const VideoInfoContainer& info);
DIGIKAM_EXPORT QDataStream& operator>>(QDataStream& ds, VideoInfoContainer& info);

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const PhotoInfoContainer& t);
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const VideoInfoContainer& t);

} // namespace Digikam

#endif // INFO_CONTAINER_H
