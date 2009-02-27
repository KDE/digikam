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

#ifndef COLLECTIONSCANNER_H
#define COLLECTIONSCANNER_H

// Qt includes.

#include <QString>
#include <QObject>
#include <QList>

// Local includes.

#include "digikam_export.h"
#include "databaseaccess.h"
#include "albuminfo.h"

class QFileInfo;

namespace Digikam
{

class CollectionLocation;
class CollectionScannerPriv;
class AlbumCopyMoveHint;
class ItemCopyMoveHint;

class DIGIKAM_DATABASE_EXPORT CollectionScanner : public QObject
{
    Q_OBJECT
public:

    CollectionScanner();
    ~CollectionScanner();

    /**
     * Carries out a full scan on all available parts of the collection.
     * Only a full scan can finally remove deleted files from the database,
     * only a full scan will mark the database as scanned.
     * The database will be locked while running (Note: this is not done for partialScans).
     */
    void completeScan();

    /**
     * Carries out a partial scan on the specified path of the collection.
     * The includes scanning for new files + albums and updating modified file data.
     * Files no longer found in the specified path however are not completely
     * removed, but only marked as removed. They will be removed only after a complete scan.
     */
    void partialScan(const QString &filePath);

    /**
     * Same procedure as above, but albumRoot and album is provided.
     */
    void partialScan(const QString &albumRoot, const QString& album);

    /**
     * The specified file will be added to the database if it is not included,
     * or rescanned as if it was modified if it is found in the database.
     */
    void scanFile(const QString &filePath);

    /**
     * Same procedure as above, but albumRoot and album is provided.
     */
    void scanFile(const QString &albumRoot, const QString &album, const QString &fileName);

    /**
     * Call this to enable the progress info signals.
     * Default is off.
     */
    void setSignalsEnabled(bool on);

    /**
     * Record hints for the collection scanner.
     */
    void recordHints(const QList<AlbumCopyMoveHint> &hint);
    void recordHints(const QList<ItemCopyMoveHint> &hint);

    /**
     * Utility method:
     * Prepare the given albums to be removed,
     * typically by setting the albums as orphan
     * and removing all entries from the albums
     */
     void safelyRemoveAlbums(const QList<int> &albumIds);

protected:

    void scanForStaleAlbums(QList<CollectionLocation> locations);
    void scanAlbumRoot(const CollectionLocation &location);
    void scanAlbum(const CollectionLocation &location, const QString &album);
    int checkAlbum(const CollectionLocation &location, const QString &album);
    void scanNewFile(const QFileInfo &info, int albumId);
    void scanModifiedFile(const QFileInfo &info, const ItemScanInfo &scanInfo);

#if 0
    /**
     * Sets a filter for the file formats which shall be included in the collection.
     * The string is a list of name wildcards (understanding * and ?),
     * separated by ';' characters.
     * Example: "*.jpg;*.png"
     */
    void setNameFilters(const QString &filters);
    /**
     * Sets a filter for the file formats which shall be included in the collection.
     * Each name filter in the list is a wildcard (globbing)
     * filter that understands * and ? wildcards (see QDir::setNameFilters)
     */
    void setNameFilters(const QStringList &filters);

    /**
     * Carries out a full scan (for new albums + new pictures,
     * stale albums, stale pictures) on the given path.
     * @param filePath a folder somewhere in the collection
     */
    void scan(const QString &filePath);
    /**
     * Same procedure as above, but albumRoot and album is provided.
     */
    void scan(const QString &albumRoot, const QString& album);
    /**
     * Scans all album roots (for new albums + new pictures).
     * Does not carry out the full scan, you need to call
     * scanForStaleAlbums, removeStaleAlbums, removeStaleFiles yourself.
     * Emits totalFilesToScan, startScanningAlbum, finishedScanningAlbum.
     */
    void scanAlbums();

    /**
     * Scans the album and all subalbums on the given path.
     * (for new albums + new pictures)
     */
    void scanAlbum(const QString& filePath);
    /**
     * Scans the album and all subalbums on given album/albumRoot.
     * (for new albums + new pictures)
     */
    void scanAlbum(const QString &albumRoot, const QString& album);

    /**
     * Scans all albums roots for stale albums, i.e. albums
     * found in the db for which the corresponding folder on
     * the disk does no longer exist.
     */
    void scanForStaleAlbums();
    /**
     * Scans the given albums root for stale albums, like the method above.
     */
    void scanForStaleAlbums(const QString &albumRoot);
    /**
     * Returns a list of the stale albums found by scanForStaleAlbums.
     */
    QStringList formattedListOfStaleAlbums();
    /**
     * Removes the stale albums found by scanForStaleAlbums from the database.
     */
    void removeStaleAlbums();

    /**
     * Returns a list of the stale files found by
     * one of the scan/scanAlbum/scanAlbums methods.
     * A stale file is a file which does not longer exist on disk,
     * but still has an entry in the database.
     */
    QStringList formattedListOfStaleFiles();
    /**
     * Remove the found stale files from the database.
     * See above for a definition of "stale files".
     */
    void removeStaleFiles();

    /**
     * Finds all items for which the date in not contained in the database,
     * scan them and insert the date into the db.
     */
    void updateItemsWithoutDate();

    // Tools
    /**
     * Adds an item with the given file name found in the album pointed to by
     * albumID, albumRoot, album name to the database.
     * This method should always be used when inserting new files found on disk.
     * It reads metadata from the file and adds the data into the db.
     */
    static void addItem(int albumID, const QString& albumRoot, const QString &album, const QString &name);
    static void addItem(DatabaseAccess &access, int albumID,
                        const QString& albumRoot, const QString &album, const QString &name);
    /**
     * Updates the date field of the item. See above for parameter description.
     */
    static void updateItemDate(int albumID, const QString& albumRoot, const QString &album, const QString &fileName);
    static void updateItemDate(Digikam::DatabaseAccess &access, int albumID,
                               const QString& albumRoot, const QString &album, const QString &fileName);
#endif

Q_SIGNALS:

    /**
     * Emitted once in scanAlbums(), the scan() methods, and updateItemsWithoutDate().
     * Gives the number of the files that need to be scanned.
     */
    void totalFilesToScan(int count);

    /**
     * Notifies the begin of the scanning of the specified album root,
     * album, of stale files, or of the whole collection (after stale files)
     */
    void startScanningAlbumRoot(const QString &albumRoot);
    void startScanningAlbum(const QString &albumRoot, const QString &album);
    void startScanningForStaleAlbums();
    void startScanningAlbumRoots();
    void startCompleteScan();

    /**
     * Emitted when the scanning has finished.
     */
    void finishedScanningAlbumRoot(const QString &albumRoot);
    void finishedScanningAlbum(const QString &albumRoot, const QString &album, int filesScanned);
    void finishedScanningForStaleAlbums();
    void finishedCompleteScan();

protected:

    void markDatabaseAsScanned();
    void updateRemovedItemsTime();
    void incrementDeleteRemovedCompleteScanCount();
    void resetDeleteRemovedSettings();
    bool checkDeleteRemoved();
    void loadNameFilters();
    int countItemsInFolder(const QString& directory);
    DatabaseItem::Category category(const QFileInfo &info);

private:

    CollectionScannerPriv* const d;
};

}  // namespace Digikam

#endif // COLLECTIONSCANNER_H
