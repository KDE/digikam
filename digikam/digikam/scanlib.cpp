////////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMAPP.CPP
//
//    Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////


#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kprogress.h>
#include <kapplication.h>
#include <klocale.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>

extern "C" {
#include <sys/time.h>
#include <time.h>
}

#include "albumdb.h"
#include "albummanager.h"
#include "scanlib.h"

/** @file scanlib.cpp*/

ScanLib::ScanLib()
{
    m_progressBar = new KProgressDialog(0);
    m_progressBar->setInitialSize(QSize(300,100), true);
    m_progressBar->hide();
    QWhatsThis::add( m_progressBar, i18n("This shows the progress of the "
        "scanning. During the scanning all files on disk are put in a "
        "database. This is needed for sorting on exif-date and speeds up "
        "overall performance of digiKam.") );
}

ScanLib::~ScanLib()
{
    delete m_progressBar;
}

void ScanLib::findMissingItems()
{
    struct timeval tv1, tv2, tv3;
    gettimeofday(&tv1, 0);

    QString albumPath =
            QDir::cleanDirPath(AlbumManager::instance()->getLibraryPath());
    m_progressBar->setAllowCancel( false );
    m_progressBar->showCancelButton (false );
    m_progressBar->progressBar()->setProgress( 0 );
    m_progressBar->setLabel(i18n("Scanning items..."));
    m_progressBar->progressBar()->
            setTotalSteps( countItemsInFolder( albumPath ) );
    m_progressBar->show();
    kapp->processEvents();

    gettimeofday(&tv2, 0);

    allFiles( albumPath );

    m_progressBar->hide();
    kapp->processEvents();

    gettimeofday(&tv3, 0);

    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->setSetting("Scanned",
                   QDateTime::currentDateTime().toString(Qt::ISODate));

    kdDebug() << "Count all files took: time taken: "
              << (((tv2.tv_sec-tv1.tv_sec)*1000000 +
                   (tv2.tv_usec-tv1.tv_usec))/1000)
              << " ms" << endl;
    kdDebug() << "Finding Missing Items: time taken: "
            << (((tv3.tv_sec-tv2.tv_sec)*1000000 +
                 (tv3.tv_usec-tv2.tv_usec))/1000)
            << " ms" << endl;
}

void ScanLib::updateItemsWithoutDate()
{
    struct timeval tv1, tv2;
    gettimeofday(&tv1, 0);

    AlbumDB* db = AlbumManager::instance()->albumDB();
    QStringList urls = db->getAllItemURLsWithoutDate();

    if (urls.isEmpty())
    {
        m_progressBar->progressBar()->setTotalSteps(1);
        m_progressBar->progressBar()->setProgress(1);
        m_progressBar->hide();
        return;
    }

    m_progressBar->setAllowCancel( false );
    m_progressBar->showCancelButton (false );
    m_progressBar->progressBar()->setProgress(0);
    m_progressBar->progressBar()->setTotalSteps(urls.count());
    m_progressBar->setLabel(i18n("Updating items..."));
    m_progressBar->show();
    kapp->processEvents();

    QString base = QDir::cleanDirPath(AlbumManager::instance()->getLibraryPath());

    int counter=0;
    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        m_progressBar->progressBar()->advance(1);
        ++counter;
        if ( counter % 30 == 0 )
            kapp->processEvents();
        QFileInfo fi(*it);
        QString albumURL = fi.dirPath();
        albumURL = QDir::cleanDirPath(albumURL.remove(base));
        int albumID = db->getOrCreateAlbumId(albumURL);
        if (fi.exists())
            updateItemDate(albumURL, fi.fileName(), albumID);
        else
        {
            kdDebug() << "Stale: " << fi.fileName() << " in " << albumID << endl;
            db->deleteItem( albumID, fi.fileName() );
        }
    }

    m_progressBar->hide();
    kapp->processEvents();

    gettimeofday(&tv2, 0);

    kdDebug() << "Updating items date: time taken: "
              << (((tv2.tv_sec-tv1.tv_sec)*1000000 +
                   (tv2.tv_usec-tv1.tv_usec))/1000)
              << " ms" << endl;
}

