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

#ifndef DIGIKAMTHUMBNAILCREATOR_H
#define DIGIKAMTHUMBNAILCREATOR_H

// Qt includes

#include <QString>
#include <QPixmap>
#include <QImage>

// Local includes

#include "drawdecoding.h"
#include "digikam_export.h"
#include "thumbnailinfo.h"

namespace Digikam
{

class IccProfile;
class DImgLoaderObserver;
class DMetadata;
class ThumbnailImage;
class ThumbsDbInfo;

class DIGIKAM_EXPORT ThumbnailCreator
{
public:

    enum StorageMethod
    {
        FreeDesktopStandard,
        ThumbnailDatabase
    };

public:

    /**
     * Create a thumbnail creator object.
     * You must call setThumbnailSize before load.
     */
    explicit ThumbnailCreator(StorageMethod method);

    /**
     * Create a thumbnail creator object, and set the thumbnail size.
     */
    ThumbnailCreator(int thumbnailSize, StorageMethod method);
    ~ThumbnailCreator();

    /**
     * Create a thumbnail for the specified file.
     */
    QImage load(const ThumbnailIdentifier& identifier) const;

    /**
     * Creates a thumbnail for the specified detail of the file.
     * A suitable custom identifier (for cache key etc.) is inserted as image.text("customIdentifier").
     */
    QImage loadDetail(const ThumbnailIdentifier& identifier, const QRect& detailRect) const;

    /**
     * Ensures that the thumbnail is pregenerated in the database, but does not load it from there.
     */
    void pregenerate(const ThumbnailIdentifier& identifier) const;
    void pregenerateDetail(const ThumbnailIdentifier& identifier, const QRect& detailRect) const;

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
    void setLoadingProperties(DImgLoaderObserver* const observer, const DRawDecoding& settings);

    /**
     * Set the Exif rotation property.
     * If exifRotate is true, the thumbnail will be rotated according
     * to the Exif information.
     * Default value is true.
     */
    void setExifRotate(bool rotate);

    /**
     * If you enable this property, the thumbnail creator will create only large
     * thumbnails on disk (256x256 as described in FreeDesktop paper).
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
     * Set a ThumbnailInfoProvider to provide custom ThumbnailInfos
     */
    void setThumbnailInfoProvider(ThumbnailInfoProvider* const provider);

    /**
     * Return the thumbnail size, the maximum size of the QImage
     * returned by load.
     */
    int thumbnailSize() const;

    /**
     * Return the stored image size, the size of the image that is stored on disk
     * (according to Storage Method).
     * This size is possibly larger than thumbnailSize.
     * Possible values: 128 or 256.
     */
    int storedSize() const;

    /**
     * Store the given image as thumbnail of the given path.
     * Image should at least have storedSize().
     */
    void store(const QString& path, const QImage& image) const;

    void storeDetailThumbnail(const QString& path, const QRect& detailRect, const QImage& image) const;

    /**
     * Returns the last error that occurred.
     * It is valid if load returned a null QImage object.
     */
    QString errorString() const;

    /**
     * Deletes all available thumbnails from the on-disk thumbnail cache.
     * A subsequent call to load() will recreate the thumbnail.
     */
    void deleteThumbnailsFromDisk(const QString& filePath) const;

    /** Creates a default ThumbnailInfo for the given path using QFileInfo only
     */
    static ThumbnailInfo fileThumbnailInfo(const QString& path);

    /**
     * Returns the customIdentifier for the detail thumbnail
     */
    static QString identifierForDetail(const ThumbnailInfo& identifier, const QRect& rect);

private:

    void initialize();

    ThumbnailImage createThumbnail(const ThumbnailInfo& info, const QRect& detailRect = QRect()) const;

    QImage load(const ThumbnailIdentifier& id, const QRect& rect, bool pregenerate) const;
    QImage loadWithDImg(const QString& path, IccProfile* const profile) const;
    QImage loadImageDetail(const ThumbnailInfo& info, const DMetadata& metadata, const QRect& detailRect, IccProfile* const profile) const;
    QImage loadImagePreview(const DMetadata& metadata) const;
    QImage loadPNG(const QString& path) const;

    QImage handleAlphaChannel(const QImage& thumb) const;
    int    exifOrientation(const ThumbnailInfo& info, const DMetadata& metadata, bool fromEmbeddedPreview, bool fromDetail) const;
    QImage exifRotate(const QImage& thumb, int orientation) const;

    void store(const QString& path, const QImage& i, const QRect& rect) const;

    ThumbnailInfo makeThumbnailInfo(const ThumbnailIdentifier& identifier, const QRect& rect) const;
    QImage scaleForStorage(const QImage& qimage) const;

    void storeInDatabase(const ThumbnailInfo& info, const ThumbnailImage& image) const;
    ThumbsDbInfo loadThumbsDbInfo(const ThumbnailInfo& info) const;
    ThumbnailImage loadFromDatabase(const ThumbnailInfo& info) const;
    bool isInDatabase(const ThumbnailInfo& info) const;
    void deleteFromDatabase(const ThumbnailInfo& info) const;

    void storeFreedesktop(const ThumbnailInfo& info, const ThumbnailImage& image) const;
    ThumbnailImage loadFreedesktop(const ThumbnailInfo& info) const;
    void deleteFromDiskFreedesktop(const QString& filePath) const;

    void initThumbnailDirs();
    QString thumbnailPath(const QString& uri) const;

    // implementations in thumbnailbasic.cpp
    static QString normalThumbnailDir();
    static QString largeThumbnailDir();
    static QString thumbnailPath(const QString& filePath, const QString& basePath);
    static QString thumbnailUri(const QString& filePath);
    static QString thumbnailPathFromUri(const QString& uri, const QString& basePath);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DIGIKAMTHUMBNAILCREATOR_H
