/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAMTHUMBNAILCREATORPRIV_H
#define DIGIKAMTHUMBNAILCREATORPRIV_H

// Local includes

#include "dmetadata.h"

namespace Digikam
{

class ThumbnailImage
{
public:
    ThumbnailImage()
    {
        exifOrientation = DMetadata::ORIENTATION_NORMAL;
    }

    bool isNull() const { return qimage.isNull(); }

    QImage qimage;
    int    exifOrientation;
};

class ThumbnailCreatorPriv
{
public:

    ThumbnailCreatorPriv()
    {
        thumbnailSize       = 0;
        cachedSize          = 0;
        observer            = 0;

        thumbnailStorage    = ThumbnailCreator::FreeDesktopStandard;
        infoProvider        = 0;

        exifRotate          = true;
        removeAlphaChannel  = true;
        onlyLargeThumbnails = false;

        digiKamFingerPrint  = QString("Digikam Thumbnail Generator");
    }

    bool                exifRotate;
    bool                removeAlphaChannel;
    bool                onlyLargeThumbnails;

    ThumbnailCreator::StorageMethod thumbnailStorage;
    ThumbnailInfoProvider          *infoProvider;

    int                 thumbnailSize;
    int                 cachedSize;

    QString             error;
    QString             bigThumbPath;
    QString             smallThumbPath;
    QString             digiKamFingerPrint;

    DImgLoaderObserver* observer;
    DRawDecoding        rawSettings;
};

}  // namespace Digikam

#endif // DIGIKAMTHUMBNAILCREATORPRIV_H
