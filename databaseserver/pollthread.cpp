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
 * Copyright (C) 2010-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "pollthread.h"

// Qt includes

#include <QString>
#include <QtGlobal>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QSystemSemaphore>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

PollThread::PollThread(QObject* const parent)
    : QThread(parent),
      stop(false),
      m_waitTime(10)
{
}

void PollThread::run()
{
    do
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Waiting " << m_waitTime << " seconds...stop: [" << stop << "]";
        sleep(m_waitTime);
    }
    while (!stop && checkDigikamInstancesRunning());

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Shutting down database server";
    emit done();
}

bool PollThread::checkDigikamInstancesRunning()
{
    QSystemSemaphore sem(QLatin1String("DigikamDBSrvAccess"), 1, QSystemSemaphore::Open);
    sem.acquire();
    QDBusConnectionInterface* const interface = QDBusConnection::sessionBus().interface();
    QDBusReply<QStringList> reply             = interface->registeredServiceNames();

    if (reply.isValid())
    {
        QStringList serviceNames = reply.value();
        QLatin1String digikamStartupService("org.kde.digikam.startup-");
        QLatin1String digikamService("org.kde.digikam-");

        foreach(const QString& service, serviceNames)
        {
            if (service.startsWith(digikamStartupService) || service.startsWith(digikamService)
            {
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "At least service [" << service << "] is using the database server";

                // At least one digikam service was found
                sem.release(1);

                return true;
            }
        }
    }

    sem.release(1);

    return false;
}

} // namespace Digikam
