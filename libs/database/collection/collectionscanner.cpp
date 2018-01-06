/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "collectionscanner.h"

// C++ includes

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QStringList>
#include <QSet>
#include <QTime>
#include <QWriteLocker>

// Local includes

#include "rawfiles.h"
#include "digikam_debug.h"
#include "coredb.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscannerhints.h"
#include "collectionscannerobserver.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "coredbtransaction.h"
#include "coredboperationgroup.h"
#include "imagecomments.h"
#include "imagecopyright.h"
#include "imageinfo.h"
#include "imagescanner.h"
#include "metadatasettings.h"
#include "tagscache.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"

namespace Digikam
{

class NewlyAppearedFile
{

public:

    NewlyAppearedFile()
        : albumId(0)
    {
    }

    NewlyAppearedFile(int albumId, const QString& fileName)
        : albumId(albumId),
          fileName(fileName)
    {
    }

    bool operator==(const NewlyAppearedFile& other) const
    {
        return (albumId  == other.albumId) &&
               (fileName == other.fileName);
    }

public:

    int     albumId;
    QString fileName;
};

// --------------------------------------------------------------------

inline uint qHash(const NewlyAppearedFile& file)
{
    return ::qHash(file.albumId) ^ ::qHash(file.fileName);
}

// --------------------------------------------------------------------

static bool modificationDateEquals(const QDateTime& a, const QDateTime& b)
{
    if (a != b)
    {
        // allow a "modify window" of one second.
        // FAT filesystems store the modify date in 2-second resolution.
        int diff = a.secsTo(b);

        if (abs(diff) > 1)
        {
            return false;
        }
    }

    return true;
}

// --------------------------------------------------------------------

class CollectionScannerHintContainerImplementation : public CollectionScannerHintContainer
{
public:

    virtual void recordHints(const QList<AlbumCopyMoveHint>& hints);
    virtual void recordHints(const QList<ItemCopyMoveHint>& hints);
    virtual void recordHints(const QList<ItemChangeHint>& hints);
    virtual void recordHint(const ItemMetadataAdjustmentHint& hint);

    virtual void clear();

    bool hasAnyNormalHint(qlonglong id)
    {
        QReadLocker locker(&lock);

        return modifiedItemHints.contains(id)          ||
               rescanItemHints.contains(id)            ||
               metadataAboutToAdjustHints.contains(id) ||
               metadataAdjustedHints.contains(id);
    }

    bool hasAlbumHints()                            { QReadLocker locker(&lock); return !albumHints.isEmpty();                   }
    bool hasModificationHint(qlonglong id)          { QReadLocker locker(&lock); return modifiedItemHints.contains(id);          }
    bool hasRescanHint(qlonglong id)                { QReadLocker locker(&lock); return rescanItemHints.contains(id);            }
    bool hasMetadataAboutToAdjustHint(qlonglong id) { QReadLocker locker(&lock); return metadataAboutToAdjustHints.contains(id); }
    bool hasMetadataAdjustedHint(qlonglong id)      { QReadLocker locker(&lock); return metadataAdjustedHints.contains(id);      }

public:

    QReadWriteLock                                                        lock;

    QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album> albumHints;
    QHash<NewlyAppearedFile, qlonglong>                                   itemHints;
    QSet<qlonglong>                                                       modifiedItemHints;
    QSet<qlonglong>                                                       rescanItemHints;
    QHash<qlonglong, QDateTime>                                           metadataAboutToAdjustHints;
    QHash<qlonglong, QDateTime>                                           metadataAdjustedHints;
};

void CollectionScannerHintContainerImplementation::recordHints(const QList<AlbumCopyMoveHint>& hints)
{
    QWriteLocker locker(&lock);

    foreach(const AlbumCopyMoveHint& hint, hints)
    {
        // automagic casting to src and dst
        albumHints[hint] = hint;
    }
}

void CollectionScannerHintContainerImplementation::recordHints(const QList<ItemCopyMoveHint>& hints)
{
    QWriteLocker locker(&lock);

    foreach(const ItemCopyMoveHint& hint, hints)
    {
        QList<qlonglong> ids = hint.srcIds();
        QStringList dstNames = hint.dstNames();

        for (int i=0; i<ids.size(); ++i)
        {
            itemHints[NewlyAppearedFile(hint.albumIdDst(), dstNames.at(i))] = ids.at(i);
        }
    }
}

void CollectionScannerHintContainerImplementation::recordHints(const QList<ItemChangeHint>& hints)
{
    QWriteLocker locker(&lock);

    foreach(const ItemChangeHint& hint, hints)
    {
        const QList<qlonglong>& ids = hint.ids();

        for (int i=0; i<ids.size(); ++i)
        {
            if (hint.isModified())
            {
                modifiedItemHints << ids.at(i);
            }
            else
            {
                rescanItemHints << ids.at(i);
            }
        }
    }
}

void CollectionScannerHintContainerImplementation::recordHint(const ItemMetadataAdjustmentHint& hint)
{
    if (hint.isAboutToEdit())
    {
        ImageInfo info(hint.id());

        if (!
            (modificationDateEquals(hint.modificationDate(), info.modDateTime())
             && hint.fileSize() == info.fileSize())
           )
        {
            // refuse to create a hint as a rescan is required already before any metadata edit
            // or, in case of multiple edits, there is already a hint with an older date, then all is fine.
            return;
        }

        QWriteLocker locker(&lock);
        metadataAboutToAdjustHints[hint.id()] = hint.modificationDate();
    }
    else if (hint.isEditingFinished())
    {
        QWriteLocker locker(&lock);
        QHash<qlonglong, QDateTime>::iterator it = metadataAboutToAdjustHints.find(hint.id());

        if (it == metadataAboutToAdjustHints.end())
        {
            return;
        }

        QDateTime date                   = it.value();
        metadataAboutToAdjustHints.erase(it);

        metadataAdjustedHints[hint.id()] = hint.modificationDate();
    }
    else // Aborted
    {
         QWriteLocker locker(&lock);
         QDateTime formerDate = metadataAboutToAdjustHints.take(hint.id());
    }
}

void CollectionScannerHintContainerImplementation::clear()
{
    QWriteLocker locker(&lock);

    albumHints.clear();
    itemHints.clear();
    modifiedItemHints.clear();
    rescanItemHints.clear();
    metadataAboutToAdjustHints.clear();
    metadataAdjustedHints.clear();
}

// --------------------------------------------------------------------

class CollectionScanner::Private
{

public:

