/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-27
 * Description : gui database error handler
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DATABASEGUIERRORHANDLER_H
#define DATABASEGUIERRORHANDLER_H

// Qt includes

#include <QThread>

// Local includes

#include "dbenginebackend.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class DatabaseConnectionChecker : public QThread
{
    Q_OBJECT

public:

    explicit DatabaseConnectionChecker(const DbEngineParameters& parameters);
    ~DatabaseConnectionChecker();

    bool checkSuccessful() const;

public Q_SLOTS:

    void stopChecking();

protected:

    virtual void run();

Q_SIGNALS:

    void failedAttempt();
    void done();

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------

class DatabaseGUIErrorHandler : public DbEngineErrorHandler
{
    Q_OBJECT

public:

    explicit DatabaseGUIErrorHandler(const DbEngineParameters& parameters);
    ~DatabaseGUIErrorHandler();

    bool checkDatabaseConnection();

public Q_SLOTS:

    virtual void connectionError(DbEngineErrorAnswer* answer, const QSqlError& error, const QString& query);
    virtual void consultUserForError(DbEngineErrorAnswer* answer, const QSqlError& error, const QString& query);

private Q_SLOTS:

    void showProgressDialog();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* DATABASEGUIERRORHANDLER_H */
