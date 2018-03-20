/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
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
#include <QList>
#include <QSize>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

// Local includes

#include "coredburl.h"
#include "dshareddata.h"
#include "coredbalbuminfo.h"
#include "imageinfocache.h"

namespace Digikam
{

class ImageInfoStatic
{
public:

    static void create();
    static void destroy();

    static ImageInfoCache* cache();

public:

    ImageInfoCache          m_cache;
    QReadWriteLock          m_lock;

    static ImageInfoStatic* m_instance;
};

// -----------------------------------------------------------------------------------

class ImageInfoReadLocker : public QReadLocker
{
public:

    ImageInfoReadLocker()
        : QReadLocker(&ImageInfoStatic::m_instance->m_lock)
    {
    }
};

// -----------------------------------------------------------------------------------

class ImageInfoWriteLocker : public QWriteLocker
{
public:

    ImageInfoWriteLocker()
        : QWriteLocker(&ImageInfoStatic::m_instance->m_lock)
    {
    }
};

// -----------------------------------------------------------------------------------

class ImageInfoData : public DSharedData
{
public:

    ImageInfoData();
    ~ImageInfoData();

public:

    qlonglong              id;
    qlonglong              currentReferenceImage;
    int                    albumId;
    int                    albumRootId;
    QString                name;

    QString                defaultComment;
    QString                defaultTitle;
    quint8                 pickLabel;
    quint8                 colorLabel;
    qint8                  rating;
    DatabaseItem::Category category;
    QString                format;
    QDateTime              creationDate;
    QDateTime              modificationDate;
    qlonglong              fileSize;
    QString                uniqueHash;
    QSize                  imageSize;
    QList<int>             tagIds;

    double                 longitude;
    double                 latitude;
    double                 altitude;
    double                 currentSimilarity;

    //! number of grouped images, if this is group leader
    int                    groupedImages;
    //! group leader, if the image is grouped
    qlonglong              groupImage;

    bool                   hasCoordinates         : 1;
    bool                   hasAltitude            : 1;

    bool                   defaultTitleCached     : 1;
    bool                   defaultCommentCached   : 1;
    bool                   pickLabelCached        : 1;
    bool                   colorLabelCached       : 1;
    bool                   ratingCached           : 1;
    bool                   categoryCached         : 1;
    bool                   formatCached           : 1;
    bool                   creationDateCached     : 1;
    bool                   modificationDateCached : 1;
    bool                   fileSizeCached         : 1;
    bool                   uniqueHashCached       : 1;
    bool                   imageSizeCached        : 1;
    bool                   tagIdsCached           : 1;
    bool                   positionsCached        : 1;
    bool                   groupedImagesCached    : 1;
    bool                   groupImageCached       : 1;

    bool                   invalid                : 1;

    // These two are initially true because we assume the data is there.
    // Once we query the data and find out it is missing, we set them to false.
    bool                   hasVideoMetadata       : 1;
    bool                   hasImageMetadata       : 1;

    DatabaseFields::VideoMetadataMinSizeType videoMetadataCached;
    DatabaseFields::ImageMetadataMinSizeType imageMetadataCached;

    typedef DatabaseFields::Hash<QVariant> DatabaseFieldsHashRaw;
    DatabaseFieldsHashRaw databaseFieldsHashRaw;
};

}  // namespace Digikam

#endif // IMAGEINFODATA_H
