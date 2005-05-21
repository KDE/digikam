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
#include <kurl.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <kfilemetainfo.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qdir.h>
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

#include "digikamtags.h"

kio_digikamtagsProtocol::kio_digikamtagsProtocol(const QCString &pool_socket,
                                                 const QCString &app_socket)
    : SlaveBase("kio_digikamtags", pool_socket, app_socket)
{
    m_db    = 0;
}

kio_digikamtagsProtocol::~kio_digikamtagsProtocol()
{
    closeDB();
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

void kio_digikamtagsProtocol::openDB()
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

void kio_digikamtagsProtocol::closeDB()
{
    if (m_db)
    {
        sqlite_close(m_db);
        m_db = 0;
    }
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

typedef struct _IDPair
{
    int id;
    int pid;
} IDPair;

typedef QValueList<IDPair> IDPairList;

static bool isChildOf(int givenID, int pid, const IDPairList& idList)
{
    if (pid == givenID)
        return true;

    if (pid == 0)
        return false;

    int gid = -1;
    for (IDPairList::const_iterator it=idList.begin(); it!=idList.end(); ++it)
    {
        if (pid == (*it).id)
        {
            gid = (*it).pid;
            break;
        }
    }

    if (gid == -1)
        return false;

    return isChildOf(givenID, gid, idList);
}

void kio_digikamtagsProtocol::special(const QByteArray& data)
{
    QString libraryPath;
    KURL    kurl;
    QString url;
    QString filter;
    int     recurse;
    int     getDimensions;
    int     tagID;
    
    QDataStream ds(data, IO_ReadOnly);
    ds >> libraryPath;
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;
    ds >> recurse;

    url = kurl.path();
    
    QValueList<QRegExp> regex = makeFilterList(filter);
    
    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        closeDB();
        openDB();
    }

    tagID = QStringList::split('/',url).last().toInt();
    
    QStringList values;

    if (recurse)
    {
        execSql(QString("SELECT id, pid FROM Tags;"), &values);

        IDPairList idPairList;
        for (QStringList::iterator it = values.begin(); it != values.end(); )
        {
            IDPair pair;
            pair.id  = (*it).toInt();
            ++it;
            pair.pid = (*it).toInt();
            ++it;

            idPairList.append(pair);
        }
        values.clear();

        QValueList<int> idList;
        idList.append(tagID);
        for (IDPairList::iterator it = idPairList.begin();
             it != idPairList.end(); ++it)
        {
            if (isChildOf(tagID, (*it).pid, idPairList))
                idList.append((*it).id);
        }

        QString subquery = QString("WHERE (ImageTags.tagid=%1")
                           .arg(tagID);
        for (QValueList<int>::iterator it = idList.begin();
             it != idList.end(); ++it)
        {
            subquery += QString(" OR ImageTags.tagid=%1")
                        .arg(*it);
        }
        subquery += ") ";

        QString query = QString("SELECT DISTINCT ImageTags.name, ImageTags.dirid, "
                                "Images.datetime, Albums.url "
                                "FROM ImageTags, Images, Albums ") +
                        subquery +
                        QString(" AND "
                                "Images.dirid=ImageTags.dirid AND "
                                "Images.name=ImageTags.name AND "
                                "Albums.id=ImageTags.dirid "
                                "ORDER BY Albums.id;");
        execSql(query, &values);
    }
    else
    {
        execSql(QString("SELECT ImageTags.name, ImageTags.dirid, Images.datetime, Albums.url "
                        "FROM ImageTags, Images, Albums "
                        "WHERE ImageTags.tagid=%1 AND "
                        "Images.dirid=ImageTags.dirid AND Images.name=ImageTags.name AND "
                        "Albums.id=ImageTags.dirid "
                        "ORDER BY Albums.id;")
                .arg(tagID), &values);
    }

    QByteArray  ba;
    
    QString name;
    QString path;
    int     dirid;
    QString date;
    QString purl;
    QSize   dims;

    int count = 0;
    QDataStream* os = new QDataStream(ba, IO_WriteOnly);
    
    struct stat stbuf;
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
    
    SlaveBase::data(ba);
    
    finished();
}

/* KIO slave registration */

extern "C"
{
    int kdemain(int argc, char **argv)
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

