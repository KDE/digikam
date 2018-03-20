/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-05
 * Description : Database engine gui error handler
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dbengineguierrorhandler.h"

// Qt includes

#include <QEventLoop>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>
#include <QWaitCondition>
#include <QProgressDialog>
#include <QMessageBox>
#include <QApplication>
#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class DbEngineConnectionChecker::Private
{

public:

    Private()
    {
        stop    = false;
        success = false;
    }

    bool               stop;
    bool               success;

    QMutex             mutex;
    QWaitCondition     condVar;

    DbEngineParameters parameters;
};

DbEngineConnectionChecker::DbEngineConnectionChecker(const DbEngineParameters& parameters)
    : d(new Private)
{
    d->parameters = parameters;
}

DbEngineConnectionChecker::~DbEngineConnectionChecker()
{
    delete d;
}

void DbEngineConnectionChecker::run()
{
    QString databaseID(QLatin1String("ConnectionTest"));

    // NOTE: wrap this code into bracket to prevent QtSQL plugin warning. See bug #339074 for details.
    {
        QSqlDatabase databaseHandler = QSqlDatabase::addDatabase(d->parameters.databaseType, databaseID);

        databaseHandler.setHostName(d->parameters.hostName);
        databaseHandler.setPort(d->parameters.port);
        databaseHandler.setDatabaseName(d->parameters.databaseNameCore);
        databaseHandler.setUserName(d->parameters.userName);
        databaseHandler.setPassword(d->parameters.password);
        databaseHandler.setConnectOptions(d->parameters.connectOptions);

        int iteration = 1;

        while (!d->stop)
        {
            if (databaseHandler.open())
            {
                d->stop    = true;
                d->success = true;
                databaseHandler.close();
                break;
            }
            else
            {
                emit failedAttempt();
                d->success = false;
                qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while opening the database. Error details ["
                                              << databaseHandler.lastError() << "]";
                QMutexLocker lock(&d->mutex);

                if (!d->stop)
                {
                    int waitingTime = qMin(2000, iteration++ * 200);
                    d->condVar.wait(&d->mutex, waitingTime);
                }
            }
        }
    }

    QSqlDatabase::removeDatabase(databaseID);
    emit done();
}

void DbEngineConnectionChecker::stopChecking()
{
    QMutexLocker lock(&d->mutex);
    d->stop = true;
    d->condVar.wakeAll();
}

bool DbEngineConnectionChecker::checkSuccessful() const
{
    return d->success;
}

// ---------------------------------------------------------------------------------------

class DbEngineGuiErrorHandler::Private
{

public:

    Private()
    {
        checker = 0;
    }

    QPointer<QProgressDialog>  dialog;

    DbEngineParameters         parameters;
    DbEngineConnectionChecker* checker;
};

DbEngineGuiErrorHandler::DbEngineGuiErrorHandler(const DbEngineParameters& parameters)
    : d(new Private)
{
    d->parameters = parameters;
}

DbEngineGuiErrorHandler::~DbEngineGuiErrorHandler()
{
    delete d;
}

bool DbEngineGuiErrorHandler::checkDatabaseConnection()
{
    // now we try to connect periodically to the database
    d->checker = new DbEngineConnectionChecker(d->parameters);
    QEventLoop loop;

    connect(d->checker, &DbEngineConnectionChecker::failedAttempt,
            this, &DbEngineGuiErrorHandler::showProgressDialog);

    connect(d->checker, &DbEngineConnectionChecker::done,
            &loop, &QEventLoop::quit);

    d->checker->start();
    loop.exec();

    delete d->dialog;

    // ensure that the connection thread is closed
    d->checker->wait();

    bool result = d->checker->checkSuccessful();
    delete d->checker;
    return result;
}

void DbEngineGuiErrorHandler::showProgressDialog()
{
    if (d->dialog || !d->checker)
    {
        return;
    }

    d->dialog = new QProgressDialog;
    d->dialog->setModal(true);
    d->dialog->setAttribute(Qt::WA_DeleteOnClose);
    d->dialog->setMinimum(0);
    d->dialog->setMaximum(0);
    d->dialog->setLabelText(i18n("Error while opening the database.\n"
                                 "digiKam will try to automatically reconnect to the database."));

    connect(d->dialog, SIGNAL(rejected()),
            d->checker, SLOT(stopChecking()));

    connect(d->dialog, SIGNAL(canceled()),
            d->checker, SLOT(stopChecking()));

    d->dialog->show();
}

void DbEngineGuiErrorHandler::connectionError(DbEngineErrorAnswer* answer, const QSqlError&, const QString&)
{
    if (checkDatabaseConnection())
    {
        answer->connectionErrorContinueQueries();
    }
    else
    {
        answer->connectionErrorAbortQueries();
    }
}

void DbEngineGuiErrorHandler::consultUserForError(DbEngineErrorAnswer* answer, const QSqlError& error, const QString&)
{
    //NOTE: not used at all currently.
    QWidget* const parent = QWidget::find(0);
    // Handle all other database errors
    QString message = i18n("<p><b>A database error occurred.</b></p>"
                           "Details:\n %1", error.text());
    QMessageBox::critical(parent, qApp->applicationName(), message);
    answer->connectionErrorAbortQueries();
}

}  // namespace Digikam
