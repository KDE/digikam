/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning of a single image
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

ImageScanner::ImageScanner(const QFileInfo& info, const ItemScanInfo& scanInfo)
    : m_hasImage(false),
      m_hasMetadata(false),
      m_loadedFromDisk(false),
      m_fileInfo(info),
      m_scanInfo(scanInfo),
      m_scanMode(ModifiedScan),
      m_hasHistoryToResolve(false)
{
}

ImageScanner::ImageScanner(const QFileInfo& info)
    : m_hasImage(false),
      m_hasMetadata(false),
      m_loadedFromDisk(false),
      m_fileInfo(info),
      m_scanMode(ModifiedScan),
      m_hasHistoryToResolve(false)
{
}

ImageScanner::ImageScanner(qlonglong imageid)
    : m_hasImage(false),
      m_hasMetadata(false),
      m_loadedFromDisk(false),
      m_scanMode(ModifiedScan),
      m_hasHistoryToResolve(false)
{
    ItemShortInfo shortInfo;
    {
        DatabaseAccess access;
        shortInfo  = access.db()->getItemShortInfo(imageid);
        m_scanInfo = access.db()->getItemScanInfo(imageid);
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(shortInfo.albumRootID);
    m_fileInfo            = QFileInfo(DatabaseUrl::fromAlbumAndName(shortInfo.itemName,
                                      shortInfo.album, albumRootPath, shortInfo.albumRootID).fileUrl().toLocalFile());
}

qlonglong ImageScanner::id() const
{
    return m_scanInfo.id;
}

void ImageScanner::setCategory(DatabaseItem::Category category)
{
    // we don't have the necessary information in this class, but in CollectionScanner
    m_scanInfo.category = category;
}

void ImageScanner::fileModified()
{
    loadFromDisk();
    updateImage();
    scanFile(ModifiedScan);
}

void ImageScanner::newFile(int albumId)
{
    loadFromDisk();
    addImage(albumId);

    if (!scanFromIdenticalFile())
    {
        scanFile(NewScan);
    }
}

void ImageScanner::newFileFullScan(int albumId)
{
    loadFromDisk();
    addImage(albumId);
    scanFile(NewScan);
}

void ImageScanner::rescan()
{
    loadFromDisk();
    updateImage();
    scanFile(Rescan);
}

void ImageScanner::copiedFrom(int albumId, qlonglong srcId)
{
    loadFromDisk();
    addImage(albumId);

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

const ItemScanInfo& ImageScanner::itemScanInfo() const
{
    return m_scanInfo;
}

bool ImageScanner::hasHistoryToResolve() const
{
    return m_hasHistoryToResolve;
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
    QList<ItemScanInfo> candidates = DatabaseAccess().db()->getIdenticalFiles(m_scanInfo.uniqueHash,
                                     m_scanInfo.fileSize, m_scanInfo.id);

    if (!candidates.isEmpty())
    {
        // Sort by priority, as implemented by custom lessThan()
        qStableSort(candidates.begin(), candidates.end(), lessThanForIdentity);

        kDebug() << "Recognized" << m_fileInfo.filePath() << "as identical to item" << candidates.first().id;

        // Copy attributes.
        // Todo for the future is to worry about syncing identical files.
        DatabaseAccess().db()->copyImageAttributes(candidates.first().id, m_scanInfo.id);
        return true;
    }

    return false;
}

bool ImageScanner::copyFromSource(qlonglong srcId)
{
    DatabaseAccess access;

    // some basic validity checking
    if (srcId == m_scanInfo.id)
    {
        return false;
    }

    ItemScanInfo info = access.db()->getItemScanInfo(srcId);

    if (!info.id)
    {
        return false;
    }

    kDebug() << "Recognized" << m_fileInfo.filePath() << "as copied from" << srcId;
    access.db()->copyImageAttributes(srcId, m_scanInfo.id);
    return true;
}

void ImageScanner::prepareImage()
{
    m_scanInfo.itemName         = m_fileInfo.fileName();
    m_scanInfo.modificationDate = m_fileInfo.lastModified();
    m_scanInfo.fileSize         = m_fileInfo.size();
    // category is set by setCategory

    // the QByteArray is an ASCII hex string
    m_scanInfo.uniqueHash       = uniqueHash();
}

void ImageScanner::addImage(int albumId)
{
    prepareImage();

    m_scanInfo.albumID          = albumId;
    m_scanInfo.status           = DatabaseItem::Visible;

    kDebug() << "Adding new item" << m_fileInfo.filePath();
    m_scanInfo.id               = DatabaseAccess().db()->addItem(m_scanInfo.albumID, m_scanInfo.itemName,
                                  m_scanInfo.status, m_scanInfo.category,
                                  m_scanInfo.modificationDate, m_scanInfo.fileSize,
                                  m_scanInfo.uniqueHash);
}

void ImageScanner::updateImage()
{
    prepareImage();

    DatabaseAccess().db()->updateItem(m_scanInfo.id, m_scanInfo.category,
                                      m_scanInfo.modificationDate, m_scanInfo.fileSize, m_scanInfo.uniqueHash);
}

void ImageScanner::scanFile(ScanMode mode)
{
    m_scanMode = mode;

    if (m_scanMode == ModifiedScan)
    {
        if (m_scanInfo.category == DatabaseItem::Image)
        {
            scanImageInformation();
            scanFaces();
            scanImageHistoryIfModified();
        }
        else if (m_scanInfo.category == DatabaseItem::Video)
        {
            scanVideoInformation();
            // TODO: Here, we only scan fields which can be expected to have changed, when we detect a change of file data.
            // It seems to me that at the moment video metadata contains such files (which may change after editing).
            // In contrast, with photos, ImageMetadata contains fields which describe the moment of taking the photo,
            //  which means they dont change.
            if (m_hasMetadata)
            {
                scanVideoMetadata();
            }
        }
    }
    else
    {
        if (m_scanInfo.category == DatabaseItem::Image)
        {
            scanImageInformation();

            if (m_hasMetadata)
            {
                scanImageMetadata();
                scanImagePosition();
                scanImageComments();
                scanImageCopyright();
                scanIPTCCore();
                scanTags();
                scanFaces();
                scanImageHistory();
            }
        }
        else if (m_scanInfo.category == DatabaseItem::Video)
        {
            scanVideoInformation();
            if (m_hasMetadata)
            {
                scanVideoMetadata();
            }
        }
        else if (m_scanInfo.category == DatabaseItem::Audio)
        {
            scanAudioFile();
        }
        else if (m_scanInfo.category == DatabaseItem::Other)
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
        dateFromMetadata = creationDateFromFilesystem(m_fileInfo);
    }
}

bool ImageScanner::checkRatingFromMetadata(const QVariant& ratingFromMetadata) const
{
    // should only be overwritten if set in metadata
    if (m_scanMode == Rescan)
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
    DatabaseFields::ImageInformation dbFields = DatabaseFields::ImageInformationAll;
    QVariantList                     infos;

    if (m_scanMode == NewScan || m_scanMode == Rescan)
    {
        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate
               << MetadataInfo::DigitizationDate
               << MetadataInfo::Orientation;
        QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

        checkCreationDateFromMetadata(metadataInfos[1]);

        if (!checkRatingFromMetadata(metadataInfos.at(0)))
        {
            dbFields &= ~DatabaseFields::Rating;
            metadataInfos.removeAt(0);
        }

        infos << metadataInfos;
    }

    QSize size = m_img.size();
    infos << size.width()
          << size.height()
          << detectFormat()
          << m_img.originalBitDepth()
          << m_img.originalColorModel();

    if (m_scanMode == NewScan)
    {
        DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos, dbFields);
    }
    else if (m_scanMode == Rescan)
    {
        DatabaseAccess().db()->changeImageInformation(m_scanInfo.id, infos, dbFields);
    }
    else // ModifiedScan
    {
        // Does _not_ update rating and orientation (unless dims were exchanged)!
        /*int orientation = m_metadata.getImageOrientation();
        QVariantList data = DatabaseAccess().db()->getImageInformation(m_scanInfo.id,
                                                                       DatabaseFields::Width |
                                                                       DatabaseFields::Height |
                                                                       DatabaseFields::Orientation);
        if (data.size() == 3 && data[2].isValid() && data[2].toInt() != orientation)
        {
            // be careful not to overwrite our value set in the database
            // But there is a special case: if the dims were
        }*/
        DatabaseAccess().db()->changeImageInformation(m_scanInfo.id, infos,
                                                      DatabaseFields::Width      |
                                                      DatabaseFields::Height     |
                                                      DatabaseFields::Format     |
                                                      DatabaseFields::ColorDepth |
                                                      DatabaseFields::ColorModel);
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
    QVariantList metadataInfos = m_metadata.getMetadataFields(allImageMetadataFields());

    if (hasValidField(metadataInfos))
    {
        DatabaseAccess().db()->addImageMetadata(m_scanInfo.id, metadataInfos);
    }
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

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (hasValidField(metadataInfos))
    {
        DatabaseAccess().db()->addImagePosition(m_scanInfo.id, metadataInfos);
    }
}

void ImageScanner::scanImageComments()
{
    MetadataFields fields;
    fields << MetadataInfo::Headline
           << MetadataInfo::Title;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    // handles all possible fields, multi-language, author, date
    CaptionsMap captions = m_metadata.getImageComments();

    if (captions.isEmpty() && !hasValidField(metadataInfos))
    {
        return;
    }

    DatabaseAccess access;
    ImageComments comments(access, m_scanInfo.id);

    // Description
    if (!captions.isEmpty())
    {
        comments.replaceComments(captions);
    }

    // Headline
    if (!metadataInfos.at(0).isNull())
    {
        comments.addHeadline(metadataInfos.at(0).toString());
    }

    // Title
    if (!metadataInfos.at(1).isNull())
    {
        comments.addTitle(metadataInfos.at(1).toMap()["x-default"].toString());
    }
}

void ImageScanner::scanImageCopyright()
{
    Template t;

    if (!m_metadata.getCopyrightInformation(t))
    {
        return;
    }

    ImageCopyright copyright(m_scanInfo.id);
    // It is not clear if removeAll() should be called if m_scanMode == Rescan
    copyright.removeAll();
    copyright.setFromTemplate(t);
}

void ImageScanner::scanIPTCCore()
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreLocationInfo
           << MetadataInfo::IptcCoreIntellectualGenre
           << MetadataInfo::IptcCoreJobID
           << MetadataInfo::IptcCoreScene
           << MetadataInfo::IptcCoreSubjectCode;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (!hasValidField(metadataInfos))
    {
        return;
    }

    ImageExtendedProperties props(m_scanInfo.id);

    if (!metadataInfos.at(0).isNull())
    {
        IptcCoreLocationInfo loc = metadataInfos.at(0).value<IptcCoreLocationInfo>();

        if (!loc.isNull())
        {
            props.setLocation(loc);
        }
    }

    if (!metadataInfos.at(1).isNull())
    {
        props.setIntellectualGenre(metadataInfos.at(1).toString());
    }

    if (!metadataInfos.at(2).isNull())
    {
        props.setJobId(metadataInfos.at(2).toString());
    }

    if (!metadataInfos.at(3).isNull())
    {
        props.setScene(metadataInfos.at(3).toStringList());
    }

    if (!metadataInfos.at(4).isNull())
    {
        props.setSubjectCode(metadataInfos.at(4).toStringList());
    }
}

void ImageScanner::scanTags()
{
    // Check Keywords tag paths.

    QVariant var         = m_metadata.getMetadataField(MetadataInfo::Keywords);
    QStringList keywords = var.toStringList();

    if (!keywords.isEmpty())
    {
        // get tag ids, create if necessary
        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(keywords);
        DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, tagIds);
    }

    // Check Pick Label tag.

    int pickId = m_metadata.getImagePickLabel();
    if (pickId != -1)
    {
        kDebug() << "Pick Label found : " << pickId;

        int tagId = TagsCache::instance()->tagForPickLabel((PickLabel)pickId);
        if (tagId)
        {
            DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, QList<int>() << tagId);
            kDebug() << "Assigned Pick Label Tag  : " << tagId;
        }
        else
        {
            kDebug() << "Cannot find Pick Label Tag for : " << pickId;
        }
    }

    // Check Color Label tag.

    int colorId = m_metadata.getImageColorLabel();

    if (colorId != -1)
    {
        kDebug() << "Color Label found : " << colorId;

        int tagId = TagsCache::instance()->tagForColorLabel((ColorLabel)colorId);
        if (tagId)
        {
            DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, QList<int>() << tagId);
            kDebug() << "Assigned Color Label Tag  : " << tagId;
        }
        else
        {
            kDebug() << "Cannot find Color Label Tag for : " << colorId;
        }
    }
}

