/* ============================================================
 * File  : digikamio.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-28
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kinstance.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kurl.h>
#include <klocale.h>
#include <klargefile.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qdir.h>

#include <config.h>

extern "C" 
{
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sqlite.h>
}

#include "digikamio.h"

kio_digikamioProtocol::kio_digikamioProtocol(const QCString &pool_socket,
                                             const QCString &app_socket)
    : SlaveBase("kio_digikamio", pool_socket, app_socket)
{
    m_db    = 0;
    m_valid = false;
    
    KConfig config("digikamrc");
    config.setGroup("Album Settings");
    m_libraryPath = config.readPathEntry("Album Path", QString::null);
    if (m_libraryPath.isEmpty() || !QFileInfo(m_libraryPath).exists())
    {
        error(KIO::ERR_UNKNOWN, i18n("Digikam Library path not set correctly"));
        return;
    }

    m_libraryPath = QDir::cleanDirPath(m_libraryPath);
    
    QString path = m_libraryPath + "/digikam.db";
    
    char *errMsg = 0;
    m_db = sqlite_open(QFile::encodeName(path), 0, &errMsg);
    if (m_db == 0)
    {
        error(KIO::ERR_UNKNOWN, i18n("Failed to open Digikam Database"));
        free(errMsg);
        return;
    }

    m_valid = true;
}

kio_digikamioProtocol::~kio_digikamioProtocol()
{
    if (m_db)
    {
        sqlite_close(m_db);
    }
}

void kio_digikamioProtocol::copy(const KURL& src, const KURL& dest,
                                 int permissions, bool overwrite)
{
    bool failed = false;
    copyInternal(src, dest, permissions, overwrite, failed);

    finished();
}

void kio_digikamioProtocol::copyInternal(const KURL& src, const KURL& dest,
                                         int permissions, bool overwrite,
                                         bool& failed)
{
    if (failed)
        return;
    
    QCString _src(  QFile::encodeName(src.path())  );
    QCString _dest( QFile::encodeName(dest.path()) );

    bool srcIsDir      = false;
    bool srcInLibrary  = true;
    bool destInLibrary = true;
    
    KDE_struct_stat buff_src;
    if ( KDE_stat( _src.data(), &buff_src ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, src.path() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, src.path() );
        failed = true;
        return;
    }

    // Is src a directory
    srcIsDir = S_ISDIR(buff_src.st_mode);

    // Is src and dest in library
    KURL libURL(m_libraryPath);
    libURL.setProtocol("digikamio");
    srcInLibrary  = libURL.isParentOf(src);
    destInLibrary = libURL.isParentOf(dest);

    if (!destInLibrary)
    {
        kdWarning() << "This should not happen. "
                    << "Destination URL not in album library Path. "
                    << dest.prettyURL() << endl;
        error(KIO::ERR_UNKNOWN, i18n("Destination URL not in album library Path."));
        failed = true;
        return;
    }

    KDE_struct_stat buff_dest;
    bool dest_exists = ( KDE_stat( _dest.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
            error( KIO::ERR_DIR_ALREADY_EXIST, dest.path() );
            failed = true;
            return;
        }

        if (!overwrite)
        {
            error( KIO::ERR_FILE_ALREADY_EXIST, dest.path() );
            failed = true;
            return;
        }
    }

    // Four possible scenarios:
    // a. dir in library being copied to another location
    // b. external dir being copied into library
    // c. file from one album being copied to another album
    // d. external file being copied into an album 
    
    
    if (srcIsDir && srcInLibrary)
    {
        // a. dir in library being copied to another location

        infoMessage(i18n("Copying folder %1 ...")
                    .arg(src.path()));
        
        // lock the database
        execSql( "BEGIN TRANSACTION;" );

        bool success = true;
        
        // first make the directory
        if (::mkdir(_dest.data(), buff_src.st_mode))
        {
            if (( errno == EACCES ) || (errno == EPERM) || (errno == EROFS)) 
                error( KIO::ERR_ACCESS_DENIED, dest.path() );
            else if (errno == EEXIST)
                error( KIO::ERR_DIR_ALREADY_EXIST, dest.path() );
            else if (errno == ENOSPC)
                error( KIO::ERR_DISK_FULL, dest.path() );
            else 
                error( KIO::ERR_COULD_NOT_MKDIR, dest.path() );

            success = false;
            failed  = true;
        }            

        if ( success )
        {
            QString oldURL = escapeString( albumURLFromKURL(src)  );
            QString newURL = escapeString( albumURLFromKURL(dest) );
            
            // delete any stale album
            removeDirFromDB( newURL );

            // copy the attributes of the original album to this
            execSql( QString("INSERT INTO Albums (url, date, caption, collection) "
                             "SELECT '%1',date,caption,collection FROM Albums "
                             "WHERE url='%2';")
                     .arg(escapeString(newURL))
                     .arg(escapeString(oldURL)) );

        }

        // unlock the database
        execSql( "COMMIT TRANSACTION;" );

        if ( !success )
        {
            failed = true;
            return;
        }
        
        // now read the old directory and start copying each of entries
        QDir dir( src.path(-1) );
        dir.setFilter( QDir::Dirs | QDir::Files | QDir::NoSymLinks );
        dir.setSorting( QDir::DirsFirst | QDir::Name );
        
        const QFileInfoList *infoList = dir.entryInfoList();
        if ( infoList )
        {
            QFileInfoListIterator it( *infoList );
            QFileInfo *fi;
            
            while ( (fi = it.current()) != 0 )
            {
                ++it;
                if (fi->fileName().startsWith("."))
                    continue;
                    
                KURL surl( src );
                surl.addPath( fi->fileName() );
                    
                KURL durl( dest );
                durl.addPath( fi->fileName() );
                    
                copyInternal( surl, durl, permissions, overwrite, failed );
                if ( failed )
                    return;
            }
        }

        return;

    }
    else if (srcIsDir)
    {
        // b. external dir being copied into library

        infoMessage(i18n("Copying folder %1 ...")
                    .arg(src.path()));
        
        bool success = true;
        
        // first make the directory
        if (::mkdir(_dest.data(), buff_src.st_mode))
        {
            if (( errno == EACCES ) || (errno == EPERM) || (errno == EROFS)) 
                error( KIO::ERR_ACCESS_DENIED, dest.path() );
            else if (errno == EEXIST)
                error( KIO::ERR_DIR_ALREADY_EXIST, dest.path() );
            else if (errno == ENOSPC)
                error( KIO::ERR_DISK_FULL, dest.path() );
            else 
                error( KIO::ERR_COULD_NOT_MKDIR, dest.path() );

            success = false;
            failed  = true;
        }            

        if ( !success )
        {
            failed = true;
            return;
        }
        
        // now read the old directory and start copying each of entries
        QDir dir( src.path(-1) );
        dir.setFilter( QDir::Dirs | QDir::Files | QDir::NoSymLinks );
        dir.setSorting( QDir::DirsFirst | QDir::Name );
        
        const QFileInfoList *infoList = dir.entryInfoList();
        if ( infoList )
        {
            QFileInfoListIterator it( *infoList );
            QFileInfo *fi;
            while ( (fi = it.current()) != 0 )
            {
                ++it;
                if (fi->fileName().startsWith("."))
                    continue;

                KURL surl( src );
                surl.addPath( fi->fileName() );
                
                KURL durl( dest );
                durl.addPath( fi->fileName() );
                
                copyInternal( surl, durl, permissions, overwrite, failed );
                if ( failed )
                    return;

            }
        }
    }
    else if (srcInLibrary)
    {
        // c. file from one album being copied to another album

        infoMessage(i18n("Copying file %1 ...")
                    .arg(src.path()));
        
        // find the parent albums
        QString oldParentURL(albumURLFromKURL(src.upURL()));
        QString newParentURL(albumURLFromKURL(dest.upURL()));

        // find the album ids
        int oldDirID;
        int newDirID;

        QStringList vals;
        
        execSql( QString("SELECT id FROM Albums WHERE url = '%1'")
                 .arg(oldParentURL), &vals );
        if (vals.isEmpty())
        {
            error(KIO::ERR_UNKNOWN,
                  i18n("Could not find source parent album for %1")
                  .arg(src.path()));
            failed = true;
            return;
        }
        oldDirID = vals.first().toInt();

        vals.clear();
        execSql( QString("SELECT id FROM Albums WHERE url = '%1'")
                 .arg(newParentURL), &vals );
        if (vals.isEmpty())
        {
            error(KIO::ERR_UNKNOWN,
                  i18n("Could not find destination parent album for %1")
                  .arg(dest.prettyURL()));
            failed = true;
            return;
        }
        newDirID = vals.first().toInt();
        
        // first copy to a tmp file
        KURL destDirURL(dest.upURL());
        destDirURL.addPath(QString(".digikamio-%1")
                           .arg(getpid()));

        QString tmpFile(destDirURL.path());

        if (!copyFile(_src, tmpFile))
        {
            error(KIO::ERR_COULD_NOT_WRITE, dest.prettyURL());
            failed = true;
            return;
        }

        // now rename to dest file
        execSql( "BEGIN TRANSACTION;" );

        if (::rename( QFile::encodeName(tmpFile), _dest.data()))
        {
            unlink( QFile::encodeName(tmpFile) );
            error(KIO::ERR_COULD_NOT_WRITE, dest.prettyURL());
            failed = true;
        }
        else
        {
            // successful copy: now copy the file metadata

            // first delete any stale database entries if any
            removeFileFromDB(newDirID, dest.fileName());
    
            execSql( QString("INSERT INTO Images (dirid, name, caption, datetime) "
                             "SELECT %1, '%2', caption, datetime FROM Images "
                             "WHERE dirid=%3 AND name='%4';")
                     .arg(newDirID)
                     .arg(escapeString(dest.fileName()))
                     .arg(oldDirID)
                     .arg(escapeString(src.fileName())) );

            execSql( QString("INSERT INTO ImageTags (dirid, name, tagid) "
                             "SELECT %1, '%2', tagid FROM ImageTags "
                             "WHERE dirid=%3 AND name='%4';")
                     .arg(newDirID)
                     .arg(escapeString(dest.fileName()))
                     .arg(oldDirID)
                     .arg(escapeString(src.fileName())) );
        }

        execSql( "COMMIT TRANSACTION;");

        return;
    }
    else
    {
        // d. external file being copied into an album

        infoMessage(i18n("Copying file %1 ...")
                    .arg(src.path()));

        // first copy to a tmp file
        KURL destDirURL(dest.upURL());
        destDirURL.addPath(QString(".digikamio-%1")
                           .arg(getpid()));

        QString tmpFile(destDirURL.path());

        if (!copyFile(_src, tmpFile))
        {
            error(KIO::ERR_COULD_NOT_WRITE, dest.prettyURL());
            failed = true;
            return;
        }

        // now rename to the dest file
        if (::rename( QFile::encodeName(tmpFile), _dest.data()))
        {
            unlink( QFile::encodeName(tmpFile) );
            error(KIO::ERR_COULD_NOT_WRITE, dest.prettyURL());
            failed = true;
            return;
        }
    }
}

void kio_digikamioProtocol::rename(const KURL &src, const KURL &dest,
                                   bool overwrite)
{
    QCString _src(  QFile::encodeName(src.path())  );
    QCString _dest( QFile::encodeName(dest.path()) );

    bool srcIsDir      = false;
    bool srcInLibrary  = true;
    bool destInLibrary = true;
    
    KDE_struct_stat buff_src;
    if ( KDE_stat( _src.data(), &buff_src ) == -1 )
    {
        if ( errno == EACCES )
            error( KIO::ERR_ACCESS_DENIED, src.path() );
        else
            error( KIO::ERR_DOES_NOT_EXIST, src.path() );
        finished();
        return;
    }

    // Is src a directory
    srcIsDir = S_ISDIR(buff_src.st_mode);

    // Is src and dest in library
    KURL libURL(m_libraryPath);
    libURL.setProtocol("digikamio");
    srcInLibrary  = libURL.isParentOf(src);
    destInLibrary = libURL.isParentOf(dest);

    if (!destInLibrary)
    {
        kdWarning() << "This should not happen. "
                    << "Destination URL not in album library Path. "
                    << dest.prettyURL() << endl;
        error(KIO::ERR_UNKNOWN, i18n("Destination URL not in album library Path."));
        finished();
        return;
    }

    KDE_struct_stat buff_dest;
    bool dest_exists = ( KDE_stat( _dest.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
            error( KIO::ERR_DIR_ALREADY_EXIST, dest.path() );
            finished();
            return;
        }

        if (!overwrite)
        {
            error( KIO::ERR_FILE_ALREADY_EXIST, dest.path() );
            finished();
            return;
        }
    }

    if (srcIsDir && srcInLibrary)
    {
        // moving or renaming an album;

        infoMessage(i18n("Moving folder %1 ...")
                    .arg(src.path()));

        // lock the database
        execSql( "BEGIN TRANSACTION;" );

        bool success = true;
        
        if (::rename( _src.data(), _dest.data()))
        {
            if (( errno == EACCES ) || (errno == EPERM)) 
                error( KIO::ERR_ACCESS_DENIED, dest.path() );
            else if (errno == EXDEV) 
                error( KIO::ERR_UNSUPPORTED_ACTION, QString::fromLatin1("rename"));
            else if (errno == EROFS)  // The file is on a read-only filesystem
                error( KIO::ERR_CANNOT_DELETE, src.path() );
            else 
                error( KIO::ERR_CANNOT_RENAME, src.path() );

            success = false;
        }

        if (success)
        {
            // first rename the album in the database

            QString oldURL = escapeString( albumURLFromKURL(src)  );
            QString newURL = escapeString( albumURLFromKURL(dest) );

            // delete any stale albums left behind
            removeDirFromDB( newURL );

            // update album url
            execSql( QString("UPDATE Albums SET url = '%1' WHERE url = '%2';")
                     .arg(newURL)
                     .arg(oldURL) );

            // Now rename all the subalbums

            QStringList suburls;
            
            execSql( QString("SELECT url FROM Albums WHERE url LIKE '%1/%'")
                     .arg(oldURL), &suburls );
            for (QStringList::iterator it = suburls.begin(); it != suburls.end();
                 ++it)
            {
                QString url(*it);
                url.remove(0,oldURL.length());
                url.prepend(newURL);

                url = escapeString(url);
                
                // delete any stale albums left behind
                execSql( QString("DELETE FROM Albums WHERE url = '%1'")
                         .arg(url) );

                // update album url
                execSql( QString("UPDATE Albums SET url = '%1' WHERE url = '%2';")
                         .arg(url)
                         .arg(escapeString(*it)) );
            }
        }
        
        // unlock the database
        execSql( "COMMIT TRANSACTION;" );

        finished();
        return;
    }
    else if (srcIsDir)
    {
        // moving an external folder into album library
        // nothing to do here. just rename the folder

        infoMessage(i18n("Moving folder %1 ...")
                    .arg(src.path()));
        
        if (::rename( _src.data(), _dest.data()))
        {
            if (( errno == EACCES ) || (errno == EPERM)) 
                error( KIO::ERR_ACCESS_DENIED, dest.path() );
            else if (errno == EXDEV) 
                error( KIO::ERR_UNSUPPORTED_ACTION, QString::fromLatin1("rename"));
            else if (errno == EROFS)  // The file is on a read-only filesystem
                error( KIO::ERR_CANNOT_DELETE, src.path() );
            else 
                error( KIO::ERR_CANNOT_RENAME, src.path() );
            finished();
        }

        finished();
        return;
    }
    else if (srcInLibrary)
    {
        // moving a file within the album library

        infoMessage(i18n("Moving file %1 ...")
                    .arg(src.path()));

        // find the parent albums
        QString oldParentURL(albumURLFromKURL(src.upURL()));
        QString newParentURL(albumURLFromKURL(dest.upURL()));

        // find the album ids
        int oldDirID;
        int newDirID;

        QStringList vals;
        
        execSql( QString("SELECT id FROM Albums WHERE url = '%1'")
                 .arg(oldParentURL), &vals );
        if (vals.isEmpty())
        {
            error(KIO::ERR_UNKNOWN,
                  i18n("Could not find source parent album for %1")
                  .arg(src.path()));
            finished();
            return;
        }
        oldDirID = vals.first().toInt();

        vals.clear();
        execSql( QString("SELECT id FROM Albums WHERE url = '%1'")
                 .arg(newParentURL), &vals );
        if (vals.isEmpty())
        {
            error(KIO::ERR_UNKNOWN,
                  i18n("Could not find destination parent album for %1")
                  .arg(dest.prettyURL()));
            finished();
            return;
        }
        newDirID = vals.first().toInt();

        // lock the database
        execSql( "BEGIN TRANSACTION;" );

        bool success = true;
        
        if (::rename( _src.data(), _dest.data()))
        {
            if (( errno == EACCES ) || (errno == EPERM)) 
                error( KIO::ERR_ACCESS_DENIED, dest.path() );
            else if (errno == EXDEV) 
                error( KIO::ERR_UNSUPPORTED_ACTION, QString::fromLatin1("rename"));
            else if (errno == EROFS)  // The file is on a read-only filesystem
                error( KIO::ERR_CANNOT_DELETE, src.path() );
            else 
                error( KIO::ERR_CANNOT_RENAME, src.path() );

            success = false;
        }

        QString oldFileName(src.fileName());
        QString newFileName(dest.fileName());
        
        if (success)
        {
            // delete stale items
            removeFileFromDB(newDirID, newFileName);
            
            execSql( QString("UPDATE Images SET dirid=%1, name='%2' "
                             "WHERE dirid=%3 AND name='%4';")
                     .arg(newDirID)
                     .arg(escapeString(newFileName))
                     .arg(oldDirID)
                     .arg(escapeString(oldFileName)) );

            execSql( QString("UPDATE ImageTags SET dirid=%1, name='%2' "
                             "WHERE dirid=%3 AND name='%4';")
                     .arg(newDirID)
                     .arg(escapeString(newFileName))
                     .arg(oldDirID)
                     .arg(escapeString(oldFileName)) );
        }
        
        // unlock the database
        execSql( "COMMIT TRANSACTION;" );

        finished();
        return;
    }
    else
    {
        // external file being moved into album library 
        // nothing to do here. just rename the file

        infoMessage(i18n("Moving file %1 ...")
                    .arg(src.path()));

        if ( ::rename( _src.data(), _dest.data()))
        {
            if (( errno == EACCES ) || (errno == EPERM)) 
                error( KIO::ERR_ACCESS_DENIED, dest.path() );
            else if (errno == EXDEV) 
                error( KIO::ERR_UNSUPPORTED_ACTION, QString::fromLatin1("rename"));
            else if (errno == EROFS)  // The file is on a read-only filesystem
                error( KIO::ERR_CANNOT_DELETE, src.path() );
            else 
                error( KIO::ERR_CANNOT_RENAME, src.path() );
            finished();
            return;
        }

        finished();
        return;
    }
}

QString kio_digikamioProtocol::albumURLFromKURL(const KURL& kurl)
{
    QString url(kurl.path(-1));
    url = QDir::cleanDirPath(url);

    url.remove(0,m_libraryPath.length());

    if (!url.startsWith("/"))
        url.prepend("/");

    return url;
}

void kio_digikamioProtocol::removeDirFromDB(const QString& url)
{
    execSql( QString("DELETE FROM Albums WHERE url = '%1'")
             .arg(url) );
}

void kio_digikamioProtocol::removeFileFromDB(int dirid, const QString& name)
{
    execSql( QString("DELETE FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(dirid)
             .arg(escapeString(name)) );

    execSql( QString("DELETE FROM ImageTags "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(dirid)
             .arg(escapeString(name)) );
}

bool kio_digikamioProtocol::copyFile(const QString& src, const QString& dest)
{
    QFile sFile(src);
    QFile dFile(dest);

    if ( !sFile.open(IO_ReadOnly) )
        return false;
    
    if ( !dFile.open(IO_WriteOnly) )
    {
        sFile.close();
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    Q_LONG len;
    while ((len = sFile.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if (len == -1 || dFile.writeBlock(buffer, (Q_ULONG)len) == -1)
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();
    
    return true;
}

bool kio_digikamioProtocol::execSql(const QString& sql, QStringList* const values, 
                                    const bool debug )
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

QString kio_digikamioProtocol::escapeString(QString str) const
{
    str.replace( "'", "''" );
    return str;
}

/* KIO slave registration */

extern "C"
{
    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_digikamio" );
        
        kdDebug() << "*** kio_digikamio started ***" << endl;
        
        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamio  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamioProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        kdDebug() << "*** kio_digikamio finished ***" << endl;
        return 0;
    }
}
