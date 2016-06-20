/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Mysql internal database server
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016 by Swati Lodha <swatilodha27 at gmail dot com>
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

#ifndef DATABASE_SERVER_H_
#define DATABASE_SERVER_H_

// QT includes

#include <QString>
#include <QVariant>
#include <QObject>
#include <QThread>

// Local includes

#include "databaseservererror.h"

class QCoreApplication;

namespace Digikam
{

class DatabaseServer : public QThread
{
    Q_OBJECT

public:

    enum DatabaseServerStateEnum
    {
        started,
        running,
        notRunning,
        stopped
    };
    DatabaseServer::DatabaseServerStateEnum databaseServerStateEnum;

public:

    explicit DatabaseServer(QObject* const application = 0);
    virtual ~DatabaseServer();

    /**
     * Starts the database management server.
     */
    DatabaseServerError startDatabaseProcess();

    /**
     * Terminates the databaser server process.
     */
    void stopDatabaseProcess();

    /**
     * Returns true if the server process is running.
     */
    bool isRunning() const;

Q_SIGNALS:

    void done();

protected :

    void run();

private:

    /**
     * Init and Starts Mysql server.
     */
    DatabaseServerError startMYSQLDatabaseProcess();

    /**
     * Creates the initial database for the internal server instance.
     */
    DatabaseServerError createDatabase();

    /**
     * Return the current user account name.
     */
    QString getcurrentAccountUserName() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* DATABASE_SERVER_H_ */
