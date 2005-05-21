/* ============================================================
 * File  : digikamdates.cpp
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
#include <kfilemetainfo.h>
#include <kdebug.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qbuffer.h>

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

#include "digikamdates.h"

kio_digikamdates::kio_digikamdates(const QCString &pool_socket,
                                     const QCString &app_socket)
    : SlaveBase("kio_digikamdates", pool_socket, app_socket)
{
    m_db    = 0;
}

kio_digikamdates::~kio_digikamdates()
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

void kio_digikamdates::special(const QByteArray& data)
{
    bool folders = (metaData("folders") == "yes");

    QString libraryPath;
    KURL    kurl;
    QString url;
    QString filter;
    int     getDimensions;
    
    QDataStream ds(data, IO_ReadOnly);
    ds >> libraryPath;
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;

    url = kurl.path();
    
    QValueList<QRegExp> regex = makeFilterList(filter);
    
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        closeDB();
        openDB();
    }

    QByteArray  ba;
    
    if (folders)
    {
        typedef QPair<int, int> MonthYear;
        QMap<MonthYear, bool> monthYears;

        const char* tail;
        sqlite_vm* vm;
        char* errorStr;
        int error;
    
        //compile SQL program to virtual machine
        error = sqlite_compile( m_db, "SELECT name, datetime FROM Images;",
                                &tail, &vm, &errorStr );

        if ( error == SQLITE_OK )
        {
            QString   itemName;
            QDate     itemDate;
            QString   dateStr;

            QDataStream os(ba, IO_WriteOnly);
            
            int number;
            const char** value;
            const char** colName;
            //execute virtual machine by iterating over rows
            while (true)
            {
                error = sqlite_step( vm, &number, &value, &colName );
                if (error == SQLITE_DONE || error == SQLITE_ERROR || number != 2)
                    break;

                itemName = QString::fromLocal8Bit(value[0]);
                if (!matchFilterList(regex, itemName))
                    continue;

                dateStr = QString::fromLocal8Bit(value[1]);
                if (dateStr.isEmpty())
                    continue;
                
                itemDate = QDate::fromString(dateStr, Qt::ISODate);
                if (!itemDate.isValid())
                {
                    continue;
                }

                if (!monthYears.contains(MonthYear(itemDate.year(), itemDate.month())))
                {
                    monthYears.insert(MonthYear(itemDate.year(), itemDate.month()), true);
                    os << itemDate;
                }
            }
    
            //deallocate vm resources
            sqlite_finalize( vm, &errorStr );

            if ( error != SQLITE_DONE )
            {
                kdWarning() << k_funcinfo << "sqlite_step error: "
                            << errorStr << endl;
            }
        }
        else
        {
            kdWarning() << k_funcinfo << "sqlite_compile error: "
                        << errorStr << endl;
            sqlite_freemem( errorStr );
        }
        
    }
    else
    {
        QStringList subpaths = QStringList::split("/", url, false);
        if (subpaths.count() == 2)
        {

            int yr = QString(subpaths[0]).toInt();
            int mo = QString(subpaths[1]).toInt();

            QString moStr1, moStr2;
            moStr1.sprintf("%.2d", mo);
            moStr2.sprintf("%.2d", mo+1);
            
            QStringList values;

            execSql(QString("SELECT Images.name, Images.dirid, Images.datetime, Albums.url "
                            "FROM Images, Albums "
                            "WHERE Images.datetime < '%1-%2-01' "
                            "AND Images.datetime >= '%3-%4-01' "
                            "AND Albums.id=Images.dirid "
                            "ORDER BY Albums.id;") 
                    .arg(yr,4)
                    .arg(moStr2)
                    .arg(yr,4)
                    .arg(moStr1,2),
                    &values, false);

            QString     name;
            QString     path;
            int         dirid;
            QString     date;
            QString     purl;
            QSize       dims;
            struct stat stbuf;

            int  count = 0;
            QDataStream* os = new QDataStream(ba, IO_WriteOnly);
            
            for (QStringList::iterator it = values.begin(); it != values.end();)
            {
                name  = *it;
                ++it;
                dirid = (*it).toInt();
                ++it;
                date  = *it;
                ++it;
                purl  = *it;
                ++it;

                if (!matchFilterList(regex, name))
                    continue;

                path = m_libraryPath + purl + "/" + name;
                if (::stat(QFile::encodeName(path), &stbuf) != 0)
                    continue;

                dims = QSize();
                if (getDimensions)
                {
                    KFileMetaInfo metaInfo(path);
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
                
                *os << dirid;
                *os << name;
                *os << date;
                *os << stbuf.st_size;
                *os << dims;

                count++;
                
                if (count > 200)
                {
                    delete os;
                    os = 0;

                    SlaveBase::data(ba);
                    ba.resize(0);

                    count = 0;
                    os = new QDataStream(ba, IO_WriteOnly);
                }
            }

            delete os;
        }            
    }
    
    SlaveBase::data(ba);
    
    finished();
}

void kio_digikamdates::openDB()
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

void kio_digikamdates::closeDB()
{
    if (m_db)
    {
        sqlite_close(m_db);
    }
    
    m_db = 0;
}

bool kio_digikamdates::execSql(const QString& sql, QStringList* const values, 
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

QString kio_digikamdates::escapeString(const QString& str) const
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
        KInstance instance( "kio_digikamdates" );
        KGlobal::locale();
        
        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamdates  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamdates slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        return 0;
    }
}

