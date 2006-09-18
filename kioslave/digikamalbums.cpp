/* ============================================================
 * File  : digikamalbums.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju
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

#include <digikam_export.h>

#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kio/global.h>
#include <kio/ioslave_defaults.h>
#include <klargefile.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qdir.h>

extern "C" 
{
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <utime.h>
}

#include <dmetadata.h>
#include <rawfiles.h>

#include "sqlitedb.h"
#include "digikamalbums.h"

#define MAX_IPC_SIZE (1024*32)

kio_digikamalbums::kio_digikamalbums(const QCString &pool_socket,
                                     const QCString &app_socket)
    : SlaveBase("kio_digikamalbums", pool_socket, app_socket)
{
}

kio_digikamalbums::~kio_digikamalbums()
{
}

static QValueList<QRegExp> makeFilterList( const QString &filter )
{
    QValueList<QRegExp> regExps;
    if ( filter.isEmpty() )
        return regExps;

    QChar sep( ';' );
    int i = filter.find( sep, 0 );
    if ( i == -1 && filter.find( ' ', 0 ) != -1 )
        sep = QChar( ' ' );

    QStringList list = QStringList::split( sep, filter );
    QStringList::Iterator it = list.begin();
    while ( it != list.end() ) {
        regExps << QRegExp( (*it).stripWhiteSpace(), false, true );
        ++it;
    }
    return regExps;
}

static bool matchFilterList( const QValueList<QRegExp>& filters,
                             const QString &fileName )
{
    QValueList<QRegExp>::ConstIterator rit = filters.begin();
    while ( rit != filters.end() ) {
        if ( (*rit).exactMatch(fileName) )
            return true;
        ++rit;
    }
    return false;
}

void kio_digikamalbums::special(const QByteArray& data)
{
    QString libraryPath;
    KURL    kurl;
    QString url;
    QString filter;
    int     getDimensions;
    int     scan = 0;
    
    QDataStream ds(data, IO_ReadOnly);
    ds >> libraryPath;
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;
    if (!ds.atEnd())
        ds >> scan;

    libraryPath = QDir::cleanDirPath(libraryPath);
    
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        m_sqlDB.closeDB();
        m_sqlDB.openDB(libraryPath);
    }

    url = QDir::cleanDirPath(kurl.path());
    
    if (scan)
    {
        scanAlbum(url);
        finished();
        return;
    }
    
    QValueList<QRegExp> regex = makeFilterList(filter);
    
    QStringList values;
    m_sqlDB.execSql(QString("SELECT id FROM Albums WHERE url='%1';")
                    .arg(escapeString(url)), &values);
    int albumid = values.first().toInt();

    
    values.clear();
    m_sqlDB.execSql(QString("SELECT id, name, datetime FROM Images "
                                    "WHERE dirid = %1;")
                            .arg(albumid), &values);

    QByteArray  ba;
    QDataStream os(ba, IO_WriteOnly);
    
    QString base = libraryPath + url + '/';
    Q_LLONG id;
    QString name;
    QString date;
    QSize   dims;

    struct stat stbuf;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        id   = (*it).toLongLong();
        ++it;
        name = *it;
        ++it;
        date = *it;
        ++it;

        if (!matchFilterList(regex, name))
            continue;

        if (::stat(QFile::encodeName(base + name), &stbuf) != 0)
            continue;

        dims = QSize();
        if (getDimensions)
        {
            QString rawFilesExt(raw_file_extentions);

            QFileInfo fileInfo(base + name);
            if (rawFilesExt.upper().contains( fileInfo.extension().upper() ))
            {
                Digikam::DMetadata metaData(base + name);
                dims = metaData.getImageDimensions();
            }
            else
            {
                KFileMetaInfo metaInfo(base + name);
                if (metaInfo.isValid())
                {
                    if (metaInfo.containsGroup("Jpeg EXIF Data"))
                    {
                        dims = metaInfo.group("Jpeg EXIF Data").
                            item("Dimensions").value().toSize();
                    }
                    else if (metaInfo.containsGroup("General"))
                    {
                        dims = metaInfo.group("General").
                            item("Dimensions").value().toSize();
                    }
                    else if (metaInfo.containsGroup("Technical"))
                    {
                        dims = metaInfo.group("Technical").
                            item("Dimensions").value().toSize();
                    }
                }
            }
        }

        os << id;
        os << albumid;
        os << name;
        os << date;
        os << static_cast<size_t>(stbuf.st_size);
        os << dims;
    }

    SlaveBase::data(ba);
    
    finished();
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

void kio_digikamalbums::get( const KURL& url )
{
    kdDebug() << k_funcinfo << " : " << url << endl;    

    // get the libraryPath
    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    // no need to open the db. we don't need to read/write to it
    
    QCString path(QFile::encodeName(libraryPath + url.path()));
    KDE_struct_stat buff;
    if ( KDE_stat( path.data(), &buff ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, url.url() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }

    if ( S_ISDIR( buff.st_mode ) )
    {
        error( KIO::ERR_IS_DIRECTORY, url.url() );
        return;
    }
    
    if ( !S_ISREG( buff.st_mode ) )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.url() );
        return;
    }

    int fd = KDE_open( path.data(), O_RDONLY);
    if ( fd < 0 )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.url() );
        return;
    }

    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work).
    KMimeType::Ptr mt = KMimeType::findByURL( libraryPath + url.path(), buff.st_mode,
                                              true);
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
            return;
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
    finished();
}

void kio_digikamalbums::put(const KURL& url, int permissions, bool overwrite, bool /*resume*/)
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;

    // get the libraryPath
    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    // open the db if needed
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        m_sqlDB.closeDB();
        m_sqlDB.openDB(m_libraryPath);
    }

    // build the album list
    buildAlbumList();

    // get the parent album
    AlbumInfo album = findAlbum(url.directory());
    if (album.id == -1)
    {
        error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
              .arg(url.directory()));
        return;
    }
    
    
    QString dest = libraryPath + url.path();
    QCString _dest( QFile::encodeName(dest));

    // check if the original file exists and we are not allowed to overwrite it
    KDE_struct_stat buff;
    bool origExists = (KDE_lstat( _dest.data(), &buff ) != -1);
    if ( origExists && !overwrite)
    {
        if (S_ISDIR(buff.st_mode))
            error( KIO::ERR_DIR_ALREADY_EXIST, url.url() );
        else
            error( KIO::ERR_FILE_ALREADY_EXIST, url.url() );
        return;
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
        kdWarning() << "####################### COULD NOT OPEN " << dest << endl;
        if ( errno == EACCES )
            error( KIO::ERR_WRITE_ACCESS_DENIED, url.url() );
        else
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, url.url() );
        return;
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
        return;
    }

    // close the file
    if ( close(fd) )
    {
        kdWarning() << "Error when closing file descriptor:" << strerror(errno) << endl;
        error( KIO::ERR_COULD_NOT_WRITE, url.url());
        return;
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

    // First check if the file is already in database
    if (!findImage(album.id, url.fileName()))
    {
        // Now insert the file into the database
        addImage(album.id, m_libraryPath + url.path());
    }
    
    // We have done our job => finish
    finished();
}

