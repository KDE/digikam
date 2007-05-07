/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2005-04-21
 * Description : a dio-slave to process file operations on 
 *               digiKam albums.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// Qt includes.

#include <qvaluelist.h>
#include <qdatetime.h>

// KDE includes

#include <kio/slavebase.h>

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

    void createDigikamPropsUDSEntry(KIO::UDSEntry& entry);

    bool createUDSEntry(const QString& path, KIO::UDSEntry& entry);
    bool file_get( const KURL& url );
    bool file_put( const KURL& url, int _mode, bool _overwrite, bool _resume );
    bool file_copy( const KURL &src, const KURL &dest, int mode, bool overwrite );
    bool file_rename( const KURL &src, const KURL &dest, bool overwrite );
    bool file_stat( const KURL& url );
    bool file_listDir( const KURL& url );
    bool file_mkdir( const KURL& url, int permissions );
    bool file_chmod( const KURL& url, int permissions );
    bool file_del( const KURL& url, bool isfile);

};


#endif /* DIGIKAMALBUMS_H */
