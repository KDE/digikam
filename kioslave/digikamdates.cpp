/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process date query on
 *               digiKam albums.
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
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

// Qt includes

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDataStream>
#include <QFile>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "digikam_export.h"
#include "imagelister.h"

kio_digikamdates::kio_digikamdates(const QByteArray& pool_socket, const QByteArray& app_socket)
    : SlaveBase("kio_digikamdates", pool_socket, app_socket)
{
}

kio_digikamdates::~kio_digikamdates()
{
}

void kio_digikamdates::special(const QByteArray& data)
{
    bool        folders = (metaData(QLatin1String("folders")) == QLatin1String("true"));
    QUrl        url;
    QDataStream ds(data);
    ds >> url;

    qCDebug(DIGIKAM_KIOSLAVES_LOG) << "Entered kio_digikamdates::special";

    Digikam::DatabaseParameters dbParameters(url);
    QDBusConnection::sessionBus().registerService(QString::fromUtf8("org.kde.digikam.KIO-digikamtags-%1")
                                                  .arg(QString::number(QCoreApplication::instance()->applicationPid())));
    Digikam::DatabaseAccess::setParameters(dbParameters);

    if (folders)
    {
        qCDebug(DIGIKAM_KIOSLAVES_LOG) << "Entered Folders inside KIO";

        QMap<QDateTime, int> dateNumberMap = Digikam::DatabaseAccess().db()->getAllCreationDatesAndNumberOfImages();
        QByteArray           ba;
        QDataStream          os(&ba, QIODevice::WriteOnly);
        os << dateNumberMap;
        SlaveBase::data(ba);
    }
    else
    {
        Digikam::ImageLister lister;
        lister.setListOnlyAvailable(metaData(QLatin1String("listOnlyAvailableImages")) == QLatin1String("true"));
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
        lister.list(&receiver, url);
        // send rest
        receiver.sendData();
    }

    finished();
}

/* KIO slave registration */

extern "C"
{
    Q_DECL_EXPORT int kdemain(int argc, char** argv)
    {
        // Needed to load SQL driver plugins
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("kio_digikamdates"));

        qCDebug(DIGIKAM_KIOSLAVES_LOG) << "*** kio_digikamdates started ***";

        if (argc != 4)
        {
            qCDebug(DIGIKAM_KIOSLAVES_LOG) << "Usage: kio_digikamdates protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        kio_digikamdates slave(argv[2], argv[3]);
        slave.dispatchLoop();

        qCDebug(DIGIKAM_KIOSLAVES_LOG) << "*** kio_digikamdates finished ***";
        return 0;
    }
}
