/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/rawfiles.h>

// libkexiv2 includes

#include <libkexiv2/kexiv2previews.h>
#include <libkexiv2/rotationmatrix.h>
#include <libkexiv2/version.h>

// Local includes

#include "databasebackend.h"
#include "dimg.h"
#include "dmetadata.h"
#include "iccmanager.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "loadsavethread.h"
#include "jpegutils.h"
#include "pgfutils.h"
#include "tagregion.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "thumbnailsize.h"

using namespace KDcrawIface;

namespace Digikam
{

ThumbnailIdentifier::ThumbnailIdentifier()
    : id(0)
{
}

ThumbnailIdentifier::ThumbnailIdentifier(const QString& filePath)
    : filePath(filePath),
      id(0)
{
}

ThumbnailInfo::ThumbnailInfo()
    : fileSize(0),
      isAccessible(false),
      orientationHint(DMetadata::ORIENTATION_UNSPECIFIED)
{
}

ThumbnailCreator::ThumbnailCreator(StorageMethod method)
    : d(new Private)
{
    d->thumbnailStorage = method;
    initialize();
}

ThumbnailCreator::ThumbnailCreator(int thumbnailSize, StorageMethod method)
    : d(new Private)
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
    {
        initThumbnailDirs();
    }
}

int ThumbnailCreator::Private::storageSize() const
{
    // on-disk thumbnail sizes according to freedesktop spec
    // for thumbnail db it's always max size
    if (onlyLargeThumbnails)
    {
        return ThumbnailSize::maxThumbsSize();
    }
    else
    {
        return (thumbnailSize <= ThumbnailSize::Medium) ? ThumbnailSize::Medium : ThumbnailSize::Huge;
    }
}

void ThumbnailCreator::setThumbnailSize(int thumbnailSize)
{
    d->thumbnailSize = thumbnailSize;
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

void ThumbnailCreator::setLoadingProperties(DImgLoaderObserver* const observer, const DRawDecoding& settings)
{
    d->observer    = observer;
    d->rawSettings = settings;
}

void ThumbnailCreator::setThumbnailInfoProvider(ThumbnailInfoProvider* const provider)
{
    d->infoProvider = provider;
}

int ThumbnailCreator::thumbnailSize() const
{
    return d->thumbnailSize;
}

int ThumbnailCreator::storedSize() const
{
    return d->storageSize();
}

QString ThumbnailCreator::errorString() const
{
    return d->error;
}

QImage ThumbnailCreator::load(const ThumbnailIdentifier& identifier) const
{
    return load(identifier, QRect(), false);
}

QImage ThumbnailCreator::loadDetail(const ThumbnailIdentifier& identifier, const QRect& rect) const
{
    if (!rect.isValid())
    {
        kWarning() << "Invalid rectangle" << rect;
        return QImage();
    }

    return load(identifier, rect, false);
}

void ThumbnailCreator::pregenerate(const ThumbnailIdentifier& identifier) const
{
    load(identifier, QRect(), true);
}

void ThumbnailCreator::pregenerateDetail(const ThumbnailIdentifier& identifier, const QRect& rect) const
{
    if (!rect.isValid())
    {
        kWarning() << "Invalid rectangle" << rect;
        return;
    }

    load(identifier, rect, true);
}

QImage ThumbnailCreator::load(const ThumbnailIdentifier& identifier, const QRect& rect, bool pregenerate) const
{
    if (d->storageSize() <= 0)
    {
        d->error = i18n("No or invalid size specified");
        kWarning() << "No or invalid size specified";
        return QImage();
    }

    if (d->thumbnailStorage == ThumbnailDatabase)
    {
        d->dbIdForReplacement = -1;    // just to prevent bugs
    }
    // get info about path
    ThumbnailInfo info = makeThumbnailInfo(identifier, rect);

    // load pregenerated thumbnail
    ThumbnailImage image;

    switch (d->thumbnailStorage)
    {
        case ThumbnailDatabase:

            if (pregenerate)
            {
                if (isInDatabase(info))
                {
                    return QImage();
                }

                // otherwise, fall through and generate
            }
            else
            {
                image = loadFromDatabase(info);
            }

            break;
        case FreeDesktopStandard:
            image = loadFreedesktop(info);
            break;
    }

    // For images in offline collections we can stop here, they are not available on disk
    if (image.isNull() && info.filePath.isEmpty())
    {
        return QImage();
    }

    // if pregenerated thumbnail is not available, generate
    if (image.isNull())
    {
        image = createThumbnail(info, rect);

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
                    {
                        image.qimage = exifRotate(image.qimage, image.exifOrientation);
                    }

                    storeFreedesktop(info, image);
                    break;
            }
        }
    }

    if (image.isNull())
    {
        d->error = i18n("Thumbnail is null");
        kWarning() << "Thumbnail is null for " << identifier.filePath;
        return image.qimage;
    }

    // If we only pregenerate, we have now created and stored in the database
    if (pregenerate)
    {
        return QImage();
    }

    // Prepare for usage in digikam
    image.qimage = image.qimage.scaled(d->thumbnailSize, d->thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image.qimage = handleAlphaChannel(image.qimage);

    if (d->thumbnailStorage == ThumbnailDatabase)
    {
        // image is stored, or created, unrotated, and is now rotated for display
        // detail thumbnails are stored readily rotated
        if (d->exifRotate && rect.isNull())
        {
            image.qimage = exifRotate(image.qimage, image.exifOrientation);
        }
    }

    if (!info.customIdentifier.isNull())
    {
        image.qimage.setText("customIdentifier", info.customIdentifier);
    }

    return image.qimage;
}

