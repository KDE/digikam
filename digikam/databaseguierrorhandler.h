/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-27
 * Description : gui database error handler
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#include <QMutex>
#include <QPointer>
#include <QThread>
#include <QWaitCondition>

// KDE includes

#include <kprogressdialog.h>

// Local includes

#include "databasecorebackend.h"
#include "databaseerrorhandler.h"

namespace Digikam
{

class DatabaseConnectionChecker : public QThread
{
    Q_OBJECT

public:

    DatabaseConnectionChecker(const DatabaseParameters parameters);
    bool checkSuccessful() const;

public Q_SLOTS:

    void stopChecking();

protected:

    virtual void run();

Q_SIGNALS:

    void failedAttempt();
    void done();

private:

    bool               m_stop;
    bool               m_success;
    DatabaseParameters m_parameters;
    QMutex             m_mutex;
    QWaitCondition     m_condVar;
};

// --------------------------------------------------------------

class DatabaseGUIErrorHandler : public DatabaseErrorHandler
{
    Q_OBJECT

public:

    DatabaseGUIErrorHandler(const DatabaseParameters& parameters);
    ~DatabaseGUIErrorHandler();

    bool checkDatabaseConnection();

    virtual void databaseError(DatabaseErrorAnswer* answer, const SqlQuery& query);

private Q_SLOTS:

    void showProgressDialog();

private:

    DatabaseParameters         m_parameters;
    QPointer<KProgressDialog>  m_dialog;
    DatabaseConnectionChecker* m_checker;

};

}  // namespace Digikam

#endif /* DATABASEGUIERRORHANDLER_H */
