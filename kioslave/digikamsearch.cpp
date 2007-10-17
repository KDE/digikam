/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2005-04-21
 * Description : a kio-slave to process search on digiKam albums
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// C++ includes.

#include <cstdlib>

// Qt includes.

#include <QCoreApplication>

// KDE includes.

#include <klocale.h>
#include <kcomponentdata.h>
#include <kdebug.h>

// Local includes.

#include "digikam_export.h"
#include "imagelister.h"
#include "imagequerybuilder.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "digikamsearch.h"

kio_digikamsearch::kio_digikamsearch(const QByteArray &pool_socket,
                                     const QByteArray &app_socket)
                 : SlaveBase("kio_digikamsearch", pool_socket, app_socket)
{
}

kio_digikamsearch::~kio_digikamsearch()
{
}

void kio_digikamsearch::special(const QByteArray& data)
{
    KUrl    kurl;
    int     listingType = 0;

    QDataStream ds(data);
    ds >> kurl;
    if (!ds.atEnd())
        ds >> listingType;

    kDebug() << "kio_digikamsearch::special " << kurl;

    Digikam::DatabaseUrl dbUrl(kurl);
    Digikam::DatabaseAccess::setParameters(dbUrl);

    Digikam::ImageQueryBuilder queryBuilder;
    QList<QVariant> boundValues;
    QString query = queryBuilder.buildQuery(dbUrl.searchUrl(), boundValues);

    Digikam::ImageLister lister;

    if (listingType == 0)
    {
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
        lister.listSearch(&receiver, query, boundValues);
        if (!receiver.hasError)
            receiver.sendData();
    }
    else
    {
        Digikam::ImageListerSlaveBaseReceiver receiver(this);
        // fast mode: do not get size, dimension, limit results to 500
        lister.listSearch(&receiver, query, boundValues, 500);
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
        // Needed to load SQL driver plugins
        QCoreApplication app(argc, argv);

        KLocale::setMainCatalog("digikam");
        KComponentData componentData( "kio_digikamsearch" );
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