void kio_digikamalbums::copy( const KURL &src, const KURL &dst, int mode, bool overwrite )
{
    kdDebug() << k_funcinfo << "Src: " << src.path() << ", Dst: " << dst.path()   << endl;        

    // get the album library path
    QString libraryPath = src.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    // check that the src and dst album library paths match
    QString dstLibraryPath = dst.user();
    if (libraryPath != dstLibraryPath)
    {
        error(KIO::ERR_UNKNOWN,
              QString("Source and Destination have different Album Library Paths. ") +
              QString("Src: ") + src.user() +
              QString(", Dest: ") + dst.user());
        return;
    }

    // open the db if needed
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        m_sqlDB.closeDB();
        m_sqlDB.openDB(m_libraryPath);
    }

    // build the album list
    buildAlbumList();

    // find the src parent album
    AlbumInfo srcAlbum = findAlbum(src.directory());
    if (srcAlbum.id == -1)
    {
        error(KIO::ERR_UNKNOWN, QString("Source album %1 not found in database")
              .arg(src.directory()));
        return;
    }

    // find the dst parent album
    AlbumInfo dstAlbum = findAlbum(dst.directory());
    if (dstAlbum.id == -1)
    {
        error(KIO::ERR_UNKNOWN, QString("Destination album %1 not found in database")
              .arg(dst.directory()));
        return;
    }

    // if the filename is .digikam_properties, we have been asked to copy the
    // metadata of the src album to the dst album
    if (src.fileName() == ".digikam_properties")
    {
        // copy metadata of album to destination album
        m_sqlDB.execSql( QString("UPDATE Albums SET date='%1', caption='%2', "
                                 "collection='%3', icon=%4 ")
                         .arg(srcAlbum.date.toString(Qt::ISODate),
                              escapeString(srcAlbum.caption),
                              escapeString(srcAlbum.collection),
                              QString::number(srcAlbum.icon)) +
                         QString( " WHERE id=%1" )
                         .arg(dstAlbum.id) );
        finished();
        return;
    }

    QCString _src( QFile::encodeName(libraryPath + src.path()));
    QCString _dst( QFile::encodeName(libraryPath + dst.path()));

    // stat the src file
    KDE_struct_stat buff_src;
    if ( KDE_stat( _src.data(), &buff_src ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, src.url() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, src.url() );
        return;
    }

    // bail out if its a directory
    if ( S_ISDIR( buff_src.st_mode ) )
    {
        error( KIO::ERR_IS_DIRECTORY, src.url() );
        return;
    }

    // bail out if its a socket or fifo
    if ( S_ISFIFO( buff_src.st_mode ) || S_ISSOCK ( buff_src.st_mode ) )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, src.url() );
        return;
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
            return;
        }

        // if !overwrite bail out
        if (!overwrite)
        {
            error( KIO::ERR_FILE_ALREADY_EXIST, dst.url() );
            return;
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
        return;
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
        return;
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
            return;
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
            return;
        }
       
        processedSize( processed_size );
    }

    
    close( src_fd );

    if (close( dest_fd))
    {
        kdWarning() << "Error when closing file descriptor[2]:" << strerror(errno) << endl;
        error( KIO::ERR_COULD_NOT_WRITE, dst.url());
        return;
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

    // now copy the metadata over
    copyImage(srcAlbum.id, src.fileName(), dstAlbum.id, dst.fileName());
    
    processedSize( buff_src.st_size );
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
    
    QString libraryPath = src.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    QString dstLibraryPath = dst.user();
    if (libraryPath != dstLibraryPath)
    {
        error(KIO::ERR_UNKNOWN,
              i18n("Source and Destination have different Album Library Paths.\n"
                   "Source: %1\n"
                   "Destination: %2")
              .arg(src.user())
              .arg(dst.user()));
        return;
    }

    // open album db if needed
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        m_sqlDB.closeDB();
        m_sqlDB.openDB(m_libraryPath);
    }

    QCString csrc( QFile::encodeName(libraryPath + src.path()));
    QCString cdst( QFile::encodeName(libraryPath + dst.path()));

    // stat the source file/folder
    KDE_struct_stat buff_src;
    if ( KDE_stat( csrc.data(), &buff_src ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, src.url() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, src.url() );
        return;
    }

    // stat the destination file/folder
    KDE_struct_stat buff_dest;
    bool dest_exists = ( KDE_stat( cdst.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
            error( KIO::ERR_DIR_ALREADY_EXIST, dst.url() );
            return;
        }

        if (!overwrite)
        {
            error( KIO::ERR_FILE_ALREADY_EXIST, dst.url() );
            return;
        }
    }

    
    // build album list
    buildAlbumList();

    AlbumInfo srcAlbum, dstAlbum;

    // check if we are renaming an album or a image
    bool renamingAlbum = S_ISDIR(buff_src.st_mode);

    if (renamingAlbum)
    {
        srcAlbum = findAlbum(src.path());
        if (srcAlbum.id == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                  .arg(src.url()));
            return;
        }
    }
    else
    {
        srcAlbum = findAlbum(src.directory());
        if (srcAlbum.id == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                  .arg(src.directory()));
            return;
        }

        dstAlbum = findAlbum(dst.directory());
        if (dstAlbum.id == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Destination album %1 not found in database")
                  .arg(dst.directory()));
            return;
        }
    }    

    // actually rename the file/folder
    if ( ::rename(csrc.data(), cdst.data()))
    {
        if (( errno == EACCES ) || (errno == EPERM))
        {
            QFileInfo toCheck(libraryPath + src.path());
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
        return;
    }

    // renaming done. now update the database
    if (renamingAlbum)
    {
        renameAlbum(srcAlbum.url, dst.path());
    }
    else
    {
        renameImage(srcAlbum.id, src.fileName(),
                    dstAlbum.id, dst.fileName());
    }

    finished();
}

