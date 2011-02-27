/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling accesss to one image and associated data
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageinfo.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QHash>

// KDE includes

#include <kdebug.h>

// Local includes

#include "globals.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "databaseinfocontainers.h"
#include "dimagehistory.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "imageinfodata.h"
#include "imageinfocache.h"
#include "imagelister.h"
#include "imagelisterrecord.h"
#include "imageinfolist.h"
#include "imagecomments.h"
#include "imagecopyright.h"
#include "imageextendedproperties.h"
#include "imageposition.h"
#include "imagescanner.h"
#include "imagetagpair.h"
#include "tagscache.h"
#include "template.h"
#include "photoinfocontainer.h"

namespace Digikam
{

ImageInfoData::ImageInfoData()
{
    id                     = -1;
    albumId                = -1;
    albumRootId            = -1;

    pickLabel              = NoPickLabel;
    colorLabel             = NoColorLabel;
    rating                 = -1;
    category               = DatabaseItem::UndefinedCategory;
    fileSize               = 0;

    longitude              = 0;
    latitude               = 0;
    altitude               = 0;

    hasCoordinates         = false;
    hasAltitude            = false;

    groupedImages          = 0;
    groupImage             = -1;

    defaultCommentCached   = false;
    pickLabelCached        = false;
    colorLabelCached       = false;
    ratingCached           = false;
    categoryCached         = false;
    formatCached           = false;
    creationDateCached     = false;
    modificationDateCached = false;
    fileSizeCached         = false;
    imageSizeCached        = false;
    tagIdsCached           = false;
    positionsCached        = false;
    groupedImagesIsCached  = false;
    groupImageIsCached     = false;

    invalid                = false;
}

ImageInfo::ImageInfo()
    : m_data(0)
{
}

ImageInfo::ImageInfo(const ImageListerRecord& record)
{
    DatabaseAccess access;
    m_data                         = access.imageInfoCache()->infoForId(record.imageID);

    m_data->albumId                = record.albumID;
    m_data->albumRootId            = record.albumRootID;
    m_data->name                   = record.name;

    m_data->rating                 = record.rating;
    m_data->category               = record.category;
    m_data->format                 = record.format;
    m_data->creationDate           = record.creationDate;
    m_data->modificationDate       = record.modificationDate;
    m_data->fileSize               = record.fileSize;
    m_data->imageSize              = record.imageSize;

    m_data->ratingCached           = true;
    m_data->categoryCached         = true;
    m_data->formatCached           = true;
    m_data->creationDateCached     = true;
    m_data->modificationDateCached = true;
    m_data->fileSizeCached         = true;
    m_data->imageSizeCached        = true;
}

ImageInfo::ImageInfo(qlonglong ID)
{
    DatabaseAccess access;
    m_data = access.imageInfoCache()->infoForId(ID);

    // is this a newly created structure, need to populate?
    if (m_data->albumId == -1)
    {
        // retrieve immutable values now, the rest on demand
        ItemShortInfo info  = access.db()->getItemShortInfo(ID);

        if (info.id)
        {
            m_data->albumId     = info.albumID;
            m_data->albumRootId = info.albumRootID;
            m_data->name        = info.itemName;
        }
        else
        {
            // invalid image id
            ImageInfoData* olddata = m_data.unassign();

            if (olddata)
            {
                access.imageInfoCache()->dropInfo(olddata);
            }

            m_data = 0;
        }
    }
}

ImageInfo::ImageInfo(const KUrl& url)
{
    DatabaseAccess access;

    CollectionLocation location = CollectionManager::instance()->locationForUrl(url);

    if (location.isNull())
    {
        m_data = 0;
        qWarning() << "No location could be retrieved for url" << url;
        return;
    }

    KUrl _url(url.directory());
    QString album = CollectionManager::instance()->album(_url.toLocalFile());
    QString name  = url.fileName();

    /*
    // if needed, the two SQL calls can be consolidated into one by adding a method to AlbumDB
    int albumId = access.db()->getAlbumForPath(location.id(), album, false);
    if (albumId == -1)
    {
        m_data = 0;
        return;
    }

    int imageId = access.db()->getImageId(albumId, name);
    if (imageId == -1)
    {
        m_data = 0;
        return;
    }
    */
    ItemShortInfo info = access.db()->getItemShortInfo(location.id(), album, name);

    if (!info.id)
    {
        m_data = 0;
        qWarning() << "No itemShortInfo could be retrieved from the database for image" << name;
        return;
    }

    m_data = access.imageInfoCache()->infoForId(info.id);
    m_data->albumId     = info.albumID;
    m_data->albumRootId = info.albumRootID;
    m_data->name        = info.itemName;
}

ImageInfo::~ImageInfo()
{
    ImageInfoData* olddata = m_data.unassign();

    if (olddata)
    {
        DatabaseAccess().imageInfoCache()->dropInfo(olddata);
    }
}

ImageInfo::ImageInfo(const ImageInfo& info)
{
    m_data = info.m_data;
}

ImageInfo& ImageInfo::operator=(const ImageInfo& info)
{
    if (m_data == info.m_data)
    {
        return *this;
    }

    ImageInfoData* olddata = m_data.assign(info.m_data);

    if (olddata)
    {
        DatabaseAccess().imageInfoCache()->dropInfo(olddata);
    }

    return *this;
}

bool ImageInfo::isNull() const
{
    return !m_data;
}

bool ImageInfo::operator==(const ImageInfo& info) const
{
    if (m_data && info.m_data)
    {
        // not null, compare id
        return m_data->id == info.m_data->id;
    }
    else
    {
        // both null?
        return m_data == info.m_data;
    }
}

bool ImageInfo::operator<(const ImageInfo& info) const
{
    if (m_data)
    {
        if (info.m_data)
            // both not null, sort by id
        {
            return m_data->id < info.m_data->id;
        }
        else
            // only other is null, this is greater than
        {
            return false;
        }
    }
    else
    {
        // this is less than if the other is not null
        return info.m_data;
    }
}

uint ImageInfo::hash() const
{
    if (m_data)
    {
        return ::qHash(m_data->id);
    }
    else
    {
        return ::qHash((int)0);
    }
}

/**
 * Access rules for all methods in this class:
 * ImageInfoData members shall be accessed only under DatabaseAccess lock.
 * The id and albumId are the exception to this rule, as they are
 * primitive and will never change during the lifetime of an object.
 */
qlonglong ImageInfo::id() const
{
    return m_data ? m_data->id : -1;
}

int ImageInfo::albumId() const
{
    return m_data ? m_data->albumId : -1;
}

int ImageInfo::albumRootId() const
{
    return m_data ? m_data->albumRootId : -1;
}

QString ImageInfo::name() const
{
    if (!m_data)
    {
        return QString();
    }

    DatabaseAccess access;
    return m_data->name;
}

uint ImageInfo::fileSize() const
{
    if (!m_data)
    {
        return 0;
    }

    DatabaseAccess access;

    if (!m_data->fileSizeCached)
    {
        QVariantList values = access.db()->getImagesFields(m_data->id, DatabaseFields::FileSize);

        if (!values.isEmpty())
        {
            m_data.constCastData()->fileSize = values.first().toUInt();
        }

        m_data.constCastData()->fileSizeCached = true;
    }

    return m_data->fileSize;
}

QString ImageInfo::comment() const
{
    if (!m_data)
    {
        return QString();
    }

    DatabaseAccess access;

    if (!m_data->defaultCommentCached)
    {
        ImageComments comments(access, m_data->id);
        m_data.constCastData()->defaultComment       = comments.defaultComment();
        m_data.constCastData()->defaultCommentCached = true;
    }

    return m_data->defaultComment;
}

int ImageInfo::pickLabel() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->pickLabelCached)
    {
        QList<int> tags = tagIds();

        foreach(int tagId, tags)
        {
            for (int i = NoPickLabel ; i <= AcceptedLabel; ++i)
            {
                if (tagId == TagsCache::instance()->getTagForPickLabel((PickLabel)i))
                {
                    m_data.constCastData()->pickLabel = i;
                    break;
                }
            }
        }

        m_data.constCastData()->pickLabelCached = true;
    }

    return m_data->pickLabel;
}

