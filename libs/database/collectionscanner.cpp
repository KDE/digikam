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

// System includes

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Qt includes

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>

// Local includes

#include "albumdb.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "databasetransaction.h"
#include "ddebug.h"
#include "dmetadata.h"
#include "collectionscanner.h"
#include "collectionscanner.moc"

namespace Digikam
{

// ------------------- CollectionScanner code -------------------------

void CollectionScanner::scanForStaleAlbums()
{
    QStringList albumRootPaths = CollectionManager::componentData().allAvailableAlbumRootPaths();
    for (QStringList::iterator it = albumRootPaths.begin(); it != albumRootPaths.end(); ++it)
        scanForStaleAlbums(*it);
}

void CollectionScanner::scanForStaleAlbums(const QString &albumRoot)
{
    Q_UNUSED(albumRoot);
    Q3ValueList<AlbumShortInfo> albumList = DatabaseAccess().db()->getAlbumShortInfos();
    Q3ValueList<AlbumShortInfo> toBeDeleted;

    for (Q3ValueList<AlbumShortInfo>::iterator it = albumList.begin(); it != albumList.end(); ++it)
    {
        QFileInfo fileInfo((*it).albumRoot + (*it).url);
        if (!fileInfo.exists() || !fileInfo.isDir())
            m_foldersToBeDeleted << (*it);
    }
}

QStringList CollectionScanner::formattedListOfStaleAlbums()
{
    QStringList list;
    for (Q3ValueList<AlbumShortInfo>::iterator it = m_foldersToBeDeleted.begin(); it != m_foldersToBeDeleted.end(); ++it)
    {
        list << (*it).url;
    }
    return list;
}

void CollectionScanner::removeStaleAlbums()
{
    DatabaseAccess access;
    DatabaseTransaction transaction(&access);
    for (Q3ValueList<AlbumShortInfo>::iterator it = m_foldersToBeDeleted.begin(); it != m_foldersToBeDeleted.end(); ++it)
    {
        DDebug() << "Removing album " << (*it).albumRoot + '/' + (*it).url << endl;
        access.db()->deleteAlbum((*it).id);
    }
}

QStringList CollectionScanner::formattedListOfStaleFiles()
{
    QStringList listToBeDeleted;

    DatabaseAccess access;
    for (Q3ValueList< QPair<QString,int> >::iterator it = m_filesToBeDeleted.begin();
        it != m_filesToBeDeleted.end(); ++it)
    {
        QString location = " (" + access.db()->getAlbumURL((*it).second) + ')';

        listToBeDeleted.append((*it).first + location);
    }

    return listToBeDeleted;
}

void CollectionScanner::removeStaleFiles()
{
    DatabaseAccess access;
    DatabaseTransaction transaction(&access);
    for (Q3ValueList< QPair<QString,int> >::iterator it = m_filesToBeDeleted.begin();
         it != m_filesToBeDeleted.end(); ++it)
    {
        DDebug() << "Removing: " << (*it).first << " in "
                << (*it).second << endl;
        access.db()->deleteItem( (*it).second, (*it).first );
    }
}

void CollectionScanner::scanAlbums()
{
    QStringList albumRootPaths = CollectionManager::componentData().allAvailableAlbumRootPaths();
    int count = 0;
    for (QStringList::iterator it = albumRootPaths.begin(); it != albumRootPaths.end(); ++it)
        count += countItemsInFolder(*it);

    emit totalFilesToScan(count);

    for (QStringList::iterator it = albumRootPaths.begin(); it != albumRootPaths.end(); ++it)
    {
        QDir dir(*it);
        QStringList fileList(dir.entryList(QDir::Dirs));

        DatabaseTransaction transaction;
        for (QStringList::iterator fileIt = fileList.begin(); fileIt != fileList.end(); ++fileIt)
        {
            if ((*fileIt) == "." || (*fileIt) == "..")
                continue;

            scanAlbum(*it, '/' + (*fileIt));
        }
    }
}

void CollectionScanner::scan(const QString& folderPath)
{
    CollectionManager *manager = CollectionManager::componentData();
    KUrl url;
    url.setPath(folderPath);
    QString albumRoot = manager->albumRootPath(url);
    QString album = manager->album(url);

    if (albumRoot.isNull())
    {
        DWarning() << "scanAlbums(QString): folder " << folderPath << " not found in collection." << endl;
        return;
    }

    scan(albumRoot, album);
}

void CollectionScanner::scan(const QString &albumRoot, const QString& album)
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
        QStringList fileList(dir.entryList(QDir::Dirs));