    Private() :
        wantSignals(false),
        needTotalFiles(false),
        hints(0),
        updatingHashHint(false),
        recordHistoryIds(false),
        deferredFileScanning(false),
        observer(0)
    {
    }

public:

    void resetRemovedItemsTime()
    {
        removedItemsTime = QDateTime();
    }

    void removedItems()
    {
        removedItemsTime = QDateTime::currentDateTime();
    }

    inline bool checkObserver()
    {
        if (observer)
        {
            return observer->continueQuery();
        }

        return true;
    }

    inline bool checkDeferred(const QFileInfo& info)
    {
        if (deferredFileScanning)
        {
            deferredAlbumPaths << info.path();
            return true;
        }

        return false;
    }

    void finishScanner(ImageScanner& scanner);

public:

    QSet<QString>                                 nameFilters;
    QSet<QString>                                 imageFilterSet;
    QSet<QString>                                 videoFilterSet;
    QSet<QString>                                 audioFilterSet;
    QList<int>                                    scannedAlbums;
    bool                                          wantSignals;
    bool                                          needTotalFiles;

    QDateTime                                     removedItemsTime;

    CollectionScannerHintContainerImplementation* hints;
    QHash<int, int>                               establishedSourceAlbums;
    bool                                          updatingHashHint;

    bool                                          recordHistoryIds;
    QSet<qlonglong>                               needResolveHistorySet;
    QSet<qlonglong>                               needTaggingHistorySet;

    bool                                          deferredFileScanning;
    QSet<QString>                                 deferredAlbumPaths;

