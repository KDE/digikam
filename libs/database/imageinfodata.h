/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEINFODATA_H
#define IMAGEINFODATA_H

// Qt includes

#include <QDateTime>
#include <QSize>
#include <QList>
#include <qatomic.h>

// Local includes

#include "databaseurl.h"
#include "dshareddata.h"
#include "albuminfo.h"

namespace Digikam
{

class ImageInfoData : public DSharedData
{
public:

    ImageInfoData();

public:

#if QT_VERSION < 0x040400
    QAtomic                ref;
#else
    QAtomicInt             ref;
#endif

    qlonglong              id;
    int                    albumId;
    int                    albumRootId;
    QString                name;

    QString                defaultComment;
    quint8                 pickLabel;
    quint8                 colorLabel;
    quint8                 groupingFlags;
    qint8                  rating;
    DatabaseItem::Category category;
    QString                format;
    QDateTime              creationDate;
    QDateTime              modificationDate;
    uint                   fileSize;
    QSize                  imageSize;
    QList<int>             tagIds;

    double                 longitude;
    double                 latitude;
    double                 altitude;

    // number of grouped images, if this is group leader
    int                    groupedImages;
    // group leader, if the is grouped
    qlonglong              groupImage;

    bool                   hasCoordinates         : 1;
    bool                   hasAltitude            : 1;

    bool                   defaultCommentCached   : 1;
    bool                   pickLabelCached        : 1;
    bool                   colorLabelCached       : 1;
    bool                   ratingCached           : 1;
    bool                   categoryCached         : 1;
    bool                   formatCached           : 1;
    bool                   creationDateCached     : 1;
    bool                   modificationDateCached : 1;
    bool                   fileSizeCached         : 1;
    bool                   imageSizeCached        : 1;
    bool                   tagIdsCached           : 1;
    bool                   positionsCached        : 1;
    bool                   groupedImagesIsCached  : 1;
    bool                   groupImageIsCached     : 1;

    bool                   invalid                : 1;
};

}  // namespace Digikam

#endif // IMAGEINFODATA_H
