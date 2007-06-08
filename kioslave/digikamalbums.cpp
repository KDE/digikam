/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process file operations on 
 *               digiKam albums. 
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *
 * Lots of the file io code is copied from KDE file kioslave.
 * Copyright for the KDE file kioslave follows:
 *  Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2000-2002 David Faure <faure@kde.org>
 *  Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>
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

#define MAX_IPC_SIZE (1024*32)

// C Ansi includes.

extern "C" 
{
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
}

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cerrno>

// Qt includes.

#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qdir.h>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kio/global.h>
#include <kio/ioslave_defaults.h>
#include <klargefile.h>
#include <kdeversion.h>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>

// Local includes.

#include "albumdb.h"
#include "digikam_export.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "collectionscanner.h"
#include "digikamalbums.h"
#include "imagelister.h"

kio_digikamalbums::kio_digikamalbums(const QCString &pool_socket,
                                     const QCString &app_socket)
    : SlaveBase("kio_digikamalbums", pool_socket, app_socket)
{
}

kio_digikamalbums::~kio_digikamalbums()
{
}

// ------------------------ Listing and Scanning ------------------------ //

void kio_digikamalbums::special(const QByteArray& data)
{
    KURL    kurl;
    QString filter;
    int     getDimensions;
    int     scan = 0;

    QDataStream ds(data, IO_ReadOnly);
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;
    if (!ds.atEnd())
        ds >> scan;

    kdDebug() << "kio_digikamalbums::special " << kurl << endl;

    Digikam::DatabaseUrl dbUrl(kurl);
    Digikam::DatabaseAccess::setParameters(dbUrl);

    if (scan)
    {
        Digikam::CollectionScanner scanner;
        scanner.scan(dbUrl.albumRootPath(), dbUrl.album());
        finished();
        return;
    }

    Digikam::ImageLister lister;
    Digikam::ImageListerSlaveBaseReceiver receiver(this);
    lister.list(&receiver, kurl, filter, getDimensions);
    receiver.sendData();

    finished();
}

// ------------------------ Implementation of KIO::SlaveBase ------------------------ //

void kio_digikamalbums::get( const KURL& url )
{
    kdDebug() << k_funcinfo << " : " << url << endl;

    // no need to open the db. we don't need to read/write to it

    Digikam::DatabaseUrl dbUrl(url);

    if (!file_get(dbUrl.fileUrl()))
        return;

    finished();
}

void kio_digikamalbums::put(const KURL& url, int permissions, bool overwrite, bool resume)
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;

    Digikam::DatabaseUrl dbUrl(url);
    Digikam::DatabaseAccess::setParameters(dbUrl);
    Digikam::DatabaseAccess access;

    // get the parent album
    int albumID = access.db()->getAlbumForPath(dbUrl.albumRootPath(), dbUrl.album(), false);
    if (albumID == -1)
    {
        error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
              .arg(url.directory()));
        return;
    }

    if (!file_put(dbUrl.fileUrl(), permissions, overwrite, resume))
        return;

    // First check if the file is already in database
    if (access.db()->getImageId(albumID, url.fileName()) == -1)
    {
        // Now insert the file into the database
        Digikam::CollectionScanner::addItem(access, albumID, dbUrl.albumRootPath(), dbUrl.album(), dbUrl.name());
    }
    
    // We have done our job => finish
    finished();
}

