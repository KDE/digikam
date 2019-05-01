/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database - scan utilities.
 *
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "collectionscanner_p.h"

namespace Digikam
{

void CollectionScanner::loadNameFilters()
{
    if (!d->nameFilters.isEmpty())
    {
        return;
    }

    QStringList imageFilter, audioFilter, videoFilter, ignoreDirectory;

    CoreDbAccess().db()->getFilterSettings(&imageFilter, &videoFilter, &audioFilter);
    CoreDbAccess().db()->getIgnoreDirectoryFilterSettings(&ignoreDirectory);

    // three sets to find category of a file
    d->imageFilterSet  = imageFilter.toSet();
    d->audioFilterSet  = audioFilter.toSet();
    d->videoFilterSet  = videoFilter.toSet();
    d->ignoreDirectory = ignoreDirectory.toSet();

    d->nameFilters     = d->imageFilterSet + d->audioFilterSet + d->videoFilterSet;
}

void CollectionScanner::mainEntryPoint(bool complete)
{
    loadNameFilters();
    d->recordHistoryIds = !complete;
}

void CollectionScanner::safelyRemoveAlbums(const QList<int>& albumIds)
{
    // Remove the items (orphan items, detach them from the album, but keep entries for a certain time)
    // Make album orphan (no album root, keep entries until next application start)
    CoreDbAccess access;
    CoreDbTransaction transaction(&access);

    foreach (int albumId, albumIds)
    {
        QList<qlonglong> ids = access.db()->getItemIDsInAlbum(albumId);
        access.db()->removeItemsFromAlbum(albumId, ids);
        access.db()->makeStaleAlbum(albumId);
        itemsWereRemoved(ids);
    }
}

int CollectionScanner::checkAlbum(const CollectionLocation& location, const QString& album)
{
    // get album id if album exists
    int albumID = CoreDbAccess().db()->getAlbumForPath(location.id(), album, false);

    d->establishedSourceAlbums.remove(albumID);

    // create if necessary
    if (albumID == -1)
    {
        QFileInfo fi(location.albumRootPath() + album);
        albumID = CoreDbAccess().db()->addAlbum(location.id(), album, QString(), fi.lastModified().date(), QString());

        // have album this one was copied from?
        if (d->hints)
        {
            CollectionScannerHints::Album src;
            {
                QReadLocker locker(&d->hints->lock);
                src = d->hints->albumHints.value(CollectionScannerHints::DstPath(location.id(), album));
            }

            if (!src.isNull())
            {
                //qCDebug(DIGIKAM_DATABASE_LOG) << "Identified album" << src.albumId << "as source of new album" << fi.filePath();
                CoreDbAccess().db()->copyAlbumProperties(src.albumId, albumID);
                d->establishedSourceAlbums[albumID] = src.albumId;
            }
        }
    }

    return albumID;
}

void CollectionScanner::copyFileProperties(const ItemInfo& source, const ItemInfo& d)
{
    if (source.isNull() || d.isNull())
    {
        return;
    }

    ItemInfo dest(d);
    CoreDbOperationGroup group;

    qCDebug(DIGIKAM_DATABASE_LOG) << "Copying properties from" << source.id() << "to" << dest.id();

    // Rating, creation dates
    DatabaseFields::ItemInformation imageInfoFields = DatabaseFields::Rating       |
                                                       DatabaseFields::CreationDate |
                                                       DatabaseFields::DigitizationDate;

    QVariantList imageInfos = CoreDbAccess().db()->getItemInformation(source.id(), imageInfoFields);

    if (!imageInfos.isEmpty())
    {
        CoreDbAccess().db()->changeItemInformation(dest.id(), imageInfos, imageInfoFields);
    }

    // Copy public tags
    foreach (int tagId, TagsCache::instance()->publicTags(source.tagIds()))
    {
        dest.setTag(tagId);
    }

    // Copy color and pick label
    dest.setPickLabel(source.pickLabel());
    dest.setColorLabel(source.colorLabel());
    // important: skip other internal tags, such a history tags. Therefore CoreDB::copyImageTags is not to be used.

    // GPS data
    QVariantList positionData = CoreDbAccess().db()->getItemPosition(source.id(), DatabaseFields::ItemPositionsAll);

    if (!positionData.isEmpty())
    {
        CoreDbAccess().db()->addItemPosition(dest.id(), positionData, DatabaseFields::ItemPositionsAll);
    }

    // Comments
    {
        CoreDbAccess access;
        ItemComments commentsSource(access, source.id());
        ItemComments commentsDest(access, dest.id());
        commentsDest.replaceFrom(commentsSource);
        commentsDest.apply(access);
    }

    // Copyright info
    ItemCopyright copyrightDest(dest.id());
    copyrightDest.replaceFrom(ItemCopyright(source.id()));

    // Image Properties
    CoreDbAccess().db()->copyImageProperties(source.id(), dest.id());
}

void CollectionScanner::itemsWereRemoved(const QList<qlonglong>& removedIds)
{
    // set time stamp
    d->removedItems();

    // manage relations
    QList<qlonglong> relatedImages = CoreDbAccess().db()->getOneRelatedImageEach(removedIds, DatabaseRelation::DerivedFrom);
    qCDebug(DIGIKAM_DATABASE_LOG) << "Removed items:" << removedIds << "related items:" << relatedImages;

    if (d->recordHistoryIds)
    {
        foreach (const qlonglong& id, relatedImages)
        {
            d->needTaggingHistorySet << id;
        }
    }
    else
    {
        int needTaggingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());
        CoreDbAccess().db()->addTagsToItems(relatedImages, QList<int>() << needTaggingTag);
    }
}

