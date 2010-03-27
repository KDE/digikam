/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
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

#ifndef DATABASESERVER_H_
#define DATABASESERVER_H_

// QT includes
#include <QProcess>
#include <QString>
#include <QDBusAbstractAdaptor>
#include <QCoreApplication>

// Local includes

#include <pollthread.h>

namespace Digikam
{

class DatabaseServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.digikam.DatabaseServer")

    public:
        DatabaseServer(QCoreApplication *application);
        void createDatabase();
        void registerOnDBus();
        void startPolling();
        void startMYSQLDatabaseProcess();

    public Q_SLOTS:
        void startDatabaseProcess(const QString dbType);
        void startDatabaseProcess();
        void stopDatabaseProcess();
        bool isRunning();


    private:
        QProcess *mDatabaseProcess;
        QString internalDBName;
        QCoreApplication *app;
        PollThread *pollThread;
};

}

#endif /* DATABASESERVER_H_ */