void kio_digikamalbums::copy( const KURL &src, const KURL &dst, int mode, bool overwrite )
{
    kdDebug() << k_funcinfo << "Src: " << src.path() << ", Dst: " << dst.path()   << endl;

    Digikam::DatabaseUrl dbUrlSrc(src);
    Digikam::DatabaseUrl dbUrlDst(dst);

    if (dbUrlSrc == dbUrlDst)
    {
        error( KIO::ERR_FILE_ALREADY_EXIST, dbUrlSrc.fileName() );
        return;
    }

    if (dbUrlSrc.parameters() != dbUrlDst.parameters())
    {
        error(KIO::ERR_UNKNOWN, "Database parameters of source and destination do not match.");
        return;
    }

    Digikam::DatabaseAccess::setParameters(dbUrlSrc);
    Digikam::DatabaseAccess access;

    // find the src parent album - do not create
    int srcAlbumID = access.db()->getAlbumForPath(dbUrlSrc.albumRootPath(), dbUrlSrc.album(), false);
    if (srcAlbumID == -1)
    {
        error(KIO::ERR_UNKNOWN, QString("Source album %1 not found in database")
              .arg(dbUrlSrc.album()));
        return;
    }

    // find the dst parent album - do not create
    int dstAlbumID = access.db()->getAlbumForPath(dbUrlDst.albumRootPath(), dbUrlDst.album(), false);
    if (dstAlbumID == -1)
    {
        error(KIO::ERR_UNKNOWN, QString("Destination album %1 not found in database")
              .arg(dbUrlDst.album()));
        return;
    }

    if (access.db()->getImageId(srcAlbumID, dbUrlSrc.fileName()) == -1)
    {
        error(KIO::ERR_UNKNOWN, i18n("Source image %1 not found in database")
                .arg(dbUrlSrc.fileName()));
        return;
    }

    // if the filename is .digikam_properties, we have been asked to copy the
    // metadata of the src album to the dst album
    if (src.fileName() == ".digikam_properties")
    {
        access.db()->copyAlbumProperties(srcAlbumID, dstAlbumID);
        finished();
        return;
    }

    if (!file_copy(dbUrlSrc.fileUrl(), dbUrlDst.fileUrl(), mode, overwrite))
        return;

    // now copy the metadata over
    access.db()->copyItem(srcAlbumID, dbUrlSrc.fileName(), dstAlbumID, dbUrlSrc.fileName());

    finished();
}

void kio_digikamalbums::rename( const KURL& src, const KURL& dst, bool overwrite )
{
    kdDebug() << k_funcinfo << "Src: " << src << ", Dst: " << dst   << endl;

    // if the filename is .digikam_properties fake that we renamed it
    if (src.fileName() == ".digikam_properties")
    {
        finished();
        return;
    }

    Digikam::DatabaseUrl dbUrlSrc(src);
    Digikam::DatabaseUrl dbUrlDst(dst);

    if (dbUrlSrc.parameters() != dbUrlDst.parameters())
    {
        error(KIO::ERR_UNKNOWN, "Database parameters of source and destination do not match.");
        return;
    }

    Digikam::DatabaseAccess::setParameters(dbUrlSrc);
    Digikam::DatabaseAccess access;

    // check if we are renaming an album or a image
    QFileInfo info(dbUrlSrc.fileUrl().path());
    bool renamingAlbum = info.isDir();

    int srcAlbumID, dstAlbumID = -1;

    if (renamingAlbum)
    {
        srcAlbumID = access.db()->getAlbumForPath(dbUrlSrc.albumRootPath(), dbUrlSrc.album(), false);
        if (srcAlbumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                  .arg(src.url()));
            return;
        }
    }
    else
    {
        srcAlbumID = access.db()->getAlbumForPath(dbUrlSrc.albumRootPath(), dbUrlSrc.album(), false);
        if (srcAlbumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                  .arg(src.directory()));
            return;
        }

        dstAlbumID = access.db()->getAlbumForPath(dbUrlDst.albumRootPath(), dbUrlDst.album(), false);
        if (dstAlbumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Destination album %1 not found in database")
                  .arg(dst.directory()));
            return;
        }
    }

    if (!file_rename(dbUrlSrc.fileUrl().path(), dbUrlDst.fileUrl(), overwrite))
        return;

    // renaming done. now update the database
    if (renamingAlbum)
    {
        // rename subalbums as well
        access.db()->renameAlbum(srcAlbumID, dbUrlDst.album(), true);
    }
    else
    {
        access.db()->moveItem(srcAlbumID, dbUrlSrc.fileName(),
                              dstAlbumID, dbUrlDst.fileName());
    }

    finished();
}