QImage ThumbnailCreator::scaleForStorage(const QImage& qimage) const
{
    if (qimage.width() > d->storageSize() || qimage.height() > d->storageSize())
    {
        /*
        Cheat scaling is disabled because of quality problems - see bug #224999
        // Perform cheat scaling (http://labs.trolltech.com/blogs/2009/01/26/creating-thumbnail-preview)
        int cheatSize = maxSize - (3*(maxSize - d->storageSize()) / 4);
        qimage        = qimage.scaled(cheatSize, cheatSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        */
        QImage scaledThumb = qimage.scaled(d->storageSize(), d->storageSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        return scaledThumb;
    }

    return qimage;
}

QString ThumbnailCreator::identifierForDetail(const ThumbnailInfo& info, const QRect& rect)
{
    QUrl url;
    url.setScheme("detail");
    url.setPath(info.filePath);
    /* A scheme to support loading by database id, but this is a hack. Solve cleanly later (schema update)
    url.setPath(identifier.fileName);
    if (!identifier.uniqueHash.isNull())
    {
        url.addQueryItem("hash", identifier.uniqueHash);
        url.addQueryItem("filesize", QString::number(identifier.fileSize));
    }
    else
    {
        url.addQueryItem("path", identifier.filePath);
    }
    */
    QString r = QString("%1,%2-%3x%4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
    url.addQueryItem("rect", r);
    return url.toString();
}

ThumbnailInfo ThumbnailCreator::makeThumbnailInfo(const ThumbnailIdentifier& identifier, const QRect& rect) const
{
    ThumbnailInfo info;

    if (d->infoProvider)
    {
        info = d->infoProvider->thumbnailInfo(identifier);
    }
    else
    {
        info = fileThumbnailInfo(identifier.filePath);
    }

    if (!rect.isNull())
    {
        // Important: Pass the filled info, not the possibly half-filled identifier here because the hash is preferred for the customIdentifier!
        info.customIdentifier = identifierForDetail(info, rect);
    }

    return info;
}

void ThumbnailCreator::store(const QString& path, const QImage& i) const
{
    store(path, i, QRect());
}

void ThumbnailCreator::storeDetailThumbnail(const QString& path, const QRect& detailRect, const QImage& i) const
{
    store(path, i, detailRect);
}

void ThumbnailCreator::store(const QString& path, const QImage& i, const QRect& rect) const
{
    if (i.isNull())
    {
        return;
    }

    QImage         qimage = scaleForStorage(i);
    ThumbnailInfo  info   = makeThumbnailInfo(ThumbnailIdentifier(path), rect);
    ThumbnailImage image;
    image.qimage          = qimage;

    switch (d->thumbnailStorage)
    {
        case ThumbnailDatabase:

            // we must call isInDatabase or loadFromDatabase before storeInDatabase for d->dbIdForReplacement!
            if (!isInDatabase(info))
            {
                storeInDatabase(info, image);
            }

            break;
        case FreeDesktopStandard:
            storeFreedesktop(info, image);
            break;
    }
}

void ThumbnailCreator::deleteThumbnailsFromDisk(const QString& filePath) const
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
            {
                info = d->infoProvider->thumbnailInfo(ThumbnailIdentifier(filePath));
            }
            else
            {
                info = fileThumbnailInfo(filePath);
            }

            deleteFromDatabase(info);
            break;
        }
    }
}

