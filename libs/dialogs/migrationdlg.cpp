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
#include <qsqlquery.h>
#include <qmap.h>

// KDE includes
#include <klocale.h>
#include <kdebug.h>

// Local includes
#include "albumsettings.h"
#include "databaseaccess.h"
#include "databasewidget.h"
#include "databasebackend.h"
#include "databaseparameters.h"
#include "schemaupdater.h"

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
        dataInit();

        connect(migrateBtn, SIGNAL(clicked()), this, SLOT(performCopy()));
    }

    void MigrationDlg::dataInit()
    {
        fromDatabaseWidget->setParametersFromSettings(AlbumSettings::instance());
        toDatabaseWidget->setParametersFromSettings(AlbumSettings::instance());
    }

    void MigrationDlg::performCopy()
    {
        DatabaseParameters toDBParameters;
        toDBParameters.readConfig();
        toDBParameters.connectOptions = toDatabaseWidget->connectionOptions->text();
        toDBParameters.databaseName   = toDatabaseWidget->databaseName->text();
        toDBParameters.databaseType   = toDatabaseWidget->databaseType->currentText();
        toDBParameters.hostName       = toDatabaseWidget->hostName->text();
        toDBParameters.password       = toDatabaseWidget->password->text();
        toDBParameters.port           = toDatabaseWidget->hostPort->text().toInt();
        toDBParameters.userName       = toDatabaseWidget->userName->text();
        DatabaseLocking toLocking;
        DatabaseBackend toDBbackend(&toLocking);
        toDBbackend.open(toDBParameters);

        DatabaseParameters fromDBParameters;
        fromDBParameters.readConfig();
        fromDBParameters.connectOptions = fromDatabaseWidget->connectionOptions->text();
        fromDBParameters.databaseName   = fromDatabaseWidget->databaseName->text();
        fromDBParameters.databaseType   = fromDatabaseWidget->databaseType->currentText();
        fromDBParameters.hostName       = fromDatabaseWidget->hostName->text();
        fromDBParameters.password       = fromDatabaseWidget->password->text();
        fromDBParameters.port           = fromDatabaseWidget->hostPort->text().toInt();
        fromDBParameters.userName       = fromDatabaseWidget->userName->text();
        DatabaseLocking fromLocking;
        DatabaseBackend fromDBbackend(&fromLocking);
        fromDBbackend.open(fromDBParameters);


        // first create the schema
        AlbumDB       albumDB(&toDBbackend);
        SchemaUpdater updater(&albumDB, &toDBbackend, toDBParameters);

        // now perform the copy action
        QMap<QString, QVariant> bindingMap;
        QList<QString> columnNames;
        QSqlQuery result = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction("Migrate_Read_AlbumRoots"), bindingMap) ;

        int columnCount = result.record().count();
        for (int i=0; i<columnCount; i++)
        {
            kDebug(50003) << "Column: ["<< result.record().fieldName(i) << "]";
            columnNames.append(result.record().fieldName(i));
        }

        while (result.next()) {
            // read the values from the fromDB into a hash
            QMap<QString, QVariant> tempBindingMap;
            int i=0;
            foreach (QString columnName, columnNames)
            {
                kDebug(50003) << "Column: ["<< columnName << "] value ["<<result.value(i++)<<"]";
                tempBindingMap.insert(columnName, result.value(i));
            }
            // insert the previous requested values to the toDB
            toDBbackend.execDBActionQuery(fromDBbackend.getDBAction("Migrate_Write_AlbumRoots"), tempBindingMap);
        }
    }
}
