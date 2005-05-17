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
#include <qdatastream.h>
#include <qregexp.h>
#include <qdir.h>

#include <config.h>

extern "C" 
{
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sqlite.h>
#include <sys/time.h>
#include <time.h>
}

#include "digikamalbums.h"

#define MAX_IPC_SIZE (1024*32)

kio_digikamalbums::kio_digikamalbums(const QCString &pool_socket,
                                     const QCString &app_socket)
    : SlaveBase("kio_digikamalbums", pool_socket, app_socket)
{
    m_db    = 0;
}

kio_digikamalbums::~kio_digikamalbums()
{
    closeDB();
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
    QString url;
    QString filter;
    int     getDimensions;
    
    QDataStream ds(data, IO_ReadOnly);
    ds >> libraryPath;
    ds >> url;
    ds >> filter;
    ds >> getDimensions;

    QValueList<QRegExp> regex = makeFilterList(filter);
    
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        closeDB();
        openDB();
    }

    QStringList values;
    execSql(QString("SELECT id FROM Albums WHERE url='%1';")
            .arg(escapeString(url)), &values);
    int id = values.first().toInt();

    
    values.clear();
    execSql(QString("SELECT Images.name, Images.datetime FROM Images "
                    "WHERE Images.dirid = %1;")
            .arg(id), &values);

    QByteArray  ba;
    QDataStream os(ba, IO_WriteOnly);
    
    QString base = libraryPath + url + "/";
    QString name;
    QString date;
    QSize   dims;

    struct stat stbuf;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
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
        
        os << id;
        os << name;
        os << date;
        os << stbuf.st_size;
        os << dims;
    }

    SlaveBase::data(ba);
    
    finished();
}

void kio_digikamalbums::openDB()
{
    // TODO: change to digikam.db for production code
    QString dbPath = m_libraryPath + "/digikam-testing.db";

#ifdef NFS_HACK
    dbPath = QDir::homeDirPath() + "/.kde/share/apps/digikam/"  +
             KIO::encodeFileName(QDir::cleanDirPath(dbPath));
#endif

    char *errMsg = 0;
    m_db = sqlite_open(QFile::encodeName(dbPath), 0, &errMsg);
    if (m_db == 0)
    {
        error(KIO::ERR_UNKNOWN, i18n("Failed to open digiKam database."));
        free(errMsg);
        return;
    }
}

void kio_digikamalbums::closeDB()
{
    if (m_db)
    {
        sqlite_close(m_db);
    }
    
    m_db = 0;
}

bool kio_digikamalbums::execSql(const QString& sql, QStringList* const values, 
                                const bool debug) const
{
    if ( debug )
        kdDebug() << "SQL-query: " << sql << endl;

    if ( !m_db ) {
        kdWarning() << k_funcinfo << "SQLite pointer == NULL"
                    << endl;
        return false;
    }

    const char* tail;
    sqlite_vm* vm;
    char* errorStr;
    int error;
    
    //compile SQL program to virtual machine
    error = sqlite_compile( m_db, sql.local8Bit(), &tail, &vm, &errorStr );

    if ( error != SQLITE_OK ) {
        kdWarning() << k_funcinfo << "sqlite_compile error: "
                    << errorStr 
                    << " on query: " << sql << endl;
        sqlite_freemem( errorStr );
        return false;
    }

    int number;
    const char** value;
    const char** colName;
    //execute virtual machine by iterating over rows
    while ( true ) {
        error = sqlite_step( vm, &number, &value, &colName );
        if ( error == SQLITE_DONE || error == SQLITE_ERROR )
            break;
        //iterate over columns
        for ( int i = 0; values && i < number; i++ ) {
            *values << QString::fromLocal8Bit( value [i] );
        }
    }
    
    //deallocate vm resources
    sqlite_finalize( vm, &errorStr );

    if ( error != SQLITE_DONE ) {
        kdWarning() << k_funcinfo << "sqlite_step error: "
                    << errorStr
                    << " on query: " << sql << endl;
        return false;
    }

    return true;
}

QString kio_digikamalbums::escapeString(const QString& str) const
{
    QString st(str);
    st.replace( "'", "''" );
    return st;
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
    kdDebug() << k_funcinfo << " : " << url.path() << endl;

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
        closeDB();
        openDB();
    }

    // build the album list
    buildAlbumList();

    // get the parent album
    bool found;
    AlbumInfo album = findAlbum(url.directory(), found);
    if (!found)
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
                    kdWarning(7101) << "Couldn't write. Error:" << strerror(errno) << endl;
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
        kdWarning(7101) << "Error when closing file descriptor:" << strerror(errno) << endl;
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