void kio_digikamalbums::stat( const KURL& url )
{
    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }
    
    KIO::UDSEntry entry;
    if (!createUDSEntry(libraryPath + url.path(), entry))
    {
        error(KIO::ERR_DOES_NOT_EXIST, url.path(-1));
        return;
    }
    
    statEntry(entry);
    finished();
}

void kio_digikamalbums::listDir( const KURL& url )
{
    kdDebug() << k_funcinfo << " : " << url.path() << endl;            

    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        kdWarning() << "Album Library Path not supplied to kioslave" << endl;
        return;
    }

    KDE_struct_stat stbuf;
    QString path = libraryPath + url.path();
    if (KDE_stat(QFile::encodeName(path), &stbuf) != 0)
    {
        error(KIO::ERR_DOES_NOT_EXIST, url.path(-1));
        return;
    }

    QDir dir(path);
    if (!dir.isReadable())
    {
        error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path());
        return;
    }
    
    const QFileInfoList *list = dir.entryInfoList(QDir::All|QDir::Hidden);
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    KIO::UDSEntry entry;
    createDigikamPropsUDSEntry(entry);
    listEntry(entry, false);
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
    finished();
}

void kio_digikamalbums::mkdir( const KURL& url, int permissions )
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;

    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        m_sqlDB.closeDB();
        m_sqlDB.openDB(m_libraryPath);
    }
    
    QString   path = libraryPath + url.path();
    QCString _path( QFile::encodeName(path));
    
    KDE_struct_stat buff;
    if ( KDE_stat( _path, &buff ) == -1 )
    {
        if ( ::mkdir( _path.data(), 0777 /*umask will be applied*/ ) != 0 )
        {
            if ( errno == EACCES )
            {
                error( KIO::ERR_ACCESS_DENIED, path );
                return;
            }
            else if ( errno == ENOSPC )
            {
                error( KIO::ERR_DISK_FULL, path );
                return;
            }
            else
            {
                error( KIO::ERR_COULD_NOT_MKDIR, path );
                return;
            }
        }
        else
        {
            m_sqlDB.execSql( QString("REPLACE INTO Albums (url, date) "
                                     "VALUES('%1','%2')")
                             .arg(escapeString(url.path()),
                                  QDate::currentDate().toString(Qt::ISODate)) );
            
            if ( permissions != -1 )
            {
                if ( ::chmod( _path.data(), permissions ) == -1 )
                    error( KIO::ERR_CANNOT_CHMOD, path );
                else
                    finished();                                   
            }
            else
                finished();
            return;
        }
    }

    if ( S_ISDIR( buff.st_mode ) )
    {
        error( KIO::ERR_DIR_ALREADY_EXIST, path );
        return;
    }

    error( KIO::ERR_FILE_ALREADY_EXIST, path );                
}

