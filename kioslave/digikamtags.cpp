/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a kio-slave to process tag query on
 *               digiKam albums.
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2004 by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "digikamtags.h"

// Qt includes

#include <QCoreApplication>
#include <QDBusConnection>
#include <QString>

// KDE includes

#include <kcomponentdata.h>
#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "digikam_export.h"
#include "imagelister.h"
#include "imagelisterreceiver.h"

kio_digikamtagsProtocol::kio_digikamtagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket)
    : SlaveBase("kio_digikamtags", pool_socket, app_socket)
{
}

kio_digikamtagsProtocol::~kio_digikamtagsProtocol()
{
}

void kio_digikamtagsProtocol::special(const QByteArray& data)
{
    KUrl    kurl;
    QString filter;

    QDataStream ds(data);
    ds >> kurl;

    Digikam::DatabaseParameters dbParameters(kurl);
    QDBusConnection::sessionBus().registerService(QString("org.kde.digikam.KIO-%1")
                                                  .arg(QString::number(QCoreApplication::instance()->applicationPid())));
    Digikam::DatabaseAccess::setParameters(dbParameters);

    bool folders     = (metaData("folders") == "true");
    bool facefolders = (metaData("facefolders") == "true");
    QString special  = metaData("specialTagListing");

    if (folders)
    {
        QMap<int, int> tagNumberMap = Digikam::DatabaseAccess().db()->getNumberOfImagesInTags();

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << tagNumberMap;
        SlaveBase::data(ba);
    }
    else if (facefolders)
    {
        QMap<QString, QMap<int, int> > facesNumberMap;
        facesNumberMap[Digikam::ImageTagPropertyName::autodetectedFace()] =
            Digikam::DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::autodetectedFace());
        facesNumberMap[Digikam::ImageTagPropertyName::tagRegion()]        =
            Digikam::DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::tagRegion());

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << facesNumberMap;
        SlaveBase::data(ba);
    }
    else
    {
        bool recursive = (metaData("listTagsRecursively") == "true");
        bool listOnlyAvailableImages = (metaData("listOnlyAvailableImages") == "true");

        Digikam::ImageLister lister;
        lister.setRecursive(recursive);
        lister.setListOnlyAvailable(listOnlyAvailableImages);
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);

        if (!special.isNull())
        {
            QString searchXml = lister.tagSearchXml(kurl, special, recursive);
            lister.setAllowExtraValues(true); // pass property value as extra value, different binary protocol
            lister.listImageTagPropertySearch(&receiver, searchXml);
        }
        else
        {
            lister.list(&receiver, kurl);
        }

        // finish sending
        receiver.sendData();
    }

    finished();
}

/* KIO slave registration */

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char** argv)
    {
        // Needed to load SQL driver plugins
        QCoreApplication app(argc, argv);

        KLocale::setMainCatalog("digikam");
        KComponentData componentData( "kio_digikamtags" );
        KGlobal::locale();

        kDebug() << "*** kio_digikamtag started ***";

        if (argc != 4)
        {
            kDebug() << "Usage: kio_digikamtags  protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        kio_digikamtagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kDebug() << "*** kio_digikamtags finished ***";
        return 0;
    }
}