int ImageInfo::colorLabel() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->colorLabelCached)
    {
        QList<int> tags = tagIds();

        foreach(int tagId, tags)
        {
            for (int i = NoColorLabel ; i <= WhiteLabel; ++i)
            {
                if (tagId == TagsCache::instance()->getTagForColorLabel((ColorLabel)i))
                {
                    m_data.constCastData()->colorLabel = i;
                    break;
                }
            }
        }

        m_data.constCastData()->colorLabelCached = true;
    }

    return m_data->colorLabel;
}

int ImageInfo::rating() const
{
    if (!m_data)
    {
        return 0;
    }

    DatabaseAccess access;

    if (!m_data->ratingCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::Rating);

        if (!values.isEmpty())
        {
            m_data.constCastData()->rating = values.first().toInt();
        }

        m_data.constCastData()->ratingCached = true;
    }

    return m_data->rating;
}

QString ImageInfo::format() const
{
    if (!m_data)
    {
        return 0;
    }

    DatabaseAccess access;

    if (!m_data->formatCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::Format);

        if (!values.isEmpty())
        {
            m_data.constCastData()->format = values.first().toString();
        }

        m_data.constCastData()->formatCached = true;
    }

    return m_data->format;
}

DatabaseItem::Category ImageInfo::category() const
{
    if (!m_data)
    {
        return DatabaseItem::UndefinedCategory;
    }

    DatabaseAccess access;

    if (!m_data->categoryCached)
    {
        QVariantList values = access.db()->getImagesFields(m_data->id, DatabaseFields::Category);

        if (!values.isEmpty())
        {
            m_data.constCastData()->category = (DatabaseItem::Category)values.first().toInt();
        }

        m_data.constCastData()->categoryCached = true;
    }

    return m_data->category;
}

