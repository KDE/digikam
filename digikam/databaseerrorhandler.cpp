/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-05
 * Description : gui database error handler
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

#include "databaseerrorhandler.h"
#include "databaseerrorhandler.moc"

// Qt includes
#include <qthread.h>
#include <qstringlist.h>
#include <qpointer.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>

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
     {
         stop=false;
         this->parameters=parameters;
     }

     void DatabaseConnectionChecker::run()
     {
         QString databaseID("ConnectionTest");
         QSqlDatabase databaseHandler = QSqlDatabase::addDatabase(parameters.databaseType, databaseID);
         databaseHandler.setHostName(parameters.hostName);
         databaseHandler.setPort(parameters.port);

         databaseHandler.setDatabaseName(parameters.databaseName);

         databaseHandler.setUserName(parameters.userName);
         databaseHandler.setPassword(parameters.password);

         databaseHandler.setConnectOptions(parameters.connectOptions);

         while (stop==false)
         {
             if (databaseHandler.open())
             {
                 stop=true;
                 databaseHandler.close();
             }else
             {
                 kDebug(50003) << "Error while opening the database. Error details [" << databaseHandler.lastError() << "]";
                 sleep(2);
             }


         }
         QSqlDatabase::removeDatabase(databaseID);
         emit done();
     }

     void DatabaseConnectionChecker::setDialog(QDialog *dialog){
         dialog = dialog;
     }


    DatabaseGUIErrorHandler::DatabaseGUIErrorHandler(DatabaseParameters parameters){
        this->parameters=parameters;
        this->refuseQueries=false;
    }

    DatabaseGUIErrorHandler::~DatabaseGUIErrorHandler(){

    }

    void DatabaseGUIErrorHandler::connectionError(DatabaseErrorAnswer *answer){
        // now we try to connect periodically to the database
        DatabaseConnectionChecker connectionChecker(parameters);

        QWidget* parent = QWidget::find(0);
        KProgressDialog *dialog = new KProgressDialog(parent);
        dialog->progressBar()->setMinimum(0);
        dialog->progressBar()->setMaximum(0);
        dialog->setModal(true);
        dialog->setLabelText(i18n("Error while opening the database.\nDigikam will try to automatically reconnect to the database."));
        connectionChecker.setDialog(dialog);
        dialog->connect(&connectionChecker, SIGNAL(done()), SLOT(accept()));
        connectionChecker.start();


        // We use a QPointer because the dialog may get deleted
        // during exec() if the parent of the dialog gets deleted.
        // In that case the QPointer will reset to 0.
        QPointer<KDialog> guardedDialog = dialog;

        guardedDialog->exec();
        connectionChecker.stop=true;

        // simple for loop to ensure, the connection thread is closed
        connectionChecker.wait();

        if ( dialog->wasCancelled() )
        {
            answer->connectionErrorAbortQueries();
        }else
        {
            answer->connectionErrorContinueQueries();
        }

        delete (KDialog *) guardedDialog;
    }




}  // namespace Digikam
