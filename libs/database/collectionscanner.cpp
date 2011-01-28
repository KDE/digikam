/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "collectionscanner.moc"

// C++ includes

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QSet>
#include <QTime>

// KDE includes

#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/rawfiles.h>

// Local includes

#include "albumdb.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscannerhints.h"
#include "collectionscannerobserver.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "databasetransaction.h"
#include "databaseoperationgroup.h"
#include "imageinfo.h"
#include "imagescanner.h"
#include "tagscache.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"

namespace Digikam
{

class NewlyAppearedFile
{

public:

    NewlyAppearedFile() : albumId(0) {}
    NewlyAppearedFile(int albumId, const QString& fileName)
        : albumId(albumId), fileName(fileName) {}

    bool operator==(const NewlyAppearedFile& other) const
    {
        return albumId == other.albumId && fileName == other.fileName;
    }

    int     albumId;
    QString fileName;
};

inline uint qHash(const NewlyAppearedFile& file)
{
    return ::qHash(file.albumId) ^ ::qHash(file.fileName);
}

class CollectionScannerPriv
{

public:

    CollectionScannerPriv() :
        wantSignals(false),
        needTotalFiles(false),
        updatingHashHint(false),
        recordHistoryIds(false),
        observer(0)
    {
    }

    QSet<QString>     nameFilters;
    QSet<QString>     imageFilterSet;
    QSet<QString>     videoFilterSet;
    QSet<QString>     audioFilterSet;
    QList<int>        scannedAlbums;
    bool              wantSignals;
    bool              needTotalFiles;

    QDateTime         removedItemsTime;

    QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album>
    albumHints;
    QHash<NewlyAppearedFile, qlonglong>
    itemHints;
    QHash<int,int>    establishedSourceAlbums;
    QSet<int>         modifiedItemHints;
    QSet<int>         rescanItemHints;
    bool              updatingHashHint;

    bool              recordHistoryIds;
    QSet<qlonglong>   needResolveHistorySet;
    QSet<qlonglong>   needTaggingHistorySet;

    CollectionScannerObserver* observer;

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

