/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning of a single image
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagescanner.h"

// Qt includes

#include <QImageReader>
#include <QTime>

// KDE includes

#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "databaseurl.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "facetagseditor.h"
#include "imagecomments.h"
#include "imagecopyright.h"
#include "imageextendedproperties.h"
#include "imagehistorygraph.h"
#include "metadatasettings.h"
#include "tagregion.h"
#include "tagscache.h"
#include "iostream"

#ifdef HAVE_BALOO
#include "baloowrap.h"
#endif

namespace Digikam
{

class ImageScannerCommit
{

public:

    enum Operation
    {
        NoOp,
        AddItem,
        UpdateItem
    };

public:

    ImageScannerCommit()
        : operation(NoOp),
          copyImageAttributesId(-1),
          commitImageInformation(false),
          commitImageMetadata(false),
          commitVideoMetadata(false),
          commitImagePosition(false),
          commitImageComments(false),
          commitImageCopyright(false),
          commitFaces(false),
          commitIPTCCore(false),
          hasColorTag(false),
          hasPickTag(false)
    {
    }

public:

    Operation                        operation;

    qlonglong                        copyImageAttributesId;

    bool                             commitImageInformation;
    bool                             commitImageMetadata;
    bool                             commitVideoMetadata;
    bool                             commitImagePosition;
    bool                             commitImageComments;
    bool                             commitImageCopyright;
    bool                             commitFaces;
    bool                             commitIPTCCore;
    bool                             hasColorTag;
    bool                             hasPickTag;

    DatabaseFields::ImageInformation imageInformationFields;
    QVariantList                     imageInformationInfos;

    QVariantList                     imageMetadataInfos;
    QVariantList                     imagePositionInfos;

    CaptionsMap                      captions;
    QString                          headline;
    QString                          title;

    Template                         copyrightTemplate;
    QMap<QString,QVariant>           metadataFacesMap;

    QVariantList                     iptcCoreMetadataInfos;

    QList<int>                       tagIds;
    QString                          historyXml;
    QString                          uuid;
};

// ---------------------------------------------------------------------------

class ImageScanner::Private
{
public:

    Private()
        : hasImage(false),
          hasMetadata(false),
          loadedFromDisk(false),
          scanMode(ModifiedScan),
          hasHistoryToResolve(false)
    {
        time.start();
    }

public:

    bool                   hasImage;
    bool                   hasMetadata;
    bool                   loadedFromDisk;

    QFileInfo              fileInfo;

    DMetadata              metadata;
    DImg                   img;
    ItemScanInfo           scanInfo;
    ImageScanner::ScanMode scanMode;

    bool                   hasHistoryToResolve;

    ImageScannerCommit     commit;

