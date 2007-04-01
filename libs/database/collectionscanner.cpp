/* ============================================================
 * Authors: Marcel Wiesweg
 *          Renchi RajuMarcel Wiesweg
 * Date   : 2007-03-21
 * Description : database interface.
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
#include "collectionscanner.h"
#include "databaseaccess.h"
#include "ddebug.h"
#include "dmetadata.h"

namespace Digikam
{

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
            subURL = access.db()->escapeString( subURL);
            access.db()->execSql( QString("SELECT url FROM Albums WHERE ") +
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
                access.db()->execSql(QString("INSERT INTO Albums (url, date) "
                                             "VALUES('%1', '%2')")
                                     .arg(access.db()->escapeString(*it),
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
            access.db()->execSql(QString("SELECT id FROM Albums WHERE url='%1'")
                                         .arg(access.db()->escapeString(album)), &values );
            if (values.isEmpty())
                return;

            albumID = values.first().toInt();

            access.db()->execSql( QString("SELECT name FROM Images WHERE dirid=%1")
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
    access.db()->execSql(QString("SELECT url FROM Albums;"),
                         &urlList);

    access.db()->execSql("BEGIN TRANSACTION");

    struct stat stbuf;

    for (QStringList::iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        if (::stat(QFile::encodeName(albumRoot + *it), &stbuf) == 0)
            continue;

        DDebug() << "Deleted Album: " << *it << endl;
        access.db()->execSql(QString("DELETE FROM Albums WHERE url='%1'")
                             .arg(access.db()->escapeString(*it)));
    }

    access.db()->execSql("COMMIT TRANSACTION");
}

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