// --------------- Thumbnail generation and image handling -----------------------

ThumbnailImage ThumbnailCreator::createThumbnail(const ThumbnailInfo& info, const QRect& detailRect) const
{
    const QString path = info.filePath;

    if (!info.isAccessible)
    {
        d->error = i18n("File does not exist");
        return ThumbnailImage();
    }

    QImage qimage;
    DMetadata metadata(path);
    bool fromEmbeddedPreview = false;
    bool fromDetail          = false;
    bool failedAtDImg        = false;
    bool failedAtJPEGScaled  = false;
    bool failedAtPGFScaled   = false;

    // -- Get the image preview --------------------------------

    IccProfile profile;
    bool colorManage = IccSettings::instance()->useManagedPreviews();

    if (!detailRect.isNull())
    {
        // when taking a detail, we have to load the image full size
        qimage     = loadImageDetail(info, metadata, detailRect, &profile);
        fromDetail = !qimage.isNull();
    }
    else
    {
        if (qimage.isNull())
        {
            // Try to extract Exif/IPTC preview first.
            qimage = loadImagePreview(metadata);
        }

        QFileInfo fileInfo(path);
        // To speed-up thumb extraction, we now try to load the images by the file extension.
        QString ext = fileInfo.suffix().toUpper();

        if (qimage.isNull() && !ext.isEmpty())
        {
            if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
            {
                if (colorManage)
                {
                    qimage = loadWithDImg(path, &profile);
                }
                else
                    // use jpegutils
                {
                    JPEGUtils::loadJPEGScaled(qimage, path, d->storageSize());
                }

                failedAtJPEGScaled = qimage.isNull();
            }
            else if (ext == QString("PNG")  ||
                     ext == QString("TIFF") ||
                     ext == QString("TIF"))
            {
                qimage       = loadWithDImg(path, &profile);
                failedAtDImg = qimage.isNull();
            }
            else if (ext == QString("PGF"))
            {
                // use pgf library to extract reduced version
                PGFUtils::loadPGFScaled(qimage, path, d->storageSize());
                failedAtPGFScaled = qimage.isNull();
            }
        }

        // Trying to load with libraw: RAW files.
        if (qimage.isNull())
        {
            kDebug() << "Trying to load Embedded preview with libraw";

            if (KDcraw::loadEmbeddedPreview(qimage, path))
            {
                fromEmbeddedPreview = true;
                profile             = metadata.getIccProfile();
            }
        }

        if (qimage.isNull())
        {
            kDebug() << "Trying to load half preview with libraw";

            //TODO: Use DImg based loader instead?
            KDcraw::loadHalfPreview(qimage, path);
        }

        // See bug #339144 : only handle preview if right libkexiv2 version is used.
#if KEXIV2_VERSION >= 0x020302

        // Special case with DNG file. See bug #338081
        if (qimage.isNull())
        {
            kDebug() << "Trying to load Embedded preview with Exiv2";

            KExiv2Iface::KExiv2Previews preview(path);
            qimage = preview.image();
        }

#endif

        // DImg-dependent loading methods: TIFF, PNG, everything supported by QImage
        if (qimage.isNull() && !failedAtDImg)
        {
            qimage = loadWithDImg(path, &profile);
        }

        // Try JPEG anyway
        if (qimage.isNull() && !failedAtJPEGScaled)
        {
            // use jpegutils
            JPEGUtils::loadJPEGScaled(qimage, path, d->storageSize());
        }

        // Try PGF anyway
        if (qimage.isNull() && !failedAtPGFScaled)
        {
            // use jpegutils
            PGFUtils::loadPGFScaled(qimage, path, d->storageSize());
        }
    }

    if (qimage.isNull())
    {
        d->error = i18n("Cannot create thumbnail for %1", path);
        kWarning() << "Cannot create thumbnail for " << path;
        return ThumbnailImage();
    }

    qimage = scaleForStorage(qimage);

    if (colorManage && !profile.isNull())
    {
        IccManager::transformToSRGB(qimage, profile);
    }

    ThumbnailImage image;
    image.qimage          = qimage;
    image.exifOrientation = exifOrientation(info, metadata, fromEmbeddedPreview, fromDetail);
    return image;
}