    QTime                  time;
};

ImageScanner::ImageScanner(const QFileInfo& info, const ItemScanInfo& scanInfo)
    : d(new Private)
{
    d->fileInfo = info;
    d->scanInfo = scanInfo;
}

ImageScanner::ImageScanner(const QFileInfo& info)
    : d(new Private)
{
    d->fileInfo = info;
}

ImageScanner::ImageScanner(qlonglong imageid)
    : d(new Private)
{
    ItemShortInfo shortInfo;
    {
        DatabaseAccess access;
        shortInfo   = access.db()->getItemShortInfo(imageid);
        d->scanInfo = access.db()->getItemScanInfo(imageid);
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(shortInfo.albumRootID);
    d->fileInfo           = QFileInfo(DatabaseUrl::fromAlbumAndName(shortInfo.itemName,
                                      shortInfo.album, albumRootPath, shortInfo.albumRootID).fileUrl().toLocalFile());
}

ImageScanner::~ImageScanner()
{
    kDebug() << "Finishing took" << d->time.elapsed() << "ms";
    delete d;
}

qlonglong ImageScanner::id() const
{
    return d->scanInfo.id;
}

void ImageScanner::setCategory(DatabaseItem::Category category)
{
    // we don't have the necessary information in this class, but in CollectionScanner
    d->scanInfo.category = category;
}

void ImageScanner::commit()
{
    kDebug() << "Scanning took" << d->time.restart() << "ms";

    switch (d->commit.operation)
    {
        case ImageScannerCommit::NoOp:
            return;
        case ImageScannerCommit::AddItem:
            commitAddImage();
            break;
        case ImageScannerCommit::UpdateItem:
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

void ImageScanner::fileModified()
{
    loadFromDisk();
    prepareUpdateImage();
    scanFile(ModifiedScan);
}

void ImageScanner::newFile(int albumId)
{
    loadFromDisk();
    prepareAddImage(albumId);

    if (!scanFromIdenticalFile())
    {
        scanFile(NewScan);
    }
}

void ImageScanner::newFileFullScan(int albumId)
{
    loadFromDisk();
    prepareAddImage(albumId);
    scanFile(NewScan);
}

void ImageScanner::rescan()
{
    loadFromDisk();
    prepareUpdateImage();
    scanFile(Rescan);
}

void ImageScanner::copiedFrom(int albumId, qlonglong srcId)
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
            if (d->commit.commitImagePosition)
            {
                commitImagePosition();
            }

            scanFile(NewScan);
        }
    }
}

const ItemScanInfo& ImageScanner::itemScanInfo() const
{
    return d->scanInfo;
}

bool ImageScanner::hasHistoryToResolve() const
{
    return d->hasHistoryToResolve;
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

bool ImageScanner::scanFromIdenticalFile()
{
    // Get a list of other images that are identical. Source image shall not be included.
    // When using the Commit functionality, d->scanInfo.id can be null.
    QList<ItemScanInfo> candidates = DatabaseAccess().db()->getIdenticalFiles(d->scanInfo.uniqueHash,
                                     d->scanInfo.fileSize, d->scanInfo.id);

    if (!candidates.isEmpty())
    {
        // Sort by priority, as implemented by custom lessThan()
        qStableSort(candidates.begin(), candidates.end(), lessThanForIdentity);

        kDebug() << "Recognized" << d->fileInfo.filePath() << "as identical to item" << candidates.first().id;

        // Copy attributes.
        // Todo for the future is to worry about syncing identical files.
        d->commit.copyImageAttributesId = candidates.first().id;

        return true;
    }

    return false;
}

void ImageScanner::commitCopyImageAttributes()
{
    DatabaseAccess().db()->copyImageAttributes(d->commit.copyImageAttributesId, d->scanInfo.id);
}

bool ImageScanner::copyFromSource(qlonglong srcId)
{
    DatabaseAccess access;

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

    kDebug() << "Recognized" << d->fileInfo.filePath() << "as copied from" << srcId;
    d->commit.copyImageAttributesId = srcId;

    return true;
}

void ImageScanner::prepareAddImage(int albumId)
{
    d->scanInfo.albumID          = albumId;
    d->scanInfo.status           = DatabaseItem::Visible;

    kDebug() << "Adding new item" << d->fileInfo.filePath();
    d->commit.operation = ImageScannerCommit::AddItem;
}

void ImageScanner::commitAddImage()
{
    d->scanInfo.id = DatabaseAccess().db()->addItem(d->scanInfo.albumID, d->scanInfo.itemName,
                                                    d->scanInfo.status, d->scanInfo.category,
                                                    d->scanInfo.modificationDate, d->scanInfo.fileSize,
                                                    d->scanInfo.uniqueHash);
}

void ImageScanner::prepareUpdateImage()
{
    d->commit.operation = ImageScannerCommit::UpdateItem;
}

void ImageScanner::commitUpdateImage()
{
    DatabaseAccess().db()->updateItem(d->scanInfo.id, d->scanInfo.category,
                                      d->scanInfo.modificationDate, d->scanInfo.fileSize,
                                      d->scanInfo.uniqueHash);
}

void ImageScanner::scanFile(ScanMode mode)
{
    d->scanMode = mode;

    if (d->scanMode == ModifiedScan)
    {
        if (d->scanInfo.category == DatabaseItem::Image)
        {
            scanImageInformation();
            scanImageHistoryIfModified();
        }
        else if (d->scanInfo.category == DatabaseItem::Video)
        {
            scanVideoInformation();

            // NOTE: Here, we only scan fields which can be expected to have changed, when we detect a change of file data.
            // It seems to me that at the moment video metadata contains such fields (which may change after editing).
            // In contrast, with photos, ImageMetadata contains fields which describe the moment of taking the photo,
            //  which means they dont change.
            if (d->hasMetadata)
            {
                scanVideoMetadata();
            }
        }
    }
    else
    {
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
        else if (d->scanInfo.category == DatabaseItem::Video)
        {
            scanVideoInformation();

            if (d->hasMetadata)
            {
                scanVideoMetadata();
            }
        }
        else if (d->scanInfo.category == DatabaseItem::Audio)
        {
            scanAudioFile();
        }
        else if (d->scanInfo.category == DatabaseItem::Other)
        {
            // unsupported
        }
    }
}

void ImageScanner::checkCreationDateFromMetadata(QVariant& dateFromMetadata) const
{
    // creation date: fall back to file system property
    if (dateFromMetadata.isNull() || !dateFromMetadata.toDateTime().isValid())
    {
        dateFromMetadata = creationDateFromFilesystem(d->fileInfo);
    }
}

bool ImageScanner::checkRatingFromMetadata(const QVariant& ratingFromMetadata) const
{
    // should only be overwritten if set in metadata
    if (d->scanMode == Rescan)
    {
        if (ratingFromMetadata.isNull() || ratingFromMetadata.toInt() == -1)
        {
            return false;
        }
    }

    return true;
}

void ImageScanner::scanImageInformation()
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
        QVariantList data = DatabaseAccess().db()->getImageInformation(d->scanInfo.id,
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

void ImageScanner::commitImageInformation()
{
    if (d->scanMode == NewScan)
    {
        DatabaseAccess().db()->addImageInformation(d->scanInfo.id,
                                                   d->commit.imageInformationInfos,
                                                   d->commit.imageInformationFields);
    }
    else // d->scanMode == Rescan or d->scanMode == ModifiedScan
    {
        DatabaseAccess().db()->changeImageInformation(d->scanInfo.id,
                                                      d->commit.imageInformationInfos,
                                                      d->commit.imageInformationFields);
    }
}

static bool hasValidField(const QVariantList& list)
{
    for (QVariantList::const_iterator it = list.constBegin();
         it != list.constEnd(); ++it)
    {
        if (!(*it).isNull())
        {
            return true;
        }
    }

    return false;
}

static MetadataFields allImageMetadataFields()
{
    // This list must reflect the order required by AlbumDB::addImageMetadata
    MetadataFields fields;
    fields << MetadataInfo::Make
           << MetadataInfo::Model
           << MetadataInfo::Lens
           << MetadataInfo::Aperture
           << MetadataInfo::FocalLength
           << MetadataInfo::FocalLengthIn35mm
           << MetadataInfo::ExposureTime
           << MetadataInfo::ExposureProgram
           << MetadataInfo::ExposureMode
           << MetadataInfo::Sensitivity
           << MetadataInfo::FlashMode
           << MetadataInfo::WhiteBalance
           << MetadataInfo::WhiteBalanceColorTemperature
           << MetadataInfo::MeteringMode
           << MetadataInfo::SubjectDistance
           << MetadataInfo::SubjectDistanceCategory;
    return fields;
}

void ImageScanner::scanImageMetadata()
{
    QVariantList metadataInfos = d->metadata.getMetadataFields(allImageMetadataFields());

    if (hasValidField(metadataInfos))
    {
        d->commit.commitImageMetadata = true;
        d->commit.imageMetadataInfos  = metadataInfos;
    }
}

void ImageScanner::commitImageMetadata()
{
    DatabaseAccess().db()->addImageMetadata(d->scanInfo.id, d->commit.imageMetadataInfos);
}

void ImageScanner::scanImagePosition()
{
    // This list must reflect the order required by AlbumDB::addImagePosition
    MetadataFields fields;
    fields << MetadataInfo::Latitude
           << MetadataInfo::LatitudeNumber
           << MetadataInfo::Longitude
           << MetadataInfo::LongitudeNumber
           << MetadataInfo::Altitude
           << MetadataInfo::PositionOrientation
           << MetadataInfo::PositionTilt
           << MetadataInfo::PositionRoll
           << MetadataInfo::PositionAccuracy
           << MetadataInfo::PositionDescription;

    QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

    if (hasValidField(metadataInfos))
    {
        d->commit.commitImagePosition = true;
        d->commit.imagePositionInfos  = metadataInfos;
    }
}

void ImageScanner::commitImagePosition()
{
    DatabaseAccess().db()->addImagePosition(d->scanInfo.id, d->commit.imagePositionInfos);
}

void ImageScanner::scanImageComments()
{
    MetadataFields fields;
    fields << MetadataInfo::Headline
           << MetadataInfo::Title;

    QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

    // handles all possible fields, multi-language, author, date
    CaptionsMap captions = d->metadata.getImageComments();

    if (captions.isEmpty() && !hasValidField(metadataInfos))
    {
        return;
    }

    d->commit.commitImageComments  = true;
    d->commit.captions             = captions;

    // Headline
    if (!metadataInfos.at(0).isNull())
    {
        d->commit.headline = metadataInfos.at(0).toString();
    }

    // Title
    if (!metadataInfos.at(1).isNull())
    {
        d->commit.title = metadataInfos.at(1).toMap()["x-default"].toString();
    }
}

void ImageScanner::commitImageComments()
{
    DatabaseAccess access;
    ImageComments comments(access, d->scanInfo.id);

    // Description
    if (!d->commit.captions.isEmpty())
    {
        comments.replaceComments(d->commit.captions);
    }

    // Headline
    if (!d->commit.headline.isNull())
    {
        comments.addHeadline(d->commit.headline);
    }

    // Title
    if (!d->commit.title.isNull())
    {
        comments.addTitle(d->commit.title);
    }
}

void ImageScanner::scanImageCopyright()
{
    Template t;

    if (!d->metadata.getCopyrightInformation(t))
    {
        return;
    }

    d->commit.commitImageCopyright = true;
    d->commit.copyrightTemplate    = t;
}

void ImageScanner::commitImageCopyright()
{
    ImageCopyright copyright(d->scanInfo.id);
    // It is not clear if removeAll() should be called if d->scanMode == Rescan
    copyright.removeAll();
    copyright.setFromTemplate(d->commit.copyrightTemplate);
}

void ImageScanner::scanIPTCCore()
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreLocationInfo
           << MetadataInfo::IptcCoreIntellectualGenre
           << MetadataInfo::IptcCoreJobID
           << MetadataInfo::IptcCoreScene
           << MetadataInfo::IptcCoreSubjectCode;

    QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

    if (!hasValidField(metadataInfos))
    {
        return;
    }

    d->commit.commitIPTCCore        = true;
    d->commit.iptcCoreMetadataInfos = metadataInfos;
}

void ImageScanner::commitIPTCCore()
{
    ImageExtendedProperties props(d->scanInfo.id);

    if (!d->commit.iptcCoreMetadataInfos.at(0).isNull())
    {
        IptcCoreLocationInfo loc = d->commit.iptcCoreMetadataInfos.at(0).value<IptcCoreLocationInfo>();

        if (!loc.isNull())
        {
            props.setLocation(loc);
        }
    }

    if (!d->commit.iptcCoreMetadataInfos.at(1).isNull())
    {
        props.setIntellectualGenre(d->commit.iptcCoreMetadataInfos.at(1).toString());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(2).isNull())
    {
        props.setJobId(d->commit.iptcCoreMetadataInfos.at(2).toString());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(3).isNull())
    {
        props.setScene(d->commit.iptcCoreMetadataInfos.at(3).toStringList());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(4).isNull())
    {
        props.setSubjectCode(d->commit.iptcCoreMetadataInfos.at(4).toStringList());
    }
}

void ImageScanner::scanTags()
{
    // Check Keywords tag paths.

    QVariant var         = d->metadata.getMetadataField(MetadataInfo::Keywords);
    QStringList keywords = var.toStringList();
    QStringList filteredKeywords;

    // Extra empty tags check, empty tag = root tag which is not asignable
    for(int index = 0; index < keywords.size(); index++)
    {
        QString keyword = keywords.at(index);

        if(!keyword.isEmpty())
        {

            // _Digikam_root_tag_ is present in some photos tagged with older
            // version of digiKam, must be removed
            if(keyword.contains(QRegExp("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)")))
            {
                keyword = keyword.replace(QRegExp("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)"),
                                          QString(""));
            }

            filteredKeywords.append(keyword);
        }
    }

    if (!filteredKeywords.isEmpty())
    {
        // get tag ids, create if necessary
        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(filteredKeywords);
        d->commit.tagIds += tagIds;
    }

    // Check Pick Label tag.

    int pickId = d->metadata.getImagePickLabel();

    if (pickId != -1)
    {
        kDebug() << "Pick Label found : " << pickId;

        int tagId = TagsCache::instance()->tagForPickLabel((PickLabel)pickId);

        if (tagId)
        {
            d->commit.tagIds << tagId;
            d->commit.hasPickTag = true;
            kDebug() << "Assigned Pick Label Tag  : " << tagId;
        }
        else
        {
            kDebug() << "Cannot find Pick Label Tag for : " << pickId;
        }
    }

    // Check Color Label tag.

    int colorId = d->metadata.getImageColorLabel();

    if (colorId != -1)
    {
        kDebug() << "Color Label found : " << colorId;

        int tagId = TagsCache::instance()->tagForColorLabel((ColorLabel)colorId);

        if (tagId)
        {
            d->commit.tagIds << tagId;
            d->commit.hasColorTag = true;
            kDebug() << "Assigned Color Label Tag  : " << tagId;
        }
        else
        {
            kDebug() << "Cannot find Color Label Tag for : " << colorId;
        }
    }
}

void ImageScanner::commitTags()
{
    QList<int> currentTags = DatabaseAccess().db()->getItemTagIDs(d->scanInfo.id);
    QVector<int> colorTags = TagsCache::instance()->colorLabelTags();
    QVector<int> pickTags  = TagsCache::instance()->pickLabelTags();
    QList<int> removeTags;

    foreach(int cTag, currentTags)
    {
        if ((d->commit.hasColorTag && colorTags.contains(cTag)) ||
            (d->commit.hasPickTag && pickTags.contains(cTag)))
        {
            removeTags << cTag;
        }
    }

    if (!removeTags.isEmpty())
    {
        DatabaseAccess().db()->removeTagsFromItems(QList<qlonglong>() << d->scanInfo.id, removeTags);
    }

    DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << d->scanInfo.id, d->commit.tagIds);
}

void ImageScanner::scanFaces()
{
    QSize size = d->img.size();

    if (!size.isValid())
    {
        return;
    }

    QMultiMap<QString,QVariant> metadataFacesMap;

    if (!d->metadata.getImageFacesMap(metadataFacesMap))
    {
        return;
    }

    d->commit.commitFaces = true;
    d->commit.metadataFacesMap = metadataFacesMap;
}

void ImageScanner::commitFaces()
{
    QSize size = d->img.size();
    QMap<QString, QVariant>::const_iterator it;

    for (it = d->commit.metadataFacesMap.constBegin(); it != d->commit.metadataFacesMap.constEnd(); ++it)
    {
        QString name = it.key();
        QRectF rect  = it.value().toRectF();

        if (name.isEmpty() || !rect.isValid())
        {
            continue;
        }

        int tagId = FaceTags::getOrCreateTagForPerson(name);

        if (!tagId)
        {
            kDebug() << "Failed to create a person tag for name" << name;
        }

        TagRegion region(TagRegion::relativeToAbsolute(rect, size));

        FaceTagsEditor editor;

        editor.add(d->scanInfo.id, tagId, region, false);
    }
}

void ImageScanner::scanImageHistory()
{
    /** Stage 1 of history scanning */

    d->commit.historyXml = d->metadata.getImageHistory();
    d->commit.uuid       = d->metadata.getImageUniqueId();
}

void ImageScanner::commitImageHistory()
{
    if (!d->commit.historyXml.isEmpty())
    {
        DatabaseAccess().db()->setImageHistory(d->scanInfo.id, d->commit.historyXml);
        // Delay history resolution by setting this tag:
        // Resolution depends on the presence of other images, possibly only when the scanning process has finished
        DatabaseAccess().db()->addItemTag(d->scanInfo.id, TagsCache::instance()->
                                          getOrCreateInternalTag(InternalTagName::needResolvingHistory()));
        d->hasHistoryToResolve = true;
    }

    if (!d->commit.uuid.isNull())
    {
        DatabaseAccess().db()->setImageUuid(d->scanInfo.id, d->commit.uuid);
    }
}

void ImageScanner::scanImageHistoryIfModified()
{
    // If a file has a modified history, it must have a new UUID
    QString previousUuid = DatabaseAccess().db()->getImageUuid(d->scanInfo.id);
    QString currentUuid  = d->metadata.getImageUniqueId();

    if (!currentUuid.isEmpty() && previousUuid != currentUuid)
    {
        scanImageHistory();
    }
}

bool ImageScanner::resolveImageHistory(qlonglong id, QList<qlonglong>* needTaggingIds)
{
    ImageHistoryEntry history = DatabaseAccess().db()->getImageHistory(id);
    return resolveImageHistory(id, history.history, needTaggingIds);
}

bool ImageScanner::resolveImageHistory(qlonglong imageId, const QString& historyXml,
                                       QList<qlonglong>* needTaggingIds)
{
    /** Stage 2 of history scanning */

    if (historyXml.isNull())
    {
        return true;    // "true" means nothing is left to resolve
    }

    DImageHistory history = DImageHistory::fromXml(historyXml);

    if (history.isNull())
    {
        return true;
    }

    ImageHistoryGraph graph;
    graph.addScannedHistory(history, imageId);

    if (!graph.hasEdges())
    {
        return true;
    }

    QPair<QList<qlonglong>, QList<qlonglong> > cloud = graph.relationCloudParallel();
    DatabaseAccess().db()->addImageRelations(cloud.first, cloud.second, DatabaseRelation::DerivedFrom);

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    // remove the needResolvingHistory tag from all images in graph
    DatabaseAccess().db()->removeTagsFromItems(graph.allImageIds(), QList<int>() << needResolvingTag);

    // mark a single image from the graph (sufficient for find the full relation cloud)
    QList<ImageInfo> roots = graph.rootImages();

    if (!roots.isEmpty())
    {
        DatabaseAccess().db()->addItemTag(roots.first().id(), needTaggingTag);

        if (needTaggingIds)
        {
            *needTaggingIds << roots.first().id();
        }
    }

    return !graph.hasUnresolvedEntries();
}

void ImageScanner::tagImageHistoryGraph(qlonglong id)
{
    /** Stage 3 of history scanning */

    ImageInfo info(id);

    if (info.isNull())
    {
        return;
    }
    //kDebug() << "tagImageHistoryGraph" << id;

    // Load relation cloud, history of info and of all leaves of the tree into the graph, fully resolved
    ImageHistoryGraph graph    = ImageHistoryGraph::fromInfo(info, ImageHistoryGraph::LoadAll, ImageHistoryGraph::NoProcessing);
    kDebug() << graph;

    int originalVersionTag     = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::originalVersion());
    int currentVersionTag      = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::currentVersion());
    int intermediateVersionTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::intermediateVersion());

    int needTaggingTag         = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    // Remove all relevant tags
    DatabaseAccess().db()->removeTagsFromItems(graph.allImageIds(), QList<int>() << originalVersionTag
        << currentVersionTag << intermediateVersionTag << needTaggingTag);

    if (!graph.hasEdges())
    {
        return;
    }

    // get category info
    QList<qlonglong>                                        originals, intermediates, currents;
    QHash<ImageInfo, HistoryImageId::Types>                 types = graph.categorize();
    QHash<ImageInfo, HistoryImageId::Types>::const_iterator it;

    for (it = types.constBegin(); it != types.constEnd(); ++it)
    {
        kDebug() << "Image" << it.key().id() << "type" << it.value();
        HistoryImageId::Types types = it.value();

        if (types & HistoryImageId::Original)
        {
            originals << it.key().id();
        }

        if (types & HistoryImageId::Intermediate)
        {
            intermediates << it.key().id();
        }

        if (types & HistoryImageId::Current)
        {
            currents << it.key().id();
        }
    }

    if (!originals.isEmpty())
    {
        DatabaseAccess().db()->addTagsToItems(originals, QList<int>() << originalVersionTag);
    }

    if (!intermediates.isEmpty())
    {
        DatabaseAccess().db()->addTagsToItems(intermediates, QList<int>() << intermediateVersionTag);
    }

    if (!currents.isEmpty())
    {
        DatabaseAccess().db()->addTagsToItems(currents, QList<int>() << currentVersionTag);
    }
}

