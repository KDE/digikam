/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database setup tab
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

#include "setupdatabase.h"
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
#include <databasewidget.h>

namespace Digikam
{

class SetupDatabasePriv
{
public:

    SetupDatabasePriv()
    {
        mainDialog       = 0;
    }

    KPageDialog             *mainDialog;
    DatabaseWidget          *databaseWidget;
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

    // --------------------------------------------------------

}

SetupDatabase::~SetupDatabase()
{
    delete d;
}

void SetupDatabase::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    if (d->databaseWidget->databaseType->currentText()=="QSQLITE"){
        QString newPath = d->databaseWidget->databasePathEdit->url().path();
        QDir oldDir(d->databaseWidget->originalDbPath);
        QDir newDir(newPath);


        if (oldDir != newDir || d->databaseWidget->databaseType->currentText()!=d->databaseWidget->originalDbType)
        {
            settings->setDatabaseType(d->databaseWidget->databaseType->currentText());
            settings->setDatabaseName(newPath);

            // clear other fields
            settings->setDatabaseConnectoptions("");
            settings->setDatabaseHostName("");
            settings->setDatabasePort(-1);
            settings->setDatabaseUserName("");
            settings->setDatabasePassword("");

            settings->saveSettings();
        }
    }else{
        settings->setDatabaseType(d->databaseWidget->databaseType->currentText());
        settings->setDatabaseName(d->databaseWidget->databaseName->text());
        settings->setDatabaseConnectoptions(d->databaseWidget->connectionOptions->text());
        settings->setDatabaseHostName(d->databaseWidget->hostName->text());
        settings->setDatabasePort(d->databaseWidget->hostPort->text().toInt());
        settings->setDatabaseUserName(d->databaseWidget->userName->text());
        settings->setDatabasePassword(d->databaseWidget->password->text());
        settings->saveSettings();
    }
}

void SetupDatabase::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->databaseWidget->originalDbPath = settings->getDatabaseFilePath();
    d->databaseWidget->originalDbType = settings->getDatabaseType();
    d->databaseWidget->databasePathEdit->setUrl(settings->getDatabaseFilePath());

    d->databaseWidget->databaseName->setText(settings->getDatabaseName());
    d->databaseWidget->hostName->setText(settings->getDatabaseHostName());
    d->databaseWidget->hostPort->setValue(settings->getDatabasePort());
    d->databaseWidget->connectionOptions->setText(settings->getDatabaseConnectoptions());

    d->databaseWidget->userName->setText(settings->getDatabaseUserName());

    d->databaseWidget->password->setText(settings->getDatabasePassword());

    /* Now set the type according the database type from the settings.
     * If no item is found, ignore the setting.
     */
    for (int i=0; i<d->databaseWidget->databaseType->count(); i++){
        kDebug(50003) << "Comparing comboboxentry on index ["<< i <<"] [" << d->databaseWidget->databaseType->itemText(i) << "] with ["<< settings->getDatabaseType() << "]";
        if (d->databaseWidget->databaseType->itemText(i)==settings->getDatabaseType()){
            d->databaseWidget->databaseType->setCurrentIndex(i);
            d->databaseWidget->setDatabaseInputFields(d->databaseWidget->databaseType->itemText(i));
        }
    }
}

}  // namespace Digikam