QImage ThumbnailCreator::loadWithDImg(const QString& path, IccProfile* const profile) const
{
    DImg img;
    img.setAttribute("scaledLoadingSize", d->storageSize());
    img.load(path, false, profile ? true : false, false, false, d->observer, d->rawSettings);
    *profile = img.getIccProfile();
    return img.copyQImage();
}

QImage ThumbnailCreator::loadImageDetail(const ThumbnailInfo& info, const DMetadata& metadata,
                                         const QRect& detailRect, IccProfile* const profile) const
{
    const QString& path = info.filePath;
    // Check the first and largest preview (Raw files)
    KExiv2Iface::KExiv2Previews previews(path);

    if (!previews.isEmpty())
    {
        // discard if smaller than half preview
        int acceptableWidth  = lround(previews.originalSize().width()  * 0.5);
        int acceptableHeight = lround(previews.originalSize().height() * 0.5);

        if (previews.width() >= acceptableWidth &&  previews.height() >= acceptableHeight)
        {
            QImage qimage           = previews.image();
            QRect reducedSizeDetail = TagRegion::mapFromOriginalSize(previews.originalSize(), qimage.size(), detailRect);
            return qimage.copy(reducedSizeDetail.intersected(qimage.rect()));
        }
    }

    // load DImg
    DImg img;
    //TODO: scaledLoading if detailRect is large
    //TODO: use code from PreviewTask, including cache storage
    img.load(path, false, profile ? true : false, false, false, d->observer, d->fastRawSettings);
    *profile = img.getIccProfile();

    // We must rotate before clipping because the rect refers to the oriented image.
    // I do not know currently how to back-rotate the rect for clipping before rotation.
    // If someone has the mathematics, have a go.
    img.rotateAndFlip(exifOrientation(info, metadata, false, false));

    QRect mappedDetail = TagRegion::mapFromOriginalSize(img, detailRect);
    img.crop(mappedDetail.intersected(QRect(0, 0, img.width(), img.height())));
    return img.copyQImage();
}

QImage ThumbnailCreator::loadImagePreview(const DMetadata& metadata) const
{
    QImage image;

    if (metadata.getImagePreview(image))
    {
        kDebug() << "Use Exif/IPTC preview extraction. Size of image: "
                 << image.width() << "x" << image.height();
    }

    return image;
}

