/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning of a single image
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

// KDE includes

#include <kmimetype.h>

// Local includes

#include "ddebug.h"
#include "databaseurl.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "imagecomments.h"
#include "imagescanner.h"

namespace Digikam
{

ImageScanner::ImageScanner(const QFileInfo &info, const ItemScanInfo &scanInfo)
    : m_fileInfo(info), m_scanInfo(scanInfo)
{
}

ImageScanner::ImageScanner(const QFileInfo &info)
    : m_fileInfo(info)
{
}

ImageScanner::ImageScanner(qlonglong imageid)
{
    ItemShortInfo shortInfo;
    {
        DatabaseAccess access;
        shortInfo  = access.db()->getItemShortInfo(imageid);
        m_scanInfo = access.db()->getItemScanInfo(imageid);
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(shortInfo.albumRootId);
    m_fileInfo = QFileInfo(DatabaseUrl::fromAlbumAndName(shortInfo.itemName, shortInfo.album, albumRootPath).fileUrl().path());
}

void ImageScanner::setCategory(DatabaseItem::Category category)
{
    // we dont have the necessary information in this class, but in CollectionScanner
    m_scanInfo.category = category;
}

void ImageScanner::fileModified()
{
    loadFromDisk();
    updateHardInfos();
}

void ImageScanner::newFile(int albumId)
{
    loadFromDisk();
    addImage(albumId);
    scanFile();
}

void ImageScanner::fullScan()
{
    loadFromDisk();
    updateImage();
    scanFile();
}

void ImageScanner::addImage(int albumId)
{
    // there is a limit here for file size <2TB
    m_scanInfo.albumID = albumId;
    m_scanInfo.itemName = m_fileInfo.fileName();
    m_scanInfo.status = DatabaseItem::Visible;
    // category is set by setCategory
    m_scanInfo.modificationDate = m_fileInfo.lastModified();
    int fileSize = (int)m_fileInfo.size();
    // the QByteArray is an ASCII hex string
    m_scanInfo.uniqueHash = QString(m_img.getUniqueHash());

    m_scanInfo.id = DatabaseAccess().db()->addItem(m_scanInfo.albumID, m_scanInfo.itemName, m_scanInfo.status, m_scanInfo.category,
                                                   m_scanInfo.modificationDate, fileSize, m_scanInfo.uniqueHash);
}

void ImageScanner::updateImage()
{
    // part from addImage()
    m_scanInfo.modificationDate = m_fileInfo.lastModified();
    int fileSize = (int)m_fileInfo.size();
    m_scanInfo.uniqueHash = QString(m_img.getUniqueHash());

    DatabaseAccess().db()->updateItem(m_scanInfo.id, m_scanInfo.category,
                                      m_scanInfo.modificationDate, fileSize, m_scanInfo.uniqueHash);
}

void ImageScanner::scanFile()
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
        }
    }
}

void ImageScanner::updateHardInfos()
{
    updateImage();
    updateImageInformation();
}

void ImageScanner::scanImageInformation()
{
    MetadataFields fields;
    fields << MetadataInfo::Rating
           << MetadataInfo::CreationDate
           << MetadataInfo::DigitizationDate
           << MetadataInfo::Orientation;
    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);
    QSize size = m_img.size();

    // TODO: creation date need not be null

    QVariantList infos;
    infos << metadataInfos
          << size.width()
          << size.height()
          << detectFormat()
          << m_img.originalBitDepth()
          << m_img.originalColorModel();

    DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos);
}

static bool hasValidField(const QVariantList &list)
{
    for (QVariantList::const_iterator it = list.begin();
         it != list.end(); ++it)
    {
        if (!(*it).isNull())
            return true;
    }
    return false;
}

void ImageScanner::updateImageInformation()
{
    QSize size = m_img.size();

    QVariantList infos;
    infos << size.width()
          << size.height()
          << detectFormat()
          << m_img.originalBitDepth()
          << m_img.originalColorModel();

    DatabaseAccess access;
    access.db()->changeImageInformation(m_scanInfo.id, infos,
                                                    DatabaseFields::Width
                                                    | DatabaseFields::Height
                                                    | DatabaseFields::Format
                                                    | DatabaseFields::ColorDepth
                                                    | DatabaseFields::ColorModel);
}


void ImageScanner::scanImageMetadata()
{
    MetadataFields fields;
    fields << MetadataInfo::Make
           << MetadataInfo::Model
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

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (hasValidField(metadataInfos))
        DatabaseAccess().db()->addImageMetadata(m_scanInfo.id, metadataInfos);
}