DImageHistory ImageScanner::resolvedImageHistory(const DImageHistory& history, bool mustBeAvailable)
{
    DImageHistory h;

    foreach(const DImageHistory::Entry& e, history.entries())
    {
        // Copy entry, without referredImages
        DImageHistory::Entry entry;
        entry.action = e.action;

        // resolve referredImages
        foreach(const HistoryImageId& id, e.referredImages)
        {
            QList<qlonglong> imageIds = resolveHistoryImageId(id);

            // append each image found in collection to referredImages
            foreach(qlonglong imageId, imageIds)
            {
                ImageInfo info(imageId);

                if (info.isNull())
                {
                    continue;
                }

                if (mustBeAvailable)
                {
                    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(info.albumRootId());

                    if (!location.isAvailable())
                    {
                        continue;
                    }
                }

                HistoryImageId newId = info.historyImageId();
                newId.setType(id.m_type);
                entry.referredImages << newId;
            }
        }

        // add to history
        h.entries() << entry;
    }

    return h;
}

bool ImageScanner::sameReferredImage(const HistoryImageId& id1, const HistoryImageId& id2)
{
    if (!id1.isValid() || !id2.isValid())
    {
        return false;
    }

    /*
     * We give the UUID the power of equivalence that none of the other criteria has:
     * For two images a,b with uuids x,y, where x and y not null,
     *  a (same image as) b   <=>   x == y
     */
    if (id1.hasUuid() && id2.hasUuid())
    {
        return id1.m_uuid == id2.m_uuid;
    }

    if (id1.hasUniqueHashIdentifier()        &&
        id1.m_uniqueHash == id2.m_uniqueHash &&
        id1.m_fileSize   == id2.m_fileSize)
    {
        return true;
    }

    if (id1.hasFileName() && id1.hasCreationDate() &&
        id1.m_fileName     == id2.m_fileName       &&
        id1.m_creationDate == id2.m_creationDate)
    {
        return true;
    }

    if (id1.hasFileOnDisk()              &&
        id1.m_filePath == id2.m_filePath &&
        id1.m_fileName == id2.m_fileName)
    {
        return true;
    }

    return false;
}