int ScanLib::countItemsInFolder(const QString& directory)
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
        if ( fi->isDir() && fi->fileName() != "." && fi->fileName() != "..")
            items += countItemsInFolder( fi->filePath() );
        ++it;
    }
    return items;
}

void ScanLib::allFiles(const QString& directory)
{
    QDir dir( directory );
    if ( !dir.exists() or !dir.isReadable() )
    {
        kdWarning() << "Folder does not exist or is not readable: "
                    << directory << endl;
        return;
    }

    QString base = QDir::cleanDirPath(AlbumManager::instance()->getLibraryPath());
    QString albumURL = directory;
    albumURL = QDir::cleanDirPath(albumURL.remove(base));

    AlbumDB* db = AlbumManager::instance()->albumDB();
    int albumID = db->getOrCreateAlbumId(albumURL);
    QStringList filesInAlbum = db->getItemNamesInAlbum( albumID );

    QMap<QString, bool> filesFoundInDB;
    for (QStringList::iterator it = filesInAlbum.begin();
         it != filesInAlbum.end(); ++it)
    {
        if (albumURL.isEmpty())
        {
            kdDebug() << "Root item found: " << *it << " in " << albumID << endl;
            db->deleteItem( albumID, *it );
        }
        else
            filesFoundInDB.insert(*it, true);
    }

    const QFileInfoList *list = dir.entryInfoList();
    if (!list)
        return;

    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    m_progressBar->progressBar()->advance(list->count());
    kapp->processEvents();

    while ( (fi = it.current()) != 0 )
    {
        if ( fi->isFile())
            if (filesFoundInDB.contains(fi->fileName()) )
                filesFoundInDB.erase(fi->fileName());
            else
                storeItemInDatabase(albumURL, fi->fileName(), albumID);
        else if ( fi->isDir() && fi->fileName() != "." && fi->fileName() != "..")
            allFiles( fi->filePath() );
        ++it;
    }

    // Removing items from the db which we did not see on disk.
    QMapIterator<QString,bool> it2;
    for (it2 = filesFoundInDB.begin() ; it2 != filesFoundInDB.end(); ++it2)
    {
        kdDebug() << "Stale: " << it2.key() << " in " << albumID << endl;
        db->deleteItem( albumID, it2.key() );
    }
}

void ScanLib::storeItemInDatabase(const QString& albumURL,
                                  const QString& filename,
                                  int albumID)
{
    QString comment;
    QDateTime datetime;
    QDir albumPath( AlbumManager::instance()->getLibraryPath());

    // Do not store items found in the root of the albumdb
    if (albumURL.isEmpty())
        return;
    KFileMetaInfo itemMetaInfo( albumPath.path()+albumURL+'/'+filename );

    if (itemMetaInfo.isValid() &&
        itemMetaInfo.containsGroup("Jpeg EXIF Data"))
    {
        comment = itemMetaInfo.group("Jpeg EXIF Data").
                  item("Comment").value().toString();
        datetime = itemMetaInfo.group("Jpeg EXIF Data").
                   item("Date/time").value().toDateTime();
    }
    
    if ( !datetime.isValid() )
    {
        QFileInfo info( albumPath.path()+albumURL+'/'+filename );
        datetime = info.lastModified();
    }

    AlbumDB* dbstore = AlbumManager::instance()->albumDB();
    dbstore->addItem(albumID, filename, datetime,comment);
}

void ScanLib::updateItemDate(const QString& albumURL,
                             const QString& filename,
                             int albumID)
{
    QDateTime datetime;
    QDir albumPath( AlbumManager::instance()->getLibraryPath());

    KFileMetaInfo itemMetaInfo( albumPath.path()+albumURL+'/'+filename );
    if (itemMetaInfo.isValid() &&
        itemMetaInfo.containsGroup("Jpeg EXIF Data"))
    {
        datetime = itemMetaInfo.group("Jpeg EXIF Data").
                   item("Date/time").value().toDateTime();
    }

    if ( !datetime.isValid() )
    {
        QFileInfo info( albumPath.path()+albumURL+'/'+filename );
        datetime = info.lastModified();
    }

    AlbumDB* dbstore = AlbumManager::instance()->albumDB();
    dbstore->setItemDate(albumID, filename, datetime);
}
