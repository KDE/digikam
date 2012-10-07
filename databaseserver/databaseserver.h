/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2012 by Gilles Caulier<caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QDBusVariant>
#include <QObject>

// Local includes

#include "databaseservererror.h"

class QCoreApplication;

namespace Digikam
{

class DatabaseServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.digikam.DatabaseServer")

public:

    explicit DatabaseServer(QCoreApplication* const application = 0);
    ~DatabaseServer();

    DatabaseServerError createDatabase();
    void registerOnDBus();
    void startPolling();
    DatabaseServerError startMYSQLDatabaseProcess();

public Q_SLOTS:

    bool startDatabaseProcess(const QString& dbType, QDBusVariant& error);
    bool startDatabaseProcess(QDBusVariant& error);
    void stopDatabaseProcess();
    bool isRunning();

private:

    class DatabaseServerPriv;
    DatabaseServerPriv* const d;
};

} // namespace Digikam

#endif /* DATABASESERVER_H_ */