// Returns true if both have the same UUID, or at least one of the two has no UUID
// Returns false iff both have a UUID and the UUIDs differ
static bool uuidDoesNotDiffer(const HistoryImageId& referenceId, qlonglong id)
{
    if (referenceId.hasUuid())
    {
        QString uuid = DatabaseAccess().db()->getImageUuid(id);

        if (!uuid.isEmpty())
        {
            return referenceId.m_uuid == uuid;
        }
    }

    return true;
}

static QList<qlonglong> mergedIdLists(const HistoryImageId& referenceId,
                         const QList<qlonglong>& uuidList, const QList<qlonglong>& candidates)
{
    QList<qlonglong> results;
    // uuidList are definite results
    results = uuidList;

    // Add a candidate if it has the same UUID, or either reference or candidate  have a UUID
    // (other way round: do not add a candidate which positively has a different UUID)
    foreach(qlonglong candidate, candidates)
    {
        if (results.contains(candidate))
        {
            continue; // already in list, skip
        }

        if (uuidDoesNotDiffer(referenceId, candidate))
        {
            results << candidate;
        }
    }

    return results;
}

QList<qlonglong> ImageScanner::resolveHistoryImageId(const HistoryImageId& historyId)
{
    // first and foremost: UUID
    QList<qlonglong> uuidList;

    if (historyId.hasUuid())
    {
        uuidList = DatabaseAccess().db()->getItemsForUuid(historyId.m_uuid);

        // If all images had a UUID, we would be finished and could return here with a result:
/*
        if (!uuidList.isEmpty())
        {
            return uuidList;
        }
*/
        // But as identical images may have no UUID yet, we need to continue
    }

    // Second: uniqueHash + fileSize. Sufficient to assume that a file is identical, but subject to frequent change.
    if (historyId.hasUniqueHashIdentifier() && DatabaseAccess().db()->isUniqueHashV2())
    {
        QList<ItemScanInfo> infos = DatabaseAccess().db()->getIdenticalFiles(historyId.m_uniqueHash, historyId.m_fileSize);

        if (!infos.isEmpty())
        {
            QList<qlonglong> ids;

            foreach(const ItemScanInfo& info, infos)
            {
                if (info.status != DatabaseItem::Removed)
                {
                    ids << info.id;
                }
            }

            return mergedIdLists(historyId, uuidList, ids);
        }
    }

    // As a third combination, we try file name and creation date. Susceptible to renaming,
    // but not to metadata changes.
    if (historyId.hasFileName() && historyId.hasCreationDate())
    {
        QList<qlonglong> ids = DatabaseAccess().db()->findByNameAndCreationDate(historyId.m_fileName, historyId.m_creationDate);

        if (!ids.isEmpty())
        {
            return mergedIdLists(historyId, uuidList, ids);
        }
    }

    // Another possibility: If the original UUID is given, we can find all relations for the image with this UUID,
    // and make an assumption from this group of images. Currently not implemented.

    // resolve old-style by full file path
    if (historyId.hasFileOnDisk())
    {
        QFileInfo file(historyId.filePath());

        if (file.exists())
        {
            CollectionLocation location = CollectionManager::instance()->locationForPath(historyId.path());

            if (!location.isNull())
            {
                QString album      = CollectionManager::instance()->album(file.path());
                QString name       = file.fileName();
                ItemShortInfo info = DatabaseAccess().db()->getItemShortInfo(location.id(), album, name);

                if (info.id)
                {
                    return mergedIdLists(historyId, uuidList, QList<qlonglong>() << info.id);
                }
            }
        }
    }

    return uuidList;
}

