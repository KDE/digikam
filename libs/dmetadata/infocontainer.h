/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : main photograph information container
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PHOTOINFOCONTAINER_H
#define PHOTOINFOCONTAINER_H

// Qt includes

#include <QtCore/QString>
#include <QtCore/QDateTime>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PhotoInfoContainer
{

public:

    PhotoInfoContainer();
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

    VideoInfoContainer();
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
    QString audioCompressor;
};

DIGIKAM_EXPORT QDataStream& operator<<(QDataStream& ds, const VideoInfoContainer& info);
DIGIKAM_EXPORT QDataStream& operator>>(QDataStream& ds, VideoInfoContainer& info);

} // namespace Digikam

#endif /* PHOTOINFOCONTAINER_H */
