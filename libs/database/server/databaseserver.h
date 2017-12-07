/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Mysql internal database server
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QProcess>
#include <QThread>
#include <QString>

// Local includes

#include "databaseservererror.h"
#include "databaseserverstarter.h"
#include "dbengineparameters.h"

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
    DatabaseServerStateEnum databaseServerStateEnum;

public:

    explicit DatabaseServer(const DbEngineParameters& params, DatabaseServerStarter* const parent = DatabaseServerStarter::instance());
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
     * Inits and Starts Mysql server.
     */
    DatabaseServerError startMysqlDatabaseProcess();

    /**
     * Checks if Mysql binaries and database directories exists and creates
     * the latter if necessary.
     */
    DatabaseServerError checkDatabaseDirs() const;

    /**
     * Finds and updates (if necessary) configuration files for the mysql
     * server.
     */
    DatabaseServerError initMysqlConfig() const;

    /**
     * Remove mysql error log files.
     */
    void removeMysqlLogs() const;

    /**
     * Creates initial Mysql database files for internal server.
     */
    DatabaseServerError createMysqlFiles() const;

    /**
     * Starts the server for the internal database.
     */
    DatabaseServerError startMysqlServer();

    /**
     * Creates or connects to database digikam in mysql.
     */
    DatabaseServerError initMysqlDatabase() const;

    /**
     * Return the current user account name.
     */
    QString getcurrentAccountUserName() const;

    /**
     * Returns i18n converted error message and writes to qCDebug.
     */
    QString processErrorLog(QProcess* const process, const QString& msg) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* DATABASE_SERVER_H_ */
