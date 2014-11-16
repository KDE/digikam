/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-11-15
 * Description : Information for thumbnails
 *
 * Copyright (C) 2006-2014 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAMTHUMBNAILINFO_H
#define DIGIKAMTHUMBNAILINFO_H

// Qt includes

#include <QDateTime>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ThumbnailIdentifier
{
public:

    ThumbnailIdentifier();
    explicit ThumbnailIdentifier(const QString& filePath);

    /** The file path from which the thumbnail shall be generated */
    QString   filePath;

    /** The database id, which needs to be translated to uniqueHash + fileSize */
    qlonglong id;
};

class DIGIKAM_EXPORT ThumbnailInfo : public ThumbnailIdentifier
{
public:

    ThumbnailInfo();
    ~ThumbnailInfo() {};

    /** If available, the uniqueHash + fileSize pair for identification
     *  of the original file by content.
     */
    QString   uniqueHash;
    qlonglong fileSize;

    /** If the original file is at all accessible on disk.
     *  May be false if a file on a removable device is used.
     */
    bool      isAccessible;

    /** The modification date of the original file.
     *  Thumbnail will be regenerated if thumb's modification date is older than this.
     */
    QDateTime modificationDate;

    /** Gives a hint at the orientation of the image.
     *  This can be used to supersede the Exif information in the file.
     *  Will not be used if DMetadata::ORIENTATION_UNSPECIFIED (default value)
     */
    int       orientationHint;

    /** The file name (the name, not the directory)
     */
    QString   fileName;

    /** A custom identifier, if neither filePath nor uniqueHash are applicable. */
    QString   customIdentifier;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbnailInfoProvider
{
public:

    ThumbnailInfoProvider() {};
    virtual ~ThumbnailInfoProvider() {};
    virtual ThumbnailInfo thumbnailInfo(const ThumbnailIdentifier&)=0;
};

}  // namespace Digikam

#endif // DIGIKAMTHUMBNAILINFO_H