    void finishScanner(const ImageScanner& scanner);
};

CollectionScanner::CollectionScanner()
    : d(new CollectionScannerPriv)
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

void CollectionScanner::recordHints(const QList<AlbumCopyMoveHint>& hints)
{
    foreach(const AlbumCopyMoveHint& hint, hints)
    {
        // automagic casting to src and dst
        d->albumHints[hint] = hint;
    }
}

void CollectionScanner::recordHints(const QList<ItemCopyMoveHint>& hints)
{
    foreach(const ItemCopyMoveHint& hint, hints)
    {
        QList<qlonglong> ids = hint.srcIds();
        QStringList dstNames = hint.dstNames();

        for (int i=0; i<ids.size(); ++i)
        {
            d->itemHints[NewlyAppearedFile(hint.albumIdDst(), dstNames[i])] = ids[i];
        }
    }
}

void CollectionScanner::recordHints(const QList<ItemChangeHint>& hints)
{
    foreach(const ItemChangeHint& hint, hints)
    {
        QList<qlonglong> ids = hint.ids();

        for (int i=0; i<ids.size(); ++i)
        {
            if (hint.isModified())
            {
                d->modifiedItemHints << ids[i];
            }
            else
            {
                d->rescanItemHints << ids[i];
            }
        }
    }
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
    DatabaseAccess().db()->getFilterSettings(&imageFilter, &videoFilter, &audioFilter);

    // three sets to find category of a file
    d->imageFilterSet = imageFilter.toSet();
    d->audioFilterSet = audioFilter.toSet();
    d->videoFilterSet = videoFilter.toSet();

    d->nameFilters = d->imageFilterSet + d->audioFilterSet + d->videoFilterSet;
}

void CollectionScanner::setObserver(CollectionScannerObserver* observer)
{
    d->observer = observer;
}

void CollectionScanner::completeScan()
{
    QTime time;
    time.start();

    emit startCompleteScan();

    // lock database
    DatabaseTransaction transaction;

    mainEntryPoint(true);

    d->resetRemovedItemsTime();

    //TODO: Implement a mechanism to watch for album root changes while we keep this list
    QList<CollectionLocation> allLocations = CollectionManager::instance()->allAvailableLocations();

    if (d->wantSignals && d->needTotalFiles)
    {
        // count for progress info
        int count = 0;
        foreach (const CollectionLocation& location, allLocations)
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
    if (d->albumHints.isEmpty())
    {
        DatabaseAccess().db()->deleteStaleAlbums();
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

    foreach (const CollectionLocation& location, allLocations)
    {
        scanAlbumRoot(location);
    }

    // do not continue to clean up without a complete scan!
    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    completeHistoryScanning();

    updateRemovedItemsTime();

    // Items may be set to status removed, without being definitely deleted.
    // This deletion shall be done after a certain time, as checked by checkedDeleteRemoved
    if (checkDeleteRemoved())
    {
        // Definitely delete items which are marked as removed.
        // Only do this in a complete scan!
        DatabaseAccess().db()->deleteRemovedItems();
        resetDeleteRemovedSettings();
    }
    else
    {
        // increment the count of complete scans during which removed items were not deleted
        incrementDeleteRemovedCompleteScanCount();
    }

    markDatabaseAsScanned();

    emit finishedCompleteScan();

    kDebug() << "Complete scan took:" << time.elapsed() << "msecs.";
}

void CollectionScanner::partialScan(const QString& filePath)
{
    QString albumRoot = CollectionManager::instance()->albumRootPath(filePath);

    if (albumRoot.isNull())
    {
        return;
    }

    QString album = CollectionManager::instance()->album(filePath);
    partialScan(albumRoot, album);
}

void CollectionScanner::partialScan(const QString& albumRoot, const QString& album)
{
    if (album.isEmpty())
    {
        // If you want to scan the album root, pass "/"
        kWarning() << "partialScan(QString, QString) called with empty album";
        return;
    }

    /*
    if (DatabaseAccess().backend()->isInTransaction())
    {
        // Install ScanController::instance()->suspendCollectionScan around your DatabaseTransaction
        kError() << "Detected an active database transaction when starting a collection scan. "
                         "Please report this error.";
        return;
    }
    */

    mainEntryPoint(false);
    d->resetRemovedItemsTime();

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        kWarning() << "Did not find a CollectionLocation for album root path " << albumRoot;
        return;
    }

    // if we have no hints to follow, clean up all stale albums
    // Hint: Rethink with next major db update
    if (d->albumHints.isEmpty())
    {
        DatabaseAccess().db()->deleteStaleAlbums();
    }

    // Usually, we can restrict stale album scanning to our own location.
    // But when there are album hints from a second location to this location,
    // also scan the second location
    QSet<int> locationIdsToScan;
    locationIdsToScan << location.id();
    QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album>::const_iterator it;
    for (it = d->albumHints.constBegin(); it != d->albumHints.constEnd(); ++it)
    {
        if (it.key().albumRootId == location.id())
        {
            locationIdsToScan << it.key().albumRootId;
        }
    }
    scanForStaleAlbums(locationIdsToScan.toList());

    if (!d->checkObserver())
    {
        emit cancelled();
        return;
    }

    if (album == "/")
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

qlonglong CollectionScanner::scanFile(const QString& albumRoot, const QString& album, const QString& fileName, FileScanMode mode)
{
    if (album.isEmpty() || fileName.isEmpty())
    {
        kWarning() << "scanFile(QString, QString, QString) called with empty album or empty filename";
        return -1;
    }

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        kWarning() << "Did not find a CollectionLocation for album root path " << albumRoot;
        return -1;
    }

    QDir dir(location.albumRootPath() + album);
    QFileInfo fi(dir, fileName);

    if (!fi.exists())
    {
        kWarning() << "File given to scan does not exist" << albumRoot << album << fileName;
        return -1;
    }

    int albumId       = checkAlbum(location, album);
    qlonglong imageId = DatabaseAccess().db()->getImageId(albumId, fileName);

    imageId = scanFile(fi, albumId, imageId, mode);
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
        ItemScanInfo scanInfo = DatabaseAccess().db()->getItemScanInfo(imageId);

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
    scanAlbum(location, "/");

    if (d->wantSignals)
    {
        emit finishedScanningAlbumRoot(location.albumRootPath());
    }
}

void CollectionScanner::scanForStaleAlbums(const QList<CollectionLocation>& locations)
{
    QList<int> locationIdsToScan;
    foreach (const CollectionLocation& location, locations)
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

    QList<AlbumShortInfo> albumList = DatabaseAccess().db()->getAlbumShortInfos();
    QList<int> toBeDeleted;

    /* see #231598
    QHash<int, CollectionLocation> albumRoots;
    foreach (const CollectionLocation& location, locations)
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

            if (!fileInfo.exists() || !fileInfo.isDir())
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
    if (!toBeDeleted.isEmpty() && !d->albumHints.isEmpty())
    {
        // go through all album copy/move hints
        QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album>::const_iterator it;
        int toBeDeletedIndex;

        for (it = d->albumHints.constBegin(); it != d->albumHints.constEnd(); ++it)
        {
            // if the src entry of a hint is found in toBeDeleted, we have a move/rename, no copy. Handle these here.
            toBeDeletedIndex = toBeDeleted.indexOf(it.value().albumId);

            // We must double check that not, for some reason, the target album has already been scanned.
            QList<AlbumShortInfo>::const_iterator it2;
            for (it2 = albumList.constBegin(); it2 != albumList.constEnd(); ++it2)
            {
                if (it2->albumRootId == it.key().albumRootId
                    && it2->relativePath == it.key().relativePath)
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

                    if (fileInfo.exists() && fileInfo.isDir())
                    {
                        // Just set a new root/relativePath to the album. Further scanning will care for all cases or error.
                        DatabaseAccess().db()->renameAlbum(it.value().albumId, it.key().albumRootId, it.key().relativePath);
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
    DatabaseAccess access;
    DatabaseTransaction transaction(&access);
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
    int albumID = DatabaseAccess().db()->getAlbumForPath(location.id(), album, false);

    d->establishedSourceAlbums.remove(albumID);

    // create if necessary
    if (albumID == -1)
    {
        QFileInfo fi(location.albumRootPath() + album);
        albumID = DatabaseAccess().db()->addAlbum(location.id(), album, QString(), fi.lastModified().date(), QString());

        // have album this one was copied from?
        CollectionScannerHints::Album src = d->albumHints.value(CollectionScannerHints::DstPath(location.id(), album));

        if (!src.isNull())
        {
            //kDebug() << "Identified album" << src.albumId << "as source of new album" << fi.filePath();
            DatabaseAccess().db()->copyAlbumProperties(src.albumId, albumID);
            d->establishedSourceAlbums[albumID] = src.albumId;
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
        kWarning() << "Folder does not exist or is not readable: "
                   << dir.path();
        return;
    }

    if (d->wantSignals)
    {
        emit startScanningAlbum(location.albumRootPath(), album);
    }

    int albumID = checkAlbum(location, album);

    QList<ItemScanInfo> scanInfos = DatabaseAccess().db()->getItemScanInfos(albumID);

    // create a hash filename -> index in list
    QHash<QString, int> fileNameIndexHash;
    QSet<qlonglong> itemIdSet;

    for (int i = 0; i < scanInfos.size(); ++i)
    {
        fileNameIndexHash[scanInfos[i].itemName] = i;
        itemIdSet << scanInfos[i].id;
    }

    const QFileInfoList list = dir.entryInfoList(QDir::AllDirs | QDir::Files  | QDir::NoDotAndDotDot);

    QFileInfoList::const_iterator fi;

    int counter = -1;

    for (fi = list.constBegin(); fi != list.constEnd(); ++fi)
    {
        if (!d->checkObserver())
        {
            return; // return directly, do not go to cleanup code after loop!
        }

        counter++;

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

            int index = fileNameIndexHash.value(fi->fileName(), -1);

            if (index != -1)
            {
                // mark item as "seen"
                itemIdSet.remove(scanInfos[index].id);

                scanFileNormal(*fi, scanInfos[index]);
            }
            // ignore temp files we created ourselves
            else if (fi->completeSuffix() == "digikamtempfile.tmp")
            {
                continue;
            }
            else
            {
                //kDebug() << "Adding item " << fi->fileName();

                scanNewFile(*fi, albumID);

                // emit signals for scanned files with much higher granularity
                if (d->wantSignals && counter && (counter % 2 == 0))
                {
                    emit scannedFiles(counter);
                    counter = 0;
                }
            }
        }
        else if ( fi->isDir() )
        {
            QString subalbum;

            if (album == "/")
            {
                subalbum = '/' + fi->fileName();
            }
            else
            {
                subalbum = album + '/' + fi->fileName();
            }

            scanAlbum( location, subalbum );
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
        DatabaseOperationGroup group;
        DatabaseAccess().db()->removeItems(ids, QList<int>() << albumID);
        itemsWereRemoved(ids);
    }

    // mark album as scanned
    d->scannedAlbums << albumID;

    if (d->wantSignals)
    {
        emit finishedScanningAlbum(location.albumRootPath(), album, list.count());
    }
}

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

void CollectionScanner::scanFileNormal(const QFileInfo& fi, const ItemScanInfo& scanInfo)
{
    // if the date is null, this signals a full rescan
    if (scanInfo.modificationDate.isNull() || d->rescanItemHints.contains(scanInfo.id))
    {
        d->rescanItemHints.remove(scanInfo.id);
        rescanFile(fi, scanInfo);
        return;
    }
    else if (d->modifiedItemHints.contains(scanInfo.id))
    {
        d->modifiedItemHints.remove(scanInfo.id);
        scanModifiedFile(fi, scanInfo);
        return;
    }
    else if (d->updatingHashHint)
    {
        // if the file need not be scanned because of modification, update the hash
        if (modificationDateEquals(fi.lastModified(), scanInfo.modificationDate)
            && (int)fi.size() == scanInfo.fileSize)
        {
            QString oldHash = scanInfo.uniqueHash;
            QString newHash = scanFileUpdateHash(fi, scanInfo);
            if (ThumbnailDatabaseAccess::isInitialized())
            {
                ThumbnailDatabaseAccess().db()->replaceUniqueHash(oldHash, scanInfo.fileSize,
                                                                  newHash, scanInfo.fileSize);
            }
            return;
        }
    }

    if (!modificationDateEquals(fi.lastModified(), scanInfo.modificationDate)
        || (int)fi.size() != scanInfo.fileSize)
    {
        scanModifiedFile(fi, scanInfo);
    }
}

void CollectionScannerPriv::finishScanner(const ImageScanner& scanner)
{
    if (recordHistoryIds && scanner.hasHistoryToResolve())
    {
        needResolveHistorySet << scanner.id();
    }
}

qlonglong CollectionScanner::scanNewFile(const QFileInfo& info, int albumId)
{
    DatabaseOperationGroup group;
    ImageScanner scanner(info);
    scanner.setCategory(category(info));

    // Check copy/move hints for single items
    qlonglong srcId = d->itemHints.value(NewlyAppearedFile(albumId, info.fileName()));

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
            srcId = DatabaseAccess().db()->getImageId(srcAlbum, info.fileName());
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
    DatabaseOperationGroup group;
    ImageScanner scanner(info);
    scanner.setCategory(category(info));
    scanner.newFileFullScan(albumId);
    d->finishScanner(scanner);
    return scanner.id();
}

void CollectionScanner::scanModifiedFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    DatabaseOperationGroup group;
    ImageScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.fileModified();
    d->finishScanner(scanner);
}

QString CollectionScanner::scanFileUpdateHash(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    // same code as scanModifiedFile
    DatabaseOperationGroup group;
    ImageScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.fileModified();
    d->finishScanner(scanner);
    return scanner.itemScanInfo().uniqueHash;
}

void CollectionScanner::rescanFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    DatabaseOperationGroup group;
    ImageScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.rescan();
    d->finishScanner(scanner);
}

void CollectionScanner::copyFileProperties(const ImageInfo& source, const ImageInfo& dest)
{
    if (source.isNull() || dest.isNull())
    {
        return;
    }

    DatabaseOperationGroup group;
    ImageScanner::copyProperties(source.id(), dest.id());
}

void CollectionScanner::itemsWereRemoved(const QList<qlonglong> &removedIds)
{
    // set time stamp
    d->removedItems();

    // manage relations
    QList<qlonglong> relatedImages = DatabaseAccess().db()->getOneRelatedImageEach(removedIds, DatabaseRelation::DerivedFrom);
    kDebug() << "Removed items:" << removedIds << "related items:" << relatedImages;

    if (d->recordHistoryIds)
    {
        foreach (qlonglong id, relatedImages)
        {
            d->needTaggingHistorySet << id;
        }
    }
    else
    {
        int needTaggingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());
        DatabaseAccess().db()->addTagsToItems(relatedImages, QList<int>() << needTaggingTag);
    }
}

void CollectionScanner::completeHistoryScanning()
{
    // scan tagged images

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    QList<qlonglong> ids = DatabaseAccess().db()->getItemIDsInTag(needResolvingTag);
    historyScanningStage2(ids);

    ids = DatabaseAccess().db()->getItemIDsInTag(needTaggingTag);
    kDebug() << "items to tag" << ids;
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
    foreach (qlonglong id, ids)
    {
        if (!d->checkObserver())
        {
            return;
        }

        DatabaseOperationGroup group;

        if (d->recordHistoryIds)
        {
            QList<qlonglong> needTaggingIds;
            ImageScanner::resolveImageHistory(id, &needTaggingIds);
            foreach (qlonglong needTag, needTaggingIds)
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
    foreach (qlonglong id, ids)
    {
        if (!d->checkObserver())
        {
            return;
        }

        DatabaseOperationGroup group;
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
        if ( fi->isDir() &&
             fi->fileName() != "." &&
             fi->fileName() != "..")
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
    DatabaseAccess access;
    access.db()->setSetting("Scanned", QDateTime::currentDateTime().toString(Qt::ISODate));
}

bool CollectionScanner::databaseInitialScanDone()
{
    DatabaseAccess access;
    return !access.db()->getSetting("Scanned").isEmpty();
}

void CollectionScanner::updateRemovedItemsTime()
{
    // Called after a complete or partial scan finishes, to write the value
    // held in d->removedItemsTime to the database
    if (!d->removedItemsTime.isNull())
    {
        DatabaseAccess().db()->setSetting("RemovedItemsTime", d->removedItemsTime.toString(Qt::ISODate));
        d->removedItemsTime = QDateTime();
    }
}

void CollectionScanner::incrementDeleteRemovedCompleteScanCount()
{
    DatabaseAccess access;
    int count = access.db()->getSetting("DeleteRemovedCompleteScanCount").toInt();
    ++count;
    access.db()->setSetting("DeleteRemovedCompleteScanCount", QString::number(count));
}

void CollectionScanner::resetDeleteRemovedSettings()
{
    DatabaseAccess().db()->setSetting("RemovedItemsTime", QString());
    DatabaseAccess().db()->setSetting("DeleteRemovedTime", QDateTime::currentDateTime().toString(Qt::ISODate));
    DatabaseAccess().db()->setSetting("DeleteRemovedCompleteScanCount", QString::number(0));
}

bool CollectionScanner::checkDeleteRemoved()
{
    // returns true if removed items shall be deleted
    DatabaseAccess access;
    // retrieve last time an item was removed (not deleted, but set to status removed)
    QString removedItemsTimeString = access.db()->getSetting("RemovedItemsTime");

    if (removedItemsTimeString.isNull())
    {
        return false;
    }

    // retrieve last time removed items were (definitely) deleted from db
    QString deleteRemovedTimeString = access.db()->getSetting("DeleteRemovedTime");
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
    int completeScans = access.db()->getSetting("DeleteRemovedCompleteScanCount").toInt();

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
    QList<AlbumShortInfo> albumList = DatabaseAccess().db()->getAlbumShortInfos();
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
    DatabaseAccess access;
    DatabaseTransaction transaction(&access);
    QList<AlbumShortInfo>::const_iterator it;

    for (it = m_foldersToBeDeleted.constBegin(); it != m_foldersToBeDeleted.constEnd(); ++it)
    {
        kDebug() << "Removing album " << (*it).albumRoot + '/' + (*it).url;
        access.db()->deleteAlbum((*it).id);
    }
}

QStringList CollectionScanner::formattedListOfStaleFiles()
{
    QStringList listToBeDeleted;

    DatabaseAccess access;
    QList< QPair<QString,int> >::const_iterator it;

    for (it = m_filesToBeDeleted.constBegin(); it != m_filesToBeDeleted.constEnd(); ++it)
    {
        QString location = " (" + access.db()->getAlbumPath((*it).second) + ')';

        listToBeDeleted.append((*it).first + location);
    }

    return listToBeDeleted;
}

void CollectionScanner::removeStaleFiles()
{
    DatabaseAccess access;
    DatabaseTransaction transaction(&access);
    QList< QPair<QString,int> >::const_iterator it;

    for (it = m_filesToBeDeleted.constBegin(); it != m_filesToBeDeleted.constEnd(); ++it)
    {
        kDebug() << "Removing: " << (*it).first << " in "
                 << (*it).second;
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

        DatabaseTransaction transaction;
        foreach (const QString& dir, fileList)
        {
            scanAlbum(*it, '/' + dir);
        }
    }
}

void CollectionScanner::scan(const QString& folderPath)
{
    CollectionManager* manager = CollectionManager::instance();
    KUrl url;
    url.setPath(folderPath);
    QString albumRoot = manager->albumRootPath(url);
    QString album = manager->album(url);

    if (albumRoot.isNull())
    {
        kWarning() << "scanAlbums(QString): folder " << folderPath << " not found in collection.";
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
    if (album == "/")
    {
        // Don't scan files under album root, only descend into directories (?)
        QDir dir(albumRoot + album);
        QStringList fileList(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));

        DatabaseTransaction transaction;

        for (QStringList::const_iterator fileIt = fileList.constBegin(); fileIt != fileList.constEnd(); ++fileIt)
        {
            scanAlbum(albumRoot, '/' + (*fileIt));
        }
    }
    else
    {
        DatabaseTransaction transaction;
        scanAlbum(albumRoot, album);
    }

    // Step three: Remove invalid files
    removeStaleFiles();
}

void CollectionScanner::scanAlbum(const QString& filePath)
{
    KUrl url;
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
        kWarning() << "Folder does not exist or is not readable: " << dir.path();
        return;
    }

    emit startScanningAlbum(albumRoot, album);

    // get album id if album exists
    int albumID = DatabaseAccess().db()->getAlbumForPath(albumRoot, album, false);

    if (albumID == -1)
    {
        QFileInfo fi(albumRoot + album);
        albumID = DatabaseAccess().db()->addAlbum(albumRoot, album, QString(), fi.lastModified().date(), QString());
    }

    QStringList filesInAlbum = DatabaseAccess().db()->getItemNamesInAlbum( albumID );

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
            else if (fi->completeSuffix() == "digikamtempfile.tmp")
            {
                continue;
            }
            else
            {
                kDebug() << "Adding item " << fi->fileName();
                addItem(albumID, albumRoot, album, fi->fileName());
            }
        }
        else if ( fi->isDir() )
        {
            scanAlbum( albumRoot, album + '/' + fi->fileName() );
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
    QStringList urls = DatabaseAccess().db()->getAllItemURLsWithoutDate();

    emit totalFilesToScan(urls.count());

    QString albumRoot = DatabaseAccess::albumRoot();

    {
        DatabaseTransaction transaction;

        for (QStringList::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
        {
            emit scanningFile(*it);

            QFileInfo fi(*it);
            QString albumURL = fi.path();
            albumURL = QDir::cleanPath(albumURL.remove(albumRoot));

            int albumID = DatabaseAccess().db()->getAlbumForPath(albumRoot, albumURL);

            if (albumID <= 0)
            {
                kWarning() << "Album ID == -1: " << albumURL;
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
        if ( fi->isDir()           &&
             fi->fileName() != "." &&
             fi->fileName() != "..")
        {
            items += countItemsInFolder( fi->filePath() );
        }
    }

    return items;
}

void CollectionScanner::markDatabaseAsScanned()
{
    DatabaseAccess access;
    access.db()->setSetting("Scanned", QDateTime::currentDateTime().toString(Qt::ISODate));
}

// ------------------- Tools ------------------------

void CollectionScanner::addItem(int albumID, const QString& albumRoot, const QString& album, const QString& fileName)
{
    DatabaseAccess access;
    addItem(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::addItem(Digikam::DatabaseAccess& access, int albumID,
                                const QString& albumRoot, const QString& album, const QString& fileName)
{
    QString filePath = albumRoot + album + '/' + fileName;

    QString     comment;
    QStringList keywords;
    QDateTime   datetime;
    int         rating;

    DMetadata metadata(filePath);

    // Try to get comments from image :
    // In first, from standard JPEG comments, or
    // In second, from EXIF comments tag, or
    // In third, from IPTC comments tag.

    comment = metadata.getImageComment();

    // Try to get date and time from image :
    // In first, from EXIF date & time tags, or
    // In second, from IPTC date & time tags.

    datetime = metadata.getImageDateTime();

    // Try to get image rating from IPTC Urgency tag
    // else use file system time stamp.
    rating = metadata.getImageRating();

    if ( !datetime.isValid() )
    {
        QFileInfo info( filePath );
        datetime = info.lastModified();
    }

    // Try to get image tags from IPTC keywords tags.

    metadata.getImageTagsPath(keywords);

    access.db()->addItem(albumID, fileName, datetime, comment, rating, keywords);
}

void CollectionScanner::updateItemDate(int albumID, const QString& albumRoot, const QString& album, const QString& fileName)
{
    DatabaseAccess access;
    updateItemDate(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::updateItemDate(Digikam::DatabaseAccess& access, int albumID,
                                       const QString& albumRoot, const QString& album, const QString& fileName)
{
    QString filePath = albumRoot + album + '/' + fileName;

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

/*
// ------------------------- Ioslave scanning code ----------------------------------

void CollectionScanner::scanAlbum(const QString& albumRoot, const QString& album)
{
    scanOneAlbum(albumRoot, album);
    removeInvalidAlbums(albumRoot);
}

void CollectionScanner::scanOneAlbum(const QString& albumRoot, const QString& album)
{
    kDebug() << "CollectionScanner::scanOneAlbum " << albumRoot << "/" << album;
    QDir dir(albumRoot + album);
    if (!dir.exists() || !dir.isReadable())
    {
        return;
    }

    {
        // scan albums

        QStringList currAlbumList;

        // get sub albums, but only direct subalbums (Album/ *, not Album/ * / *)
        currAlbumList = DatabaseAccess().db()->getSubalbumsForPath(albumRoot, album, true);
        kDebug() << "currAlbumList is " << currAlbumList;

        const QFileInfoList* infoList = dir.entryInfoList(QDir::Dirs);
        if (!infoList)
            return;

        QFileInfoListIterator it(*infoList);
        QFileInfo* fi;

        QStringList newAlbumList;
        while ((fi = it.current()) != 0)
        {
            ++it;

            if (fi->fileName() == "." || fi->fileName() == "..")
            {
                continue;
            }

            QString u = QDir::cleanPath(album + '/' + fi->fileName());

            if (currAlbumList.contains(u))
            {
                continue;
            }

            newAlbumList.append(u);
        }

        for (QStringList::iterator it = newAlbumList.begin();
             it != newAlbumList.end(); ++it)
        {
            kDebug() << "New Album: " << *it;

            QFileInfo fi(albumRoot + *it);
            DatabaseAccess().db()->addAlbum(albumRoot, *it, QString(), fi.lastModified().date(), QString());

            scanAlbum(albumRoot, *it);
        }
    }

    if (album != "/")
    {
        // scan files

        QStringList values;
        int albumID;
        QStringList currItemList;

        {
            DatabaseAccess access;
            albumID = access.db()->getAlbumForPath(albumRoot, album, false);

            if (albumID == -1)
                return;

            currItemList = access.db()->getItemNamesInAlbum(albumID);
        }

        const QFileInfoList* infoList = dir.entryInfoList(QDir::Files);
        if (!infoList)
            return;

        QFileInfoListIterator it(*infoList);
        QFileInfo* fi;

        // add any new files we find to the db
        while ((fi = it.current()) != 0)
        {
            ++it;

            // ignore temp files we created ourselves
            if (fi->extension(true) == "digikamtempfile.tmp")
            {
                continue;
            }

            if (currItemList.contains(fi->fileName()))
            {
                currItemList.remove(fi->fileName());
                continue;
            }

            addItem(albumID, albumRoot, album, fi->fileName());
        }

        DatabaseAccess access;
        // currItemList now contains deleted file list. remove them from db
        for (QStringList::iterator it = currItemList.begin();
             it != currItemList.end(); ++it)
        {
            access.db()->deleteItem(albumID, *it);
        }
    }
}

void CollectionScanner::removeInvalidAlbums(const QString& albumRoot)
{
    DatabaseAccess access;

    QValueList<AlbumShortInfo> albumList = access.db()->getAlbumShortInfos();
    QValueList<AlbumShortInfo> toBeDeleted;

    for (QValueList<AlbumShortInfo>::iterator it = albumList.begin(); it != albumList.end(); ++it)
    {
        QFileInfo fileInfo((*it).albumRoot + (*it).url);
        if (!fileInfo.exists())
            toBeDeleted << (*it);
    }

    DatabaseTransaction transaction(&access);
    for (QValueList<AlbumShortInfo>::iterator it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it)
    {
        kDebug() << "Removing album " << (*it).albumRoot + '/' + (*it).url;
        access.db()->deleteAlbum((*it).id);
    }
}
*/
#endif

}  // namespace Digikam
