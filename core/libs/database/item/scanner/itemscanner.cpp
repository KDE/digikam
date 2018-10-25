/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item.
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemscanner_p.h"

namespace Digikam
{

ItemScanner::ItemScanner(const QFileInfo& info, const ItemScanInfo& scanInfo)
    : d(new Private)
{
    d->fileInfo = info;
    d->scanInfo = scanInfo;
}

ItemScanner::ItemScanner(const QFileInfo& info)
    : d(new Private)
{
    d->fileInfo = info;
}

ItemScanner::ItemScanner(qlonglong imageid)
    : d(new Private)
{
    ItemShortInfo shortInfo;
    {
        CoreDbAccess access;
        shortInfo   = access.db()->getItemShortInfo(imageid);
        d->scanInfo = access.db()->getItemScanInfo(imageid);
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(shortInfo.albumRootID);
    d->fileInfo           = QFileInfo(CoreDbUrl::fromAlbumAndName(shortInfo.itemName,
                                      shortInfo.album, QUrl::fromLocalFile(albumRootPath), shortInfo.albumRootID).fileUrl().toLocalFile());
}

ItemScanner::~ItemScanner()
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Finishing took" << d->time.elapsed() << "ms";
    delete d;
}

qlonglong ItemScanner::id() const
{
    return d->scanInfo.id;
}

void ItemScanner::setCategory(DatabaseItem::Category category)
{
    // we don't have the necessary information in this class, but in CollectionScanner
    d->scanInfo.category = category;
}

void ItemScanner::commit()
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Scanning took" << d->time.restart() << "ms";

    switch (d->commit.operation)
    {
        case ItemScannerCommit::NoOp:
            return;
        case ItemScannerCommit::AddItem:
            commitAddImage();
            break;
        case ItemScannerCommit::UpdateItem:
            commitUpdateImage();
            break;
    }

    if (d->commit.copyImageAttributesId != -1)
    {
        commitCopyImageAttributes();
        return;
    }

    if (d->commit.commitImageInformation)
    {
        commitImageInformation();
    }

    if (d->commit.commitImageMetadata)
    {
        commitImageMetadata();
    }
    else if (d->commit.commitVideoMetadata)
    {
        commitVideoMetadata();
    }

    if (d->commit.commitImagePosition)
    {
        commitImagePosition();
    }

    if (d->commit.commitImageComments)
    {
        commitImageComments();
    }

    if (d->commit.commitImageCopyright)
    {
        commitImageCopyright();
    }

    if (d->commit.commitIPTCCore)
    {
        commitIPTCCore();
    }

    if (!d->commit.tagIds.isEmpty())
    {
        commitTags();
    }

    if (d->commit.commitFaces)
    {
        commitFaces();
    }

    commitImageHistory();
}

void ItemScanner::fileModified()
{
    loadFromDisk();
    prepareUpdateImage();
    scanFile(ModifiedScan);
}

void ItemScanner::newFile(int albumId)
{
    loadFromDisk();
    prepareAddImage(albumId);

    if (!scanFromIdenticalFile())
    {
        scanFile(NewScan);
    }
}

void ItemScanner::newFileFullScan(int albumId)
{
    loadFromDisk();
    prepareAddImage(albumId);
    scanFile(NewScan);
}

void ItemScanner::rescan()
{
    loadFromDisk();
    prepareUpdateImage();
    scanFile(Rescan);
}

void ItemScanner::copiedFrom(int albumId, qlonglong srcId)
{
    loadFromDisk();
    prepareAddImage(albumId);

    // first use source, if it exists
    if (!copyFromSource(srcId))
    {
        // check if we can establish identity
        if (!scanFromIdenticalFile())
        {
            // scan newly
            scanFile(NewScan);
        }
    }
}

const ItemScanInfo& ItemScanner::itemScanInfo() const
{
    return d->scanInfo;
}