void kio_digikamalbums::mkdir( const KURL& url, int permissions )
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;

    Digikam::DatabaseUrl dbUrl(url);
    Digikam::DatabaseAccess::setParameters(dbUrl);
    Digikam::DatabaseAccess access;

    if (!file_mkdir(dbUrl.fileUrl(), permissions))
        return;

    access.db()->addAlbum(dbUrl.albumRootPath(), dbUrl.album(), QString(), QDate::currentDate(), QString());

    finished();
}

void kio_digikamalbums::chmod( const KURL& url, int permissions )
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;

    Digikam::DatabaseUrl dbUrl(url);

    if (!file_chmod(dbUrl.fileUrl(), permissions))
        return;

    finished();
}

void kio_digikamalbums::del( const KURL& url, bool isFile)
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;

    // if the filename is .digikam_properties fake that we deleted it
    if (isFile && url.fileName() == ".digikam_properties")
    {
        finished();
        return;
    }

    Digikam::DatabaseUrl dbUrl(url);
    Digikam::DatabaseAccess::setParameters(dbUrl);
    Digikam::DatabaseAccess access;

    int albumID;

    if (isFile)
    {
        // find the Album to which this file belongs.
        albumID = access.db()->getAlbumForPath(dbUrl.albumRootPath(), dbUrl.album(), false);
        if (albumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                    .arg(url.directory()));
            return;
        }
    }
    else
    {
        // find the corresponding album entry
        albumID = access.db()->getAlbumForPath(dbUrl.albumRootPath(), dbUrl.album(), false);
        if (albumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                  .arg(url.path()));
            return;
        }
    }

    if (!file_del(dbUrl.fileUrl(), isFile))
        return;

    if (isFile)
    {
        // successful deletion. now remove file entry from the database
        access.db()->deleteItem(albumID, url.fileName());
    }
    else
    {
        // successful deletion. now remove album entry from the database
        access.db()->deleteAlbum(albumID);
    }

    finished();
}

void kio_digikamalbums::stat( const KURL& url )
{
    Digikam::DatabaseUrl dbUrl(url);

    if (!file_stat(dbUrl.fileUrl()))
        return;

    finished();
}

void kio_digikamalbums::listDir( const KURL& url )
{
    kdDebug() << k_funcinfo << " : " << url.path() << endl;

    Digikam::DatabaseUrl dbUrl(url);

    KIO::UDSEntry entry;
    createDigikamPropsUDSEntry(entry);
    listEntry(entry, false);

    if (!file_listDir(dbUrl.fileUrl()))
        return;

    finished();
}

void kio_digikamalbums::createDigikamPropsUDSEntry(KIO::UDSEntry& entry)
{
    entry.clear();

    KIO::UDSAtom  atom;

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFREG;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = 00666;
    entry.append( atom );

    atom.m_uds = KIO::UDS_SIZE;
    atom.m_long = 0;
    entry.append( atom );       

    atom.m_uds = KIO::UDS_MODIFICATION_TIME;
    atom.m_long = QDateTime::currentDateTime().toTime_t();
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS_TIME;
    atom.m_long = QDateTime::currentDateTime().toTime_t();
    entry.append( atom );   

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = ".digikam_properties";
    entry.append(atom);
}

// ------------------------ Code that operates on files ------------------------ //

// This code is mostly duplicated from the file:// ioslave.
// When porting to KDE4, check if a chained ioslave is available to remove this here.

bool kio_digikamalbums::file_stat( const KURL& url )
{
    KIO::UDSEntry entry;
    if (!createUDSEntry(url.path(), entry))
    {
        error(KIO::ERR_DOES_NOT_EXIST, url.path(-1));
        return false;
    }

    statEntry(entry);
    return true;
}