// ---------------------------------------------------------------------------------------

class lessThanByProximityToSubject
{
public:

    explicit lessThanByProximityToSubject(const ImageInfo& subject)
        : subject(subject)
    {
    }

    bool operator()(const ImageInfo& a, const ImageInfo& b)
    {
        if (a.isNull() || b.isNull())
        {
            // both null: false
            // only a null: a greater than b (null infos at end of list)
            //  (a && b) || (a && !b) = a
            // only b null: a less than b
            if (a.isNull())
            {
                return false;
            }

            return true;
        }

        if (a == b)
        {
            return false;
        }

        // same collection
        if (a.albumId() != b.albumId())
        {
            // same album
            if (a.albumId() == subject.albumId())
            {
                return true;
            }

            if (b.albumId() == subject.albumId())
            {
                return false;
            }

            if (a.albumRootId() != b.albumRootId())
            {
                // different collection
                if (a.albumRootId() == subject.albumRootId())
                {
                    return true;
                }

                if (b.albumRootId() == subject.albumRootId())
                {
                    return false;
                }
            }
        }

        if (a.modDateTime() != b.modDateTime())
        {
            return a.modDateTime() < b.modDateTime();
        }

        if (a.name() != b.name())
        {
            return qAbs(a.name().compare(subject.name())) < qAbs(b.name().compare(subject.name()));
        }

        // last resort
        return (a.id() < b.id());
    }

public:

    ImageInfo subject;
};

void ImageScanner::sortByProximity(QList<ImageInfo>& list, const ImageInfo& subject)
{
    if (!list.isEmpty() && !subject.isNull())
    {
        qStableSort(list.begin(), list.end(), lessThanByProximityToSubject(subject));
    }
}

// ---------------------------------------------------------------------------------------

static MetadataFields allVideoMetadataFields()
{
    // This list must reflect the order required by AlbumDB::addVideoMetadata
    MetadataFields fields;
    fields << MetadataInfo::AspectRatio
           << MetadataInfo::AudioBitRate
           << MetadataInfo::AudioChannelType
           << MetadataInfo::AudioCompressor
           << MetadataInfo::Duration
           << MetadataInfo::FrameRate
           << MetadataInfo::VideoCodec;

    return fields;
}

void ImageScanner::scanVideoInformation()
{
    d->commit.commitImageInformation = true;

    if (d->scanMode == NewScan || d->scanMode == Rescan)
    {
        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate
               << MetadataInfo::DigitizationDate;
        QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

        d->commit.imageInformationFields = DatabaseFields::Rating | DatabaseFields::CreationDate | DatabaseFields::DigitizationDate;

        checkCreationDateFromMetadata(metadataInfos[1]);

        if (!checkRatingFromMetadata(metadataInfos.at(0)))
        {
            d->commit.imageInformationFields &= ~DatabaseFields::Rating;
            metadataInfos.removeAt(0);
        }

        d->commit.imageInformationInfos = metadataInfos;
    }

    d->commit.imageInformationInfos << d->metadata.getMetadataField(MetadataInfo::VideoWidth)
                                    << d->metadata.getMetadataField(MetadataInfo::VideoHeight);
    d->commit.imageInformationFields |= DatabaseFields::Width | DatabaseFields::Height;

    // TODO: Please check / improve / rewrite detectVideoFormat().
    // The format strings shall be uppercase, and a clearly defined set
    // (all format strings used in the database should be defined in advance)
    d->commit.imageInformationInfos  << detectVideoFormat();
    d->commit.imageInformationFields |= DatabaseFields::Format;

    // There is use of bit depth, but not ColorModel
    // For bit depth - 8bit, 16bit with videos
    d->commit.imageInformationInfos  << d->metadata.getMetadataField(MetadataInfo::VideoBitDepth);
    d->commit.imageInformationFields |= DatabaseFields::ColorDepth;
}

// commitImageInformation method is reused

void ImageScanner::scanVideoMetadata()
{
    QVariantList metadataInfos = d->metadata.getMetadataFields(allVideoMetadataFields());

    if (hasValidField(metadataInfos))
    {
        d->commit.commitVideoMetadata = true;
        // reuse imageMetadataInfos field
        d->commit.imageMetadataInfos  = metadataInfos;
    }
}