QImage ThumbnailCreator::handleAlphaChannel(const QImage& qimage) const
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
                QImage chbImage(20, 20, QImage::Format_RGB32);

                // create checkerboard brush
                QPainter chb(&chbImage);
                chb.fillRect( 0,  0, 20, 20, Qt::white);
                chb.fillRect( 0, 10 ,10, 10, Qt::lightGray);
                chb.fillRect(10,  0, 10, 10, Qt::lightGray);
                QBrush chbBrush(chbImage);

                // use raster paint engine
                QPainter p(&newImage);
                // blend over white, or a checkerboard?
                p.fillRect(newImage.rect(), chbBrush);
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

int ThumbnailCreator::exifOrientation(const ThumbnailInfo& info, const DMetadata& metadata,
                                      bool fromEmbeddedPreview, bool fromDetail) const
{
    if (fromDetail)
    {
        return DMetadata::ORIENTATION_NORMAL;
    }

    return LoadSaveThread::exifOrientation(info.filePath, metadata,
                                           DImg::fileFormat(info.filePath) == DImg::RAW,
                                           fromEmbeddedPreview);
}

QImage ThumbnailCreator::exifRotate(const QImage& thumb, int orientation) const
{
    if (orientation == DMetadata::ORIENTATION_NORMAL ||
        orientation == DMetadata::ORIENTATION_UNSPECIFIED)
    {
        return thumb;
    }

    QMatrix matrix = KExiv2Iface::RotationMatrix::toMatrix((KExiv2::ImageOrientation)orientation);
    // transform accordingly
    return thumb.transformed(matrix);
}


// --------------- PGF Database thumbnail storage -----------------------


void ThumbnailCreator::storeInDatabase(const ThumbnailInfo& info, const ThumbnailImage& image) const
{
    DatabaseThumbnailInfo dbInfo;

    // We rely on loadDatabaseThumbnailInfo() being called before, so we do not need to look up
    // by filepath of uniqueHash to find out if a thumb need to be replaced.
    dbInfo.id               = d->dbIdForReplacement;
    d->dbIdForReplacement   = -1;
    dbInfo.type             = DatabaseThumbnail::PGF;
    dbInfo.modificationDate = info.modificationDate;
    dbInfo.orientationHint  = image.exifOrientation;

    if (dbInfo.type == DatabaseThumbnail::PGF)
    {
        // NOTE: see bug #233094: using PGF compression level 4 there. Do not use a value > 4,
        // else image is blurred due to down-sampling.
        if (!PGFUtils::writePGFImageData(image.qimage, dbInfo.data, 4))
        {
            kWarning() << "Cannot save PGF thumb in DB";
            return;
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::WriteOnly);
        image.qimage.save(&buffer, "JPEG", 90);  // Here we will use JPEG quality = 90 to reduce artifacts.

        if (dbInfo.data.isNull())
        {
            kWarning() << "Cannot save JPEG thumb in DB";
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
            kWarning() << "Cannot save JPEG2000 thumb in DB";
            return;
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::PNG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::WriteOnly);
        image.qimage.save(&buffer, "PNG", 0);

        if (dbInfo.data.isNull())
        {
            kWarning() << "Cannot save PNG thumb in DB";
            return;
        }
    }

    ThumbnailDatabaseAccess access;

    DatabaseCoreBackend::QueryState lastQueryState = DatabaseCoreBackend::ConnectionError;

    while (lastQueryState == DatabaseCoreBackend::ConnectionError)
    {
        lastQueryState = access.backend()->beginTransaction();

        if (DatabaseCoreBackend::NoErrors != lastQueryState)
        {
            continue;
        }

        // Insert thumbnail data
        if (dbInfo.id == -1)
        {
            QVariant id;
            lastQueryState = access.db()->insertThumbnail(dbInfo, &id);

            if (DatabaseCoreBackend::NoErrors != lastQueryState)
            {
                continue;
            }
            else
            {
                dbInfo.id = id.toInt();
            }
        }
        else
        {
            lastQueryState = access.db()->replaceThumbnail(dbInfo);

            if (DatabaseCoreBackend::NoErrors != lastQueryState)
            {
                continue;
            }
        }

        // Insert lookup data used to locate thumbnail data
        if (!info.customIdentifier.isNull())
        {
            lastQueryState = access.db()->insertCustomIdentifier(info.customIdentifier, dbInfo.id);

            if (DatabaseCoreBackend::NoErrors != lastQueryState)
            {
                continue;
            }
        }
        else
        {
            if (!info.uniqueHash.isNull())
            {
                lastQueryState = access.db()->insertUniqueHash(info.uniqueHash, info.fileSize, dbInfo.id);

                if (DatabaseCoreBackend::NoErrors != lastQueryState)
                {
                    continue;
                }
            }

            if (!info.filePath.isNull())
            {
                lastQueryState = access.db()->insertFilePath(info.filePath, dbInfo.id);

                if (DatabaseCoreBackend::NoErrors != lastQueryState)
                {
                    continue;
                }
            }
        }

        lastQueryState = access.backend()->commitTransaction();

        if (DatabaseCoreBackend::NoErrors != lastQueryState)
        {
            continue;
        }

    }
}

