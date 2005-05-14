/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-09
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <kinstance.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kurl.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/global.h>

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
#include <sqlite.h>
#include <sys/time.h>
#include <time.h>
}

#include "digikamtags.h"
#include "digikam_export.h"
kio_digikamtagsProtocol::kio_digikamtagsProtocol(const QCString &pool_socket,
                                               const QCString &app_socket)
    : SlaveBase("kio_digikamtags", pool_socket, app_socket)
{
    m_db    = 0;
    m_valid = false;
    
    KConfig config("digikamrc");
    config.setGroup("Album Settings");
    m_libraryPath = config.readPathEntry("Album Path", QString::null);
    if (m_libraryPath.isEmpty() || !QFileInfo(m_libraryPath).exists())
    {
        error(KIO::ERR_UNKNOWN, i18n("Digikam library path not set correctly."));
        return;
    }

    QString dbPath = m_libraryPath + "/digikam.db";


#ifdef NFS_HACK
    dbPath = QDir::homeDirPath() + "/.kde/share/apps/digikam/"  +
             KIO::encodeFileName(QDir::cleanDirPath(dbPath));
#endif

    char *errMsg = 0;
    m_db = sqlite_open(QFile::encodeName(dbPath), 0, &errMsg);
    if (m_db == 0)
    {
        error(KIO::ERR_UNKNOWN, i18n("Failed to open Digikam database."));
        free(errMsg);
        return;
    }

    m_valid = true;
}

kio_digikamtagsProtocol::~kio_digikamtagsProtocol()
{
    if (m_db)
    {
        sqlite_close(m_db);
    }
}

void kio_digikamtagsProtocol::stat(const KURL& url)
{
    if (url.equals(KURL("digikamtags:/")))
    {
        statRoot();
    }
    else
    {
        // TODO: provide some protection here against rogue apps
        statTag(url);
    }
}

void kio_digikamtagsProtocol::statRoot()
{
    KIO::UDSEntry entry;
    KIO::UDSAtom  atom;

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = "/";
    entry.append(atom);

    atom.m_uds  = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFDIR;
    entry.append(atom);

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = S_IRUSR | S_IRGRP | S_IROTH |
                  S_IWUSR | S_IWGRP | S_IWOTH;
    entry.append(atom);

    statEntry(entry);
    
    finished();
}

void kio_digikamtagsProtocol::statTag(const KURL &url)
{
    KIO::UDSEntry entry;
    KIO::UDSAtom  atom;

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = url.fileName();
    entry.append(atom);

    atom.m_uds  = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFDIR;
    entry.append(atom);

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = S_IRUSR | S_IRGRP | S_IROTH |
                  S_IWUSR | S_IWGRP | S_IWOTH;
    entry.append(atom);

    statEntry(entry);
    finished();
}

void kio_digikamtagsProtocol::listDir(const KURL& url)
{
    kdDebug() << k_funcinfo << url.url() << endl;

    if (QDir::cleanDirPath(url.path()) == "/")
    {
        kdDebug() << "Listing root " << url.url() << endl;
        
        QStringList values;
        
        execSql( QString("SELECT id, name "
                         "FROM Tags where pid=0 ORDER by name;"),
                 &values );

        int     id;
        QString name;

        KURL xurl;
        
        for (QStringList::iterator it = values.begin(); it != values.end(); )
        {
            id   = (*it++).toInt();
            name =  *it++;

            KIO::UDSEntry entry;
            KIO::UDSAtom  atom;

            atom.m_uds = KIO::UDS_NAME;
            atom.m_str = name;
            entry.append(atom);

            atom.m_uds  = KIO::UDS_FILE_TYPE;
            atom.m_long = S_IFDIR;
            entry.append(atom);

            atom.m_uds  = KIO::UDS_ACCESS;
            atom.m_long = S_IRUSR | S_IRGRP | S_IROTH |
                          S_IWUSR | S_IWGRP | S_IWOTH;
            entry.append(atom);

            atom.m_uds = KIO::UDS_URL;
            xurl.setProtocol("digikamtags");
            xurl.setPath(QString("/%1").arg(id));
            atom.m_str = xurl.url();
            entry.append(atom);

            listEntry(entry, false);
        }
    }
    else if (url.protocol() == "digikamtags")
    {
        kdDebug() << "Listing child " << url.url() << endl;

        int id = url.fileName().toInt();
        if (id == 0)
        {
            KIO::UDSEntry entry;
            listEntry(entry, true);
            finished();
            return;
        }
        
        // list directories first

        QStringList values;
        execSql( QString("SELECT id, name "
                         "FROM Tags where pid=%1 ORDER by name;")
                 .arg(QString::number(id)),
                 &values );

        QString name, path;
        KURL xurl;

        KIO::UDSEntry entry;
        KIO::UDSAtom  atom;
        
        int childid;
        for (QStringList::iterator it = values.begin(); it != values.end(); )
        {
            childid  = (*it++).toInt();
            name     =  *it++;

            entry.clear();

            atom.m_uds = KIO::UDS_NAME;
            atom.m_str = name;
            entry.append(atom);

            atom.m_uds  = KIO::UDS_FILE_TYPE;
            atom.m_long = S_IFDIR;
            entry.append(atom);

            atom.m_uds  = KIO::UDS_ACCESS;
            atom.m_long = S_IRUSR | S_IRGRP | S_IROTH |
                          S_IWUSR | S_IWGRP | S_IWOTH;
            entry.append(atom);

            atom.m_uds = KIO::UDS_URL;
            xurl.setProtocol("digikamtags");
            xurl.setPath(url.path(1) + QString::number(childid));
            atom.m_str = xurl.url();
            entry.append(atom);

            listEntry(entry, false);
        }

        // if host app told us to recursively get items from
        // sub tags, do so.
        bool recurse = false;
        if (url.queryItem("recurse") == "yes")
        {
            recurse = true;
        }
        
        // now list files
        buildAlbumMap();
        listDir(url, id, recurse);

        m_items.clear();
    }
        
        
    KIO::UDSEntry entry;
    listEntry(entry, true);
    
    finished();
}