void kio_digikamalbums::chmod( const KURL& url, int permissions )
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;            

    // get the album library path
    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    QCString path( QFile::encodeName(libraryPath + url.path()));
    if ( ::chmod( path.data(), permissions ) == -1 )
        error( KIO::ERR_CANNOT_CHMOD, url.url() );
    else
        finished();
}

void kio_digikamalbums::del( const KURL& url, bool isfile)
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;            

    // get the album library path
    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    // open the db if needed
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        m_sqlDB.closeDB();
        m_sqlDB.openDB(m_libraryPath);
    }

    // build the album list
    buildAlbumList();

    QCString path( QFile::encodeName(libraryPath + url.path()));
    
    if (isfile)
    {
        kdDebug(  ) <<  "Deleting file "<< url.url() << endl;

        // if the filename is .digikam_properties fake that we deleted it
        if (url.fileName() == ".digikam_properties")
        {
            finished();
            return;
        }

        // find the Album to which this file belongs.
        AlbumInfo album = findAlbum(url.directory());
        if (album.id == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                  .arg(url.directory()));
            return;
        }

        // actually delete the file
        if ( unlink( path.data() ) == -1 )
        {
            if ((errno == EACCES) || (errno == EPERM))
                error( KIO::ERR_ACCESS_DENIED, url.url());
            else if (errno == EISDIR)
                error( KIO::ERR_IS_DIRECTORY, url.url());
            else
                error( KIO::ERR_CANNOT_DELETE, url.url() );
            return;
        }

        // successful deletion. now remove file entry from the database
        delImage(album.id, url.fileName());
    }
    else
    {
        kdDebug(  ) << "Deleting directory " << url.url() << endl;

        // find the corresponding album entry
        AlbumInfo album = findAlbum(url.path());
        if (album.id == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database")
                  .arg(url.path()));
            return;
        }
      
        if ( ::rmdir( path.data() ) == -1 )
        {
            // TODO handle symlink delete
            
            if ((errno == EACCES) || (errno == EPERM))
            {
                error( KIO::ERR_ACCESS_DENIED, url.url());
                return;
            }
            else
            {
                kdDebug() << "could not rmdir " << perror << endl;
                error( KIO::ERR_COULD_NOT_RMDIR, url.url() );
                return;
            }
        }

        // successful deletion. now remove album entry from the database
        delAlbum(album.id);
    }

    finished();
    
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

    atom.m_uds = KIO::UDS_LOCAL_PATH;
    atom.m_str = path;
    entry.append(atom);

    return true;
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