QDateTime ImageInfo::dateTime() const
{
    if (!m_data)
    {
        return QDateTime();
    }

    DatabaseAccess access;

    if (!m_data->creationDateCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::CreationDate);

        if (!values.isEmpty())
        {
            m_data.constCastData()->creationDate = values.first().toDateTime();
        }

        m_data.constCastData()->creationDateCached = true;
    }

    return m_data->creationDate;
}

QDateTime ImageInfo::modDateTime() const
{
    if (!m_data)
    {
        return QDateTime();
    }

    DatabaseAccess access;

    if (!m_data->modificationDateCached)
    {
        QVariantList values = access.db()->getImagesFields(m_data->id, DatabaseFields::ModificationDate);

        if (!values.isEmpty())
        {
            m_data.constCastData()->modificationDate = values.first().toDateTime();
        }

        m_data.constCastData()->modificationDateCached = true;
    }

    return m_data->modificationDate;
}

QSize ImageInfo::dimensions() const
{
    if (!m_data)
    {
        return QSize();
    }

    DatabaseAccess access;

    if (!m_data->imageSizeCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::Width | DatabaseFields::Height);

        if (values.size() == 2)
        {
            m_data.constCastData()->imageSize = QSize(values[0].toInt(), values[1].toInt());
        }

        m_data.constCastData()->imageSizeCached = true;
    }

    return m_data->imageSize;
}

QList<int> ImageInfo::tagIds() const
{
    if (!m_data)
    {
        return QList<int>();
    }

    DatabaseAccess access;

    if (!m_data->tagIdsCached)
    {
        m_data.constCastData()->tagIds       = access.db()->getItemTagIDs(m_data->id);
        m_data.constCastData()->tagIdsCached = true;
    }

    return m_data->tagIds;
}

