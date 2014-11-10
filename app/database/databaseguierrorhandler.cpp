/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-05
 * Description : gui database error handler
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2014 by Gilles Caulier<caulier dot gilles at gmail dot com>
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

#include "databaseguierrorhandler.moc"

// Qt includes

#include <QEventLoop>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>

// KDE includes

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kguiitem.h>
#include <kdialog.h>

// Local includes

#include <setup.h>

namespace Digikam
{

class DatabaseConnectionChecker::Private
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

    DatabaseParameters parameters;
};

DatabaseConnectionChecker::DatabaseConnectionChecker(const DatabaseParameters& parameters)
    : d(new Private)
{
    d->parameters = parameters;
}

DatabaseConnectionChecker::~DatabaseConnectionChecker()
{
    delete d;
}

void DatabaseConnectionChecker::run()
{
    QString databaseID("ConnectionTest");

    // NOTE: wrap this code into bracket to prevent QtQSL plugin warning. See bug #339074 for details.
    {
        QSqlDatabase databaseHandler = QSqlDatabase::addDatabase(d->parameters.databaseType, databaseID);

        databaseHandler.setHostName(d->parameters.hostName);
        databaseHandler.setPort(d->parameters.port);
        databaseHandler.setDatabaseName(d->parameters.databaseName);
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
                kError() << "Error while opening the database. Error details ["
                         << databaseHandler.lastError() << "]";
                QMutexLocker lock(&d->mutex);

                if (!d->stop)
                {
                    int waitingTime = qMin(2000, iteration++*200);
                    d->condVar.wait(&d->mutex, waitingTime);
                }
            }
        }
    }

    QSqlDatabase::removeDatabase(databaseID);
    emit done();
}

void DatabaseConnectionChecker::stopChecking()
{
    QMutexLocker lock(&d->mutex);
    d->stop = true;
    d->condVar.wakeAll();
}

bool DatabaseConnectionChecker::checkSuccessful() const
{
    return d->success;
}

// ---------------------------------------------------------------------------------------

class DatabaseGUIErrorHandler::Private
{

public:

    Private()
    {
        checker = 0;
    }

    QPointer<KProgressDialog>  dialog;

    DatabaseParameters         parameters;
    DatabaseConnectionChecker* checker;
};

DatabaseGUIErrorHandler::DatabaseGUIErrorHandler(const DatabaseParameters& parameters)
    : d(new Private)
{
    d->parameters = parameters;
}

DatabaseGUIErrorHandler::~DatabaseGUIErrorHandler()
{
    delete d;
}

bool DatabaseGUIErrorHandler::checkDatabaseConnection()
{
    // now we try to connect periodically to the database
    d->checker = new DatabaseConnectionChecker(d->parameters);
    QEventLoop loop;

    connect(d->checker, SIGNAL(failedAttempt()),
            this, SLOT(showProgressDialog()));

    connect(d->checker, SIGNAL(done()),
            &loop, SLOT(quit()));

    d->checker->start();
    loop.exec();

    delete d->dialog;

    // ensure that the connection thread is closed
    d->checker->wait();

    bool result = d->checker->checkSuccessful();
    delete d->checker;
    return result;
}

void DatabaseGUIErrorHandler::showProgressDialog()
{
    if (d->dialog || !d->checker)
    {
        return;
    }

    d->dialog = new KProgressDialog;
    d->dialog->setModal(true);
    d->dialog->setAttribute(Qt::WA_DeleteOnClose);
    d->dialog->showCancelButton(true);
    d->dialog->progressBar()->setMinimum(0);
    d->dialog->progressBar()->setMaximum(0);
    d->dialog->setLabelText(i18n("Error while opening the database.\n"
                                 "digiKam will try to automatically reconnect to the database."));

    connect(d->dialog, SIGNAL(rejected()),
            d->checker, SLOT(stopChecking()));

    d->dialog->show();
}

void DatabaseGUIErrorHandler::connectionError(DatabaseErrorAnswer* answer, const QSqlError&, const QString&)
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

void DatabaseGUIErrorHandler::consultUserForError(DatabaseErrorAnswer* answer, const QSqlError& error, const QString&)
{
    //NOTE: not used at all currently.
    QWidget* const parent = QWidget::find(0);
    // Handle all other database errors
    QString message = i18n("<p><b>A database error occurred.</b></p>"
                           "Details:\n %1", error.text());
    KMessageBox::error(parent, message);
    answer->connectionErrorAbortQueries();
}

}  // namespace Digikam