void kio_digikamalbums::copy( const KURL &src, const KURL &dst, int /*mode*/, bool /*overwrite*/ )
{
    kdDebug() << k_funcinfo << "Src: " << src.path() << ", Dst: " << dst.path()   << endl;        

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
              QString("Source and Destination have different Album Library Paths. ") +
              QString("Src: ") + src.user() +
              QString(", Dest: ") + dst.user());
        return;
    }

    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        closeDB();
        openDB();
    }

    buildAlbumList();

    bool found;

    AlbumInfo srcAlbum = findAlbum(src.directory(), found);
    if (!found)
    {
        error(KIO::ERR_UNKNOWN, QString("Source album %1 not found in database")
              .arg(src.directory()));
        return;
    }

    AlbumInfo dstAlbum = findAlbum(dst.directory(), found);
    if (!found)
    {
        error(KIO::ERR_UNKNOWN, QString("Destination album %1 not found in database")
              .arg(dst.directory()));
        return;
    }
    
    if (src.fileName() == ".digikam_properties")
    {
        // copy metadata of album to destination album
        execSql(QString("UPDATE Albums SET date='%1', caption='%2', "
                        "collection='%3', icon='%4' WHERE id=%5")
                .arg(srcAlbum.date.toString(Qt::ISODate))
                .arg(escapeString(srcAlbum.caption))
                .arg(escapeString(srcAlbum.collection))
                .arg(escapeString(srcAlbum.icon))
                .arg(dstAlbum.id));
        finished();
        return;
    }

    QCString _src( QFile::encodeName(libraryPath + src.path()));
    QCString _dst( QFile::encodeName(libraryPath + dst.path()));
    KDE_struct_stat buff_src;
    if ( KDE_stat( _src.data(), &buff_src ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, src.url() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, src.url() );
        return;
    }

    if ( S_ISDIR( buff_src.st_mode ) )
    {
        error( KIO::ERR_IS_DIRECTORY, src.url() );
        return;
    }

    if ( S_ISFIFO( buff_src.st_mode ) || S_ISSOCK ( buff_src.st_mode ) )
    {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, src.url() );
        return;
    } 

    /* LOTS OF TODO */
}

void kio_digikamalbums::rename( const KURL& src, const KURL& dst, bool /*overwrite*/ )
{
    kdDebug() << k_funcinfo << "Src: " << src << ", Dst: " << dst   << endl;        

    /* TODO */
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
    
    const QFileInfoList *list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    KIO::UDSEntry entry;
    createDigikamPropsUDSEntry(entry);
    listEntry(entry, false);
    while ((fi = it.current()) != 0)
    {
        if (fi->fileName() != "." && fi->fileName() != "..")
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
    kdDebug() << k_funcinfo << " : " << url.path() << endl;

    QString libraryPath = url.user();
    if (libraryPath.isEmpty())
    {
        error(KIO::ERR_UNKNOWN, "Album Library Path not supplied to kioslave");
        return;
    }

    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        closeDB();
        openDB();
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
            execSql( QString("REPLACE INTO Albums (url, date) "
                             "VALUES('%1','%2')")
                     .arg(escapeString(url.path()))
                     .arg(QDate::currentDate().toString(Qt::ISODate)));
            
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

void kio_digikamalbums::chmod( const KURL& url, int /*permissions*/ )
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;            

    /* TODO */
}

void kio_digikamalbums::del( const KURL& url, bool /*isfile*/)
{
    kdDebug() << k_funcinfo << " : " << url.url() << endl;            

    /* TODO */
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
    execSql( QString("SELECT id, url, date, caption, collection, icon "
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
        info.icon = *it;
        ++it;
        
        m_albumList.append(info);        
    }
}

AlbumInfo kio_digikamalbums::findAlbum(const QString& url, bool& ok) const
{
    ok = false;

    AlbumInfo album;
    for (QValueList<AlbumInfo>::const_iterator it = m_albumList.begin();
         it != m_albumList.end(); ++it)
    {
        if ((*it).url == url)
        {
            ok = true;
            album = *it;
            break;
        }
    }

    return album;
}

bool kio_digikamalbums::findImage(int albumID, const QString& name) const
{
    QStringList values;
    
    execSql( QString("SELECT name FROM Images "
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
    
    KFileMetaInfo itemMetaInfo(filePath);

    if (itemMetaInfo.isValid() &&
        itemMetaInfo.containsGroup("Jpeg EXIF Data"))
    {
        comment = itemMetaInfo.group("Jpeg EXIF Data").
                  item("Comment").value().toString();
        datetime = itemMetaInfo.group("Jpeg EXIF Data").
                   item("Date/time").value().toDateTime();
    }

    if (!datetime.isValid())
    {
        QFileInfo info(filePath);
        datetime = info.lastModified();
    }

    execSql(QString("REPLACE INTO Images "
                    "(dirid, name, datetime, caption) "
                    "VALUES(%1, '%2', '%3', '%4')")
            .arg(albumID)
            .arg(escapeString(QFileInfo(filePath).fileName()))
            .arg(datetime.toString(Qt::ISODate))
            .arg(escapeString(comment)));
}

/* KIO slave registration */

extern "C"
{
    int kdemain(int argc, char **argv)
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