    CollectionScannerObserver*                    observer;
};

void CollectionScanner::Private::finishScanner(ImageScanner& scanner)
{
    // Perform the actual write operation to the database
    {
        CoreDbOperationGroup group;
        scanner.commit();
    }

    if (recordHistoryIds && scanner.hasHistoryToResolve())
    {
        needResolveHistorySet << scanner.id();
    }
}

// --------------------------------------------------------------------------

CollectionScanner::CollectionScanner()
    : d(new Private)
{
}

CollectionScanner::~CollectionScanner()
{
    delete d;
}

void CollectionScanner::setSignalsEnabled(bool on)
{
    d->wantSignals = on;
}

void CollectionScanner::setNeedFileCount(bool on)
{
    d->needTotalFiles = on;
}

CollectionScannerHintContainer* CollectionScanner::createHintContainer()
{
    return new CollectionScannerHintContainerImplementation;
}

void CollectionScanner::setHintContainer(CollectionScannerHintContainer* const container)
{
    // the API specs require the object given here to be created by createContainer, so we can cast.
    d->hints = static_cast<CollectionScannerHintContainerImplementation*>(container);
}

void CollectionScanner::setUpdateHashHint(bool hint)
{
    d->updatingHashHint = hint;
}

void CollectionScanner::loadNameFilters()
{
    if (!d->nameFilters.isEmpty())
    {
        return;
    }

    QStringList imageFilter, audioFilter, videoFilter;
    CoreDbAccess().db()->getFilterSettings(&imageFilter, &videoFilter, &audioFilter);

    // three sets to find category of a file
    d->imageFilterSet = imageFilter.toSet();
    d->audioFilterSet = audioFilter.toSet();
    d->videoFilterSet = videoFilter.toSet();

    d->nameFilters = d->imageFilterSet + d->audioFilterSet + d->videoFilterSet;
}

void CollectionScanner::setObserver(CollectionScannerObserver* const observer)
{
    d->observer = observer;
}

void CollectionScanner::setDeferredFileScanning(bool defer)
{
    d->deferredFileScanning = defer;
}

QStringList CollectionScanner::deferredAlbumPaths() const
{
    return d->deferredAlbumPaths.toList();
}

void CollectionScanner::completeScan()
{
    QTime time;
    time.start();

    emit startCompleteScan();

    // lock database
    CoreDbTransaction transaction;

    mainEntryPoint(true);
    d->resetRemovedItemsTime();

    //TODO: Implement a mechanism to watch for album root changes while we keep this list
    QList<CollectionLocation> allLocations = CollectionManager::instance()->allAvailableLocations();

    if (d->wantSignals && d->needTotalFiles)
    {
        // count for progress info
        int count = 0;

        foreach(const CollectionLocation& location, allLocations)
        {
            count += countItemsInFolder(location.albumRootPath());
        }

        emit totalFilesToScan(count);
    }

    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    // if we have no hints to follow, clean up all stale albums
    if (!d->hints || !d->hints->hasAlbumHints())
    {
        CoreDbAccess().db()->deleteStaleAlbums();
    }

    scanForStaleAlbums(allLocations);

    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    if (d->wantSignals)
    {
        emit startScanningAlbumRoots();
    }

    foreach(const CollectionLocation& location, allLocations)
    {
        scanAlbumRoot(location);
    }

    // do not continue to clean up without a complete scan!
    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    if (d->deferredFileScanning)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Complete scan (file scanning deferred) took:" << time.elapsed() << "msecs.";
        emit finishedCompleteScan();
        return;
    }

    completeScanCleanupPart();

    qCDebug(DIGIKAM_DATABASE_LOG) << "Complete scan took:" << time.elapsed() << "msecs.";
}

void CollectionScanner::finishCompleteScan(const QStringList& albumPaths)
{
    emit startCompleteScan();

    {
        CoreDbTransaction transaction;

        mainEntryPoint(true);
        d->resetRemovedItemsTime();
    }

    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    if (d->wantSignals)
    {
        emit startScanningAlbumRoots();
    }

    // remove subalbums from list if parent album is already contained
    QStringList sortedPaths = albumPaths;
    std::sort(sortedPaths.begin(), sortedPaths.end());
    QStringList::iterator it, it2;

    for (it = sortedPaths.begin(); it != sortedPaths.end(); )
    {
        // remove all following entries as long as they have the same beginning (= are subalbums)
        for (it2 = it + 1; it2 != sortedPaths.end() && it2->startsWith(*it); )
        {
            it2 = sortedPaths.erase(it2);
        }

        it = it2;
    }

    if (d->wantSignals && d->needTotalFiles)
    {
        // count for progress info
        int count = 0;

        foreach(const QString& path, sortedPaths)
        {
            count += countItemsInFolder(path);
        }

        emit totalFilesToScan(count);
    }

    foreach(const QString& path, sortedPaths)
    {
        CollectionLocation location = CollectionManager::instance()->locationForPath(path);
        QString album               = CollectionManager::instance()->album(path);

        if (album == QLatin1String("/"))
        {
            scanAlbumRoot(location);
        }
        else
        {
            scanAlbum(location, album);
        }
    }

    // do not continue to clean up without a complete scan!
    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    CoreDbTransaction transaction;
    completeScanCleanupPart();
}

void CollectionScanner::completeScanCleanupPart()
{
    completeHistoryScanning();

    updateRemovedItemsTime();

    // Items may be set to status removed, without being definitely deleted.
    // This deletion shall be done after a certain time, as checked by checkedDeleteRemoved
    if (checkDeleteRemoved())
    {
        // Mark items that are old enough and have the status trashed as obsolete
        // Only do this in a complete scan!
        CoreDbAccess access;
        QList<qlonglong> trashedItems = access.db()->getImageIds(DatabaseItem::Status::Trashed);
        foreach(qlonglong item, trashedItems)
        {
            access.db()->setItemStatus(item, DatabaseItem::Status::Obsolete);
        }

        resetDeleteRemovedSettings();
    }
    else
    {
        // increment the count of complete scans during which removed items were not deleted
        incrementDeleteRemovedCompleteScanCount();
    }

    markDatabaseAsScanned();

    emit finishedCompleteScan();
}

void CollectionScanner::partialScan(const QString& filePath)
{
    QString albumRoot = CollectionManager::instance()->albumRootPath(filePath);
    QString album     = CollectionManager::instance()->album(filePath);
    partialScan(albumRoot, album);
}