bool kio_digikamalbums::file_listDir( const KURL& url )
{
    KDE_struct_stat stbuf;
    if (KDE_stat(QFile::encodeName(url.path()), &stbuf) != 0)
    {
        error(KIO::ERR_DOES_NOT_EXIST, url.path(-1));
        return false;
    }

    QDir dir(url.path());
    if (!dir.isReadable())
    {
        error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path());
        return false;
    }

    const QFileInfoList *list = dir.entryInfoList(QDir::All|QDir::Hidden);
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    KIO::UDSEntry entry;
    while ((fi = it.current()) != 0)
    {
        if (fi->fileName() != "." && fi->fileName() != ".." || fi->extension(true) == "digikamtempfile.tmp")
        {
            createUDSEntry(fi->absFilePath(), entry);
            listEntry(entry, false);
        }
        ++it;
    }

    entry.clear();
    listEntry(entry, true);
    return true;
}

bool kio_digikamalbums::createUDSEntry(const QString& path, KIO::UDSEntry& entry)
{
    entry.clear();

    KDE_struct_stat stbuf;
    if (KDE_stat(QFile::encodeName(path), &stbuf) != 0)
        return false;

    KIO::UDSAtom  atom;

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = stbuf.st_mode & S_IFMT;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = stbuf.st_mode & 07777;
    entry.append( atom );

    atom.m_uds = KIO::UDS_SIZE;
    atom.m_long = stbuf.st_size;
    entry.append( atom );       

    atom.m_uds = KIO::UDS_MODIFICATION_TIME;
    atom.m_long = stbuf.st_mtime;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS_TIME;
    atom.m_long = stbuf.st_atime;
    entry.append( atom );   

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = QFileInfo(path).fileName();
    entry.append(atom);

    /*
    // If we provide the local path, a KIO::CopyJob will optimize away
    // the use of our custom digikamalbums:/ ioslave, which breaks
    // copying the database entry:
    // Disabling this as a temporary solution for bug #137282
    // This code is intended as a fix for bug #122653.
#if KDE_IS_VERSION(3,4,0)
    atom.m_uds = KIO::UDS_LOCAL_PATH;
    atom.m_str = path;
    entry.append(atom);
#endif
    */

    return true;
}

static int write_all(int fd, const char *buf, size_t len)
{
    while (len > 0)
    {
        ssize_t written = write(fd, buf, len);
        if (written < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        buf += written;
        len -= written;
    }
    return 0;
}

bool kio_digikamalbums::file_get(const KURL &url)
{
    QCString path(QFile::encodeName(url.path()));
    KDE_struct_stat buff;
    if ( KDE_stat( path.data(), &buff ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, url.url() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return false;
    }

    if ( S_ISDIR( buff.st_mode ) )
    {
        error( KIO::ERR_IS_DIRECTORY, url.url() );
        return false;
    }
    
    if ( !S_ISREG( buff.st_mode ) )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.url() );
        return false;
    }

    int fd = KDE_open( path.data(), O_RDONLY);
    if ( fd < 0 )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.url() );
        return false;
    }

    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work).
    KMimeType::Ptr mt = KMimeType::findByURL( url.path(), buff.st_mode, true);
    emit mimeType( mt->name() );

    totalSize( buff.st_size );

    char buffer[ MAX_IPC_SIZE ];
    QByteArray array;
    KIO::filesize_t processed_size = 0;

    while (1)
    {
        int n = ::read( fd, buffer, MAX_IPC_SIZE );
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            error( KIO::ERR_COULD_NOT_READ, url.url());
            close(fd);
            return false;
        }
        if (n == 0)
            break; // Finished

        array.setRawData(buffer, n);
        data( array );
        array.resetRawData(buffer, n);

        processed_size += n;
        processedSize( processed_size );
    }

    data( QByteArray() );
    close( fd );

    processedSize( buff.st_size );
    return true;
}