void ImageScanner::scanFaces()
{
    QSize size = m_img.size();
    if (!size.isValid())
    {
        return;
    }

    QMap<QString,QVariant> metadataFacesMap;
    if (!m_metadata.getImageFacesMap(metadataFacesMap))
    {
        return;
    }

    QMap<QString,QVariant>::const_iterator it;
    for (it = metadataFacesMap.constBegin(); it != metadataFacesMap.constEnd(); ++it)
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
        editor.add(m_scanInfo.id, tagId, region, false);
    }
}

void ImageScanner::scanImageHistory()
{
    /** Stage 1 of history scanning */

    QString historyXml = m_metadata.getImageHistory();

    if (!historyXml.isEmpty())
    {
        DatabaseAccess().db()->setImageHistory(m_scanInfo.id, historyXml);
        // Delay history resolution by setting this tag:
        // Resolution depends on the presence of other images, possibly only when the scanning process has finished
        DatabaseAccess().db()->addItemTag(m_scanInfo.id, TagsCache::instance()->
                                          getOrCreateInternalTag(InternalTagName::needResolvingHistory()));
        m_hasHistoryToResolve = true;
    }

    QString uuid = m_metadata.getImageUniqueId();

    if (!uuid.isNull())
    {
        DatabaseAccess().db()->setImageUuid(m_scanInfo.id, uuid);
    }
}