void CollectionScanner::partialScan(const QString& albumRoot, const QString& album)
{
    if (albumRoot.isNull() || album.isEmpty())
    {
        // If you want to scan the album root, pass "/"
        qCWarning(DIGIKAM_DATABASE_LOG) << "partialScan(QString, QString) called with invalid values";
        return;
    }

/*
    if (CoreDbAccess().backend()->isInTransaction())
    {
        // Install ScanController::instance()->suspendCollectionScan around your CoreDbTransaction
        qCDebug(DIGIKAM_DATABASE_LOG) << "Detected an active database transaction when starting a collection scan. "
                         "Please report this error.";
        return;
    }
*/

    mainEntryPoint(false);
    d->resetRemovedItemsTime();

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Did not find a CollectionLocation for album root path " << albumRoot;
        return;
    }

    // if we have no hints to follow, clean up all stale albums
    // Hint: Rethink with next major db update
    if (!d->hints || !d->hints->hasAlbumHints())
    {
        CoreDbAccess().db()->deleteStaleAlbums();
    }

    // Usually, we can restrict stale album scanning to our own location.
    // But when there are album hints from a second location to this location,
    // also scan the second location
    QSet<int> locationIdsToScan;
    locationIdsToScan << location.id();

    if (d->hints)
    {
        QReadLocker locker(&d->hints->lock);
        QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album>::const_iterator it;

        for (it = d->hints->albumHints.constBegin(); it != d->hints->albumHints.constEnd(); ++it)
        {
            if (it.key().albumRootId == location.id())
            {
                locationIdsToScan << it.key().albumRootId;
            }
        }
    }

    scanForStaleAlbums(locationIdsToScan.toList());

    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    if (album == QLatin1String("/"))
    {
        scanAlbumRoot(location);
    }
    else
    {
        scanAlbum(location, album);
    }

    finishHistoryScanning();

    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    updateRemovedItemsTime();
}

qlonglong CollectionScanner::scanFile(const QString& filePath, FileScanMode mode)
{
    QFileInfo info(filePath);
    QString dirPath   = info.path(); // strip off filename
    QString albumRoot = CollectionManager::instance()->albumRootPath(dirPath);

    if (albumRoot.isNull())
    {
        return -1;
    }

    QString album = CollectionManager::instance()->album(dirPath);

    return scanFile(albumRoot, album, info.fileName(), mode);
}

qlonglong CollectionScanner::scanFile(const QString& albumRoot, const QString& album,
                                      const QString& fileName, FileScanMode mode)
{
    if (album.isEmpty() || fileName.isEmpty())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "scanFile(QString, QString, QString) called with empty album or empty filename";
        return -1;
    }

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Did not find a CollectionLocation for album root path " << albumRoot;
        return -1;
    }

    QDir dir(location.albumRootPath() + album);
    QFileInfo fi(dir, fileName);

    if (!fi.exists())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "File given to scan does not exist" << albumRoot << album << fileName;
        return -1;
    }

    int albumId       = checkAlbum(location, album);
    qlonglong imageId = CoreDbAccess().db()->getImageId(albumId, fileName);
    imageId           = scanFile(fi, albumId, imageId, mode);

    return imageId;
}

void CollectionScanner::scanFile(const ImageInfo& info, FileScanMode mode)
{
    if (info.isNull())
    {
        return;
    }

    QFileInfo fi(info.filePath());
    scanFile(fi, info.albumId(), info.id(), mode);
}

qlonglong CollectionScanner::scanFile(const QFileInfo& fi, int albumId, qlonglong imageId, FileScanMode mode)
{
    mainEntryPoint(false);

    if (imageId == -1)
    {
        switch (mode)
        {
            case NormalScan:
            case ModifiedScan:
                imageId = scanNewFile(fi, albumId);
                break;
            case Rescan:
                imageId = scanNewFileFullScan(fi, albumId);
                break;
        }
    }
    else
    {
        ItemScanInfo scanInfo = CoreDbAccess().db()->getItemScanInfo(imageId);

        switch (mode)
        {
            case NormalScan:
                scanFileNormal(fi, scanInfo);
                break;
            case ModifiedScan:
                scanModifiedFile(fi, scanInfo);
                break;
            case Rescan:
                rescanFile(fi, scanInfo);
                break;
        }
    }

    finishHistoryScanning();
    return imageId;
}

void CollectionScanner::mainEntryPoint(bool complete)
{
    loadNameFilters();
    d->recordHistoryIds = !complete;
}

void CollectionScanner::scanAlbumRoot(const CollectionLocation& location)
{
    if (d->wantSignals)
    {
        emit startScanningAlbumRoot(location.albumRootPath());
    }

/*
    QDir dir(location.albumRootPath());
    QStringList fileList(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));

    for (QStringList::iterator fileIt = fileList.begin(); fileIt != fileList.end(); ++fileIt)
    {
        scanAlbum(location, '/' + (*fileIt));
    }
*/

    // scan album that covers the root directory of this album root,
    // all contained albums, and their subalbums recursively.
    scanAlbum(location, QLatin1String("/"));

    if (d->wantSignals)
    {
        emit finishedScanningAlbumRoot(location.albumRootPath());
    }
}

