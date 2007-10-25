/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 * 
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "databaseurl.h"
#include "dshareddata.h"

namespace Digikam
{

class ImageInfoData : public DSharedData
{
public:

    ImageInfoData();

    QAtomic       ref;

    qlonglong     id;
    int           albumId;
    int           albumRootId;
    QString       name;

    QString       defaultComment;
    int           rating;
    QDateTime     creationDate;
    QDateTime     modificationDate;
    uint          fileSize;
    QSize         imageSize;
    QList<int>    tagIds;

    bool          defaultCommentCached    : 1;
    bool          ratingCached            : 1;
    bool          creationDateCached      : 1;
    bool          modificationDateCached  : 1;
    bool          fileSizeCached          : 1;
    bool          imageSizeCached         : 1;
    bool          tagIdsCached            : 1;

};

}  // namespace Digikam

#endif // IMAGEINFODATA_H