DatabaseThumbnailInfo ThumbnailCreator::loadDatabaseThumbnailInfo(const ThumbnailInfo& info) const
{
    ThumbnailDatabaseAccess access;
    DatabaseThumbnailInfo   dbInfo;

    // Custom identifier takes precedence
    if (!info.customIdentifier.isEmpty())
    {
        dbInfo = access.db()->findByCustomIdentifier(info.customIdentifier);
    }
    else
    {
        if (!info.uniqueHash.isEmpty())
        {
            dbInfo = access.db()->findByHash(info.uniqueHash, info.fileSize);
        }

        if (dbInfo.data.isNull() && !info.filePath.isEmpty())
        {
            dbInfo = access.db()->findByFilePath(info.filePath, info.uniqueHash);
        }
    }

    // store for use in storeInDatabase()
    d->dbIdForReplacement = dbInfo.id;

    return dbInfo;
}

bool ThumbnailCreator::isInDatabase(const ThumbnailInfo& info) const
{
    DatabaseThumbnailInfo dbInfo = loadDatabaseThumbnailInfo(info);

    if (dbInfo.data.isNull())
    {
        return false;
    }

    // check modification date
    if (dbInfo.modificationDate < info.modificationDate)
    {
        return false;
    }

    return true;
}

ThumbnailImage ThumbnailCreator::loadFromDatabase(const ThumbnailInfo& info) const
{
    DatabaseThumbnailInfo dbInfo = loadDatabaseThumbnailInfo(info);

    ThumbnailImage image;

    if (dbInfo.data.isNull())
    {
        return ThumbnailImage();
    }

    // check modification date
    if (dbInfo.modificationDate < info.modificationDate)
    {
        return ThumbnailImage();
    }

    // Read QImage from data blob
    if (dbInfo.type == DatabaseThumbnail::PGF)
    {
        if (!PGFUtils::readPGFImageData(dbInfo.data, image.qimage))
        {
            kWarning() << "Cannot load PGF thumb from DB";
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
            kWarning() << "Cannot load JPEG thumb from DB";
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
            kWarning() << "Cannot load JPEG2000 thumb from DB";
            return ThumbnailImage();
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::PNG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::ReadOnly);
        image.qimage.load(&buffer, "PNG");

        if (dbInfo.data.isNull())
        {
            kWarning() << "Cannot load PNG thumb from DB";
            return ThumbnailImage();
        }
    }

    // Give priority to main database's rotation flag
    // NOTE: Breaks rotation of RAWs which do not contain JPEG previews
    image.exifOrientation = info.orientationHint;
    if (image.exifOrientation == DMetadata::ORIENTATION_UNSPECIFIED && !info.filePath.isEmpty() && LoadSaveThread::infoProvider())
    {
        image.exifOrientation = LoadSaveThread::infoProvider()->orientationHint(info.filePath);
    }
    if (image.exifOrientation == DMetadata::ORIENTATION_UNSPECIFIED)
    {
        image.exifOrientation = dbInfo.orientationHint;
    }

    return image;
}

