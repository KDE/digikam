/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
        exifOrientation = DMetadata::ORIENTATION_UNSPECIFIED;
    }

    bool isNull() const
    {
        return qimage.isNull();
    }

public:

    QImage qimage;
    int    exifOrientation;
};

// -------------------------------------------------------------------

class ThumbnailCreator::Private
{
public:

    Private()
    {
        thumbnailSize                             = 0;
        observer                                  = 0;

        thumbnailStorage                          = ThumbnailCreator::FreeDesktopStandard;
        infoProvider                              = 0;
        dbIdForReplacement                        = -1;

        exifRotate                                = true;
        removeAlphaChannel                        = true;
        onlyLargeThumbnails                       = false;

        // Used internaly as PNG metadata. Do not use i18n.
        digiKamFingerPrint                        = QLatin1String("Digikam Thumbnail Generator");

        fastRawSettings.optimizeTimeLoading();
        fastRawSettings.rawPrm.halfSizeColorImage = true;
        fastRawSettings.rawPrm.sixteenBitsImage   = false;

    }

    bool                            exifRotate;
    bool                            removeAlphaChannel;
    bool                            onlyLargeThumbnails;

    ThumbnailCreator::StorageMethod thumbnailStorage;
    ThumbnailInfoProvider*          infoProvider;
    int                             dbIdForReplacement;

    int                             thumbnailSize;

    QString                         error;
    QString                         bigThumbPath;
    QString                         smallThumbPath;
    QString                         digiKamFingerPrint;

    DImgLoaderObserver*             observer;
    DRawDecoding                    rawSettings;
    DRawDecoding                    fastRawSettings;

public:

    int                             storageSize() const;
};

}  // namespace Digikam

#endif // DIGIKAMTHUMBNAILCREATORPRIV_H
