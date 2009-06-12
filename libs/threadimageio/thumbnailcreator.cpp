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

#include "thumbnailcreator.h"
#include "thumbnailcreator_p.h"

// Qt includes

#include <QFileInfo>
#include <QFile>
#include <QPainter>
#include <QBuffer>
#include <QIODevice>

// KDE includes

#include <kcodecs.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kio/global.h>
#include <kio/thumbcreator.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>
#include <kdeversion.h>
#include <kde_file.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/rawfiles.h>

// Local includes

#include "databasebackend.h"
#include "dimg.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "pgfutils.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"

namespace Digikam
{

ThumbnailInfo::ThumbnailInfo()
{
    isAccessible    = false;
    fileSize        = 0;
    orientationHint = DMetadata::ORIENTATION_UNSPECIFIED;
}

ThumbnailCreator::ThumbnailCreator(StorageMethod method)
                : d(new ThumbnailCreatorPriv)
{
    d->thumbnailStorage = method;
    initialize();
}

ThumbnailCreator::ThumbnailCreator(int thumbnailSize, StorageMethod method)
                : d(new ThumbnailCreatorPriv)
{
    setThumbnailSize(thumbnailSize);
    d->thumbnailStorage = method;
    initialize();
}

ThumbnailCreator::~ThumbnailCreator()
{
    delete d;
}

void ThumbnailCreator::initialize()
{
    if (d->thumbnailStorage == FreeDesktopStandard)
        initThumbnailDirs();
}

void ThumbnailCreator::setThumbnailSize(int thumbnailSize)
{
    d->thumbnailSize = thumbnailSize;
    // on-disk thumbnail sizes according to spec
    if (d->onlyLargeThumbnails)
        d->cachedSize = 256;
    else
        d->cachedSize = (thumbnailSize <= 128) ? 128 : 256;
}

void ThumbnailCreator::setExifRotate(bool rotate)
{
    d->exifRotate = rotate;
}

void ThumbnailCreator::setOnlyLargeThumbnails(bool onlyLarge)
{
    d->onlyLargeThumbnails = onlyLarge;
}

void ThumbnailCreator::setRemoveAlphaChannel(bool removeAlpha)
{
    d->removeAlphaChannel = removeAlpha;
}

void ThumbnailCreator::setLoadingProperties(DImgLoaderObserver *observer, const DRawDecoding& settings)
{
    d->observer    = observer;
    d->rawSettings = settings;
}

void ThumbnailCreator::setThumbnailInfoProvider(ThumbnailInfoProvider *provider)
{
    d->infoProvider = provider;
}

int ThumbnailCreator::thumbnailSize() const
{
    return d->thumbnailSize;
}

int ThumbnailCreator::cachedSize() const
{
    return d->cachedSize;
}

QString ThumbnailCreator::errorString() const
{
    return d->error;
}

QImage ThumbnailCreator::load(const QString& path)
{
    if (d->cachedSize <= 0)
    {
        d->error = i18n("No or invalid size specified");
        kWarning(50003) << "No or invalid size specified" << endl;
        return QImage();
    }

    if (d->thumbnailStorage == ThumbnailDatabase)
        d->dbIdForReplacement = -1; // just to prevent bugs

    // get info about path
    ThumbnailInfo info;
    if (d->infoProvider)
        info = d->infoProvider->thumbnailInfo(path);
    else
        info = fileThumbnailInfo(path);

    // load pregenerated thumbnail
    ThumbnailImage image;
    switch (d->thumbnailStorage)
    {
        case ThumbnailDatabase:
            image = loadFromDatabase(info);
            break;
        case FreeDesktopStandard:
            image = loadFreedesktop(info);
            break;
    }

    // if pregenerated thumbnail is not available, generate
    if (image.isNull())
    {
        image = createThumbnail(info);
        if (!image.isNull())
        {
            switch (d->thumbnailStorage)
            {
                case ThumbnailDatabase:
                    storeInDatabase(info, image);
                    break;
                case FreeDesktopStandard:
                    // image is stored rotated
                    if (d->exifRotate)
                        image.qimage = exifRotate(image.qimage, image.exifOrientation);
                    storeFreedesktop(info, image);
                    break;
            }
        }
    }

    if (image.isNull())
    {
        d->error = i18n("Thumbnail is null");
        kWarning(50003) << "Thumbnail is null for " << path << endl;
        return image.qimage;
    }

    // Prepare for usage in digikam
    image.qimage = image.qimage.scaled(d->thumbnailSize, d->thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    handleAlphaChannel(image.qimage);

    if (d->thumbnailStorage == ThumbnailDatabase)
    {
        // image is stored, or created, unrotated, and is now rotated for display
        if (d->exifRotate)
            image.qimage = exifRotate(image.qimage, image.exifOrientation);
    }

    return image.qimage;
}

void ThumbnailCreator::deleteThumbnailsFromDisk(const QString& filePath)
{
    switch (d->thumbnailStorage)
    {
        case FreeDesktopStandard:
            deleteFromDiskFreedesktop(filePath);
            break;
        case ThumbnailDatabase:
        {
            ThumbnailInfo info;
            if (d->infoProvider)
                info = d->infoProvider->thumbnailInfo(filePath);
            else
                info = fileThumbnailInfo(filePath);
            deleteFromDatabase(info);
            break;
        }
    }
}


// --------------- Thumbnail generation and image handling -----------------------


ThumbnailImage ThumbnailCreator::createThumbnail(const ThumbnailInfo &info)
{
    QString path = info.filePath;
    if (!info.isAccessible)
    {
        d->error = i18n("File does not exist");
        return ThumbnailImage();
    }

    QImage qimage;
    bool fromEmbeddedPreview = false;
    bool failedAtDImg        = false;
    bool failedAtJPEGScaled  = false;
    bool failedAtPGFScaled   = false;

    // -- Get the image preview --------------------------------

    // Try to extract Exif/IPTC preview first.
    qimage = loadImagePreview(path);

    QFileInfo fileInfo(path);
    // To speed-up thumb extraction, we now try to load the images by the file extension.
    QString ext = fileInfo.suffix().toUpper();

    if (qimage.isNull() && !ext.isEmpty())
    {
        if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
        {
            // use jpegutils
            loadJPEGScaled(qimage, path, d->cachedSize);
            failedAtJPEGScaled = qimage.isNull();
        }
        else if (ext == QString("PNG")  ||
            ext == QString("TIFF") ||
            ext == QString("TIF"))
        {
            qimage       = loadWithDImg(path);
            failedAtDImg = qimage.isNull();
        }
        else if (ext == QString("PGF"))
        {
            // use pgf library to extract reduced version
            loadPGFScaled(qimage, path, d->cachedSize);
            failedAtPGFScaled = qimage.isNull();
        }
    }

    // Trying to load with dcraw: RAW files.
    if (qimage.isNull())
    {
        if (KDcrawIface::KDcraw::loadEmbeddedPreview(qimage, path))
            fromEmbeddedPreview = true;
    }

    if (qimage.isNull())
    {
        //TODO: Use DImg based loader instead?
        KDcrawIface::KDcraw::loadHalfPreview(qimage, path);
    }

    // Try JPEG anyway
    if (qimage.isNull() && !failedAtJPEGScaled)
    {
        // use jpegutils
        loadJPEGScaled(qimage, path, d->cachedSize);
    }

    // DImg-dependent loading methods: TIFF, PNG, everything supported by QImage
    if (qimage.isNull() && !failedAtDImg)
    {
        qimage = loadWithDImg(path);
    }

    // Try PGF anyway
    if (qimage.isNull() && !failedAtPGFScaled)
    {
        // use jpegutils
        loadPGFScaled(qimage, path, d->cachedSize);
    }

    if (qimage.isNull())
    {
        d->error = i18n("Cannot create thumbnail for %1", path);
        kWarning(50003) << "Cannot create thumbnail for " << path << endl;
        return ThumbnailImage();
    }

    int maxSize = qMax(qimage.width(), qimage.height());
    if (maxSize != d->cachedSize)
    {
        // Perform cheat scaling (http://labs.trolltech.com/blogs/2009/01/26/creating-thumbnail-preview)
        int cheatSize = maxSize - (3*(maxSize - d->cachedSize) / 4);
        qimage        = qimage.scaled(cheatSize, cheatSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        qimage        = qimage.scaled(d->cachedSize, d->cachedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    ThumbnailImage image;
    image.qimage = qimage;
    image.exifOrientation = exifOrientation(path, fromEmbeddedPreview);
    return image;
}

QImage ThumbnailCreator::loadWithDImg(const QString& path)
{
    DImg img;
    if (d->observer)
        img.load(path, d->observer, d->rawSettings);
    else
        img.load(path);
    return img.copyQImage();
}

QImage ThumbnailCreator::loadImagePreview(const QString& path)
{
    QImage image;
    DMetadata metadata(path);
    if (metadata.getImagePreview(image))
    {
        kDebug(50003) << "Use Exif/IPTC preview extraction. Size of image: "
                      << image.width() << "x" << image.height() << endl;
    }

    return image;
}

QImage ThumbnailCreator::handleAlphaChannel(const QImage& qimage)
{
    switch (qimage.format())
    {
        case QImage::Format_RGB32:
            break;
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        {
            if (d->removeAlphaChannel)
            {
                QImage newImage(qimage.size(), QImage::Format_RGB32);
                // use raster paint engine
                QPainter p(&newImage);
                // blend over white, or a checkerboard?
                p.fillRect(newImage.rect(), Qt::white);
                p.drawImage(0, 0, qimage);
                return newImage;
            }
            break;
        }
        default: // indexed and monochrome formats
        {
            return qimage.convertToFormat(QImage::Format_RGB32);
        }
    }
    return qimage;
}

int ThumbnailCreator::exifOrientation(const QString& filePath, bool fromEmbeddedPreview)
{
    // Keep in sync with main version in loadsavethread.cpp

    if (DImg::fileFormat(filePath) == DImg::RAW && !fromEmbeddedPreview )
        return DMetadata::ORIENTATION_NORMAL;

    DMetadata metadata(filePath);
    return metadata.getImageOrientation();
}

QImage ThumbnailCreator::exifRotate(const QImage& thumb, int orientation)
{
    if (orientation == DMetadata::ORIENTATION_NORMAL ||
        orientation == DMetadata::ORIENTATION_UNSPECIFIED)
        return thumb;

    QMatrix matrix;

    switch (orientation)
    {
        case DMetadata::ORIENTATION_NORMAL:
        case DMetadata::ORIENTATION_UNSPECIFIED:
            break;

        case DMetadata::ORIENTATION_HFLIP:
            matrix.scale(-1, 1);
            break;

        case DMetadata::ORIENTATION_ROT_180:
            matrix.rotate(180);
            break;

        case DMetadata::ORIENTATION_VFLIP:
            matrix.scale(1, -1);
            break;

        case DMetadata::ORIENTATION_ROT_90_HFLIP:
            matrix.scale(-1, 1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90:
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90_VFLIP:
            matrix.scale(1, -1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_270:
            matrix.rotate(270);
            break;
    }

    // transform accordingly
    return thumb.transformed(matrix);
}


// --------------- PGF Database thumbnail storage -----------------------


void ThumbnailCreator::storeInDatabase(const ThumbnailInfo& info, const ThumbnailImage& image)
{
    DatabaseThumbnailInfo dbInfo;

    // We rely on loadFromDatabase() being called before, so we do not need to look up
    // by filepath of uniqueHash to find out if a thumb need to be replaced.
    dbInfo.id               = d->dbIdForReplacement;
    d->dbIdForReplacement   = -1;
    dbInfo.type             = DatabaseThumbnail::PGF;
    dbInfo.modificationDate = info.modificationDate;
    dbInfo.orientationHint  = image.exifOrientation;

    if (dbInfo.type == DatabaseThumbnail::PGF)
    {
        if (!writePGFImageData(image.qimage, dbInfo.data, 4))
        {
            kWarning(50003) << "Cannot save PGF thumb in DB" << endl;
            return;
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::WriteOnly);
        image.qimage.save(&buffer, "JPEG");
        if (dbInfo.data.isNull())
        {
            kWarning(50003) << "Cannot save JPEG thumb in DB" << endl;
            return;
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG2000)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::WriteOnly);
        image.qimage.save(&buffer, "JP2");
        if (dbInfo.data.isNull())
        {
            kWarning(50003) << "Cannot save JPEG2000 thumb in DB" << endl;
            return;
        }
    }

    ThumbnailDatabaseAccess access;

    access.backend()->beginTransaction();
    // Insert thumbnail data
    if (dbInfo.id == -1)
        dbInfo.id = access.db()->insertThumbnail(dbInfo);
    else
        access.db()->replaceThumbnail(dbInfo);

    // Insert lookup data used to locate thumbnail data
    if (!info.uniqueHash.isNull())
        access.db()->insertUniqueHash(info.uniqueHash, info.fileSize, dbInfo.id);
    if (!info.filePath.isNull())
        access.db()->insertFilePath(info.filePath, dbInfo.id);

    access.backend()->commitTransaction();
}

ThumbnailImage ThumbnailCreator::loadFromDatabase(const ThumbnailInfo& info)
{
    ThumbnailDatabaseAccess access;
    DatabaseThumbnailInfo   dbInfo;

    if (!info.uniqueHash.isNull())
    {
        dbInfo = access.db()->findByHash(info.uniqueHash, info.fileSize);
    }
    if (dbInfo.data.isNull() && !info.filePath.isNull())
    {
        dbInfo = access.db()->findByFilePath(info.filePath);
    }

    // store for use in storeInDatabase()
    d->dbIdForReplacement = dbInfo.id;

    ThumbnailImage image;
    if (dbInfo.data.isNull())
        return ThumbnailImage();

    // check modification date
    if (dbInfo.modificationDate < info.modificationDate)
        return ThumbnailImage();

    // Read QImage from data blob
    if (dbInfo.type == DatabaseThumbnail::PGF)
    {
        if (!readPGFImageData(dbInfo.data, image.qimage))
        {
            kWarning(50003) << "Cannot load PGF thumb from DB" << endl;
            return ThumbnailImage();
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::ReadOnly);
        image.qimage.load(&buffer, "JPEG");
        if (dbInfo.data.isNull())
        {
            kWarning(50003) << "Cannot load JPEG thumb from DB" << endl;
            return ThumbnailImage();
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG2000)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::ReadOnly);
        image.qimage.load(&buffer, "JP2");
        if (dbInfo.data.isNull())
        {
            kWarning(50003) << "Cannot load JPEG2000 thumb from DB" << endl;
            return ThumbnailImage();
        }
    }

    image.exifOrientation = dbInfo.orientationHint;

    return image;
}

void ThumbnailCreator::deleteFromDatabase(const ThumbnailInfo& info)
{
    ThumbnailDatabaseAccess access;

    access.backend()->beginTransaction();
    if (!info.uniqueHash.isNull())
        access.db()->removeByUniqueHash(info.uniqueHash, info.fileSize);
    if (!info.filePath.isNull())
        access.db()->removeByFilePath(info.filePath);
    access.backend()->commitTransaction();
}

// --------------- Freedesktop.org standard implementation -----------------------


ThumbnailInfo ThumbnailCreator::fileThumbnailInfo(const QString &path)
{
    ThumbnailInfo info;
    info.filePath = path;
    QFileInfo fileInfo(path);
    info.isAccessible = fileInfo.exists();
    if (!info.isAccessible)
        return info;
    info.modificationDate = fileInfo.lastModified();
    return info;
}

ThumbnailImage ThumbnailCreator::loadFreedesktop(const ThumbnailInfo &info)
{
    QString uri       = thumbnailUri(info.filePath);
    QString thumbPath = thumbnailPath(info.filePath);

    QImage qimage = loadPNG(thumbPath);

    // NOTE: if thumbnail have not been generated by digiKam (konqueror for example),
    // force to recompute it, else we use it.
    if (!qimage.isNull())
    {
        if (qimage.text("Thumb::MTime") == QString::number(info.modificationDate.toTime_t()) &&
            qimage.text("Software")     == d->digiKamFingerPrint)
        {
            ThumbnailImage info;
            info.qimage = qimage;
            // is stored rotated. Not needed to rotate.
            info.exifOrientation = DMetadata::ORIENTATION_NORMAL;
            return info;
        }
    }
    return ThumbnailImage();
}

void ThumbnailCreator::storeFreedesktop(const ThumbnailInfo &info, const ThumbnailImage &image)
{
    QString path = info.filePath;
    QImage qimage = image.qimage;

    QString uri       = thumbnailUri(path);
    QString thumbPath = thumbnailPath(path);

    // required by spec
    if (qimage.format() != QImage::Format_ARGB32)
        qimage = qimage.convertToFormat(QImage::Format_ARGB32);

    qimage.setText(QString("Thumb::URI").toLatin1(),   0, uri);
    qimage.setText(QString("Thumb::MTime").toLatin1(), 0, QString::number(info.modificationDate.toTime_t()));
    qimage.setText(QString("Software").toLatin1(),     0, d->digiKamFingerPrint);

    KTemporaryFile temp;
    temp.setPrefix(thumbPath + "-digikam-");
    temp.setSuffix(".png");
    temp.setAutoRemove(false);
    if (temp.open())
    {
        QString tempFileName   = temp.fileName();
        if (qimage.save(tempFileName, "PNG", 0))
        {
            int ret = 0;
            Q_ASSERT(!tempFileName.isEmpty());

            #if KDE_IS_VERSION(4,2,85)
            // KDE 4.3.0
            ret = KDE::rename(QFile::encodeName(tempFileName),
                              QFile::encodeName(thumbPath));
            #else
            // KDE 4.2.x or 4.1.x
            ret = KDE_rename(QFile::encodeName(tempFileName),
                             QFile::encodeName(thumbPath));
            #endif

            if (ret != 0)
            {
                kDebug(50003) << "Cannot rename thumb file (" << tempFileName << ")" << endl;
                kDebug(50003) << "to (" << thumbPath << ")..." << endl;
            }
        }
    }
}

void ThumbnailCreator::deleteFromDiskFreedesktop(const QString filePath)
{
    QFile smallThumb(thumbnailPath(filePath, normalThumbnailDir()));
    QFile largeThumb(thumbnailPath(filePath, largeThumbnailDir()));

    smallThumb.remove();
    largeThumb.remove();
}

}  // namespace Digikam