int CollectionScanner::countItemsInFolder(const QString& directory)
{
    int items = 0;

    QDir dir(directory);

    if (!dir.exists() || !dir.isReadable())
    {
        return 0;
    }

    const QFileInfoList& list = dir.entryInfoList(QDir::Dirs    |
                                                  QDir::Files   |
                                                  QDir::NoDotAndDotDot);

    items += list.count();

    QFileInfoList::const_iterator fi;

    for (fi = list.constBegin() ; fi != list.constEnd() ; ++fi)
    {
        if (fi->isDir())
        {
            items += countItemsInFolder(fi->filePath());
        }
    }

    return items;
}

DatabaseItem::Category CollectionScanner::category(const QFileInfo& info)
{
    QString suffix = info.suffix().toLower();

    if (d->imageFilterSet.contains(suffix))
    {
        return DatabaseItem::Image;
    }
    else if (d->audioFilterSet.contains(suffix))
    {
        return DatabaseItem::Audio;
    }
    else if (d->videoFilterSet.contains(suffix))
    {
        return DatabaseItem::Video;
    }
    else
    {
        return DatabaseItem::Other;
    }
}

void CollectionScanner::markDatabaseAsScanned()
{
    CoreDbAccess access;
    access.db()->setSetting(QLatin1String("Scanned"), QDateTime::currentDateTime().toString(Qt::ISODate));
}

void CollectionScanner::updateRemovedItemsTime()
{
    // Called after a complete or partial scan finishes, to write the value
    // held in d->removedItemsTime to the database
    if (!d->removedItemsTime.isNull())
    {
        CoreDbAccess().db()->setSetting(QLatin1String("RemovedItemsTime"), d->removedItemsTime.toString(Qt::ISODate));
        d->removedItemsTime = QDateTime();
    }
}

void CollectionScanner::incrementDeleteRemovedCompleteScanCount()
{
    CoreDbAccess access;
    int count = access.db()->getSetting(QLatin1String("DeleteRemovedCompleteScanCount")).toInt();
    ++count;
    access.db()->setSetting(QLatin1String("DeleteRemovedCompleteScanCount"), QString::number(count));
}

void CollectionScanner::resetDeleteRemovedSettings()
{
    CoreDbAccess().db()->setSetting(QLatin1String("RemovedItemsTime"),               QString());
    CoreDbAccess().db()->setSetting(QLatin1String("DeleteRemovedTime"),              QDateTime::currentDateTime().toString(Qt::ISODate));
    CoreDbAccess().db()->setSetting(QLatin1String("DeleteRemovedCompleteScanCount"), QString::number(0));
}

