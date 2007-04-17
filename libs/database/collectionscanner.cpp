/* ============================================================
 * Authors: Marcel Wiesweg
 *          Renchi Raju
 *          Tom Albers <tomalbers@kde.nl>
 * Date   : 2007-03-21
 * Description : database interface.
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright 2005-2006 by Tom Albers
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

namespace Digikam
{


// ------------------------- Ioslave scanning code ----------------------------------

void CollectionScanner::scanAlbum(const QString &albumRoot, const QString &album)
{
    scanOneAlbum(albumRoot, album);
    removeInvalidAlbums(albumRoot);
}

void CollectionScanner::scanOneAlbum(const QString &albumRoot, const QString &album)
{
    QDir dir(albumRoot + album);
    if (!dir.exists() || !dir.isReadable())
    {
        return;
    }

    QString subURL = album;
    if (!album.endsWith("/"))
        subURL += '/';

    {
        // scan albums

        QStringList currAlbumList;

        {
            DatabaseAccess access;
            subURL = access.backend()->escapeString( subURL);
            access.backend()->execSql( QString("SELECT url FROM Albums WHERE ") +
                                       QString("url LIKE '") + subURL + QString("%' ") +
                                       QString("AND url NOT LIKE '") + subURL + QString("%/%' "),
                                       &currAlbumList );
        }

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

            QString u = QDir::cleanDirPath(album + '/' + fi->fileName());

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

            {
                DatabaseAccess access;
                access.backend()->execSql(QString("INSERT INTO Albums (url, date) "
                                                 "VALUES('%1', '%2')")
                                          .arg(access.backend()->escapeString(*it),
                                               fi.lastModified().date().toString(Qt::ISODate)));
            }

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
            access.backend()->execSql(QString("SELECT id FROM Albums WHERE url='%1'")
                                      .arg(access.backend()->escapeString(album)), &values );
            if (values.isEmpty())
                return;

            albumID = values.first().toInt();

            access.backend()->execSql( QString("SELECT name FROM Images WHERE dirid=%1")
                                       .arg(albumID), &currItemList );
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

    QStringList urlList;

    //Attention: When allowing multiple album roots
    access.backend()->execSql(QString("SELECT url FROM Albums;"),
                              &urlList);

    access.backend()->execSql("BEGIN TRANSACTION");

    struct stat stbuf;

    for (QStringList::iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        if (::stat(QFile::encodeName(albumRoot + *it), &stbuf) == 0)
            continue;

        DDebug() << "Deleted Album: " << *it << endl;
        access.backend()->execSql(QString("DELETE FROM Albums WHERE url='%1'")
                                  .arg(access.backend()->escapeString(*it)));
    }

    access.backend()->execSql("COMMIT TRANSACTION");
}


// ------------------- CollectionScanner code -------------------------

void CollectionScanner::scanForStaleAlbums()
{
    QStringList albumRootPaths = CollectionManager::instance()->allAvailableAlbumRootPaths();
    for (QStringList::iterator it = albumRootPaths.begin(); it != albumRootPaths.end(); ++it)
        scanForStaleAlbums(*it);
}

void CollectionScanner::scanForStaleAlbums(const QString &albumRoot)
{
    AlbumInfo::List aList = DatabaseAccess().db()->scanAlbums();

    for (AlbumInfo::List::iterator it = aList.begin(); it != aList.end(); ++it)
    {
        AlbumInfo info = *it;
        info.url = QDir::cleanDirPath(info.url);
        QFileInfo fi(albumRoot + info.url);
        if (!fi.exists() || !fi.isDir())
        {
            m_foldersToBeDeleted[info.url] = info.id;
        }
    }
}

QStringList CollectionScanner::formattedListOfStaleAlbums()
{
    return m_foldersToBeDeleted.keys();
}

void CollectionScanner::removeStaleAlbums()
{
    DatabaseAccess access;
    for (QMap<QString, int>::const_iterator it = m_foldersToBeDeleted.begin() ; it != m_foldersToBeDeleted.end(); ++it)
    {
        DDebug() << "Removing Album: " << it.key() << endl;
        access.db()->deleteAlbum( it.data() );
    }
}

void CollectionScanner::scanAlbums()
{
    QStringList albumRootPaths = CollectionManager::instance()->allAvailableAlbumRootPaths();
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

            scanAlbumScanLib(*it, '/' + (*fileIt));
        }
    }
}

void CollectionScanner::scanAlbumScanLib(const QString& filePath)
{
    QString albumRoot = DatabaseAccess::albumRoot();
    QString album = filePath;
    album = QDir::cleanDirPath(album.remove(albumRoot));
    scanAlbumScanLib(albumRoot, album);
}

void CollectionScanner::scanAlbumScanLib(const QString &albumRoot, const QString& album)
{
    QDir dir( albumRoot + album );
    if ( !dir.exists() or !dir.isReadable() )
    {
        DWarning() << "Folder does not exist or is not readable: "
                    << dir.path() << endl;
        return;
    }

    emit startScanningAlbum(albumRoot, album);

    int albumID = DatabaseAccess().db()->getAlbumForPath(albumRoot, album, true);

    if (albumID <= 0)
    {
        DWarning() << "Album ID == -1: " << album << endl;
    }

    QStringList filesInAlbum = DatabaseAccess().db()->getItemNamesInAlbum( albumID );

    QMap<QString, bool> filesFoundInDB;

    for (QStringList::iterator it = filesInAlbum.begin();
         it != filesInAlbum.end(); ++it)
    {
        filesFoundInDB.insert(*it, true);
    }

    const QFileInfoList *list = dir.entryInfoList();
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
            else
            {
                DDebug() << "Adding item " << fi->fileName() << endl;
                addItem(albumID, albumRoot, album, fi->fileName());
            }
        }
        else if ( fi->isDir() && fi->fileName() != "." && fi->fileName() != "..")
        {
            scanAlbumScanLib( fi->filePath() );
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
            QString albumURL = fi.dirPath();
            albumURL = QDir::cleanDirPath(albumURL.remove(albumRoot));

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

QStringList CollectionScanner::formattedListOfStaleFiles()
{
    QStringList listToBeDeleted;

    DatabaseAccess access;
    for (QValueList< QPair<QString,int> >::iterator it = m_filesToBeDeleted.begin();
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
    for (QValueList< QPair<QString,int> >::iterator it = m_filesToBeDeleted.begin();
         it != m_filesToBeDeleted.end(); ++it)
    {
        DDebug() << "Removing: " << (*it).first << " in "
                << (*it).second << endl;
        access.db()->deleteItem( (*it).second, (*it).first );
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




}