bool lessThanForIdentity(const ItemScanInfo& a, const ItemScanInfo& b)
{
    if (a.status != b.status)
    {
        // First: sort by status

        // put UndefinedStatus to back
        if (a.status == DatabaseItem::UndefinedStatus)
        {
            return false;
        }

        // enum values are in the order we want it
        return a.status < b.status;
    }
    else
    {
        // Second: sort by modification date, descending
        return a.modificationDate > b.modificationDate;
    }
}

bool ItemScanner::scanFromIdenticalFile()
{
    // Get a list of other images that are identical. Source image shall not be included.
    // When using the Commit functionality, d->scanInfo.id can be null.
    QList<ItemScanInfo> candidates = CoreDbAccess().db()->getIdenticalFiles(d->scanInfo.uniqueHash,
                                                                            d->scanInfo.fileSize,
                                                                            d->scanInfo.id);

    if (!candidates.isEmpty())
    {
        // Sort by priority, as implemented by custom lessThan()
        std::stable_sort(candidates.begin(), candidates.end(), lessThanForIdentity);

        qCDebug(DIGIKAM_DATABASE_LOG) << "Recognized" << d->fileInfo.filePath()
                                      << "as identical to item" << candidates.first().id;

        // Copy attributes.
        // Todo for the future is to worry about syncing identical files.
        d->commit.copyImageAttributesId = candidates.first().id;

        return true;
    }

    return false;
}

void ItemScanner::commitCopyImageAttributes()
{
    CoreDbAccess().db()->copyImageAttributes(d->commit.copyImageAttributesId, d->scanInfo.id);
    // Also copy the similarity information
    SimilarityDbAccess().db()->copySimilarityAttributes(d->commit.copyImageAttributesId, d->scanInfo.id);
    // Remove grouping for copied or identical images.
    CoreDbAccess().db()->removeAllImageRelationsFrom(d->scanInfo.id, DatabaseRelation::Grouped);
    CoreDbAccess().db()->removeAllImageRelationsTo(d->scanInfo.id, DatabaseRelation::Grouped);
}

bool ItemScanner::copyFromSource(qlonglong srcId)
{
    CoreDbAccess access;

    // some basic validity checking
    if (srcId == d->scanInfo.id)
    {
        return false;
    }

    ItemScanInfo info = access.db()->getItemScanInfo(srcId);

    if (!info.id)
    {
        return false;
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Recognized" << d->fileInfo.filePath() << "as copied from" << srcId;
    d->commit.copyImageAttributesId = srcId;

    return true;
}

void ItemScanner::prepareAddImage(int albumId)
{
    d->scanInfo.albumID          = albumId;
    d->scanInfo.status           = DatabaseItem::Visible;

    qCDebug(DIGIKAM_DATABASE_LOG) << "Adding new item" << d->fileInfo.filePath();
    d->commit.operation = ItemScannerCommit::AddItem;
}

void ItemScanner::commitAddImage()
{
    // get the image id of a deleted image info if existent and mark it as valid.
    // otherwise, create a new item.
    qlonglong imageId = CoreDbAccess().db()->getImageId(-1, d->scanInfo.itemName, DatabaseItem::Status::Trashed,
                                                         d->scanInfo.category, d->scanInfo.modificationDate,
                                                         d->scanInfo.fileSize, d->scanInfo.uniqueHash);
    if (imageId != -1)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Detected identical image info with id" << imageId
                                      << "and album id NULL of a removed image for image" << d->scanInfo.itemName;
        qCDebug(DIGIKAM_DATABASE_LOG) << "Will reuse this image info and set the status to visible and the album id to" << d->scanInfo.albumID;

        d->scanInfo.id = imageId;
        CoreDbAccess().db()->setItemAlbum(imageId, d->scanInfo.albumID);
        CoreDbAccess().db()->setItemStatus(imageId, DatabaseItem::Status::Visible);
    }
    else
    {
        d->scanInfo.id = CoreDbAccess().db()->addItem(d->scanInfo.albumID, d->scanInfo.itemName,
                                                      d->scanInfo.status, d->scanInfo.category,
                                                      d->scanInfo.modificationDate, d->scanInfo.fileSize,
                                                      d->scanInfo.uniqueHash);
    }
}