        DatabaseTransaction transaction;
        for (QStringList::iterator fileIt = fileList.begin(); fileIt != fileList.end(); ++fileIt)
        {
            if ((*fileIt) == "." || (*fileIt) == "..")
                continue;

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
    scanAlbum(CollectionManager::componentData().albumRootPath(url), CollectionManager::componentData().album(url));
}

void CollectionScanner::scanAlbum(const QString &albumRoot, const QString& album)
{
    // + Adds album if it does not yet exist in the db.
    // + Recursively scans subalbums of album.
    // + Adds files if they do not yet exist in the db.
    // + Adds stale files from the db to m_filesToBeDeleted
    // - Does not add stale albums to m_foldersToBeDeleted.

    QDir dir( albumRoot + album );
    if ( !dir.exists() or !dir.isReadable() )
    {
        DWarning() << "Folder does not exist or is not readable: "
                    << dir.path() << endl;
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

    // Qt4: use QSet<QString>
    QMap<QString, bool> filesFoundInDB;

    for (QStringList::iterator it = filesInAlbum.begin();
         it != filesInAlbum.end(); ++it)
    {
        filesFoundInDB.insert(*it, true);
    }

    const QFileInfoList *list = dir.entryInfoList();
    // Qt4: remove if-clause
    if (!list)
    {
        emit finishedScanningAlbum(albumRoot, album, 0);
        return;
    }

    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    while ( (fi = it.current()) != 0 )
    {
        if ( fi->isFile())
        {
            if (filesFoundInDB.contains(fi->fileName()) )
            {
                filesFoundInDB.erase(fi->fileName());
            }
            // ignore temp files we created ourselves
            else if (fi->extension(true) == "digikamtempfile.tmp")
            {
                continue;
            }
            else
            {
                DDebug() << "Adding item " << fi->fileName() << endl;
                addItem(albumID, albumRoot, album, fi->fileName());
            }
        }
        else if ( fi->isDir() && fi->fileName() != "." && fi->fileName() != "..")
        {
            scanAlbum( albumRoot, album + '/' + fi->fileName() );
        }

        ++it;
    }

    // Removing items from the db which we did not see on disk.
    if (!filesFoundInDB.isEmpty())
    {
        QMapIterator<QString,bool> it;
        for (it = filesFoundInDB.begin(); it != filesFoundInDB.end(); ++it)
        {
            if (m_filesToBeDeleted.findIndex(qMakePair(it.key(),albumID)) == -1)
            {
                m_filesToBeDeleted.append(qMakePair(it.key(),albumID));
            }
        }
    }

    emit finishedScanningAlbum(albumRoot, album, list->count());
}

void CollectionScanner::updateItemsWithoutDate()
{
    QStringList urls = DatabaseAccess().db()->getAllItemURLsWithoutDate();

    emit totalFilesToScan(urls.count());

    QString albumRoot = DatabaseAccess::albumRoot();

    {
        DatabaseTransaction transaction;
        for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
        {
            emit scanningFile(*it);

            QFileInfo fi(*it);
            QString albumURL = fi.path();
            albumURL = QDir::cleanPath(albumURL.remove(albumRoot));

            int albumID = DatabaseAccess().db()->getAlbumForPath(albumRoot, albumURL);

            if (albumID <= 0)
            {
                DWarning() << "Album ID == -1: " << albumURL << endl;
            }

            if (fi.exists())
            {
                CollectionScanner::updateItemDate(albumID, albumRoot, albumURL, fi.fileName());
            }
            else
            {
                QPair<QString, int> fileID = qMakePair(fi.fileName(), albumID);

                if (m_filesToBeDeleted.findIndex(fileID) == -1)
                {
                    m_filesToBeDeleted.append(fileID);
                }
            }
        }
    }
}

int CollectionScanner::countItemsInFolder(const QString& directory)
{
    int items = 0;

    QDir dir( directory );
    if ( !dir.exists() or !dir.isReadable() )
        return 0;

    const QFileInfoList *list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    items += list->count();

    while ( (fi = it.current()) != 0 )
    {
        if ( fi->isDir() &&
             fi->fileName() != "." &&
             fi->fileName() != "..")
        {
            items += countItemsInFolder( fi->filePath() );
        }

        ++it;
    }

    return items;
}

void CollectionScanner::markDatabaseAsScanned()
{
    DatabaseAccess access;
    access.db()->setSetting("Scanned", QDateTime::currentDateTime().toString(Qt::ISODate));
}


// ------------------- Tools ------------------------

void CollectionScanner::addItem(int albumID, const QString& albumRoot, const QString &album, const QString &fileName)
{
    DatabaseAccess access;
    addItem(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::addItem(Digikam::DatabaseAccess &access, int albumID,
                                const QString& albumRoot, const QString &album, const QString &fileName)
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

    keywords = metadata.getImageKeywords();

    access.db()->addItem(albumID, fileName, datetime, comment, rating, keywords);
}

void CollectionScanner::updateItemDate(int albumID, const QString& albumRoot, const QString &album, const QString &fileName)
{
    DatabaseAccess access;
    updateItemDate(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::updateItemDate(Digikam::DatabaseAccess &access, int albumID,
                                 const QString& albumRoot, const QString &album, const QString &fileName)
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

void CollectionScanner::scanAlbum(const QString &albumRoot, const QString &album)
{
    scanOneAlbum(albumRoot, album);
    removeInvalidAlbums(albumRoot);
}

void CollectionScanner::scanOneAlbum(const QString &albumRoot, const QString &album)
{
    DDebug() << "CollectionScanner::scanOneAlbum " << albumRoot << "/" << album << endl;
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
        DDebug() << "currAlbumList is " << currAlbumList << endl;

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
            DDebug() << "New Album: " << *it << endl;

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

void CollectionScanner::removeInvalidAlbums(const QString &albumRoot)
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
        DDebug() << "Removing album " << (*it).albumRoot + '/' + (*it).url << endl;
        access.db()->deleteAlbum((*it).id);
    }
}
*/



}