void kio_digikamtagsProtocol::listDir(const KURL& url, int tagid, bool recurse)
{
    QStringList values;

    static const QString sqlStr = "SELECT dirid, name "
                                  "FROM ImageTags "
                                  "WHERE tagid=%1  "
                                  "ORDER BY name;";
 
    execSql( sqlStr.arg(tagid), &values );

    QString path;
    KURL    xurl;
    int     dirid;
    QString name;

    KIO::UDSEntry entry;
    KIO::UDSAtom  atom;
    
    for (QStringList::iterator it = values.begin(); it != values.end(); )
    {
        dirid = (*(it++)).toInt();
        name  =  *it++;
        path  = QDir::cleanDirPath( m_libraryPath + QString("/") +
                                    m_albumMap[dirid] + "/" + name );

        // check if this item is already between listed to avoid duplicate items
        // (this problem arises when you have same item under different subtags)
        if (std::binary_search(m_items.begin(), m_items.end(), path))
        {
            continue;
        }
        m_items.push_back(path);
        
        struct stat st;
        if (::stat(QFile::encodeName(path), &st) != 0)
            continue;

        entry.clear();

        atom.m_uds  = KIO::UDS_FILE_TYPE;
        atom.m_long = S_IFREG;
        entry.append(atom);

        atom.m_uds  = KIO::UDS_ACCESS;
        atom.m_long = st.st_mode & 07777;
        entry.append(atom);

        atom.m_uds  = KIO::UDS_SIZE;
        atom.m_long = st.st_size;
        entry.append( atom );

        atom.m_uds = KIO::UDS_MODIFICATION_TIME;
        atom.m_long = st.st_mtime;
        entry.append( atom );

        atom.m_uds  = KIO::UDS_ACCESS_TIME;
        atom.m_long = st.st_atime;
        entry.append( atom );

        atom.m_uds = KIO::UDS_URL;
        xurl.setProtocol("file");
        xurl.setPath(path);
        atom.m_str = xurl.url();
        entry.append(atom);

        atom.m_uds = KIO::UDS_NAME;
        atom.m_str = xurl.fileName();
        entry.append( atom );
        
        // TODO: for now we pass the dirid for this item
        // as the xml_properties. once kde 3.2 becomes a
        // requirement for kde, change this to UDS_EXTRA
        atom.m_uds = KIO::UDS_XML_PROPERTIES;
        atom.m_str = QString::number(dirid);
        entry.append( atom );
        
        listEntry(entry, false);
    }

    m_items.sort();

    if (!recurse)
        return;

    // recursively list files in subtags
    values.clear();
    execSql( QString("SELECT id, name FROM Tags where pid=%1;")
             .arg(QString::number(tagid)),
             &values );

    if (values.isEmpty())
        return;

    int childid;
    for (QStringList::iterator it = values.begin(); it != values.end(); )
    {
        childid  = (*it++).toInt();
        
        xurl.setProtocol("digikamtags");
        xurl.setPath(url.path(1) + (*it++));
        listDir(xurl, childid, recurse);
    }
}

void kio_digikamtagsProtocol::buildAlbumMap()
{
    m_albumMap.clear();

    static const QString sqlStr = "SELECT id, url FROM Albums;";

    QStringList values;
    execSql(sqlStr, &values);

    int     id;
    QString url;
    for (QStringList::iterator it = values.begin(); it != values.end(); )
    {
        id  = (*it++).toInt();
        url = *it++;
        m_albumMap.insert(id, url);
    }
}

bool kio_digikamtagsProtocol::execSql(const QString& sql,
                                      QStringList* const values, 
                                      const bool debug)
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

/* KIO slave registration */

extern "C"
{
    DIGIKAMIMAGEPLUGINS_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalogue("digikam");
        KInstance instance( "kio_digikamtags" );
        ( void ) KGlobal::locale();
        
        kdDebug() << "*** kio_digikamtag started ***" << endl;
        
        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamtags  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamtagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        kdDebug() << "*** kio_digikamtags finished ***" << endl;
        return 0;
    }
}