void CollectionScanner::scanForStaleAlbums(const QList<CollectionLocation>& locations)
{
    QList<int> locationIdsToScan;

    foreach(const CollectionLocation& location, locations)
    {
        locationIdsToScan << location.id();
    }

    scanForStaleAlbums(locationIdsToScan);
}

void CollectionScanner::scanForStaleAlbums(const QList<int>& locationIdsToScan)
{
    if (d->wantSignals)
    {
        emit startScanningForStaleAlbums();
    }

    QList<AlbumShortInfo> albumList = CoreDbAccess().db()->getAlbumShortInfos();
    QList<int> toBeDeleted;

/*
    // See bug #231598
    QHash<int, CollectionLocation> albumRoots;

    foreach(const CollectionLocation& location, locations)
    {
        albumRoots[location.id()] = location;
    }
*/

    QList<AlbumShortInfo>::const_iterator it;

    for (it = albumList.constBegin(); it != albumList.constEnd(); ++it)
    {
        if (!locationIdsToScan.contains((*it).albumRootId))
        {
            continue;
        }

        CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId((*it).albumRootId);

        // Only handle albums on available locations
        if (location.isAvailable())
        {
            QFileInfo fileInfo(location.albumRootPath() + (*it).relativePath);

            // let digikam think that ignored directories got deleted
            // (if they already exist in the database, this will delete them)
            if (!fileInfo.exists() || !fileInfo.isDir() || ignoredDirectoryContainsFileName(fileInfo.fileName()))
            {
                toBeDeleted << (*it).id;
                d->scannedAlbums << (*it).id;
            }
        }
    }

    // At this point, it is important to handle album renames.
    // We can still copy over album attributes later, but we cannot identify
    // the former album of removed images.
    // Just renaming the album is also much cheaper than rescanning all files.
    if (!toBeDeleted.isEmpty() && d->hints)
    {
        // shallow copy for reading without caring for locks
        QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album> albumHints;
        {
            QReadLocker locker(&d->hints->lock);
            albumHints = d->hints->albumHints;
        }

        // go through all album copy/move hints
        QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album>::const_iterator it;
        int toBeDeletedIndex;

        for (it = albumHints.constBegin(); it != albumHints.constEnd(); ++it)
        {
            // if the src entry of a hint is found in toBeDeleted, we have a move/rename, no copy. Handle these here.
            toBeDeletedIndex = toBeDeleted.indexOf(it.value().albumId);

            // We must double check that not, for some reason, the target album has already been scanned.
            QList<AlbumShortInfo>::const_iterator it2;

            for (it2 = albumList.constBegin(); it2 != albumList.constEnd(); ++it2)
            {
                if (it2->albumRootId  == it.key().albumRootId &&
                    it2->relativePath == it.key().relativePath)
                {
                    toBeDeletedIndex = -1;
                    break;
                }
            }

            if (toBeDeletedIndex != -1)
            {
                // check for existence of target
                CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(it.key().albumRootId);

                if (location.isAvailable())
                {
                    QFileInfo fileInfo(location.albumRootPath() + it.key().relativePath);

                    // Make sure ignored directories are not used in renaming operations
                    if (fileInfo.exists() && fileInfo.isDir() && ignoredDirectoryContainsFileName(fileInfo.fileName()))
                    {
                        // Just set a new root/relativePath to the album. Further scanning will care for all cases or error.
                        CoreDbAccess().db()->renameAlbum(it.value().albumId, it.key().albumRootId, it.key().relativePath);
                        // No need any more to delete the album
                        toBeDeleted.removeAt(toBeDeletedIndex);
                    }
                }
            }
        }
    }

    safelyRemoveAlbums(toBeDeleted);

    if (d->wantSignals)
    {
        emit finishedScanningForStaleAlbums();
    }
}

