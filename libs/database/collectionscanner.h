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

// Qt includes

#include <QString>
#include <QObject>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "databaseaccess.h"
#include "albuminfo.h"

class QFileInfo;

namespace Digikam
{

class CollectionLocation;
class CollectionScannerObserver;
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
    void partialScan(const QString& filePath);

    /**
     * Same procedure as above, but albumRoot and album is provided.
     */
    void partialScan(const QString& albumRoot, const QString& album);

    /**
     * The specified file will be added to the database if it is not included,
     * or rescanned as if it was modified if it is found in the database.
     */
    void scanFile(const QString& filePath);

    /**
     * Same procedure as above, but albumRoot and album is provided.
     */
    void scanFile(const QString& albumRoot, const QString& album, const QString& fileName);

    /**
     * Call this to enable the progress info signals.
     * Default is off.
     */
    void setSignalsEnabled(bool on);

    /**
     * Record hints for the collection scanner.
     */
    void recordHints(const QList<AlbumCopyMoveHint>& hint);
    void recordHints(const QList<ItemCopyMoveHint>& hint);

    /**
     * Utility method:
     * Prepare the given albums to be removed,
     * typically by setting the albums as orphan
     * and removing all entries from the albums
     */
    void safelyRemoveAlbums(const QList<int>& albumIds);

    /**
     * Set an observer to be able to cancel a running scan
     */
    void setObserver(CollectionScannerObserver *observer);

    /**
     * Returns if the initial scan of the database has been done.
     * This is the first complete scan after creation of a new database file
     * (or update requiring a rescan)
     */
    static bool databaseInitialScanDone();

protected:

    void scanForStaleAlbums(QList<CollectionLocation> locations);
    void scanAlbumRoot(const CollectionLocation& location);
    void scanAlbum(const CollectionLocation& location, const QString& album);
    int checkAlbum(const CollectionLocation& location, const QString& album);
    void scanNewFile(const QFileInfo& info, int albumId);
    void scanModifiedFile(const QFileInfo& info, const ItemScanInfo& scanInfo);

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
    void startScanningAlbumRoot(const QString& albumRoot);
    void startScanningAlbum(const QString& albumRoot, const QString& album);
    void startScanningForStaleAlbums();
    void startScanningAlbumRoots();
    void startCompleteScan();

    /**
     * Emitted when the scanning has finished.
     */
    void finishedScanningAlbumRoot(const QString& albumRoot);
    void finishedScanningAlbum(const QString& albumRoot, const QString& album, int filesScanned);
    void finishedScanningForStaleAlbums();
    void finishedCompleteScan();
    /**
     * Emitted when the observer told to cancel the scan
     */
    void cancelled();

protected:

    void markDatabaseAsScanned();
    void updateRemovedItemsTime();
    void incrementDeleteRemovedCompleteScanCount();
    void resetDeleteRemovedSettings();
    bool checkDeleteRemoved();
    void loadNameFilters();
    int countItemsInFolder(const QString& directory);
    DatabaseItem::Category category(const QFileInfo& info);

private:

    CollectionScannerPriv* const d;
};

}  // namespace Digikam

#endif // COLLECTIONSCANNER_H
