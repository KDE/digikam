/* ============================================================
 * File  : digikamsearch.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Description : 
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
#include <qtextstream.h>
#include <qregexp.h>
#include <qdir.h>
#include <qvariant.h>
#include <qmap.h>

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
#include <utime.h>
}

#include "digikamsearch.h"

kio_digikamsearch::kio_digikamsearch(const QCString &pool_socket,
                                     const QCString &app_socket)
    : SlaveBase("kio_digikamsearch", pool_socket, app_socket)
{
    m_db    = 0;
}

kio_digikamsearch::~kio_digikamsearch()
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

void kio_digikamsearch::special(const QByteArray& data)
{
    QString libraryPath;
    KURL    url;
    QString filter;
    int     getDimensions;
    int     recurse;
    int     listingType;

    QDataStream ds(data, IO_ReadOnly);
    ds >> libraryPath;
    ds >> url;
    ds >> filter;
    ds >> getDimensions;
    ds >> recurse;
    ds >> listingType;

    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        closeDB();
        openDB();
    }

    QValueList<QRegExp> regex = makeFilterList(filter);
    QByteArray ba;
    
    if (listingType == 1)
    {
        //TODO: QString sqlQuery = fullListing(url);
    }
    else
    {
        QString sqlQuery = miniListing(url);
        QStringList values;
        QString     errMsg;
        if (!execSql(sqlQuery, &values, &errMsg))
        {
            error(KIO::ERR_INTERNAL, errMsg);
            return;
        }
    
        QDataStream ds(ba, IO_WriteOnly);
        for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
        {
            if (matchFilterList(regex, *it))
            {
                ds << m_libraryPath + *it;
            }
        }
    }
    
    SlaveBase::data(ba);

    
    finished();
}

QString kio_digikamsearch::miniListing(const KURL& url) const
{
    int  count = url.queryItem("count").toInt();
    if (count <= 0)
        return QString();

    QMap<int, RuleType> rulesMap;
    bool needTagTables = false;
    
    for (int i=1; i<=count; i++)
    {
        RuleType rule;

        QString key = url.queryItem(QString::number(i) + ".key").lower();
        QString op  = url.queryItem(QString::number(i) + ".op").lower();

        if (key == "album")
        {
            rule.key = ALBUM;
        }
        else if (key == "albumname")
        {
            rule.key = ALBUMNAME;
        }
        else if (key == "albumcaption")
        {
            rule.key = ALBUMCAPTION;
        }
        else if (key == "albumcollection")
        {
            rule.key = ALBUMCOLLECTION;
        }
        else if (key == "imagename")
        {
            rule.key = IMAGENAME;
        }
        else if (key == "imagecaption")
        {
            rule.key = IMAGECAPTION;
        }
        else if (key == "imagedate")
        {
            rule.key = IMAGEDATE;
        }
        else if (key == "tag")
        {
            rule.key = TAG;
            needTagTables = true;
        }
        else if (key == "tagname")
        {
            rule.key = TAGNAME;
            needTagTables = true;
        }
        else
        {
            kdWarning() << "Unknown rule type: " << key << " passed to kioslave"
                        << endl;
            continue;
        }

        if (op == "eq")
            rule.op = EQ;
        else if (op == "ne")
            rule.op = NE;
        else if (op == "lt")
            rule.op = LT;
        else if (op == "gt")
            rule.op = GT;
        else if (op == "like")
            rule.op = LIKE;
        else if (op == "nlike")
            rule.op = NLIKE;
        else
        {
            kdWarning() << "Unknown op type: " << op << " passed to kioslave"
                        << endl;
            continue;
        }

        rule.val = url.queryItem(QString::number(i) + ".val");

        rulesMap.insert(i, rule);
    }

    QString sqlQuery = "SELECT Albums.url||'/'||Images.name "
                       "FROM Images, Albums "
                       "WHERE ( ";
    
    QStringList strList = QStringList::split(" ", url.path());
    for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it )
    {
        bool ok;
        int  num = (*it).toInt(&ok);
        if (ok)
        {
            RuleType rule = rulesMap[num];
            sqlQuery += subQuery(rule.key, rule.op, rule.val);
        }
        else 
        {
            sqlQuery += " " + *it + " ";
        }
    }                     

    sqlQuery += " ) ";
    sqlQuery += " AND (Albums.id=Images.dirid) ";
    sqlQuery += " LIMIT 500;";

    return sqlQuery;
}

QString kio_digikamsearch::fullListing(const KURL&) const
{
    /* TODO */
    return QString();
}