void CollectionScanner::safelyRemoveAlbums(const QList<int>& albumIds)
{
    // Remove the items (orphan items, detach them from the album, but keep entries for a certain time)
    // Make album orphan (no album root, keep entries until next application start)
    CoreDbAccess access;
    CoreDbTransaction transaction(&access);

    foreach(int albumId, albumIds)
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

void CollectionScanner::scanAlbum(const CollectionLocation& location, const QString& album)
{
    // + Adds album if it does not yet exist in the db.
    // + Recursively scans subalbums of album.
    // + Adds files if they do not yet exist in the db.
    // + Marks stale files as removed

    QDir dir(location.albumRootPath() + album);

    if ( !dir.exists() || !dir.isReadable() )
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Folder does not exist or is not readable: "
                   << dir.path();
        return;
    }

    if (d->wantSignals)
    {
        emit startScanningAlbum(location.albumRootPath(), album);
    }

    int albumID                   = checkAlbum(location, album);
    QList<ItemScanInfo> scanInfos = CoreDbAccess().db()->getItemScanInfos(albumID);

    // create a hash filename -> index in list
    QHash<QString, int> fileNameIndexHash;
    QSet<qlonglong> itemIdSet;

    for (int i = 0; i < scanInfos.size(); ++i)
    {
        fileNameIndexHash[scanInfos.at(i).itemName] = i;
        itemIdSet << scanInfos.at(i).id;
    }

    const QFileInfoList list = dir.entryInfoList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot,
                                                 QDir::Name | QDir::DirsLast);

    QFileInfoList::const_iterator fi;

    int counter = -1;

    for (fi = list.constBegin(); fi != list.constEnd(); ++fi)
    {
        if (!d->checkObserver())
        {
            return; // return directly, do not go to cleanup code after loop!
        }

        ++counter;

        if (d->wantSignals && counter && (counter % 100 == 0))
        {
            emit scannedFiles(counter);
            counter = 0;
        }

        if (fi->isFile())
        {
            // filter with name filter
            QString suffix = fi->suffix().toLower();

            if (!d->nameFilters.contains(suffix))
            {
                continue;
            }

            // ignore new files in subdirectories of ignored directories
            if(pathContainsIgnoredDirectory(fi->dir().path()))
            {
                continue;
            }

            int index = fileNameIndexHash.value(fi->fileName(), -1);

            if (index != -1)
            {
                // mark item as "seen"
                itemIdSet.remove(scanInfos.at(index).id);

                scanFileNormal(*fi, scanInfos.at(index));
            }
            // ignore temp files we created ourselves
            else if (fi->completeSuffix().contains(QLatin1String("digikamtempfile.")))
            {
                continue;
            }
            else
            {
                //qCDebug(DIGIKAM_DATABASE_LOG) << "Adding item " << fi->fileName();

                scanNewFile(*fi, albumID);

                // emit signals for scanned files with much higher granularity
                if (d->wantSignals && counter && (counter % 2 == 0))
                {
                    emit scannedFiles(counter);
                    counter = 0;
                }
            }
        }
        else if (fi->isDir())
        {
#ifdef Q_OS_WIN
            //Hide album that starts with a dot, as under Linux.
            if (fi->fileName().startsWith(QLatin1Char('.')))
            {
                continue;
            }
#endif
            QString subalbum;

            if (ignoredDirectoryContainsFileName(fi->fileName()))
            {
                continue;
            }

            if (album == QLatin1String("/"))
            {
                subalbum = QLatin1Char('/') + fi->fileName();
            }
            else
            {
                subalbum = album + QLatin1Char('/') + fi->fileName();
            }

            scanAlbum(location, subalbum);
        }
    }

    if (d->wantSignals && counter)
    {
        emit scannedFiles(counter);
    }

    // Mark items in the db which we did not see on disk.
    if (!itemIdSet.isEmpty())
    {
        QList<qlonglong> ids = itemIdSet.toList();
        CoreDbOperationGroup group;
        CoreDbAccess().db()->removeItems(ids, QList<int>() << albumID);
        itemsWereRemoved(ids);
    }

    // mark album as scanned
    d->scannedAlbums << albumID;

    if (d->wantSignals)
    {
        emit finishedScanningAlbum(location.albumRootPath(), album, list.count());
    }
}

void CollectionScanner::scanFileNormal(const QFileInfo& fi, const ItemScanInfo& scanInfo)
{
    bool hasAnyHint = d->hints && d->hints->hasAnyNormalHint(scanInfo.id);

    // if the date is null, this signals a full rescan
    if (scanInfo.modificationDate.isNull() ||
        (hasAnyHint && d->hints->hasRescanHint(scanInfo.id)) )
    {
        if (hasAnyHint)
        {
            QWriteLocker locker(&d->hints->lock);
            d->hints->rescanItemHints.remove(scanInfo.id);
        }

        rescanFile(fi, scanInfo);

        return;
    }
    else if (hasAnyHint && d->hints->hasModificationHint(scanInfo.id))
    {
        {
            QWriteLocker locker(&d->hints->lock);
            d->hints->modifiedItemHints.remove(scanInfo.id);
        }

        scanModifiedFile(fi, scanInfo);

        return;
    }
    else if (hasAnyHint) // metadata adjustment hints
    {
        if (d->hints->hasMetadataAboutToAdjustHint(scanInfo.id))
        {
            // postpone scan
            return;
        }
        else // hasMetadataAdjustedHint
        {
            {
                QWriteLocker locker(&d->hints->lock);
                d->hints->metadataAdjustedHints.remove(scanInfo.id);
            }

            scanFileUpdateHashReuseThumbnail(fi, scanInfo, true);

            return;
        }
    }
    else if (d->updatingHashHint)
    {
        // if the file need not be scanned because of modification, update the hash
        if (modificationDateEquals(fi.lastModified(), scanInfo.modificationDate) &&
            fi.size() == scanInfo.fileSize)
        {
            scanFileUpdateHashReuseThumbnail(fi, scanInfo, false);

            return;
        }
    }

    if (!modificationDateEquals(fi.lastModified(), scanInfo.modificationDate) ||
        fi.size() != scanInfo.fileSize)
    {
        if (MetadataSettings::instance()->settings().rescanImageIfModified)
        {
            rescanFile(fi, scanInfo);
        }
        else
        {
            scanModifiedFile(fi, scanInfo);
        }
    }
}