void kio_digikamalbums::buildAlbumList()
{
    m_albumList.clear();
    
    QStringList values;
    m_sqlDB.execSql( QString("SELECT id, url, date, caption, collection, icon "
                             "FROM Albums;"), &values );

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        AlbumInfo info;
        
        info.id = (*it).toInt();
        ++it;
        info.url = *it;
        ++it;
        info.date = QDate::fromString(*it, Qt::ISODate);
        ++it;
        info.caption = *it;
        ++it;
        info.collection = *it;
        ++it;
        info.icon = (*it).toLongLong();
        ++it;
        
        m_albumList.append(info);        
    }
}

AlbumInfo kio_digikamalbums::findAlbum(const QString& url, bool addIfNotExists)
{
    AlbumInfo album;
    for (QValueList<AlbumInfo>::const_iterator it = m_albumList.begin();
         it != m_albumList.end(); ++it)
    {
        if ((*it).url == url)
        {
            album = *it;
            return album;
        }
    }

    album.id = -1;
    
    if (addIfNotExists)
    {
        QFileInfo fi(m_libraryPath + url);
        if (!fi.exists() || !fi.isDir())
            return album;

        m_sqlDB.execSql(QString("INSERT INTO Albums (url, date) "
                                "VALUES('%1', '%2')")
                        .arg(escapeString(url),
                             fi.lastModified().date().toString(Qt::ISODate)));

        album.id   = m_sqlDB.lastInsertedRow();
        album.url  = url;
        album.date = fi.lastModified().date();
        album.icon = 0;
        
        m_albumList.append(album);
    }
    
    return album;
}

void kio_digikamalbums::delAlbum(int albumID)
{
    m_sqlDB.execSql(QString("DELETE FROM Albums WHERE id='%1'")
                    .arg(albumID));    
}

void kio_digikamalbums::renameAlbum(const QString& oldURL, const QString& newURL)
{
    // first update the url of the album which was renamed

    m_sqlDB.execSql( QString("UPDATE Albums SET url='%1' WHERE url='%2'")
                     .arg(escapeString(newURL),
                          escapeString(oldURL)));

    // now find the list of all subalbums which need to be updated
    QStringList values;
    m_sqlDB.execSql( QString("SELECT url FROM Albums WHERE url LIKE '%1/%';")
                     .arg(oldURL), &values );

    // and update their url
    QString newChildURL;
    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        newChildURL = *it;
        newChildURL.replace(oldURL, newURL);
        m_sqlDB.execSql(QString("UPDATE Albums SET url='%1' WHERE url='%2'")
                        .arg(escapeString(newChildURL),
                             escapeString(*it)));
    }
}