DatabaseUrl ImageInfo::databaseUrl() const
{
    if (!m_data)
    {
        return DatabaseUrl();
    }

    DatabaseAccess access;

    QString album     = access.imageInfoCache()->albumName(access, m_data->albumId);
    QString albumRoot = CollectionManager::instance()->albumRootPath(m_data->albumRootId);

    return DatabaseUrl::fromAlbumAndName(m_data->name, album, albumRoot, m_data->albumRootId);
}

KUrl ImageInfo::fileUrl() const
{
    return KUrl::fromPath(filePath());
}

QString ImageInfo::filePath() const
{
    if (!m_data)
    {
        return QString();
    }

    DatabaseAccess access;

    QString albumRoot = CollectionManager::instance()->albumRootPath(m_data->albumRootId);

    if (albumRoot.isNull())
    {
        return QString();
    }

    QString album = access.imageInfoCache()->albumName(access, m_data->albumId);

    if (album == "/")
    {
        return albumRoot + album + m_data->name;
    }
    else
    {
        return albumRoot + album + '/' + m_data->name;
    }
}

bool ImageInfo::isVisible() const
{
    if (!m_data)
    {
        return false;
    }

    QVariantList value = DatabaseAccess().db()->getImagesFields(m_data->id, DatabaseFields::Status);

    if (!value.isEmpty())
    {
        return value.first().toInt() == DatabaseItem::Visible;
    }

    return false;
}

void ImageInfo::setVisible(bool isVisible)
{
    if (!m_data)
    {
        return;
    }

    if (m_data->albumId == 0)
    {
        kWarning() << "Attempt to make a Removed item visible with ImageInfo::setVisible";
        return;
    }

    DatabaseAccess().db()->setItemStatus(m_data->id, isVisible ? DatabaseItem::Visible : DatabaseItem::Hidden);
}

bool ImageInfo::hasDerivedImages() const
{
    if (!m_data)
    {
        return false;
    }

    return DatabaseAccess().db()->hasImagesRelatingTo(m_data->id, DatabaseRelation::DerivedFrom);
}

bool ImageInfo::hasAncestorImages() const
{
    if (!m_data)
    {
        return false;
    }

    return DatabaseAccess().db()->hasImagesRelatedFrom(m_data->id, DatabaseRelation::DerivedFrom);
}

QList<ImageInfo> ImageInfo::derivedImages() const
{
    if (!m_data)
    {
        return QList<ImageInfo>();
    }

    return ImageInfoList(DatabaseAccess().db()->getImagesRelatingTo(m_data->id, DatabaseRelation::DerivedFrom));
}

QList<ImageInfo> ImageInfo::ancestorImages() const
{
    if (!m_data)
    {
        return QList<ImageInfo>();
    }

    return ImageInfoList(DatabaseAccess().db()->getImagesRelatedFrom(m_data->id, DatabaseRelation::DerivedFrom));
}

QList<QPair<qlonglong, qlonglong> > ImageInfo::relationCloud() const
{
    if (!m_data)
    {
        return QList<QPair<qlonglong, qlonglong> >();
    }

    return DatabaseAccess().db()->getRelationCloud(m_data->id, DatabaseRelation::DerivedFrom);
}

void ImageInfo::markDerivedFrom(const ImageInfo& ancestor)
{
    if (!m_data || ancestor.isNull())
    {
        return;
    }

    DatabaseAccess().db()->addImageRelation(m_data->id, ancestor.id(), DatabaseRelation::DerivedFrom);
}

bool ImageInfo::hasGroupedImages() const
{
    return numberOfGroupedImages();
}

int ImageInfo::numberOfGroupedImages() const
{
    if (!m_data)
    {
        return false;
    }

    if (!m_data->groupedImagesIsCached)
    {
        m_data.constCastData()->groupedImages
          = DatabaseAccess().db()->getImagesRelatingTo(m_data->id, DatabaseRelation::Grouped).size();
        m_data.constCastData()->groupedImagesIsCached = true;
    }

    return m_data->groupedImages;
}

