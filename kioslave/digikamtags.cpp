/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a kio-slave to process tag query on
 *               digiKam albums.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// C Ansi includes.

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

// Qt incudes.

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qdir.h>
#include <qregexp.h>

// KDE includes.

#include <kinstance.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <kfilemetainfo.h>

// Local includes.

#include "digikam_export.h"
#include "digikamtags.h"

kio_digikamtagsProtocol::kio_digikamtagsProtocol(const QCString &pool_socket,
                                                 const QCString &app_socket)
                       : SlaveBase("kio_digikamtags", pool_socket, app_socket)
{
}

kio_digikamtagsProtocol::~kio_digikamtagsProtocol()
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

void kio_digikamtagsProtocol::special(const QByteArray& data)
{
    bool folders = (metaData("folders") == "yes");

    QString libraryPath;
    KURL    kurl;
    QString url;
    QString filter;
    int     getDimensions;
    int     tagID;
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

    if (folders)       // Special mode to stats all tag items
    {
        QMap<int, int> tagsStatMap;
        int            tagID, imageID;
        QStringList    values, allTagIDs;

        // initialize allTagIDs with all existing tags from db to prevent
        // wrong tag counters
        m_db.execSql(QString("SELECT id from Tags"), &allTagIDs);

        for ( QStringList::iterator it = allTagIDs.begin(); it != allTagIDs.end(); ++it)
        {
            tagID = (*it).toInt();
            tagsStatMap.insert(tagID, 0);
        }

        // now we can count the tags assigned to images
        m_db.execSql(QString("SELECT ImageTags.tagid, Images.name FROM ImageTags, Images "
                "WHERE ImageTags.imageid=Images.id"), &values);

        for ( QStringList::iterator it = values.begin(); it != values.end(); )
        {
            tagID = (*it).toInt();
            ++it;

            if ( matchFilterList( regex, *it ) )
            {
                QMap<int, int>::iterator it2 = tagsStatMap.find(tagID);
                if ( it2 == tagsStatMap.end() )
                    tagsStatMap.insert(tagID, 1);
                else
                    tagsStatMap.replace(tagID, it2.data() + 1);
            }

            ++it;
        }

        QDataStream os(ba, IO_WriteOnly);
        os << tagsStatMap;
    }
    else
    {
        tagID = QStringList::split('/',url).last().toInt();

        QStringList values;

        if (recurseTags)
        {
            // Obtain all images with the given tag, or with this tag as a parent.
            m_db.execSql( QString( "SELECT DISTINCT Images.id, Images.name, Images.dirid, \n "
                                "       Images.datetime, Albums.url \n "
                                " FROM Images, Albums \n "
                                " WHERE Images.id IN \n "
                                "       (SELECT imageid FROM ImageTags \n "
                                "        WHERE tagid=%1 \n "
                                "           OR tagid IN (SELECT id FROM TagsTree WHERE pid=%2)) \n "
                                "   AND Albums.id=Images.dirid \n " )
                        .arg(tagID)
                        .arg(tagID), &values );
        }
        else
        {
            // Obtain all images with the given tag
            m_db.execSql( QString( "SELECT DISTINCT Images.id, Images.name, Images.dirid, \n "
                                    "       Images.datetime, Albums.url \n "
                                    " FROM Images, Albums \n "
                                    " WHERE Images.id IN \n "
                                    "       (SELECT imageid FROM ImageTags \n "
                                    "        WHERE tagid=%1) \n "
                                    "   AND Albums.id=Images.dirid \n " )
                        .arg(tagID), &values );
        }

        Q_LLONG imageid;
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

    SlaveBase::data(ba);

    finished();
}

/* KIO slave registration */

extern "C"
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
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

