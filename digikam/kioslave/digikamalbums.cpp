/* ============================================================
 * File  : digikamalbums.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <kio/global.h>
#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kdebug.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>

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

#include "digikamalbums.h"

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
    
    QDataStream ds(data, IO_ReadOnly);
    ds >> libraryPath;
    ds >> url;
    ds >> filter;

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

    struct stat stbuf;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        name = *it;
        ++it;
        date = *it;
        ++it;

        if (!matchFilterList(regex, name))
            continue;

        name = base + name;
        if (::stat(QFile::encodeName(name), &stbuf) != 0)
            continue;
        
        os << id;
        os << base + name;
        os << date;
        os << stbuf.st_size;
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

QString kio_digikamalbums::escapeString(const QString& str) const
{
    QString st(str);
    st.replace( "'", "''" );
    return st;
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