qlonglong CollectionScanner::scanNewFile(const QFileInfo& info, int albumId)
{
    if (d->checkDeferred(info))
    {
        return -1;
    }

    ImageScanner scanner(info);
    scanner.setCategory(category(info));

    // Check copy/move hints for single items
    qlonglong srcId = 0;

    if (d->hints)
    {
        QReadLocker locker(&d->hints->lock);
        srcId = d->hints->itemHints.value(NewlyAppearedFile(albumId, info.fileName()));
    }

    if (srcId != 0)
    {
        scanner.copiedFrom(albumId, srcId);
    }
    else
    {
        // Check copy/move hints for whole albums
        int srcAlbum = d->establishedSourceAlbums.value(albumId);

        if (srcAlbum)
        {
            // if we have one source album, find out if there is a file with the same name
            srcId = CoreDbAccess().db()->getImageId(srcAlbum, info.fileName());
        }

        if (srcId != 0)
        {
            scanner.copiedFrom(albumId, srcId);
        }
        else
        {
            // Establishing identity with the unique hsah
            scanner.newFile(albumId);
        }
    }

    d->finishScanner(scanner);

    return scanner.id();
}

qlonglong CollectionScanner::scanNewFileFullScan(const QFileInfo& info, int albumId)
{
    if (d->checkDeferred(info))
    {
        return -1;
    }

    ImageScanner scanner(info);
    scanner.setCategory(category(info));
    scanner.newFileFullScan(albumId);
    d->finishScanner(scanner);

    return scanner.id();
}

void CollectionScanner::scanModifiedFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ImageScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.fileModified();
    d->finishScanner(scanner);
}

void CollectionScanner::scanFileUpdateHashReuseThumbnail(const QFileInfo& info, const ItemScanInfo& scanInfo,
                                                         bool fileWasEdited)
{
    QString oldHash   = scanInfo.uniqueHash;
    qlonglong oldSize = scanInfo.fileSize;

    // same code as scanModifiedFile
    ImageScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.fileModified();

    QString newHash   = scanner.itemScanInfo().uniqueHash;
    qlonglong newSize = scanner.itemScanInfo().fileSize;

    if (ThumbsDbAccess::isInitialized())
    {
        if (fileWasEdited)
        {
            // The file was edited in such a way that we know that the pixel content did not change, so we can reuse the thumbnail.
            // We need to add a link to the thumbnail data with the new hash/file size _and_ adjust
            // the file modification date in the data table.
            ThumbsDbInfo thumbDbInfo = ThumbsDbAccess().db()->findByHash(oldHash, oldSize);

            if (thumbDbInfo.id != -1)
            {
                ThumbsDbAccess().db()->insertUniqueHash(newHash, newSize, thumbDbInfo.id);
                ThumbsDbAccess().db()->updateModificationDate(thumbDbInfo.id, scanner.itemScanInfo().modificationDate);
                // TODO: also update details thumbnails (by file path and URL scheme)
            }
        }
        else
        {
            ThumbsDbAccess().db()->replaceUniqueHash(oldHash, oldSize, newHash, newSize);
        }
    }

    d->finishScanner(scanner);
}

void CollectionScanner::rescanFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ImageScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.rescan();
    d->finishScanner(scanner);
}