void ItemScanner::prepareUpdateImage()
{
    d->commit.operation = ItemScannerCommit::UpdateItem;
}

void ItemScanner::commitUpdateImage()
{
    CoreDbAccess().db()->updateItem(d->scanInfo.id, d->scanInfo.category,
                                    d->scanInfo.modificationDate, d->scanInfo.fileSize,
                                    d->scanInfo.uniqueHash);
}

void ItemScanner::scanFile(ScanMode mode)
{
    d->scanMode = mode;

    if (d->scanMode == ModifiedScan)
    {
        if (d->scanInfo.category == DatabaseItem::Image)
        {
            scanImageInformation();
            scanImageHistoryIfModified();
        }
        else if (d->scanInfo.category == DatabaseItem::Video ||
                 d->scanInfo.category == DatabaseItem::Audio)
        {
            scanVideoInformation();

            // NOTE: Here, we only scan fields which can be expected to have changed, when we detect a change of file data.
            // It seems to me that at the moment video metadata contains such fields (which may change after editing).
            // In contrast, with photos, ImageMetadata contains fields which describe the moment of taking the photo,
            // which means they don't change.
            if (d->hasMetadata)
            {
                scanVideoMetadata();
            }
        }
        else if (d->scanInfo.category == DatabaseItem::Other)
        {
            // unsupported
        }
    }
    else
    {
        if (d->scanMode == Rescan && d->scanInfo.id != -1 &&
            MetaEngineSettings::instance()->settings().clearMetadataIfRescan)
        {
            CoreDbAccess().db()->clearMetadataFromImage(d->scanInfo.id);
        }

        if (d->scanInfo.category == DatabaseItem::Image)
        {
            scanImageInformation();

            if (d->hasMetadata)
            {
                scanImageMetadata();
                scanImagePosition();
                scanImageComments();
                scanImageCopyright();
                scanIPTCCore();
                scanTags();
                scanFaces();
                scanImageHistory();
                scanBalooInfo();
            }
        }
        else if (d->scanInfo.category == DatabaseItem::Video ||
                 d->scanInfo.category == DatabaseItem::Audio)
        {
            scanVideoInformation();

            if (d->hasMetadata)
            {
                scanVideoMetadata();
                scanImagePosition();
                scanImageComments();
                scanImageCopyright();
                scanIPTCCore();
                scanTags();
            }
        }
        else if (d->scanInfo.category == DatabaseItem::Other)
        {
            // unsupported
        }
    }
}

void ItemScanner::scanImageInformation()
{
    d->commit.commitImageInformation = true;

    if (d->scanMode == NewScan || d->scanMode == Rescan)
    {
        d->commit.imageInformationFields = DatabaseFields::ImageInformationAll;

        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate
               << MetadataInfo::DigitizationDate
               << MetadataInfo::Orientation;
        QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

        checkCreationDateFromMetadata(metadataInfos[1]);

        if (!checkRatingFromMetadata(metadataInfos.at(0)))
        {
            d->commit.imageInformationFields &= ~DatabaseFields::Rating;
            metadataInfos.removeAt(0);
        }

        d->commit.imageInformationInfos = metadataInfos;
    }
    else
    {
        // Does _not_ update rating and orientation (unless dims were exchanged)!
/*
        int orientation = d->metadata.getImageOrientation();
        QVariantList data = CoreDbAccess().db()->getImageInformation(d->scanInfo.id,
                                                                       DatabaseFields::Width |
                                                                       DatabaseFields::Height |
                                                                       DatabaseFields::Orientation);
        if (data.size() == 3 && data[2].isValid() && data[2].toInt() != orientation)
        {
            // be careful not to overwrite our value set in the database
            // But there is a special case: if the dims were
        }
*/

        d->commit.imageInformationFields =
                DatabaseFields::Width      |
                DatabaseFields::Height     |
                DatabaseFields::Format     |
                DatabaseFields::ColorDepth |
                DatabaseFields::ColorModel;
    }

    QSize size = d->img.size();
    d->commit.imageInformationInfos
          << size.width()
          << size.height()
          << detectImageFormat()
          << d->img.originalBitDepth()
          << d->img.originalColorModel();
}

