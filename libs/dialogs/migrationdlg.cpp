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

#include "migrationdlg.h"

// QT includes
#include <qgridlayout.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qwidget.h>
#include <qlist.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "databasewidget.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "databaseparameters.h"

namespace Digikam
{
    MigrationDlg::MigrationDlg(QWidget* parent): KDialog(parent)
    {
        setupMainArea();
    }

    MigrationDlg::~MigrationDlg()
    {

    }

    void MigrationDlg::setupMainArea()
    {
        fromDatabaseWidget              = new DatabaseWidget(this);
        toDatabaseWidget                = new DatabaseWidget(this);
        QPushButton *migrateBtn         = new QPushButton(i18n("Migrate ->"), this);
        QProgressBar *progressBar       = new QProgressBar();

        QWidget *mainWidget     = new QWidget;
        QGridLayout *layout     = new QGridLayout;
        mainWidget->setLayout(layout);

        layout->addWidget(fromDatabaseWidget, 0,0);
        layout->addWidget(migrateBtn,0,1);
        layout->addWidget(toDatabaseWidget, 0, 2);
        layout->addWidget(progressBar, 1, 0, 1,3);

        setMainWidget(mainWidget);


        DatabaseAccess     toDBAccess;
        DatabaseParameters toDBParameters;
        toDBParameters.readConfig();
        toDBParameters.connectOptions = toDatabaseWidget->connectionOptions->text();
        toDBParameters.databaseName   = toDatabaseWidget->databaseName->text();
        toDBParameters.databaseType   = toDatabaseWidget->databaseType->currentText();
        toDBParameters.hostName       = toDatabaseWidget->hostName->text();
        toDBParameters.password       = toDatabaseWidget->password->text();
        toDBParameters.port           = toDatabaseWidget->hostPort->text().toInt();
        toDBParameters.userName       = toDatabaseWidget->userName->text();
        toDBAccess.setParameters(toDBParameters);


        DatabaseAccess     fromDBAccess;
        DatabaseParameters fromDBParameters;
        fromDBParameters.readConfig();
        fromDBParameters.connectOptions = fromDatabaseWidget->connectionOptions->text();
        fromDBParameters.databaseName   = fromDatabaseWidget->databaseName->text();
        fromDBParameters.databaseType   = fromDatabaseWidget->databaseType->currentText();
        fromDBParameters.hostName       = fromDatabaseWidget->hostName->text();
        fromDBParameters.password       = fromDatabaseWidget->password->text();
        fromDBParameters.port           = fromDatabaseWidget->hostPort->text().toInt();
        fromDBParameters.userName       = fromDatabaseWidget->userName->text();
        fromDBAccess.setParameters(fromDBParameters);

        // now perform the action

        // AlbumRoots
        QList<QVariant> result;
        fromDBAccess.backend()->execDBAction(fromDBAccess.backend()->getDBAction("Migrate_Read_AlbumRoots"), &result);

//        fromDBAccess.backend()->getDBAction();
    }
}