bool kio_digikamalbums::file_put(const KURL& url, int permissions, bool overwrite, bool /*resume*/)
{
    QCString _dest( QFile::encodeName(url.path()));

    // check if the original file exists and we are not allowed to overwrite it
    KDE_struct_stat buff;
    bool origExists = (KDE_lstat( _dest.data(), &buff ) != -1);
    if ( origExists && !overwrite)
    {
        if (S_ISDIR(buff.st_mode))
            error( KIO::ERR_DIR_ALREADY_EXIST, url.url() );
        else
            error( KIO::ERR_FILE_ALREADY_EXIST, url.url() );
        return false;
    }

    // get the permissions we are supposed to set
    mode_t initialPerms;
    if (permissions != -1)
        initialPerms = permissions | S_IWUSR | S_IRUSR;
    else
        initialPerms = 0666;

    // open the destination file
    int fd = KDE_open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialPerms);
    if ( fd < 0 )
    {
        kdWarning() << "####################### COULD NOT OPEN " << _dest << endl;
        if ( errno == EACCES )
            error( KIO::ERR_WRITE_ACCESS_DENIED, url.url() );
        else
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, url.url() );
        return false;
    }

    int result;
    
    // Loop until we get 0 (end of data)
    do
    {
        QByteArray buffer;
        dataReq();
        result = readData( buffer );

        if (result >= 0)
        {
            if (write_all( fd, buffer.data(), buffer.size()))
            {
                if ( errno == ENOSPC ) // disk full
                {
                    error( KIO::ERR_DISK_FULL, url.url());
                    result = -1; 
                }
                else
                {
                    kdWarning() << "Couldn't write. Error:" << strerror(errno) << endl;
                    error( KIO::ERR_COULD_NOT_WRITE, url.url());
                    result = -1;
                }
            }
        }
    }
    while ( result > 0 );

    // An error occurred deal with it.
    if (result < 0)
    {
        kdDebug() << "Error during 'put'. Aborting." << endl;

        close(fd);
        remove(_dest);
        return false;
    }

    // close the file
    if ( close(fd) )
    {
        kdWarning() << "Error when closing file descriptor:" << strerror(errno) << endl;
        error( KIO::ERR_COULD_NOT_WRITE, url.url());
        return false;
    }

    // set final permissions
    if ( permissions != -1 )
    {
        if (::chmod(_dest.data(), permissions) != 0)
        {
            // couldn't chmod. Eat the error if the filesystem apparently doesn't support it.
            if ( KIO::testFileSystemFlag( _dest, KIO::SupportsChmod ) )
                warning( i18n( "Could not change permissions for\n%1" ).arg( url.url() ) );
        }
    }

    // set modification time
    const QString mtimeStr = metaData( "modified" );
    if ( !mtimeStr.isEmpty() ) {
        QDateTime dt = QDateTime::fromString( mtimeStr, Qt::ISODate );
        if ( dt.isValid() ) {
            KDE_struct_stat dest_statbuf;
            if (KDE_stat( _dest.data(), &dest_statbuf ) == 0) {
                struct utimbuf utbuf;
                utbuf.actime = dest_statbuf.st_atime; // access time, unchanged
                utbuf.modtime = dt.toTime_t(); // modification time
                kdDebug() << k_funcinfo << "setting modtime to " << utbuf.modtime << endl;
                utime( _dest.data(), &utbuf );
            }
        }

    }

    return true;
}