bool ImageInfo::isGrouped() const
{
    if (!m_data)
    {
        return false;
    }

    if (!m_data->groupImageIsCached)
    {
        QList<qlonglong> ids = DatabaseAccess().db()->getImagesRelatedFrom(m_data->id, DatabaseRelation::Grouped);
        // list size should be 0 or 1
        m_data.constCastData()->groupImage = ids.isEmpty() ? -1 : ids.first();
        m_data.constCastData()->groupImageIsCached = true;
    }

    return m_data->groupImage != -1;
}

ImageInfo ImageInfo::groupImage() const
{
    // isGrouped() will cache the value
    if (!m_data || !isGrouped())
    {
        return ImageInfo();
    }
    return ImageInfo(m_data->groupImage);
}

QList<ImageInfo> ImageInfo::groupedImages() const
{
    if (!m_data || (m_data->groupedImagesIsCached && !m_data->groupedImages) || !hasGroupedImages())
    {
        return QList<ImageInfo>();
    }

    return ImageInfoList(DatabaseAccess().db()->getImagesRelatingTo(m_data->id, DatabaseRelation::Grouped));
}

void ImageInfo::addToGroup(const ImageInfo& leader)
{
    if (!m_data || leader.isNull())
    {
        return;
    }

    QList<ImageInfo> ownGroup = groupedImages();
    foreach (const ImageInfo& info, ownGroup)
    {
        ImageInfo(info).addToGroup(leader);
    }

    DatabaseAccess().db()->removeAllImageRelationsFrom(m_data->id, DatabaseRelation::Grouped);
    DatabaseAccess().db()->addImageRelation(m_data->id, leader.id(), DatabaseRelation::Grouped);
}

void ImageInfo::removeFromGroup()
{
    if (!m_data)
    {
        return;
    }

    if (!isGrouped())
    {
        return;
    }

    DatabaseAccess().db()->removeAllImageRelationsFrom(m_data->id, DatabaseRelation::Grouped);
}

void ImageInfo::clearGroup()
{
    if (!m_data)
    {
        return;
    }

    if (!hasGroupedImages())
    {
        return;
    }

    DatabaseAccess().db()->removeAllImageRelationsTo(m_data->id, DatabaseRelation::Grouped);
}

ImageComments ImageInfo::imageComments(DatabaseAccess& access) const
{
    if (!m_data)
    {
        return ImageComments();
    }

    return ImageComments(access, m_data->id);
}

ImageCopyright ImageInfo::imageCopyright() const
{
    if (!m_data)
    {
        return ImageCopyright();
    }

    return ImageCopyright(m_data->id);
}

ImageExtendedProperties ImageInfo::imageExtendedProperties() const
{
    if (!m_data)
    {
        return ImageExtendedProperties();
    }

    return ImageExtendedProperties(m_data->id);
}

ImagePosition ImageInfo::imagePosition() const
{
    if (!m_data)
    {
        return ImagePosition();
    }

    DatabaseAccess access;
    ImagePosition pos(access, m_data->id);

    if (!m_data->positionsCached)
    {
        m_data.constCastData()->longitude      = pos.longitudeNumber();
        m_data.constCastData()->latitude       = pos.latitudeNumber();
        m_data.constCastData()->altitude       = pos.altitude();
        m_data.constCastData()->hasCoordinates = pos.hasCoordinates();
        m_data.constCastData()->hasAltitude    = pos.hasAltitude();

        m_data.constCastData()->positionsCached = true;
    }

    return pos;
}

double ImageInfo::longitudeNumber() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->longitude;
}

double ImageInfo::latitudeNumber() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->latitude;
}

double ImageInfo::altitudeNumber() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->altitude;
}

bool ImageInfo::hasCoordinates() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->hasCoordinates;
}

bool ImageInfo::hasAltitude() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->hasAltitude;
}

ImageTagPair ImageInfo::imageTagPair(int tagId) const
{
    if (!m_data)
    {
        return ImageTagPair();
    }

    return ImageTagPair(*this, tagId);
}