void ImageScanner::scanImageHistoryIfModified()
{
    // If a file has a modified history, it must have a new UUID
    QString previousUuid = DatabaseAccess().db()->getImageUuid(m_scanInfo.id);
    QString currentUuid  = m_metadata.getImageUniqueId();

    if (previousUuid != currentUuid)
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

    if (id1.hasUniqueHashIdentifier()
        && id1.m_uniqueHash == id2.m_uniqueHash
        && id1.m_fileSize == id2.m_fileSize)
    {
        return true;
    }

    if (id1.hasFileName() && id1.hasCreationDate()
        && id1.m_fileName == id2.m_fileName
        && id1.m_creationDate == id2.m_creationDate)
    {
        return true;
    }

    if (id1.hasFileOnDisk()
        && id1.m_filePath == id2.m_filePath
        && id1.m_fileName == id2.m_fileName)
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
        /*if (!uuidList.isEmpty())
        {
            return uuidList;
        }*/
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
                QString album = CollectionManager::instance()->album(file.path());
                QString name  = file.fileName();
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

    lessThanByProximityToSubject(const ImageInfo& subject) 
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
        return a.id() < b.id();
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
           << MetadataInfo::Resolution
           << MetadataInfo::VideoCodec;

    return fields;
}


void ImageScanner::scanVideoInformation()
{
    DatabaseFields::ImageInformation dbFields;
    QVariantList                     infos;

    if (m_scanMode == NewScan || m_scanMode == Rescan)
    {
        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate
               << MetadataInfo::DigitizationDate;
        QVariantList metadataInfos = m_metadata.getMetadataFields(fields);
        dbFields |= DatabaseFields::Rating | DatabaseFields::CreationDate | DatabaseFields::DigitizationDate;

        checkCreationDateFromMetadata(metadataInfos[1]);

        if (!checkRatingFromMetadata(metadataInfos.at(0)))
        {
            dbFields &= ~DatabaseFields::Rating;
            metadataInfos.removeAt(0);
        }

        infos << metadataInfos;
    }

    // TODO: Have we got the video resolution as width x height?
    QSize size;// = ...;
    infos << size.width()
          << size.height();
    dbFields |= DatabaseFields::Width | DatabaseFields::Height;

    // TODO: Please check / improve / rewrite detectVideoFormat().
    // The format strings shall be uppercase, and a clearly defined set
    // (all format strings used in the database should be defined in advance)
    infos << detectVideoFormat();
    dbFields |= DatabaseFields::Format;


    // There is use of bit depth, but not ColorModel
    // For bit depth - 8bit, 16bit with videos
    infos << m_metadata.getMetadataField(MetadataInfo::VideoBitDepth);
    dbFields |= DatabaseFields::ColorDepth;


    if (m_scanMode == NewScan)
    {
        DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos, dbFields);
    }
    else if (m_scanMode == Rescan)
    {
        DatabaseAccess().db()->changeImageInformation(m_scanInfo.id, infos, dbFields);
    }
    else // ModifiedScan
    {
        // TODO: which flags are passed here depends on the three TODOs above
        // We dont overwrite Rating and date here.
        DatabaseAccess().db()->changeImageInformation(m_scanInfo.id, infos,
                                                      DatabaseFields::Width      |
                                                      DatabaseFields::Height     |
                                                      DatabaseFields::Format     |
                                                      DatabaseFields::ColorDepth);
    }
}