void ThumbnailCreator::deleteFromDatabase(const ThumbnailInfo& info) const
{
    ThumbnailDatabaseAccess access;
    DatabaseCoreBackend::QueryState lastQueryState=DatabaseCoreBackend::ConnectionError;

    while (DatabaseCoreBackend::ConnectionError==lastQueryState)
    {
        lastQueryState = access.backend()->beginTransaction();

        if (DatabaseCoreBackend::NoErrors!=lastQueryState)
        {
            continue;
        }

        if (!info.uniqueHash.isNull())
        {
            lastQueryState=access.db()->removeByUniqueHash(info.uniqueHash, info.fileSize);

            if (DatabaseCoreBackend::NoErrors!=lastQueryState)
            {
                continue;
            }
        }

        if (!info.filePath.isNull())
        {
            lastQueryState=access.db()->removeByFilePath(info.filePath);

            if (DatabaseCoreBackend::NoErrors!=lastQueryState)
            {
                continue;
            }
        }

        lastQueryState = access.backend()->commitTransaction();

        if (DatabaseCoreBackend::NoErrors!=lastQueryState)
        {
            continue;
        }
    }
}

// --------------- Freedesktop.org standard implementation -----------------------


ThumbnailInfo ThumbnailCreator::fileThumbnailInfo(const QString& path)
{
    ThumbnailInfo info;
    info.filePath     = path;
    QFileInfo fileInfo(path);
    info.isAccessible = fileInfo.exists();
    info.fileName     = fileInfo.fileName();

    if (!info.isAccessible)
    {
        return info;
    }

    info.modificationDate = fileInfo.lastModified();
    return info;
}

ThumbnailImage ThumbnailCreator::loadFreedesktop(const ThumbnailInfo& info) const
{
    QString path;

    if (!info.customIdentifier.isNull())
    {
        path = info.customIdentifier;
    }
    else
    {
        path = info.filePath;
    }

    QString uri       = thumbnailUri(path);
    QString thumbPath = thumbnailPath(path);
    QImage qimage     = loadPNG(thumbPath);

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

void ThumbnailCreator::storeFreedesktop(const ThumbnailInfo& info, const ThumbnailImage& image) const
{
    QImage qimage = image.qimage;

    QString path;

    if (!info.customIdentifier.isNull())
    {
        path = info.customIdentifier;
    }
    else
    {
        path = info.filePath;
    }

    QString uri       = thumbnailUri(path);
    QString thumbPath = thumbnailPath(path);

    // required by spec
    if (qimage.format() != QImage::Format_ARGB32)
    {
        qimage = qimage.convertToFormat(QImage::Format_ARGB32);
    }

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

            temp.close();

#ifndef Q_OS_WIN
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
#else
            if(::MoveFileEx(tempFileName.utf16(), thumbPath.utf16(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0)
#endif
            {
                kDebug() << "Cannot rename thumb file (" << tempFileName << ")";
                kDebug() << "to (" << thumbPath << ")...";
            }
        }
    }
}

void ThumbnailCreator::deleteFromDiskFreedesktop(const QString& filePath) const
{
    QFile smallThumb(thumbnailPath(filePath, normalThumbnailDir()));
    QFile largeThumb(thumbnailPath(filePath, largeThumbnailDir()));

    smallThumb.remove();
    largeThumb.remove();
}

}  // namespace Digikam
