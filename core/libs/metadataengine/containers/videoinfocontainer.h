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

#ifndef DIGIKAM_VIDEO_INFO_CONTAINER_H
#define DIGIKAM_VIDEO_INFO_CONTAINER_H

// Qt includes

#include <QString>
#include <QDateTime>
#include <QDebug>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

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
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const VideoInfoContainer& t);

} // namespace Digikam

#endif // DIGIKAM_VIDEO_INFO_CONTAINER_H
