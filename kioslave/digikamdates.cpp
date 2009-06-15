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

#include "digikamdates.h"

// C++ includes

#include <cstdlib>

// Qt includes

#include <QCoreApplication>
#include <QFile>
#include <QDataStream>

// KDE includes

#include <kio/global.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kdebug.h>

// Local includes

#include "digikam_export.h"
#include "imagelister.h"
#include "albumdb.h"
#include "databaseaccess.h"

kio_digikamdates::kio_digikamdates(const QByteArray& pool_socket,
                                   const QByteArray& app_socket)
                : SlaveBase("kio_digikamdates", pool_socket, app_socket)
{
}

kio_digikamdates::~kio_digikamdates()
{
}

void kio_digikamdates::special(const QByteArray& data)
{
    bool folders = (metaData("folders") == "true");

    KUrl    kurl;
    QString url;
    QString filter;

    QDataStream ds(data);
    ds >> kurl;

    Digikam::DatabaseUrl dbUrl(kurl);
    Digikam::DatabaseAccess::setParameters(dbUrl);

    url = kurl.path();

    if (folders)
    {
        QMap<QDateTime, int> dateNumberMap = Digikam::DatabaseAccess().db()->getAllCreationDatesAndNumberOfImages();

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << dateNumberMap;
        SlaveBase::data(ba);
    }
    else
    {
        Digikam::ImageLister lister;
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
        lister.list(&receiver, kurl);
        // send rest
        receiver.sendData();
    }

    finished();
}

/* KIO slave registration */

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char **argv)
    {
        // Needed to load SQL driver plugins
        QCoreApplication app(argc, argv);

        KLocale::setMainCatalog("digikam");
        KComponentData componentData( "kio_digikamdates" );
        KGlobal::locale();

        if (argc != 4) {
            kDebug(50004) << "Usage: kio_digikamdates  protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        kio_digikamdates slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }
}