void ImageScanner::commitVideoMetadata()
{
    DatabaseAccess().db()->addVideoMetadata(d->scanInfo.id, d->commit.imageMetadataInfos);
}

// ---------------------------------------------------------------------------------------

void ImageScanner::scanAudioFile()
{
    /**
    * @todo
    */

    d->commit.commitImageInformation = true;

    d->commit.imageInformationInfos
          << -1
          << creationDateFromFilesystem(d->fileInfo)
          << detectAudioFormat();

    d->commit.imageInformationFields =
            DatabaseFields::Rating       |
            DatabaseFields::CreationDate |
            DatabaseFields::Format;
}

void ImageScanner::loadFromDisk()
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

    d->scanInfo.itemName         = d->fileInfo.fileName();
    d->scanInfo.modificationDate = d->fileInfo.lastModified();
    d->scanInfo.fileSize         = d->fileInfo.size();
    // category is set by setCategory
    // NOTE: call uniqueHash after loading the image above, else it will fail
    d->scanInfo.uniqueHash       = uniqueHash();

   // faster than loading twice from disk
    if (d->hasMetadata)
    {
        d->img.setMetadata(d->metadata.data());
    }
}

QString ImageScanner::uniqueHash() const
{
    // the QByteArray is an ASCII hex string
    if (d->scanInfo.category == DatabaseItem::Image)
    {
        if (DatabaseAccess().db()->isUniqueHashV2())
            return QString(d->img.getUniqueHashV2());
        else
            return QString(d->img.getUniqueHash());
    }
    else
    {
        if (DatabaseAccess().db()->isUniqueHashV2())
            return QString(DImg::getUniqueHashV2(d->fileInfo.filePath()));
        else
            return QString(DImg::getUniqueHash(d->fileInfo.filePath()));
    }
}

QString ImageScanner::detectImageFormat() const
{
    DImg::FORMAT dimgFormat = d->img.detectedFormat();

    switch (dimgFormat)
    {
        case DImg::JPEG:
            return "JPG";
        case DImg::PNG:
            return "PNG";
        case DImg::TIFF:
            return "TIFF";
        case DImg::PPM:
            return "PPM";
        case DImg::JP2K:
            return "JP2";
        case DImg::PGF:
            return "PGF";
        case DImg::RAW:
        {
            QString format = "RAW-";
            format += d->fileInfo.suffix().toUpper();
            return format;
        }
        case DImg::NONE:
        case DImg::QIMAGE:
        {
            QByteArray format = QImageReader::imageFormat(d->fileInfo.filePath());

            if (!format.isEmpty())
            {
                return QString(format).toUpper();
            }
            break;
        }
    }

    // See BUG #339341: KMimeType can return garbage. Take file name suffix instead.
    return d->fileInfo.suffix().toUpper();
}

QString ImageScanner::detectVideoFormat() const
{
    QString suffix = d->fileInfo.suffix().toUpper();

    if (suffix == "MPEG" || suffix == "MPG" || suffix == "MPO" || suffix == "MPE")
    {
        return "MPEG";
    }

    if (suffix == "ASF" || suffix == "WMV")
    {
        return "WMV";
    }

    if (suffix == "AVI" || suffix == "DIVX" )
    {
        return "AVI";
    }

    if (suffix == "MKV" || suffix == "MKS")
    {
        return "MKV";
    }

    if (suffix == "M4V" || suffix == "MOV" || suffix == "M2V" )
    {
        return "MOV";
    }

    if (suffix == "3GP" || suffix == "3G2" )
    {
        return "3GP";
    }

    return suffix;
}

QString ImageScanner::detectAudioFormat() const
{
    return d->fileInfo.suffix().toUpper();
}

QDateTime ImageScanner::creationDateFromFilesystem(const QFileInfo& info)
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

QString ImageScanner::formatToString(const QString& format)
{
    // image -------------------------------------------------------------------
    if (format == "JPG")
    {
        return "JPEG";
    }
    else if (format == "PNG")
    {
        return format;
    }
    else if (format == "TIFF")
    {
        return format;
    }
    else if (format == "PPM")
    {
        return format;
    }
    else if (format == "JP2" || format == "JP2k" || format == "JP2K")
    {
        return "JPEG 2000";
    }
    else if (format.startsWith(QLatin1String("RAW-")))
    {
        return i18nc("RAW image file (), the parentheses contain the file suffix, like MRW",
                     "RAW image file (%1)",
                     format.mid(4));
    }
    // video -------------------------------------------------------------------
    else if (format == "MPEG")
    {
        return format;
    }
    else if (format == "AVI")
    {
        return format;
    }
    else if (format == "MOV")
    {
        return "Quicktime";
    }
    else if (format == "WMF")
    {
        return "Windows MetaFile";
    }
    else if (format == "WMV")
    {
        return "Windows Media Video";
    }
    else if (format == "MP4")
    {
        return "MPEG-4";
    }
    else if (format == "3GP")
    {
        return "3GPP";
    }
    // audio -------------------------------------------------------------------
    else if (format == "OGG")
    {
        return "Ogg";
    }
    else if (format == "MP3")
    {
        return format;
    }
    else if (format == "WMA")
    {
        return "Windows Media Audio";
    }
    else if (format == "WAV")
    {
        return "WAVE";
    }
    else
    {
        return format;
    }
}

