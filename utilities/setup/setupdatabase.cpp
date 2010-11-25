/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database setup tab
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

#include "setupdatabase.moc"

// Qt includes

#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QList>
#include <QFileInfo>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QSpinBox>
#include <QFormLayout>
#include <QSqlDatabase>
#include <QSqlError>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <kpagedialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <ktemporaryfile.h>

// Local includes

#include "albumsettings.h"
#include "databasewidget.h"
#include "databaseparameters.h"

namespace Digikam
{

class SetupDatabasePriv
{
public:

    SetupDatabasePriv()
    {
        mainDialog     = 0;
        databaseWidget = 0;
    }

    KPageDialog*    mainDialog;
    DatabaseWidget* databaseWidget;
};

SetupDatabase::SetupDatabase(KPageDialog* dialog, QWidget* parent)
    : QScrollArea(parent), d(new SetupDatabasePriv)
{
    d->mainDialog  = dialog;
    d->databaseWidget = new DatabaseWidget(viewport());
    d->databaseWidget->setAutoFillBackground(false);
    setWidget(d->databaseWidget);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupDatabase::~SetupDatabase()
{
    delete d;
}

void SetupDatabase::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    if (d->databaseWidget->currentDatabaseType() == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        QString newPath = d->databaseWidget->databasePathEdit->url().path();
        QDir oldDir(d->databaseWidget->originalDbPath);
        QDir newDir(newPath);

        if (oldDir != newDir || d->databaseWidget->currentDatabaseType() != d->databaseWidget->originalDbType)
        {
            settings->setDatabaseParameters(DatabaseParameters::parametersForSQLiteDefaultFile(newPath));

            // clear other fields
            d->databaseWidget->internalServer->setChecked(false);

            settings->saveSettings();
        }
    }
    else
    {
        if (d->databaseWidget->internalServer->isChecked())
        {
            DatabaseParameters internalServerParameters = DatabaseParameters::defaultParameters(d->databaseWidget->currentDatabaseType());
            settings->setInternalDatabaseServer(true);
            settings->setDatabaseType(d->databaseWidget->currentDatabaseType());
            settings->setDatabaseName(internalServerParameters.databaseName);
            settings->setDatabaseNameThumbnails(internalServerParameters.databaseName);
            settings->setDatabaseConnectoptions(internalServerParameters.connectOptions);
            settings->setDatabaseHostName(internalServerParameters.hostName);
            settings->setDatabasePort(internalServerParameters.port);
            settings->setDatabaseUserName(internalServerParameters.userName);
            settings->setDatabasePassword(internalServerParameters.password);
        }
        else
        {
            settings->setInternalDatabaseServer(d->databaseWidget->internalServer->isChecked());
            settings->setDatabaseType(d->databaseWidget->currentDatabaseType());
            settings->setDatabaseName(d->databaseWidget->databaseName->text());
            settings->setDatabaseNameThumbnails(d->databaseWidget->databaseNameThumbnails->text());
            settings->setDatabaseConnectoptions(d->databaseWidget->connectionOptions->text());
            settings->setDatabaseHostName(d->databaseWidget->hostName->text());
            settings->setDatabasePort(d->databaseWidget->hostPort->text().toInt());
            settings->setDatabaseUserName(d->databaseWidget->userName->text());
            settings->setDatabasePassword(d->databaseWidget->password->text());
        }

        settings->saveSettings();
    }
}

void SetupDatabase::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    d->databaseWidget->setParametersFromSettings(settings);
}

}  // namespace Digikam
