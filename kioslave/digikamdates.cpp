/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process date query on 
 *               digiKam albums. 
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C ansi includes.

extern "C"
{
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
}

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <ctime>

// Qt includes.

#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qbuffer.h>

// KDE includes.

#include <kio/global.h>
#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kfilemetainfo.h>
#include <kdebug.h>

// Local includes.

#include "digikam_export.h"
#include "digikamdates.h"

kio_digikamdates::kio_digikamdates(const QCString &pool_socket,
                                   const QCString &app_socket)
                : SlaveBase("kio_digikamdates", pool_socket, app_socket)
{
}

kio_digikamdates::~kio_digikamdates()
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

void kio_digikamdates::special(const QByteArray& data)
{
    bool folders = (metaData("folders") == "yes");

    QString libraryPath;
    KURL    kurl;
    QString url;
    QString filter;
    int     getDimensions;
    int     recurseAlbums;
    int     recurseTags;

    QDataStream ds(data, IO_ReadOnly);
    ds >> libraryPath;
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;
    ds >> recurseAlbums;
    ds >> recurseTags;

    url = kurl.path();

    QValueList<QRegExp> regex = makeFilterList(filter);

    if (m_libraryPath != libraryPath)
    {
        m_libraryPath = libraryPath;
        m_db.closeDB();
        m_db.openDB(libraryPath);
    }

    QByteArray  ba;

    if (folders)       // Special mode to stats all dates from collection
    {
        QMap<QDateTime, int> datesStatMap;
        QStringList          values;
        QString              name, dateStr;
        QDateTime            dateTime;

        m_db.execSql( "SELECT name, datetime FROM Images;", &values );

        for ( QStringList::iterator it = values.begin(); it != values.end(); )
        {
            name    = *it;
            ++it;
            dateStr = *it;
            ++it;

            if ( !matchFilterList( regex, name ) )
                continue;

            dateTime = QDateTime::fromString( dateStr, Qt::ISODate );
            if ( !dateTime.isValid() )
                continue;

            QMap<QDateTime, int>::iterator it2 = datesStatMap.find(dateTime);
            if ( it2 == datesStatMap.end() )
            {
                datesStatMap.insert( dateTime, 1 );
            }
            else
            {
                datesStatMap.replace( dateTime, it2.data() + 1 );
            }
        }

        QDataStream os(ba, IO_WriteOnly);
        os << datesStatMap;
    }
    else
    {
        QStringList subpaths = QStringList::split("/", url, false);
        if (subpaths.count() == 4)
        {
            int yrStart = QString(subpaths[0]).toInt();
            int moStart = QString(subpaths[1]).toInt();
            int yrEnd   = QString(subpaths[2]).toInt();
            int moEnd   = QString(subpaths[3]).toInt();

            QString moStartStr, moEndStr;
            moStartStr.sprintf("%.2d", moStart);
            moEndStr.sprintf("%.2d", moEnd);

            QStringList values;

            m_db.execSql(QString("SELECT Images.id, Images.name, Images.dirid, \n "
                                 "  Images.datetime, Albums.url \n "
                                 "FROM Images, Albums \n "
                                 "WHERE Images.datetime < '%1-%2-01' \n "
                                 "AND Images.datetime >= '%3-%4-01' \n "
                                 "AND Albums.id=Images.dirid \n "
                                 "ORDER BY Albums.id;")
                         .arg(yrEnd, 4)
                         .arg(moEndStr, 2)
                         .arg(yrStart, 4)
                         .arg(moStartStr, 2),
                         &values, false);

            Q_LLONG     imageid;
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
                imageid = (*it).toLongLong();
                ++it;
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

                path = m_libraryPath + purl + '/' + name;
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

                *os << imageid;
                *os << dirid;
                *os << name;
                *os << date;
                *os << static_cast<size_t>(stbuf.st_size);
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

/* KIO slave registration */

extern "C"  
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
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