bool kio_digikamalbums::file_copy( const KURL &src, const KURL &dst, int mode, bool overwrite )
{
    QCString _src( QFile::encodeName(src.path()));
    QCString _dst( QFile::encodeName(dst.path()));

    // stat the src file
    KDE_struct_stat buff_src;
    if ( KDE_stat( _src.data(), &buff_src ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, src.url() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, src.url() );
        return false;
    }

    // bail out if its a directory
    if ( S_ISDIR( buff_src.st_mode ) )
    {
        error( KIO::ERR_IS_DIRECTORY, src.url() );
        return false;
    }

    // bail out if its a socket or fifo
    if ( S_ISFIFO( buff_src.st_mode ) || S_ISSOCK ( buff_src.st_mode ) )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, src.url() );
        return false;
    } 

    // stat the dst file
    KDE_struct_stat buff_dest;
    bool dest_exists = ( KDE_lstat( _dst.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        // bail out if its a directory
        if (S_ISDIR(buff_dest.st_mode))
        {
            error( KIO::ERR_DIR_ALREADY_EXIST, dst.url() );
            return false;
        }

        // if !overwrite bail out
        if (!overwrite)
        {
            error( KIO::ERR_FILE_ALREADY_EXIST, dst.url() );
            return false;
        }

        // If the destination is a symlink and overwrite is true,
        // remove the symlink first to prevent the scenario where
        // the symlink actually points to current source!
        if (overwrite && S_ISLNK(buff_dest.st_mode))
        {
            remove( _dst.data() );
        }
    }

    // now open the src file
    int src_fd = KDE_open( _src.data(), O_RDONLY);
    if ( src_fd < 0 )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, src.path() );
        return false;
    }

    // get the permissions we are supposed to set
    mode_t initialMode;
    if (mode != -1)
        initialMode = mode | S_IWUSR;
    else
        initialMode = 0666;

    // open the destination file
    int dest_fd = KDE_open(_dst.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
    if ( dest_fd < 0 )
    {
        kdDebug() << "###### COULD NOT WRITE " << dst.url() << endl;
        if ( errno == EACCES )
        {
            error( KIO::ERR_WRITE_ACCESS_DENIED, dst.url() );
        }
        else
        {
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, dst.url() );
        }
        close(src_fd);
        return false;
    }

    // emit the total size for copying
    totalSize( buff_src.st_size );

    KIO::filesize_t processed_size = 0;
    char buffer[ MAX_IPC_SIZE ];
    int n;

    while (1)
    {
        // read in chunks of MAX_IPC_SIZE 
        n = ::read( src_fd, buffer, MAX_IPC_SIZE );

        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            error( KIO::ERR_COULD_NOT_READ, src.path());
            close(src_fd);
            close(dest_fd);
            return false;
        }
       
        // Finished ?
        if (n == 0)
            break; 

        // write to the destination file
        if (write_all( dest_fd, buffer, n))
        {
            close(src_fd);
            close(dest_fd);

            if ( errno == ENOSPC ) // disk full
            {
                error( KIO::ERR_DISK_FULL, dst.url());
                remove( _dst.data() );
            }
            else
            {
                kdWarning() << "Couldn't write[2]. Error:" << strerror(errno) << endl;
                error( KIO::ERR_COULD_NOT_WRITE, dst.url());
            }
            return false;
        }
       
        processedSize( processed_size );
    }

    
    close( src_fd );

    if (close( dest_fd))
    {
        kdWarning() << "Error when closing file descriptor[2]:" << strerror(errno) << endl;
        error( KIO::ERR_COULD_NOT_WRITE, dst.url());
        return false;
    }

    // set final permissions
    if ( mode != -1 )
    {
        if (::chmod(_dst.data(), mode) != 0)
        {
            // Eat the error if the filesystem apparently doesn't support chmod.
            if ( KIO::testFileSystemFlag( _dst, KIO::SupportsChmod ) )
                warning( i18n( "Could not change permissions for\n%1" ).arg( dst.url() ) );
        }
    }

    // copy access and modification time
    struct utimbuf ut;
    ut.actime = buff_src.st_atime;
    ut.modtime = buff_src.st_mtime;
    if ( ::utime( _dst.data(), &ut ) != 0 )
    {
        kdWarning() << QString::fromLatin1("Couldn't preserve access and modification time for\n%1")
            .arg( dst.url() ) << endl;
    }

    processedSize( buff_src.st_size );
    return true;
}