void ImageScanner::scanImagePosition()
{
    MetadataFields fields;
    fields << MetadataInfo::Latitude
           << MetadataInfo::LatitudeNumber
           << MetadataInfo::Longitude
           << MetadataInfo::LongitudeNumber
           << MetadataInfo::Altitude
           << MetadataInfo::PositionOrientation
           << MetadataInfo::PositionTilt
           << MetadataInfo::PositionRoll
           << MetadataInfo::PositionDescription;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (hasValidField(metadataInfos))
        DatabaseAccess().db()->addImagePosition(m_scanInfo.id, metadataInfos);
}

void ImageScanner::scanImageComments()
{
    MetadataFields fields;
    fields << MetadataInfo::Comment
           << MetadataInfo::Description
           << MetadataInfo::Headline
           << MetadataInfo::Title
           << MetadataInfo::DescriptionWriter;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    bool noComment = (metadataInfos[0].isNull() && metadataInfos[1].isNull()
                      && metadataInfos[2].isNull() && metadataInfos[3].isNull());
    if (noComment)
        return;

    DatabaseAccess access;
    ImageComments comments(access, m_scanInfo.id);

    // Description
    if (!metadataInfos[1].isNull())
    {
        QString author = metadataInfos[4].toString(); // possibly null

        const QMap<QString, QVariant> &map = metadataInfos[1].toMap();

        for (QMap<QString, QVariant>::const_iterator it = map.begin();
             it != map.end(); ++it)
        {
            comments.addComment(it.value().toString(), it.key(), author);
            /*
             * Add here any handling of always adding one x-default comment, if needed
             * TODO: Store always one x-default value? Or is it ok to have only real language values? Then we will need
             * a global default language based on which x-default is chosen, when writing to image metadata
             * (not by this class!)
             */
        }
    }

    // old-style comment. Maybe overwrite x-default from above.
    if (!metadataInfos[0].isNull())
    {
        comments.addComment(metadataInfos[0].toString());
    }

    // Headline
    if (!metadataInfos[2].isNull())
    {
        comments.addHeadline(metadataInfos[2].toString());
    }

    // Title
    if (!metadataInfos[3].isNull())
    {
        comments.addTitle(metadataInfos[2].toString());
    }
}

void ImageScanner::scanImageCopyright()
{
    // TODO
}

void ImageScanner::scanIPTCCore()
{
    // TODO or remove
}

void ImageScanner::scanTags()
{
    QVariant var = m_metadata.getMetadataField(MetadataInfo::Keywords);
    QStringList keywords = var.toStringList();
    if (!keywords.isEmpty())
    {
        DatabaseAccess access;
        // get tag ids, create if necessary
        QList<int> tagIds = access.db()->getTagsFromTagPaths(keywords, true);
        access.db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, tagIds);
    }
}

void ImageScanner::loadFromDisk()
{
    QTime time;
    time.start();
    m_hasMetadata = m_metadata.load(m_fileInfo.filePath());
    int metadataTime = time.elapsed();
    time.restart();
    m_hasImage    = m_img.loadImageInfo(m_fileInfo.filePath(), false, false);
    int dimgTime = time.elapsed();
    time.restart();
    // faster than loading twice from disk
    if (m_hasMetadata)
    {
        m_img.setComments(m_metadata.getComments());
        m_img.setExif(m_metadata.getExif());
        m_img.setIptc(m_metadata.getIptc());
        m_img.setXmp(m_metadata.getXmp());
    }
    int setTime = time.elapsed();
    //DDebug() << "loadFromDisk times:" << metadataTime << dimgTime << setTime << endl;
}

QString ImageScanner::detectFormat()
{
    DImg::FORMAT dimgFormat = m_img.fileFormat();
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
            return "JP2k";
        case DImg::RAW:
        {
            QString format = "RAW-";
            format += m_fileInfo.suffix().toUpper();
        }
        case DImg::NONE:
        case DImg::QIMAGE:
        {
            KMimeType::Ptr mimetype = KMimeType::mimeType(m_fileInfo.path(), KMimeType::ResolveAliases);
            if (mimetype)
            {
                QString name = mimetype->name();
                if (name.startsWith("image/"))
                {
                    return name.mid(6).toUpper();
                }
            }
        }
    }
    return QString();
}




}