QString ImageScanner::iptcCorePropertyName(MetadataInfo::Field field)
{
    // These strings are specified in DBSCHEMA.ods
    switch (field)
    {
            // copyright table
        case MetadataInfo::IptcCoreCopyrightNotice:
            return "copyrightNotice";
        case MetadataInfo::IptcCoreCreator:
            return "creator";
        case MetadataInfo::IptcCoreProvider:
            return "provider";
        case MetadataInfo::IptcCoreRightsUsageTerms:
            return "rightsUsageTerms";
        case MetadataInfo::IptcCoreSource:
            return "source";
        case MetadataInfo::IptcCoreCreatorJobTitle:
            return "creatorJobTitle";
        case MetadataInfo::IptcCoreInstructions:
            return "instructions";

            // ImageProperties table
        case MetadataInfo::IptcCoreCountryCode:
            return "countryCode";
        case MetadataInfo::IptcCoreCountry:
            return "country";
        case MetadataInfo::IptcCoreCity:
            return "city";
        case MetadataInfo::IptcCoreLocation:
            return "location";
        case MetadataInfo::IptcCoreProvinceState:
            return "provinceState";
        case MetadataInfo::IptcCoreIntellectualGenre:
            return "intellectualGenre";
        case MetadataInfo::IptcCoreJobID:
            return "jobId";
        case MetadataInfo::IptcCoreScene:
            return "scene";
        case MetadataInfo::IptcCoreSubjectCode:
            return "subjectCode";
        case MetadataInfo::IptcCoreContactInfoCity:
            return "creatorContactInfo.city";
        case MetadataInfo::IptcCoreContactInfoCountry:
            return "creatorContactInfo.country";
        case MetadataInfo::IptcCoreContactInfoAddress:
            return "creatorContactInfo.address";
        case MetadataInfo::IptcCoreContactInfoPostalCode:
            return "creatorContactInfo.postalCode";
        case MetadataInfo::IptcCoreContactInfoProvinceState:
            return "creatorContactInfo.provinceState";
        case MetadataInfo::IptcCoreContactInfoEmail:
            return "creatorContactInfo.email";
        case MetadataInfo::IptcCoreContactInfoPhone:
            return "creatorContactInfo.phone";
        case MetadataInfo::IptcCoreContactInfoWebUrl:
            return "creatorContactInfo.webUrl";
        default:
            return QString();
    }
}

void ImageScanner::scanBalooInfo()
{
#ifdef HAVE_BALOO

    BalooWrap* const baloo = BalooWrap::instance();

    if(!baloo->getSyncToDigikam())
    {
        return;
    }

    BalooInfo bInfo = baloo->getSemanticInfo(KUrl(d->fileInfo.absoluteFilePath()));

    if (!bInfo.tags.isEmpty())
    {
        // get tag ids, create if necessary
        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(bInfo.tags);
        d->commit.tagIds += tagIds;
    }

    if(bInfo.rating != -1)
    {
        if(!d->commit.imageInformationFields.testFlag(DatabaseFields::Rating))
        {
            d->commit.imageInformationFields |= DatabaseFields::Rating;
            d->commit.imageInformationInfos.insert(0, QVariant(bInfo.rating));
        }
    }

    if(!bInfo.comment.isEmpty())
    {
        kDebug() << "+++++++++++++++++++++Comment " << bInfo.comment;

        if(!d->commit.captions.contains("x-default"))
        {
            CaptionValues val;
            val.caption                   = bInfo.comment;
            d->commit.commitImageComments = true;
            d->commit.captions.insert(QString("x-default"), val);
        }
    }

#endif // HAVE_BALOO
}

void ImageScanner::fillCommonContainer(qlonglong imageid, ImageCommonContainer* const container)
{
    QVariantList imagesFields;
    QVariantList imageInformationFields;

    {
        DatabaseAccess access;
        imagesFields = access.db()->getImagesFields(imageid,
                                                    DatabaseFields::Name             |
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
        container->fileModificationDate = imagesFields.at(1).toDateTime();
        container->fileSize             = imagesFields.at(2).toLongLong();
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
        container->colorModel       = DImg::colorModelToString((DImg::COLORMODEL)imageInformationFields.at(8).toInt());
    }
}

void ImageScanner::fillMetadataContainer(qlonglong imageid, ImageMetadataContainer* const container)
{
    // read from database
    QVariantList fields      = DatabaseAccess().db()->getImageMetadata(imageid);
    // check we have at least one valid field
    container->allFieldsNull = !hasValidField(fields);

    if (container->allFieldsNull)
    {
        return;
    }

    // DMetadata does all translation work
    QStringList strings = DMetadata::valuesToString(fields, allImageMetadataFields());

    // associate with hard-coded variables
    container->make                         = strings.at(0);
    container->model                        = strings.at(1);
    container->lens                         = strings.at(2);
    container->aperture                     = strings.at(3);
    container->focalLength                  = strings.at(4);
    container->focalLength35                = strings.at(5);
    container->exposureTime                 = strings.at(6);
    container->exposureProgram              = strings.at(7);
    container->exposureMode                 = strings.at(8);
    container->sensitivity                  = strings.at(9);
    container->flashMode                    = strings.at(10);
    container->whiteBalance                 = strings.at(11);
    container->whiteBalanceColorTemperature = strings.at(12);
    container->meteringMode                 = strings.at(13);
    container->subjectDistance              = strings.at(14);
    container->subjectDistanceCategory      = strings.at(15);
}

void ImageScanner::fillVideoMetadataContainer(qlonglong imageid, VideoMetadataContainer* const container)
{
    // read from database
    QVariantList fields      = DatabaseAccess().db()->getVideoMetadata(imageid);
    // check we have at least one valid field
    container->allFieldsNull = !hasValidField(fields);

    if (container->allFieldsNull)
    {
        return;
    }

    // DMetadata does all translation work
    QStringList strings = DMetadata::valuesToString(fields, allVideoMetadataFields());
    // associate with hard-coded variables
    container->aspectRatio                  = strings.at(0);
    container->audioBitRate                 = strings.at(1);
    container->audioChannelType             = strings.at(2);
    container->audioCompressor              = strings.at(3);
    container->duration                     = strings.at(4);
    container->frameRate                    = strings.at(5);
    container->videoCodec                   = strings.at(6);
}

} // namespace Digikam
