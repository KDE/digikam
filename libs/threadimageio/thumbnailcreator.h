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

#ifndef DIGIKAMTHUMBNAILCREATOR_H
#define DIGIKAMTHUMBNAILCREATOR_H

// Qt includes

#include <QString>
#include <QPixmap>
#include <QImage>

// KDE includes

#include <kfileitem.h>

// Local includes

#include "drawdecoding.h"
#include "digikam_export.h"

namespace Digikam
{

class DImgLoaderObserver;
class ThumbnailCreatorPriv;

class DIGIKAM_EXPORT ThumbnailCreator
{
public:

    /**
     * Create a thumbnail creator object.
     * You must call setThumbnailSize before load.
     */
    ThumbnailCreator();
    /**
     * Create a thumbnail creator object, and set the thumbnail size.
     */
    ThumbnailCreator(int thumbnailSize);
    ~ThumbnailCreator();

    /**
     * Create a thumbnail for the specified file.
     */
    QImage load(const QString& filePath);

    /**
     * Sets the thumbnail size. This is the maximum size of the QImage
     * returned by load.
     */
    void setThumbnailSize(int thumbnailSize);

    /**
     * If you plan to load thumbnail from the context of the threadimageio framework,
     * you can specify the relevant parameters. They will be passed if a thumbnail
     * is created by loading with DImg.
     * Note that DImg is not used in most cases (Raw files, JPEG)
     */
    void setLoadingProperties(DImgLoaderObserver* observer, const DRawDecoding& settings);

    /**
     * Set the Exif rotation property.
     * If exifRotate is true, the thumbnail will be rotated according
     * to the Exif information.
     * Default value is true.
     */
    void setExifRotate(bool rotate);

    /**
     * If you enable this property, the thumbnail creator will create only large (256x256)
     * thumbnails on disk.
     * Normally, for requested sizes below 128, thumbnails of 128x128 will be cached on disk.
     * Default value is false.
     */
    void setOnlyLargeThumbnails(bool onlyLarge);

    /**
     * If you enable this property, the returned QImage objects will not have an alpha channel.
     * Images with transparency will be blended over an opaque background.
     */
    void setRemoveAlphaChannel(bool removeAlpha);

    /**
     * Return the thumbnail size, the maximum size of the QImage
     * returned by load.
     */
    int thumbnailSize() const;
    /**
     * Return the cached image size, the size of the image that is stored on disk
     * according to the FreeDesktop specification.
     * This size is larger than thumbnailSize.
     * Possible values: 128 or 256.
     */
    int cachedSize() const;

    /**
     * Returns the last error that occurred.
     * It is valid if load returned a null QImage object.
     */
    QString errorString() const;

    /**
     * Deletes all available thumbnails from the on-disk thumbnail cache.
     * A subsequent call to load() will recreate the thumbnail.
     */
    static void deleteThumbnailsFromDisk(const QString& filePath);

private:

    QImage loadWithDImg(const QString& path);
    QImage loadImagePreview(const QString& path);
    void handleAlphaChannel(QImage& thumb);
    void exifRotate(const QString& filePath, QImage& thumb, bool fromEmbeddedPreview);

    // implementations in thumbnailbasic.cpp
    static QString normalThumbnailDir();
    static QString largeThumbnailDir();
    static QString thumbnailPath(const QString& filePath, const QString& basePath);
    static QString thumbnailUri(const QString& filePath);
    static QString thumbnailPathFromUri(const QString &uri, const QString& basePath);

    void initThumbnailDirs();
    QString thumbnailPath(const QString& uri);
    QImage loadPNG(const QString& path);

private:

    ThumbnailCreatorPriv* const d;
};

}  // namespace Digikam

#endif // DIGIKAMTHUMBNAILCREATOR_H
