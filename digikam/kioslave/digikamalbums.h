/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
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
 * ============================================================ */

#ifndef DIGIKAMALBUMS_H
#define DIGIKAMALBUMS_H

#include <kio/slavebase.h>
#include <qvaluelist.h>
#include <qdatetime.h>

typedef struct sqlite sqleet;
class QStringList;

class AlbumInfo
{
public:

    int      id;
    QString  icon;
    QString  url;
    QString  caption;
    QString  collection;
    QDate    date;
};        
    
class kio_digikamalbums : public KIO::SlaveBase
{
public:

    kio_digikamalbums(const QCString &pool_socket,
                      const QCString &app_socket);
    ~kio_digikamalbums();

    void special(const QByteArray& data);

    void get( const KURL& url );
    void put( const KURL& url, int _mode, bool _overwrite, bool _resume );
    void copy( const KURL &src, const KURL &dest, int mode, bool overwrite );
    void rename( const KURL &src, const KURL &dest, bool overwrite );

    void stat( const KURL& url );
    void listDir( const KURL& url );
    void mkdir( const KURL& url, int permissions );
    void chmod( const KURL& url, int permissions );
    void del( const KURL& url, bool isfile);         
    
    
private:

    void openDB();
    void closeDB();
    bool execSql(const QString& sql, QStringList* const values = 0, 
                 const bool debug = false) const;
    QString escapeString(const QString& str) const;

    bool createUDSEntry(const QString& path, KIO::UDSEntry& entry);
    void createDigikamPropsUDSEntry(KIO::UDSEntry& entry);

    void       buildAlbumList();
    AlbumInfo  findAlbum(const QString& url, bool& ok) const;
    bool       findImage(int albumID, const QString& name) const;
    void       addImage(int albumID, const QString& filePath);

private:
    
    mutable sqleet*       m_db;
    QString               m_libraryPath;
    QValueList<AlbumInfo> m_albumList;
};


#endif /* DIGIKAMALBUMS_H */