void ImageScanner::scanVideoMetadata()
{
    QVariantList metadataInfos = m_metadata.getMetadataFields(allVideoMetadataFields());

    if (hasValidField(metadataInfos))
    {
        DatabaseAccess().db()->addVideoMetadata(m_scanInfo.id, metadataInfos);
    }
}

// ---------------------------------------------------------------------------------------

void ImageScanner::scanAudioFile()
{
    /**
    * @todo
    */

    QVariantList infos;
    infos << -1
          << creationDateFromFilesystem(m_fileInfo)
          << detectAudioFormat();

    DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos,
                                               DatabaseFields::Rating       |
                                               DatabaseFields::CreationDate |
                                               DatabaseFields::Format);
}

void ImageScanner::loadFromDisk()
{
    if (m_loadedFromDisk)
    {
        return;
    }
    m_loadedFromDisk = true;

    m_metadata.registerMetadataSettings();
    m_hasMetadata    = m_metadata.load(m_fileInfo.filePath());

    if (m_scanInfo.category == DatabaseItem::Image)
    {
        m_hasImage = m_img.loadImageInfo(m_fileInfo.filePath(), false, false, false, false);
    }
    else
    {
        m_hasImage = false;
    }

    // faster than loading twice from disk
    if (m_hasMetadata)
    {
        m_img.setMetadata(m_metadata.data());
    }
}

