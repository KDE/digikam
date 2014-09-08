/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database setup tab
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2012-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QCursor>
#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QList>
#include <QFileInfo>
#include <QGridLayout>
#include <QHelpEvent>
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
#include <kapplication.h>
#include <kpagedialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <ktemporaryfile.h>

// Local includes

#include "applicationsettings.h"
#include "databasewidget.h"
#include "databaseparameters.h"
#include "scancontroller.h"
#include "schemaupdater.h"

namespace Digikam
{

class SetupDatabase::Private
{
public:

    Private() :
        mainDialog(0),
        databaseWidget(0),
        updateBox(0),
        hashesButton(0)
    {
    }

    KPageDialog*    mainDialog;
    DatabaseWidget* databaseWidget;
    QGroupBox*      updateBox;
    QPushButton*    hashesButton;
};

SetupDatabase::SetupDatabase(KPageDialog* const dialog, QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    d->mainDialog           = dialog;
    QWidget* const page     = new QWidget;
    QVBoxLayout* mainLayout = new QVBoxLayout;

    d->databaseWidget       = new DatabaseWidget;
    mainLayout->addWidget(d->databaseWidget);

    if (!SchemaUpdater::isUniqueHashUpToDate())
    {
        d->updateBox                    = new QGroupBox(i18nc("@title:group", "Updates"));
        QGridLayout* const updateLayout = new QGridLayout;

        d->hashesButton                 = new QPushButton(i18nc("@action:button", "Update File Hashes"));
        d->hashesButton->setWhatsThis(i18nc("@info:tooltip",
                                            "File hashes are used to identify identical files and to display thumbnails. "
                                            "A new, improved algorithm to create the hash is now used. "
                                            "The old algorithm, though, still works quite well, so it is recommended to "
                                            "carry out this upgrade, but not required.<nl/> "
                                            "<note>After the upgrade you cannot use your database with a digiKam version "
                                            "prior to 2.0.</note>"));

        QPushButton* const infoHash     = new QPushButton;
        infoHash->setIcon(SmallIcon("dialog-information"));
        infoHash->setToolTip(i18nc("@info:tooltip", "Get information about <interface>Update File Hashes</interface>"));

        updateLayout->addWidget(d->hashesButton, 0, 0);
        updateLayout->addWidget(infoHash,        0, 1);
        updateLayout->setColumnStretch(2, 1);

        d->updateBox->setLayout(updateLayout);

        mainLayout->addStretch(10);
        mainLayout->addWidget(d->updateBox);

        connect(d->hashesButton, SIGNAL(clicked()),
                this, SLOT(upgradeUniqueHashes()));

        connect(infoHash, SIGNAL(clicked()),
                this, SLOT(showHashInformation()));
    }

    page->setLayout(mainLayout);
    setWidget(page);
    setWidgetResizable(true);

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
    ApplicationSettings* const settings = ApplicationSettings::instance();

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
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    d->databaseWidget->setParametersFromSettings(settings);
}

void SetupDatabase::upgradeUniqueHashes()
{
    int result = KMessageBox::warningContinueCancel(this, i18nc("@info",
                                                                "<para>The process of updating the file hashes takes a few minutes.</para> "
                                                                "<para>Please ensure that any important collections on removable media are connected. "
                                                                "<note>After the upgrade you cannot use your database with a digiKam version "
                                                                "prior to 2.0.</note></para> "
                                                                "<para>Do you want to begin the update?</para>"));

    if (result == KMessageBox::Continue)
    {
        ScanController::instance()->updateUniqueHash();
    }
}

void SetupDatabase::showHashInformation()
{
    kapp->postEvent(d->hashesButton, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), QCursor::pos()));
}

}  // namespace Digikam
