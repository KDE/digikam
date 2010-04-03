/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-05
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

DatabaseConnectionChecker::DatabaseConnectionChecker(DatabaseParameters parameters)
                         : m_stop(false), m_success(false), m_parameters(parameters)
{
}

void DatabaseConnectionChecker::run()
{
    QString databaseID("ConnectionTest");
    QSqlDatabase databaseHandler = QSqlDatabase::addDatabase(m_parameters.databaseType, databaseID);
    databaseHandler.setHostName(m_parameters.hostName);
    databaseHandler.setPort(m_parameters.port);

    databaseHandler.setDatabaseName(m_parameters.databaseName);

    databaseHandler.setUserName(m_parameters.userName);
    databaseHandler.setPassword(m_parameters.password);

    databaseHandler.setConnectOptions(m_parameters.connectOptions);

    int iteration = 1;
    while (!m_stop)
    {
        if (databaseHandler.open())
        {
            m_stop = true;
            m_success = true;
            databaseHandler.close();
            break;
        }
        else
        {
            emit failedAttempt();
            m_success = false;
            kError(50003) << "Error while opening the database. Error details [" << databaseHandler.lastError() << "]";
            QMutexLocker lock(&m_mutex);
            if (!m_stop)
            {
                int waitingTime = qMin(2000, iteration++*200);
                m_condVar.wait(&m_mutex, waitingTime);
            }
        }
    }

    QSqlDatabase::removeDatabase(databaseID);
    emit done();
}

void DatabaseConnectionChecker::stopChecking()
{
    QMutexLocker lock(&m_mutex);
    m_stop = true;
    m_condVar.wakeAll();
}

bool DatabaseConnectionChecker::checkSuccessful() const
{
    return m_success;
}

// ---------------------------------------------------------------------------------------

DatabaseGUIErrorHandler::DatabaseGUIErrorHandler(const DatabaseParameters& parameters)
                       : m_parameters(parameters), m_checker(0)
{
}

DatabaseGUIErrorHandler::~DatabaseGUIErrorHandler()
{
}

bool DatabaseGUIErrorHandler::checkDatabaseConnection()
{
    // now we try to connect periodically to the database
    m_checker = new DatabaseConnectionChecker(m_parameters);
    QEventLoop loop;

    connect(m_checker, SIGNAL(failedAttempt()),
            this, SLOT(showProgressDialog()));

    connect(m_checker, SIGNAL(done()),
            &loop, SLOT(quit()));

    m_checker->start();
    loop.exec();

    if (m_dialog)
        delete m_dialog;

    // ensure that the connection thread is closed
    m_checker->wait();

    bool result = m_checker->checkSuccessful();
    delete m_checker;
    return result;
}

void DatabaseGUIErrorHandler::showProgressDialog()
{
    if (m_dialog || !m_checker)
        return;

    m_dialog = new KProgressDialog;
    m_dialog->setModal(true);
    m_dialog->setAttribute(Qt::WA_DeleteOnClose);
    m_dialog->showCancelButton(true);
    m_dialog->progressBar()->setMinimum(0);
    m_dialog->progressBar()->setMaximum(0);
    m_dialog->setLabelText(i18n("Error while opening the database.\nDigikam will try to automatically reconnect to the database."));

    connect(m_dialog, SIGNAL(rejected()),
            m_checker, SLOT(stopChecking()));

    m_dialog->show();
}

void DatabaseGUIErrorHandler::databaseError(DatabaseErrorAnswer* answer, const SqlQuery& query)
{
    if (query.lastError().type() == QSqlError::ConnectionError || query.lastError().number()==2006)
    {
        if (checkDatabaseConnection())
        {
            answer->connectionErrorAbortQueries();
        }
        else
        {
            answer->connectionErrorContinueQueries();
        }
    }
    else
    {
        QWidget* parent = QWidget::find(0);
        // Handle all other database errors
        QString message = i18n("<p><b>A database error occurred.</b></p>"
                                "Details:\n"
                                "%1", query.lastError().text());
        KMessageBox::error(parent, message);
        answer->connectionErrorAbortQueries();
    }
}

}  // namespace Digikam