bool CollectionScanner::checkDeleteRemoved()
{
    // returns true if removed items shall be deleted
    CoreDbAccess access;
    // retrieve last time an item was removed (not deleted, but set to status removed)
    QString removedItemsTimeString = access.db()->getSetting(QLatin1String("RemovedItemsTime"));

    if (removedItemsTimeString.isNull())
    {
        return false;
    }

    // retrieve last time removed items were (definitely) deleted from db
    QString deleteRemovedTimeString = access.db()->getSetting(QLatin1String("DeleteRemovedTime"));
    QDateTime removedItemsTime, deleteRemovedTime;

    if (!removedItemsTimeString.isNull())
    {
        removedItemsTime = QDateTime::fromString(removedItemsTimeString, Qt::ISODate);
    }

    if (!deleteRemovedTimeString.isNull())
    {
        deleteRemovedTime = QDateTime::fromString(deleteRemovedTimeString, Qt::ISODate);
    }

    QDateTime now = QDateTime::currentDateTime();

    // retrieve number of complete collection scans since the last time that removed items were deleted
    int completeScans = access.db()->getSetting(QLatin1String("DeleteRemovedCompleteScanCount")).toInt();

    // No removed items? No need to delete any
    if (!removedItemsTime.isValid())
    {
        return false;
    }

    // give at least a week between removed item deletions
    if (deleteRemovedTime.isValid())
    {
        if (deleteRemovedTime.daysTo(now) <= 7)
        {
            return false;
        }
    }

    // Now look at time since items were removed, and the number of complete scans
    // since removed items were deleted. Values arbitrarily chosen.
    int daysPast = removedItemsTime.daysTo(now);

    return (daysPast > 7  && completeScans > 2) ||
           (daysPast > 30 && completeScans > 0) ||
           (completeScans > 30);
}

// ------------------------------------------------------------------------------------------

#if 0

void CollectionScanner::scanForStaleAlbums()
{
    QStringList albumRootPaths = CollectionManager::instance()->allAvailableAlbumRootPaths();

    for (QStringList::const_iterator it = albumRootPaths.constBegin(); it != albumRootPaths.constEnd(); ++it)
    {
        scanForStaleAlbums(*it);
    }
}

void CollectionScanner::scanForStaleAlbums(const QString& albumRoot)
{
    Q_UNUSED(albumRoot);
    QList<AlbumShortInfo> albumList = CoreDbAccess().db()->getAlbumShortInfos();
    QList<AlbumShortInfo> toBeDeleted;

    QList<AlbumShortInfo>::const_iterator it;

    for (it = albumList.constBegin(); it != albumList.constEnd(); ++it)
    {
        QFileInfo fileInfo((*it).albumRoot + (*it).url);

        if (!fileInfo.exists() || !fileInfo.isDir())
        {
            m_foldersToBeDeleted << (*it);
        }
    }
}

QStringList CollectionScanner::formattedListOfStaleAlbums()
{
    QStringList list;
    QList<AlbumShortInfo>::const_iterator it;

    for (it = m_foldersToBeDeleted.constBegin(); it != m_foldersToBeDeleted.constEnd(); ++it)
    {
        list << (*it).url;
    }

    return list;
}

void CollectionScanner::removeStaleAlbums()
{
    CoreDbAccess access;
    CoreDbTransaction transaction(&access);
    QList<AlbumShortInfo>::const_iterator it;

    for (it = m_foldersToBeDeleted.constBegin(); it != m_foldersToBeDeleted.constEnd(); ++it)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Removing album " << (*it).albumRoot + QLatin1Char('/') + (*it).url;
        access.db()->deleteAlbum((*it).id);
    }
}

QStringList CollectionScanner::formattedListOfStaleFiles()
{
    QStringList listToBeDeleted;

    CoreDbAccess access;
    QList<QPair<QString, int> >::const_iterator it;

    for (it = m_filesToBeDeleted.constBegin(); it != m_filesToBeDeleted.constEnd(); ++it)
    {
        QString location = QLatin1String(" (") + access.db()->getAlbumPath((*it).second) + QLatin1Char(')');

        listToBeDeleted.append((*it).first + location);
    }

    return listToBeDeleted;
}

void CollectionScanner::removeStaleFiles()
{
    CoreDbAccess access;
    CoreDbTransaction transaction(&access);
    QList<QPair<QString, int> >::const_iterator it;

    for (it = m_filesToBeDeleted.constBegin(); it != m_filesToBeDeleted.constEnd(); ++it)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Removing: " << (*it).first << " in " << (*it).second;
        access.db()->deleteItem( (*it).second, (*it).first );
    }
}