bool kio_digikamalbums::findImage(int albumID, const QString& name) const
{
    QStringList values;
    
    m_sqlDB.execSql( QString("SELECT name FROM Images "
                             "WHERE dirid=%1 AND name='%2';")
                     .arg(albumID)
                     .arg(escapeString(name)),
                     &values );

    return !(values.isEmpty());
}

void kio_digikamalbums::addImage(int albumID, const QString& filePath)
{
    QString   comment;
    QDateTime datetime;
    int       rating = 0;

    Digikam::DMetadata metadata(filePath);

    // Trying to get comments from image :
    // In first, from standard JPEG comments, or
    // In second, from EXIF comments tag, or
    // In third, from IPTC comments tag.

    comment = metadata.getImageComment();

    // Trying to get date and time from image :
    // In first, from EXIF date & time tags, or
    // In second, from IPTC date & time tags.

    datetime = metadata.getImageDateTime();

    // Trying to get image rating from IPTC Urgency tag.
    rating = metadata.getImageRating();

    if (!datetime.isValid())
    {
        QFileInfo info(filePath);
        datetime = info.lastModified();
    }

    m_sqlDB.execSql(QString("REPLACE INTO Images "
                            "(dirid, name, datetime, caption) "
                            "VALUES(%1, '%2', '%3', '%4')")
                    .arg(QString::number(albumID),
                         escapeString(QFileInfo(filePath).fileName()),
                         datetime.toString(Qt::ISODate),
                         escapeString(comment)));

    Q_LLONG imageID = m_sqlDB.lastInsertedRow();

    if (imageID != -1 && rating != -1)
    {
        m_sqlDB.execSql(QString("REPLACE INTO ImageProperties "
                                "(imageid, property, value) "
                                "VALUES(%1, '%2', '%3');")
                        .arg(imageID)
                        .arg("Rating")
                        .arg(rating) );                        
    } 
}

void kio_digikamalbums::delImage(int albumID, const QString& name)
{
    m_sqlDB.execSql( QString("DELETE FROM Images "
                             "WHERE dirid=%1 AND name='%2';")
                     .arg(albumID)
                     .arg(escapeString(name)) );
}

void kio_digikamalbums::renameImage(int oldAlbumID, const QString& oldName,
                                    int newAlbumID, const QString& newName)
{
    // first delete any stale entries for the destination file
    m_sqlDB.execSql( QString("DELETE FROM Images "
                             "WHERE dirid=%1 AND name='%2';")
                     .arg(newAlbumID)
                     .arg(escapeString(newName)) );

    // now update the dirid and/or name of the file
    m_sqlDB.execSql( QString("UPDATE Images SET dirid=%1, name='%2' "
                             "WHERE dirid=%3 AND name='%4';")
                     .arg(QString::number(newAlbumID),
                          escapeString(newName),
                          QString::number(oldAlbumID),
                          escapeString(oldName)) );
}

void kio_digikamalbums::copyImage(int srcAlbumID, const QString& srcName,
                                  int dstAlbumID, const QString& dstName)
{
    // check for src == dest
    if (srcAlbumID == dstAlbumID && srcName == dstName)
    {
        error( KIO::ERR_FILE_ALREADY_EXIST, dstName );
        return;
    }

    // find id of src image
    QStringList values;
    m_sqlDB.execSql( QString("SELECT id FROM Images "
                             "WHERE dirid=%1 AND name='%2';")
                     .arg(QString::number(srcAlbumID), escapeString(srcName)),
                     &values);

    if (values.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, i18n("Source image %1 not found in database")
                .arg(srcName));
        return;
    }

    int srcId = values[0].toInt();

    // first delete any stale entries for the destination file
    m_sqlDB.execSql( QString("DELETE FROM Images "
                             "WHERE dirid=%1 AND name='%2';")
                     .arg(QString::number(dstAlbumID), escapeString(dstName)) );

    // copy entry in Images table
    m_sqlDB.execSql( QString("INSERT INTO Images (dirid, name, caption, datetime) "
                             "SELECT %1, '%2', caption, datetime FROM Images "
                             "WHERE id=%3;")
                     .arg(QString::number(dstAlbumID), escapeString(dstName),
                          QString::number(srcId)) );

    int dstId = m_sqlDB.lastInsertedRow();

    // copy tags
    m_sqlDB.execSql( QString("INSERT INTO ImageTags (imageid, tagid) "
                             "SELECT %1, tagid FROM ImageTags "
                             "WHERE imageid=%2;")
                     .arg(QString::number(dstId), QString::number(srcId)) );

    // copy properties (rating)
    m_sqlDB.execSql( QString("INSERT INTO ImageProperties (imageid, property, value) "
                             "SELECT %1, property, value FROM ImageProperties "
                             "WHERE imageid=%2;")
                     .arg(QString::number(dstId), QString::number(srcId)) );
}