QList<ImageTagPair> ImageInfo::availableImageTagPairs() const
{
    if (!m_data)
    {
        return QList<ImageTagPair>();
    }

    return ImageTagPair::availablePairs(*this);
}

DImageHistory ImageInfo::imageHistory() const
{
    if (!m_data)
    {
        return DImageHistory();
    }

    ImageHistoryEntry entry = DatabaseAccess().db()->getImageHistory(m_data->id);
    return DImageHistory::fromXml(entry.history);
}

void ImageInfo::setImageHistory(const DImageHistory& history)
{
    if (!m_data)
    {
        return;
    }

    DatabaseAccess().db()->setImageHistory(m_data->id, history.toXml());
}

bool ImageInfo::hasImageHistory() const
{
    if (!m_data)
    {
        return false;
    }

    return DatabaseAccess().db()->hasImageHistory(m_data->id);
}

QString ImageInfo::uuid() const
{
    if (!m_data)
    {
        return QString();
    }

    return DatabaseAccess().db()->getImageUuid(m_data->id);
}

void ImageInfo::setUuid(const QString& uuid)
{
    if (!m_data)
    {
        return;
    }

    DatabaseAccess().db()->setImageUuid(m_data->id, uuid);
}

HistoryImageId ImageInfo::historyImageId() const
{
    if (!m_data)
    {
        return HistoryImageId();
    }

    HistoryImageId id(uuid());
    id.setCreationDate(dateTime());
    id.setFileName(name());
    id.setPathOnDisk(filePath());

    if (DatabaseAccess().db()->isUniqueHashV2())
    {
        ItemScanInfo info = DatabaseAccess().db()->getItemScanInfo(m_data->id);
        id.setUniqueHash(info.uniqueHash, info.fileSize);
    }

    return id;
}

ImageCommonContainer ImageInfo::imageCommonContainer() const
{
    if (!m_data)
    {
        return ImageCommonContainer();
    }

    ImageCommonContainer container;
    ImageScanner::fillCommonContainer(m_data->id, &container);
    return container;
}

ImageMetadataContainer ImageInfo::imageMetadataContainer() const
{
    if (!m_data)
    {
        return ImageMetadataContainer();
    }

    ImageMetadataContainer container;
    ImageScanner::fillMetadataContainer(m_data->id, &container);
    return container;
}

PhotoInfoContainer ImageInfo::photoInfoContainer() const
{
    if (!m_data)
    {
        return PhotoInfoContainer();
    }

    ImageMetadataContainer meta = imageMetadataContainer();
    PhotoInfoContainer photoInfo;

    photoInfo.make            = meta.make;
    photoInfo.model           = meta.model;
    photoInfo.lens            = meta.lens;
    photoInfo.exposureTime    = meta.exposureTime;
    photoInfo.exposureMode    = meta.exposureMode;
    photoInfo.exposureProgram = meta.exposureProgram;
    photoInfo.aperture        = meta.aperture;
    photoInfo.focalLength     = meta.focalLength;
    photoInfo.focalLength35mm = meta.focalLength35;
    photoInfo.sensitivity     = meta.sensitivity;
    photoInfo.flash           = meta.flashMode;
    photoInfo.whiteBalance    = meta.whiteBalance;
    photoInfo.dateTime        = dateTime();

    return photoInfo;
}

Template ImageInfo::metadataTemplate() const
{
    if (!m_data)
    {
        return Template();
    }

    Template t;
    imageCopyright().fillTemplate(t);

    ImageExtendedProperties ep = imageExtendedProperties();
    t.setLocationInfo(ep.location());
    t.setIptcSubjects(ep.subjectCode());
    return t;
}

void ImageInfo::setMetadataTemplate(const Template& t)
{
    if (!m_data)
    {
        return;
    }

    removeMetadataTemplate();

    imageCopyright().setFromTemplate(t);

    ImageExtendedProperties ep = imageExtendedProperties();
    ep.setLocation(t.locationInfo());
    ep.setSubjectCode(t.IptcSubjects());
}