void ItemScanner::commitImageInformation()
{
    if (d->scanMode == NewScan)
    {
        CoreDbAccess().db()->addImageInformation(d->scanInfo.id,
                                                 d->commit.imageInformationInfos,
                                                 d->commit.imageInformationFields);
    }
    else // d->scanMode == Rescan or d->scanMode == ModifiedScan
    {
        CoreDbAccess().db()->changeImageInformation(d->scanInfo.id,
                                                    d->commit.imageInformationInfos,
                                                    d->commit.imageInformationFields);
    }
}

bool ItemScanner::hasValidField(const QVariantList& list)
{
    for (QVariantList::const_iterator it = list.constBegin() ;
         it != list.constEnd() ; ++it)
    {
        if (!(*it).isNull())
        {
            return true;
        }
    }

    return false;
}

void ItemScanner::sortByProximity(QList<ImageInfo>& list, const ImageInfo& subject)
{
    if (!list.isEmpty() && !subject.isNull())
    {
        std::stable_sort(list.begin(), list.end(), lessThanByProximityToSubject(subject));
    }
}

// ---------------------------------------------------------------------------------------

void ItemScanner::loadFromDisk()
{
    if (d->loadedFromDisk)
    {
        return;
    }

    d->loadedFromDisk = true;
    d->metadata.registerMetadataSettings();
    d->hasMetadata    = d->metadata.load(d->fileInfo.filePath());

    if (d->scanInfo.category == DatabaseItem::Image)
    {
        d->hasImage = d->img.loadImageInfo(d->fileInfo.filePath(), false, false, false, false);
    }
    else
    {
        d->hasImage = false;
    }

    QDateTime modificationDate = d->fileInfo.lastModified();

    if (DMetadata::hasSidecar(d->fileInfo.filePath()))
    {
        QString filePath      = DMetadata::sidecarPath(d->fileInfo.filePath());
        QDateTime sidecarDate = QFileInfo(filePath).lastModified();

        if (sidecarDate > modificationDate)
        {
            modificationDate = sidecarDate;
        }
    }

    d->scanInfo.itemName         = d->fileInfo.fileName();
    d->scanInfo.fileSize         = d->fileInfo.size();
    d->scanInfo.modificationDate = modificationDate;
    // category is set by setCategory
    // NOTE: call uniqueHash after loading the image above, else it will fail
    d->scanInfo.uniqueHash       = uniqueHash();

   // faster than loading twice from disk
    if (d->hasMetadata)
    {
        d->img.setMetadata(d->metadata.data());
    }
}

QDateTime ItemScanner::creationDateFromFilesystem(const QFileInfo& info)
{
    // creation date is not what it seems on Unix
    QDateTime ctime = info.created();
    QDateTime mtime = info.lastModified();

    if (ctime.isNull())
    {
        return mtime;
    }

    if (mtime.isNull())
    {
        return ctime;
    }

    return qMin(ctime, mtime);
}