void kio_digikamalbums::scanAlbum(const QString& url)
{
    scanOneAlbum(url);
    removeInvalidAlbums();
}

void kio_digikamalbums::scanOneAlbum(const QString& url)
{
    QDir dir(m_libraryPath + url);
    if (!dir.exists() || !dir.isReadable())
    {
        return;
    }

    QString subURL = url;
    if (!url.endsWith("/"))
        subURL += '/';
    subURL = escapeString( subURL);

    {
        // scan albums
        
        QStringList currAlbumList;
        m_sqlDB.execSql( QString("SELECT url FROM Albums WHERE ") +
                         QString("url LIKE '") + subURL + QString("%' ") +
                         QString("AND url NOT LIKE '") + subURL + QString("%/%' "),
                         &currAlbumList );


        const QFileInfoList* infoList = dir.entryInfoList(QDir::Dirs);
        if (!infoList)
            return;
        
        QFileInfoListIterator it(*infoList);
        QFileInfo* fi;
        
        QStringList newAlbumList;
        while ((fi = it.current()) != 0)
        {
            ++it;
            
            if (fi->fileName() == "." || fi->fileName() == ".." || fi->extension(true) == "digikamtempfile.tmp")
            {
                continue;
            }
            
            QString u = QDir::cleanDirPath(url + '/' + fi->fileName());
            
            if (currAlbumList.contains(u))
            {
                continue;
            }

            newAlbumList.append(u);
        }

        for (QStringList::iterator it = newAlbumList.begin();
             it != newAlbumList.end(); ++it)
        {
            kdDebug() << "New Album: " << *it << endl;

            QFileInfo fi(m_libraryPath + *it);
            m_sqlDB.execSql(QString("INSERT INTO Albums (url, date) "
                                    "VALUES('%1', '%2')")
                            .arg(escapeString(*it),
                                 fi.lastModified().date().toString(Qt::ISODate)));

            scanAlbum(*it);
        }
    }

    if (url != "/")
    {
        // scan files

        QStringList values;

        m_sqlDB.execSql( QString("SELECT id FROM Albums WHERE url='%1'")
                         .arg(escapeString(url)), &values );
        if (values.isEmpty())
            return;

        int albumID = values.first().toInt();

        QStringList currItemList;
        m_sqlDB.execSql( QString("SELECT name FROM Images WHERE dirid=%1")
                         .arg(albumID), &currItemList );
        
        const QFileInfoList* infoList = dir.entryInfoList(QDir::Files);
        if (!infoList)
            return;

        QFileInfoListIterator it(*infoList);
        QFileInfo* fi;

        // add any new files we find to the db
        while ((fi = it.current()) != 0)
        {
            ++it;

            if (currItemList.contains(fi->fileName()))
            {
                currItemList.remove(fi->fileName());
                continue;
            }

            addImage(albumID, m_libraryPath + url + '/' + fi->fileName());
        }

        // currItemList now contains deleted file list. remove them from db
        for (QStringList::iterator it = currItemList.begin();
             it != currItemList.end(); ++it)
        {
            delImage(albumID, *it);
        }
    }
}

void kio_digikamalbums::removeInvalidAlbums()
{
    QStringList urlList;

    m_sqlDB.execSql(QString("SELECT url FROM Albums;"),
                    &urlList);

    m_sqlDB.execSql("BEGIN TRANSACTION");

    struct stat stbuf;
    
    for (QStringList::iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        if (::stat(QFile::encodeName(m_libraryPath + *it), &stbuf) == 0)
            continue;

        kdDebug() << "Deleted Album: " << *it << endl;
        m_sqlDB.execSql(QString("DELETE FROM Albums WHERE url='%1'")
                    .arg(escapeString(*it)));    
    }

    m_sqlDB.execSql("COMMIT TRANSACTION");
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