void CollectionScanner::scanAlbums()
{
    QStringList albumRootPaths = CollectionManager::instance()->allAvailableAlbumRootPaths();
    int count = 0;

    for (QStringList::const_iterator it = albumRootPaths.constBegin(); it != albumRootPaths.constEnd(); ++it)
    {
        count += countItemsInFolder(*it);
    }

    emit totalFilesToScan(count);

    for (QStringList::const_iterator it = albumRootPaths.constBegin(); it != albumRootPaths.constEnd(); ++it)
    {
        QDir dir(*it);
        QStringList fileList(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));
        CoreDbTransaction transaction;

        foreach(const QString& dir, fileList)
        {
            scanAlbum(*it, QLatin1Char('/') + dir);
        }
    }
}

void CollectionScanner::scan(const QString& folderPath)
{
    CollectionManager* const manager = CollectionManager::instance();
    QUrl url;
    url.setPath(folderPath);
    QString albumRoot = manager->albumRootPath(url);
    QString album     = manager->album(url);

    if (albumRoot.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "scanAlbums(QString): folder " << folderPath << " not found in collection.";
        return;
    }

    scan(albumRoot, album);
}

void CollectionScanner::scan(const QString& albumRoot, const QString& album)
{
    // Step one: remove invalid albums
    scanForStaleAlbums(albumRoot);
    removeStaleAlbums();

    emit totalFilesToScan(countItemsInFolder(albumRoot + album));

    // Step two: Scan directories
    if (album == QLatin1String("/"))
    {
        // Don't scan files under album root, only descend into directories (?)
        QDir dir(albumRoot + album);
        QStringList fileList(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));

        CoreDbTransaction transaction;

        for (QStringList::const_iterator fileIt = fileList.constBegin(); fileIt != fileList.constEnd(); ++fileIt)
        {
            scanAlbum(albumRoot, QLatin1Char('/') + (*fileIt));
        }
    }
    else
    {
        CoreDbTransaction transaction;
        scanAlbum(albumRoot, album);
    }

    // Step three: Remove invalid files
    removeStaleFiles();
}

void CollectionScanner::scanAlbum(const QString& filePath)
{
    QUrl url;
    url.setPath(filePath);
    scanAlbum(CollectionManager::instance()->albumRootPath(url), CollectionManager::instance()->album(url));
}

void CollectionScanner::scanAlbum(const QString& albumRoot, const QString& album)
{
    // + Adds album if it does not yet exist in the db.
    // + Recursively scans subalbums of album.
    // + Adds files if they do not yet exist in the db.
    // + Adds stale files from the db to m_filesToBeDeleted
    // - Does not add stale albums to m_foldersToBeDeleted.

    QDir dir( albumRoot + album );

    if ( !dir.exists() || !dir.isReadable() )
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Folder does not exist or is not readable: " << dir.path();
        return;
    }

    emit startScanningAlbum(albumRoot, album);

    // get album id if album exists
    int albumID = CoreDbAccess().db()->getAlbumForPath(albumRoot, album, false);

    if (albumID == -1)
    {
        QFileInfo fi(albumRoot + album);
        albumID = CoreDbAccess().db()->addAlbum(albumRoot, album, QString(), fi.lastModified().date(), QString());
    }

    QStringList filesInAlbum = CoreDbAccess().db()->getItemNamesInAlbum( albumID );

    QSet<QString> filesFoundInDB;

    for (QStringList::const_iterator it = filesInAlbum.constBegin();
         it != filesInAlbum.constEnd(); ++it)
    {
        filesFoundInDB << *it;
    }

    const QFileInfoList list = dir.entryInfoList(m_nameFilters, QDir::AllDirs | QDir::Files  | QDir::NoDotAndDotDot /*not CaseSensitive*/);

    QFileInfoList::const_iterator fi;

    for (fi = list.constBegin(); fi != list.constEnd(); ++fi)
    {
        if ( fi->isFile())
        {
            if (filesFoundInDB.contains(fi->fileName()) )
            {
                filesFoundInDB.remove(fi->fileName());
            }
            // ignore temp files we created ourselves
            else if (fi->completeSuffix() == QLatin1String("digikamtempfile.tmp"))
            {
                continue;
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Adding item " << fi->fileName();
                addItem(albumID, albumRoot, album, fi->fileName());
            }
        }
        else if ( fi->isDir() )
        {
            scanAlbum( albumRoot, album + QLatin1Char('/') + fi->fileName() );
        }
    }

    // Removing items from the db which we did not see on disk.
    if (!filesFoundInDB.isEmpty())
    {
        QSetIterator<QString> it(filesFoundInDB);

        while (it.hasNext())
        {
            QPair<QString, int> pair(it.next(),albumID);

            if (m_filesToBeDeleted.indexOf(pair) == -1)
            {
                m_filesToBeDeleted << pair;
            }
        }
    }

    emit finishedScanningAlbum(albumRoot, album, list.count());
}