QString kio_digikamsearch::subQuery(enum kio_digikamsearch::SKey key,
                                    enum kio_digikamsearch::SOperator op,
                                    const QString& val) const
{
    QString query;

    switch (key)
    {
    case(ALBUM):
    {
        query = " (Images.dirid $$##$$ $$@@$$) ";
        break;
    }
    case(ALBUMNAME):
    {
        query = " (Images.dirid IN "
                "  (SELECT id FROM Albums WHERE url $$##$$ $$@@$$)) ";
        break;
    }
    case(ALBUMCAPTION):
    {
        query = " (Images.dirid IN "
                "  (SELECT id FROM Albums WHERE caption $$##$$ $$@@$$)) ";
        break;
    }
    case(ALBUMCOLLECTION):
    {
        query = " (Images.dirid IN "
                "  (SELECT id FROM Albums WHERE collection $$##$$ $$@@$$)) ";
        break;
    }
    case(TAG):
    {
        query = " (Images.dirid||','||Images.name IN "
                "   (SELECT dirid||','||name FROM ImageTags "
                "    WHERE tagid $$##$$ $$@@$$)) ";
        break;
    }
    case(TAGNAME):
    {
        query = " (Images.dirid||','||Images.name IN "
                "   (SELECT dirid||','||name FROM ImageTags "
                "    WHERE tagid IN "
                "    (SELECT id FROM Tags WHERE name $$##$$ $$@@$$))) ";
        break;
    }
    case(IMAGENAME):
    {
        query = " (Images.name $$##$$ $$@@$$) ";
        break;
    }
    case(IMAGECAPTION):
    {
        query = " (Images.caption $$##$$ $$@@$$) ";
        break;
    }
    case(IMAGEDATE):
    {
        query = " (Images.datetime $$##$$ $$@@$$) ";
        break;
    }
    }

    switch (op)
    {
    case(EQ):
    {
        query.replace("$$##$$", "=");
        query.replace("$$@@$$", QString::fromLatin1("'") + escapeString(val)
                      + QString::fromLatin1("'"));
        break;
    }
    case(NE):
    {
        query.replace("$$##$$", "<>");
        query.replace("$$@@$$", QString::fromLatin1("'") + escapeString(val)
                      + QString::fromLatin1("'"));
        break;
    }
    case(LT):
    {
        query.replace("$$##$$", "<");
        query.replace("$$@@$$", QString::fromLatin1("'") + escapeString(val)
                      + QString::fromLatin1("'"));
        break;
    }
    case(GT):
    {
        query.replace("$$##$$", ">");
        query.replace("$$@@$$", QString::fromLatin1("'") + escapeString(val)
                      + QString::fromLatin1("'"));
        break;
    }
    case(LIKE):
    {
        query.replace("$$##$$", "LIKE");
        query.replace("$$@@$$", QString::fromLatin1("'%") + escapeString(val)
                      + QString::fromLatin1("%'"));
        break;
    }
    case(NLIKE):
    {
        query.replace("$$##$$", "NOT LIKE");
        query.replace("$$@@$$", QString::fromLatin1("%") + escapeString(val)
                      + QString::fromLatin1("%"));
        break;
    }
    }
    
    return query;
}

void kio_digikamsearch::openDB()
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

void kio_digikamsearch::closeDB()
{
    if (m_db)
    {
        sqlite_close(m_db);
    }
    
    m_db = 0;
}

bool kio_digikamsearch::execSql(const QString& sql, QStringList* const values, 
                                QString* const errMsg, const bool debug) const
{
    if ( debug )
        kdDebug() << "SQL-query: " << sql << endl;

    if ( !m_db )
    {
        if (errMsg)
            *errMsg = i18n("Database not open");
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

    if ( error != SQLITE_OK )
    {
        if (errMsg)
            *errMsg = i18n("Sqlite compile error: %1 on query: %2")
                      .arg(errorStr)
                      .arg(sql);
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

    if ( error != SQLITE_DONE )
    {
        if (errMsg)
            *errMsg = i18n("Sqlite step error: %1 on query: %2")
                      .arg(errorStr)
                      .arg(sql);
        kdWarning() << k_funcinfo << "sqlite_step error: "
                    << errorStr
                    << " on query: " << sql << endl;
        return false;
    }

    return true;
}

QString kio_digikamsearch::escapeString(const QString& str) const
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
        KInstance instance( "kio_digikamsearch" );
        KGlobal::locale();
        
        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamsearch  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamsearch slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        return 0;
    }
}