void ImageInfo::removeMetadataTemplate()
{
    if (!m_data)
    {
        return;
    }

    imageCopyright().removeAll();

    ImageExtendedProperties ep = imageExtendedProperties();
    ep.removeLocation();
    ep.removeSubjectCode();
}

void ImageInfo::setPickLabel(int pickId)
{
    if (!m_data)
    {
        return;
    }

    TagsCache* tc = TagsCache::instance();
    int tagId     = tc->getTagForPickLabel((PickLabel)pickId);
    if (!tagId) return;

    // Color Label is an exclusive tags.

    for (int i = NoPickLabel ; i <= AcceptedLabel ; ++i)
        removeTag(tc->getTagForPickLabel((PickLabel)i));

    setTag(tagId);

    m_data->pickLabel                       = pickId;
    m_data.constCastData()->pickLabelCached = true;
}

void ImageInfo::setColorLabel(int colorId)
{
    if (!m_data)
    {
        return;
    }

    TagsCache* tc = TagsCache::instance();
    int tagId     = tc->getTagForColorLabel((ColorLabel)colorId);
    if (!tagId) return;

    // Color Label is an exclusive tags.

    for (int i = NoColorLabel ; i <= WhiteLabel ; ++i)
        removeTag(tc->getTagForColorLabel((ColorLabel)i));

    setTag(tagId);

    m_data->colorLabel                       = colorId;
    m_data.constCastData()->colorLabelCached = true;
}

void ImageInfo::setRating(int value)
{
    if (!m_data)
    {
        return;
    }

    DatabaseAccess access;
    access.db()->changeImageInformation(m_data->id, QVariantList() << value, DatabaseFields::Rating);

    m_data->rating = value;
    m_data.constCastData()->ratingCached = true;
}

void ImageInfo::setDateTime(const QDateTime& dateTime)
{
    if (!m_data)
    {
        return;
    }

    if (dateTime.isValid())
    {
        DatabaseAccess access;
        access.db()->changeImageInformation(m_data->id, QVariantList() << dateTime, DatabaseFields::CreationDate);

        m_data->creationDate = dateTime;
        m_data.constCastData()->creationDateCached = true;
    }
}

void ImageInfo::setTag(int tagID)
{
    if (!m_data)
    {
        return;
    }

    kDebug() << m_data->id << tagID;
    DatabaseAccess access;
    access.db()->addItemTag(m_data->id, tagID);
}

void ImageInfo::removeTag(int tagID)
{
    if (!m_data)
    {
        return;
    }

    DatabaseAccess access;
    access.db()->removeItemTag(m_data->id, tagID);
    access.db()->removeImageTagProperties(m_data->id, tagID);
}

void ImageInfo::removeAllTags()
{
    if (!m_data)
    {
        return;
    }

    DatabaseAccess access;
    access.db()->removeItemAllTags(m_data->id, tagIds());
}

void ImageInfo::addTagPaths(const QStringList& tagPaths)
{
    if (!m_data)
    {
        return;
    }

    QList<int> tagIds = TagsCache::instance()->tagsForPaths(tagPaths);
    DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_data->id, tagIds);
}

ImageInfo ImageInfo::copyItem(int dstAlbumID, const QString& dstFileName)
{
    if (!m_data)
    {
        return ImageInfo();
    }

    DatabaseAccess access;
    //kDebug() << "ImageInfo::copyItem " << m_data->albumId << " " << m_data->name << " to " << dstAlbumID << " " << dstFileName;

    if (dstAlbumID == m_data->albumId && dstFileName == m_data->name)
    {
        return (*this);
    }

    int id = access.db()->copyItem(m_data->albumId, m_data->name, dstAlbumID, dstFileName);

    if (id == -1)
    {
        return ImageInfo();
    }

    return ImageInfo(id);
}

QDebug& operator<<(QDebug& stream, const ImageInfo& info)
{
    return stream << "ImageInfo [id = " << info.id() << ", databaseurl = "
           << info.databaseUrl() << "]";
}

}  // namespace Digikam