bool kio_digikamalbums::file_rename( const KURL& src, const KURL& dst, bool overwrite )
{
    QCString csrc( QFile::encodeName(src.path()));
    QCString cdst( QFile::encodeName(dst.path()));

    // stat the source file/folder
    KDE_struct_stat buff_src;
    if ( KDE_stat( csrc.data(), &buff_src ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, src.url() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, src.url() );
        return false;
    }

    // stat the destination file/folder
    KDE_struct_stat buff_dest;
    bool dest_exists = ( KDE_stat( cdst.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
            error( KIO::ERR_DIR_ALREADY_EXIST, dst.url() );
            return false;
        }

        if (!overwrite)
        {
            error( KIO::ERR_FILE_ALREADY_EXIST, dst.url() );
            return false;
        }
    }

    // actually rename the file/folder
    if ( ::rename(csrc.data(), cdst.data()))
    {
        if (( errno == EACCES ) || (errno == EPERM))
        {
            QFileInfo toCheck(src.path());
            if (!toCheck.isWritable())
                error( KIO::ERR_CANNOT_RENAME_ORIGINAL, src.path() );
            else
                error( KIO::ERR_ACCESS_DENIED, dst.path() );
        }
        else if (errno == EXDEV)
        {
            error( KIO::ERR_UNSUPPORTED_ACTION, i18n("This file/folder is on a different "
                                                     "filesystem through symlinks. "
                                                     "Moving/Renaming files between "
                                                     "them is currently unsupported "));
        }
        else if (errno == EROFS)
        { // The file is on a read-only filesystem
            error( KIO::ERR_CANNOT_DELETE, src.url() );
        }
        else {
            error( KIO::ERR_CANNOT_RENAME, src.url() );
        }
        return false;
    }

    return true;
}

bool kio_digikamalbums::file_mkdir( const KURL& url, int permissions )
{
    QCString _path( QFile::encodeName(url.path()));

    KDE_struct_stat buff;
    if ( KDE_stat( _path, &buff ) == -1 )
    {
        if ( ::mkdir( _path.data(), 0777 /*umask will be applied*/ ) != 0 )
        {
            if ( errno == EACCES )
            {
                error( KIO::ERR_ACCESS_DENIED, url.path() );
                return false;
            }
            else if ( errno == ENOSPC )
            {
                error( KIO::ERR_DISK_FULL, url.path() );
                return false;
            }
            else
            {
                error( KIO::ERR_COULD_NOT_MKDIR, url.path() );
                return false;
            }
        }
        else
        {
            if ( permissions != -1 )
            {
                if ( ::chmod( _path.data(), permissions ) == -1 )
                {
                    error( KIO::ERR_CANNOT_CHMOD, url.path() );
                    return false;
                }
            }
            return true;
        }
    }

    if ( S_ISDIR( buff.st_mode ) )
    {
        error( KIO::ERR_DIR_ALREADY_EXIST, url.path() );
        return false;
    }

    error( KIO::ERR_FILE_ALREADY_EXIST, url.path() );
    return false;
}

bool kio_digikamalbums::file_chmod( const KURL& url, int permissions )
{
    QCString path( QFile::encodeName(url.path()));
    if ( ::chmod( path.data(), permissions ) == -1 )
    {
        error( KIO::ERR_CANNOT_CHMOD, url.url() );
        return false;
    }
    else
        return true;
}

bool kio_digikamalbums::file_del( const KURL& url, bool isfile)
{
    QCString path( QFile::encodeName(url.path()));

    if (isfile)
    {
        kdDebug(  ) <<  "Deleting file "<< url.url() << endl;

        // actually delete the file
        if ( unlink( path.data() ) == -1 )
        {
            if ((errno == EACCES) || (errno == EPERM))
                error( KIO::ERR_ACCESS_DENIED, url.url());
            else if (errno == EISDIR)
                error( KIO::ERR_IS_DIRECTORY, url.url());
            else
                error( KIO::ERR_CANNOT_DELETE, url.url() );
            return false;
        }
    }
    else
    {
        kdDebug(  ) << "Deleting directory " << url.url() << endl;

        if ( ::rmdir( path.data() ) == -1 )
        {
            // TODO handle symlink delete

            if ((errno == EACCES) || (errno == EPERM))
            {
                error( KIO::ERR_ACCESS_DENIED, url.url());
                return false;
            }
            else
            {
                kdDebug() << "could not rmdir " << perror << endl;
                error( KIO::ERR_COULD_NOT_RMDIR, url.url() );
                return false;
            }
        }
    }

    return true;
}

/* KIO slave registration */

extern "C"
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalogue("digikam");
        KInstance instance( "kio_digikamalbums" );
        KGlobal::locale();
        
        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamalbums  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamalbums slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        return 0;
    }
}
