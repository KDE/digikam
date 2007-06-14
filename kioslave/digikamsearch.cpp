//Added by qt3to4:
#include <Q3CString>
/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2005-04-21
 * Description : a kio-slave to process search on digiKam albums
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
}

// C++ includes.

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <ctime>

// Qt includes.

#include <qfile.h>
#include <qdatastream.h>
#include <q3textstream.h>
#include <qregexp.h>
#include <qdir.h>
#include <qvariant.h>
#include <qmap.h>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <kinstance.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kio/global.h>
#include <kio/ioslave_defaults.h>
#include <klargefile.h>

// Local includes.

#include "digikam_export.h"
#include "imagelister.h"
#include "imagequerybuilder.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "digikamsearch.h"

kio_digikamsearch::kio_digikamsearch(const Q3CString &pool_socket,
                                     const Q3CString &app_socket)
                 : SlaveBase("kio_digikamsearch", pool_socket, app_socket)
{
}

kio_digikamsearch::~kio_digikamsearch()
{
}

void kio_digikamsearch::special(const QByteArray& data)
{
    KUrl    kurl;
    QString filter;
    int     getDimensions;
    int     listingType = 0;

    QDataStream ds(data, QIODevice::ReadOnly);
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;
    if (!ds.atEnd())
        ds >> listingType;

    kDebug() << "kio_digikamsearch::special " << kurl << endl;

    Digikam::DatabaseUrl dbUrl(kurl);
    Digikam::DatabaseAccess::setParameters(dbUrl);

    Digikam::ImageQueryBuilder queryBuilder;
    QString query = queryBuilder.buildQuery(dbUrl.searchUrl());

    Digikam::ImageLister lister;

    if (listingType == 0)
    {
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
        lister.listSearch(&receiver, query, filter, getDimensions);
        if (!receiver.hasError)
            receiver.sendData();
    }
    else
    {
        Digikam::ImageListerSlaveBaseReceiver receiver(this);
        // fast mode: do not get size, dimension, limit results to 500
        lister.listSearch(&receiver, query, filter, false, false, 500);
        if (!receiver.hasError)
            receiver.sendData();
        //        ds << m_libraryPath + *it;
    }

    finished();
}

/* KIO slave registration */

extern "C"
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalog("digikam");
        KInstance instance( "kio_digikamsearch" );
        KGlobal::locale();

        if (argc != 4) 
        {
            kDebug() << "Usage: kio_digikamsearch  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamsearch slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }
}

