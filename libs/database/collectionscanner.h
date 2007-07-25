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

namespace Digikam
{

class DIGIKAM_EXPORT CollectionScanner : public QObject
{
    Q_OBJECT
public:

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

    /**
     * Writes into the database that it has been scanned at this point in time.
     */
    void markDatabaseAsScanned();

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

signals:

    /**
     * Emitted once in scanAlbums(), the scan() methods, and updateItemsWithoutDate().
     * Gives the number of the files that need to be scanned.
     */
    void totalFilesToScan(int count);
    /**
     * Notifies the begin of the scanning of the specified album.
     * Emitted from all scan... methods.
     */
    void startScanningAlbum(const QString &albumRoot, const QString &album);
    /**
     * As above, when the scanning has finished
     */
    void finishedScanningAlbum(const QString &albumRoot, const QString &album, int filesScanned);

    /**
     * Emitted from updateItemDate when an item is updated.
     */
    void scanningFile(const QString &filePath);

protected:

    int countItemsInFolder(const QString& directory);

    QList< QPair<QString,int> >  m_filesToBeDeleted;
    QList<AlbumShortInfo> m_foldersToBeDeleted;
};

}  // namespace Digikam

#endif // COLLECTIONSCANNER_H