void CollectionScanner::updateItemsWithoutDate()
{
    QStringList urls  = CoreDbAccess().db()->getAllItemURLsWithoutDate();

    emit totalFilesToScan(urls.count());

    QString albumRoot = CoreDbAccess::albumRoot();

    {
        CoreDbTransaction transaction;

        for (QStringList::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
        {
            emit scanningFile(*it);

            QFileInfo fi(*it);
            QString albumURL = fi.path();
            albumURL         = QDir::cleanPath(albumURL.remove(albumRoot));
            int albumID      = CoreDbAccess().db()->getAlbumForPath(albumRoot, albumURL);

            if (albumID <= 0)
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Album ID == -1: " << albumURL;
            }

            if (fi.exists())
            {
                CollectionScanner::updateItemDate(albumID, albumRoot, albumURL, fi.fileName());
            }
            else
            {
                QPair<QString, int> pair(fi.fileName(), albumID);

                if (m_filesToBeDeleted.indexOf(pair) == -1)
                {
                    m_filesToBeDeleted << pair;
                }
            }
        }
    }
}

int CollectionScanner::countItemsInFolder(const QString& directory)
{
    int items = 0;

    QDir dir( directory );

    if ( !dir.exists() || !dir.isReadable() )
    {
        return 0;
    }

    QFileInfoList list = dir.entryInfoList();

    items += list.count();

    QFileInfoList::const_iterator fi;

    for (fi = list.constBegin(); fi != list.constEnd(); ++fi)
    {
        if ( fi->isDir()                          &&
             fi->fileName() != QLatin1String(".") &&
             fi->fileName() != QLatin1String(".."))
        {
            items += countItemsInFolder( fi->filePath() );
        }
    }

    return items;
}

void CollectionScanner::markDatabaseAsScanned()
{
    CoreDbAccess access;
    access.db()->setSetting("Scanned", QDateTime::currentDateTime().toString(Qt::ISODate));
}

// ------------------- Tools ------------------------

void CollectionScanner::addItem(int albumID, const QString& albumRoot,
                                const QString& album, const QString& fileName)
{
    CoreDbAccess access;
    addItem(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::addItem(Digikam::CoreDbAccess& access, int albumID,
                                const QString& albumRoot, const QString& album,
                                const QString& fileName)
{
    QString filePath = albumRoot + album + QLatin1Char('/') + fileName;

    QString     comment;
    QStringList keywords;
    QDateTime   datetime;
    int         rating;

    DMetadata metadata(filePath);

    // Try to get comments from image :
    // In first, from standard JPEG comments, or
    // In second, from EXIF comments tag, or
    // In third, from IPTC comments tag.

    comment  = metadata.getImageComment();

    // Try to get date and time from image :
    // In first, from EXIF date & time tags, or
    // In second, from IPTC date & time tags.

    datetime = metadata.getItemDateTime();

    // Try to get image rating from IPTC Urgency tag
    // else use file system time stamp.
    rating   = metadata.getItemRating();

    if ( !datetime.isValid() )
    {
        QFileInfo info( filePath );
        datetime = info.lastModified();
    }

    // Try to get image tags from IPTC keywords tags.

    metadata.getItemTagsPath(keywords);

    access.db()->addItem(albumID, fileName, datetime, comment, rating, keywords);
}

void CollectionScanner::updateItemDate(int albumID, const QString& albumRoot,
                                       const QString& album, const QString& fileName)
{
    CoreDbAccess access;
    updateItemDate(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::updateItemDate(Digikam::CoreDbAccess& access, int albumID,
                                       const QString& albumRoot, const QString& album,
                                       const QString& fileName)
{
    QString filePath = albumRoot + album + QLatin1Char('/') + fileName;

    QDateTime datetime;

    DMetadata metadata(filePath);

    // Trying to get date and time from image :
    // In first, from EXIF date & time tags, or
    // In second, from IPTC date & time tags.

    datetime = metadata.getItemDateTime();

    if ( !datetime.isValid() )
    {
        QFileInfo info( filePath );
        datetime = info.lastModified();
    }

    access.db()->setItemDate(albumID, fileName, datetime);
}

#endif

} // namespace Digikam
