/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-08
 * Description : polling thread checks if there are digikam
 *               components registered on DBus
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
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

#include "pollthread.moc"

// Qt includes

#include <QString>
#include <QtGlobal>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QThread>
#include <QSystemSemaphore>

// KDE includes

#include <kdebug.h>
#include <kstandarddirs.h>

namespace Digikam
{

PollThread::PollThread(QObject* const parent)
    : QThread(parent), stop(false), waitTime(10)
{
}

void PollThread::run()
{
    do
    {
        kDebug() << "Waiting " << waitTime << " seconds...stop: [" << stop << "]";
        sleep(waitTime);
    }
    while (!stop && checkDigikamInstancesRunning());

    kDebug() << "Shutting down database server";
    emit done();
}

bool PollThread::checkDigikamInstancesRunning()
{
    QSystemSemaphore sem("DigikamDBSrvAccess", 1, QSystemSemaphore::Open);
    sem.acquire();
    QDBusConnectionInterface* interface = QDBusConnection::sessionBus().interface();
    QDBusReply<QStringList> reply       = interface->registeredServiceNames();

    if (reply.isValid())
    {
        QStringList serviceNames = reply.value();
        QLatin1String digikamStartupService("org.kde.digikam.startup-");
        QLatin1String digikamService("org.kde.digikam-");
        QLatin1String digikamKioService("org.kde.digikam.KIO-");
        foreach(const QString& service, serviceNames)
        {
            if (service.startsWith(digikamStartupService) ||
                service.startsWith(digikamService)        ||
                service.startsWith(digikamKioService))
            {
                kDebug() << "At least service ["<< service <<"] is using the database server";

                // At least one digikam/kio service was found
                sem.release(1);
                return true;
            }
        }
    }

    sem.release(1);
    return false;
}

} // namespace Digikam