QString ItemScanner::formatToString(const QString& format)
{
    // image -------------------------------------------------------------------

    if (format == QLatin1String("JPG"))
    {
        return QLatin1String("JPEG");
    }
    else if (format == QLatin1String("PNG"))
    {
        return format;
    }
    else if (format == QLatin1String("TIFF"))
    {
        return format;
    }
    else if (format == QLatin1String("PPM"))
    {
        return format;
    }
    else if (format == QLatin1String("JP2") || format == QLatin1String("JP2k") || format == QLatin1String("JP2K"))
    {
        return QLatin1String("JPEG 2000");
    }
    else if (format.startsWith(QLatin1String("RAW-")))
    {
        return i18nc("RAW image file (), the parentheses contain the file suffix, like MRW",
                     "RAW image file (%1)",
                     format.mid(4));
    }

    // video -------------------------------------------------------------------

    else if (format == QLatin1String("MPEG"))
    {
        return format;
    }
    else if (format == QLatin1String("AVI"))
    {
        return format;
    }
    else if (format == QLatin1String("MOV"))
    {
        return QLatin1String("Quicktime");
    }
    else if (format == QLatin1String("WMF"))
    {
        return QLatin1String("Windows MetaFile");
    }
    else if (format == QLatin1String("WMV"))
    {
        return QLatin1String("Windows Media Video");
    }
    else if (format == QLatin1String("MP4"))
    {
        return QLatin1String("MPEG-4");
    }
    else if (format == QLatin1String("3GP"))
    {
        return QLatin1String("3GPP");
    }

    // audio -------------------------------------------------------------------

    else if (format == QLatin1String("OGG"))
    {
        return QLatin1String("Ogg");
    }
    else if (format == QLatin1String("MP3"))
    {
        return format;
    }
    else if (format == QLatin1String("WMA"))
    {
        return QLatin1String("Windows Media Audio");
    }
    else if (format == QLatin1String("WAV"))
    {
        return QLatin1String("WAVE");
    }
    else
    {
        return format;
    }
}

void ItemScanner::fillCommonContainer(qlonglong imageid, ImageCommonContainer* const container)
{
    QVariantList imagesFields;
    QVariantList imageInformationFields;

    {
        CoreDbAccess access;
        imagesFields = access.db()->getImagesFields(imageid,
                                                    DatabaseFields::Name             |
                                                    DatabaseFields::Category         |
                                                    DatabaseFields::ModificationDate |
                                                    DatabaseFields::FileSize);

        imageInformationFields = access.db()->getImageInformation(imageid,
                                                                  DatabaseFields::Rating           |
                                                                  DatabaseFields::CreationDate     |
                                                                  DatabaseFields::DigitizationDate |
                                                                  DatabaseFields::Orientation      |
                                                                  DatabaseFields::Width            |
                                                                  DatabaseFields::Height           |
                                                                  DatabaseFields::Format           |
                                                                  DatabaseFields::ColorDepth       |
                                                                  DatabaseFields::ColorModel);
    }

    if (!imagesFields.isEmpty())
    {
        container->fileName             = imagesFields.at(0).toString();
        container->fileModificationDate = imagesFields.at(2).toDateTime();
        container->fileSize             = imagesFields.at(3).toLongLong();
    }

    if (!imageInformationFields.isEmpty())
    {
        container->rating           = imageInformationFields.at(0).toInt();
        container->creationDate     = imageInformationFields.at(1).toDateTime();
        container->digitizationDate = imageInformationFields.at(2).toDateTime();
        container->orientation      = DMetadata::valueToString(imageInformationFields.at(3), MetadataInfo::Orientation);
        container->width            = imageInformationFields.at(4).toInt();
        container->height           = imageInformationFields.at(5).toInt();
        container->format           = formatToString(imageInformationFields.at(6).toString());
        container->colorDepth       = imageInformationFields.at(7).toInt();

        container->colorModel       = (imagesFields.at(1).toInt() == DatabaseItem::Video)                        ?
            DMetadata::videoColorModelToString((DMetadata::VIDEOCOLORMODEL)imageInformationFields.at(8).toInt()) :
            DImg::colorModelToString((DImg::COLORMODEL)imageInformationFields.at(8).toInt());
    }
}

} // namespace Digikam
