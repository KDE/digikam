/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-08
 * Description : database server starter
 *
 * Copyright (C) 2009 by Holger Foerster <Hamsi2k at freenet dot de>
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
#include <databaseserverstarter.h>

// KDE includes
#include <kdebug.h>
#include <kstandarddirs.h>

// Qt includes
#include <QString>
#include <QStringList>
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
#include <QDBusInterface>
#include <QDBusReply>
#include <QProcess>
#include <QSystemSemaphore>
#include <QApplication>
#include <QThread>

// For whatever reason, these methods are "static protected"
class sotoSleep : public QThread
{
public:

    static void sleep(unsigned long secs)
    {
        QThread::sleep(secs);
    }
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
    static void usleep(unsigned long usecs)
    {
        QThread::usleep(usecs);
    }
};

// Local includes

DatabaseServerStarter::DatabaseServerStarter(QObject *parent=0): QObject(parent)
{
}

void DatabaseServerStarter::startServerManagerProcess()
{
    /*
     * TODO:
     * 1. Acquire semaphore lock on "DigikamDBSrvAccess"
     * 2. Check if there is an database server manager service already registered on DBus
     * 3. If not, start the database server manager
     * 4. Release semaphore lock
     */
  QSystemSemaphore sem("DigikamDBSrvAccess", 1, QSystemSemaphore::Open);
  sem.acquire();
  if (!isServerRegistered())
  {
      const QString dbServerMgrPath("/usr/bin/digikamdatabaseserver");
      if ( dbServerMgrPath.isEmpty() )
        kDebug(50003) << "No path to digikamdatabaseserver set in server manager configuration!";

      const QStringList arguments;

      bool result = QProcess::startDetached( dbServerMgrPath, arguments );
      if ( !result ) {
        kDebug(50003) << "Could not start database server manager !";
        kDebug(50003) << "executable:" << dbServerMgrPath;
        kDebug(50003) << "arguments:" << arguments;
      }
  }



  // wait until the server has successfully registered on DBUS
  // TODO Use another way for that! Sleep isn't good :-/
  for (int i=0; i<30; i++)
  {
      if (!isServerRegistered())
      {
          sotoSleep sleepThread;
          sleepThread.sleep(2);
          sleepThread.wait();
      }else
      {
          break;
      }
  }
  QDBusInterface dbus_iface("org.kde.digikam.DatabaseServer", "/DatabaseServer");
  QDBusMessage stateMsg = dbus_iface.call("isRunning");
  if (!stateMsg.arguments().at(0).toBool())
  {
      dbus_iface.call("startDatabaseProcess");
  }
  sem.release();
}

bool DatabaseServerStarter::isServerRegistered()
{
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    QDBusReply<QStringList> reply = interface->registeredServiceNames();
    if (reply.isValid())
    {
        QStringList serviceNames = reply.value();
        return serviceNames.contains("org.kde.digikam.DatabaseServer");
    }
    return false;
}