void CollectionScanner::copyFileProperties(const ImageInfo& source, const ImageInfo& d)
{
    if (source.isNull() || d.isNull())
    {
        return;
    }

    ImageInfo dest(d);
    CoreDbOperationGroup group;

    qCDebug(DIGIKAM_DATABASE_LOG) << "Copying properties from" << source.id() << "to" << dest.id();

    // Rating, creation dates
    DatabaseFields::ImageInformation imageInfoFields = DatabaseFields::Rating       |
                                                       DatabaseFields::CreationDate |
                                                       DatabaseFields::DigitizationDate;

    QVariantList imageInfos = CoreDbAccess().db()->getImageInformation(source.id(), imageInfoFields);

    if (!imageInfos.isEmpty())
    {
        CoreDbAccess().db()->changeImageInformation(dest.id(), imageInfos, imageInfoFields);
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
    QVariantList positionData = CoreDbAccess().db()->getImagePosition(source.id(), DatabaseFields::ImagePositionsAll);

    if (!positionData.isEmpty())
    {
        CoreDbAccess().db()->addImagePosition(dest.id(), positionData, DatabaseFields::ImagePositionsAll);
    }

    // Comments
    {
        CoreDbAccess access;
        ImageComments commentsSource(access, source.id());
        ImageComments commentsDest(access, dest.id());
        commentsDest.replaceFrom(commentsSource);
        commentsDest.apply(access);
    }

    // Copyright info
    ImageCopyright copyrightDest(dest.id());
    copyrightDest.replaceFrom(source.id());

    // Image Properties
    CoreDbAccess().db()->copyImageProperties(source.id(), dest.id());
}

void CollectionScanner::itemsWereRemoved(const QList<qlonglong> &removedIds)
{
    // set time stamp
    d->removedItems();

    // manage relations
    QList<qlonglong> relatedImages = CoreDbAccess().db()->getOneRelatedImageEach(removedIds, DatabaseRelation::DerivedFrom);
    qCDebug(DIGIKAM_DATABASE_LOG) << "Removed items:" << removedIds << "related items:" << relatedImages;

    if (d->recordHistoryIds)
    {
        foreach(qlonglong id, relatedImages)
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

void CollectionScanner::completeHistoryScanning()
{
    // scan tagged images

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    QList<qlonglong> ids = CoreDbAccess().db()->getItemIDsInTag(needResolvingTag);
    historyScanningStage2(ids);

    ids                  = CoreDbAccess().db()->getItemIDsInTag(needTaggingTag);
    qCDebug(DIGIKAM_DATABASE_LOG) << "items to tag" << ids;
    historyScanningStage3(ids);
}

void CollectionScanner::finishHistoryScanning()
{
    // scan recorded ids

    QList<qlonglong> ids;

    // stage 2
    ids = d->needResolveHistorySet.toList();
    d->needResolveHistorySet.clear();
    historyScanningStage2(ids);

    if (!d->checkObserver())
    {
        return;
    }

    // stage 3
    ids = d->needTaggingHistorySet.toList();
    d->needTaggingHistorySet.clear();
    historyScanningStage3(ids);
}

void CollectionScanner::historyScanningStage2(const QList<qlonglong>& ids)
{
    foreach(qlonglong id, ids)
    {
        if (!d->checkObserver())
        {
            return;
        }

        CoreDbOperationGroup group;

        if (d->recordHistoryIds)
        {
            QList<qlonglong> needTaggingIds;
            ImageScanner::resolveImageHistory(id, &needTaggingIds);

            foreach(qlonglong needTag, needTaggingIds)
            {
                d->needTaggingHistorySet << needTag;
            }
        }
        else
        {
            ImageScanner::resolveImageHistory(id);
        }
    }
}

void CollectionScanner::historyScanningStage3(const QList<qlonglong>& ids)
{
    foreach(qlonglong id, ids)
    {
        if (!d->checkObserver())
        {
            return;
        }

        CoreDbOperationGroup group;
        ImageScanner::tagImageHistoryGraph(id);
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

bool CollectionScanner::databaseInitialScanDone()
{
    CoreDbAccess access;
    return !access.db()->getSetting(QLatin1String("Scanned")).isEmpty();
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

bool CollectionScanner::pathContainsIgnoredDirectory(const QString& path)
{
    QStringList ignoreDirectoryList;
    CoreDbAccess().db()->getIgnoreDirectoryFilterSettings(&ignoreDirectoryList);

    if (ignoreDirectoryList.isEmpty())
    {
        return false;
    }

    foreach(const QString& dir, ignoreDirectoryList)
    {
        if (path.contains(dir))
        {
            return true;
        }
    }

    return false;
}

bool CollectionScanner::ignoredDirectoryContainsFileName(const QString& fileName)
{
    QStringList ignoreDirectoryList;
    CoreDbAccess().db()->getIgnoreDirectoryFilterSettings(&ignoreDirectoryList);

    if (ignoreDirectoryList.isEmpty())
    {
        return false;
    }

    return ignoreDirectoryList.contains(fileName);
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
    QList< QPair<QString,int> >::const_iterator it;

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
    QList< QPair<QString,int> >::const_iterator it;

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
            QPair<QString,int> pair(it.next(),albumID);

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

    datetime = metadata.getImageDateTime();

    // Try to get image rating from IPTC Urgency tag
    // else use file system time stamp.
    rating   = metadata.getImageRating();

    if ( !datetime.isValid() )
    {
        QFileInfo info( filePath );
        datetime = info.lastModified();
    }

    // Try to get image tags from IPTC keywords tags.

    metadata.getImageTagsPath(keywords);

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

    datetime = metadata.getImageDateTime();

    if ( !datetime.isValid() )
    {
        QFileInfo info( filePath );
        datetime = info.lastModified();
    }

    access.db()->setItemDate(albumID, fileName, datetime);
}

#endif

} // namespace Digikam