QString ImageScanner::uniqueHash()
{
    // the QByteArray is an ASCII hex string
    if (m_scanInfo.category == DatabaseItem::Image)
    {
        if (DatabaseAccess().db()->isUniqueHashV2())
            return QString(m_img.getUniqueHashV2());
        else
            return QString(m_img.getUniqueHash());
    }
    else
    {
        if (DatabaseAccess().db()->isUniqueHashV2())
            return QString(DImg::getUniqueHashV2(m_fileInfo.filePath()));
        else
            return QString(DImg::getUniqueHash(m_fileInfo.filePath()));
    }
}

QString ImageScanner::detectFormat()
{
    DImg::FORMAT dimgFormat = m_img.detectedFormat();

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
            format += m_fileInfo.suffix().toUpper();
            return format;
        }
        case DImg::NONE:
        case DImg::QIMAGE:
        {
            QByteArray format = QImageReader::imageFormat(m_fileInfo.filePath());

            if (!format.isEmpty())
            {
                return QString(format).toUpper();
            }

            KMimeType::Ptr mimetype = KMimeType::findByPath(m_fileInfo.filePath());

            if (mimetype)
            {
                QString name = mimetype->name();

                if (name.startsWith(QLatin1String("image/")))
                {
                    QString imageTypeName = name.mid(6).toUpper();

                    // cut off the "X-" from some mimetypes
                    if (imageTypeName.startsWith(QLatin1String("X-")))
                    {
                        imageTypeName = imageTypeName.mid(2);
                    }

                    return imageTypeName;
                }
            }

            kWarning() << "Detecting file format failed: KMimeType for" << m_fileInfo.filePath()
                       << "is null";

            break;
        }
    }

    return QString();
}

QString ImageScanner::detectVideoFormat()
{
    QString suffix = m_fileInfo.suffix().toUpper();

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

    if(suffix == "M4V" || suffix == "MOV" || suffix == "M2V" )
    {
        return "MOV";
    }

    if(suffix == "3GP" || suffix == "3G2" )
    {
        return "3GP";
    }

    return suffix;
}

QString ImageScanner::detectAudioFormat()
{
    QString suffix = m_fileInfo.suffix().toUpper();
    return suffix;
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
    QVariantList fields = DatabaseAccess().db()->getImageMetadata(imageid);

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
    QVariantList fields = DatabaseAccess().db()->getVideoMetadata(imageid);

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
    container->resolution                   = strings.at(6);
    container->videoCodec                   = strings.at(7);
}

} // namespace Digikam
