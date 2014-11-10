/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
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

#include "migrationdlg.moc"

// QT includes

#include <QGridLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QWidget>
#include <QList>
#include <QSqlQuery>
#include <QMap>
#include <QSqlError>
#include <QLabel>

// KDE includes

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

// Local includes

#include "applicationsettings.h"
#include "databaseaccess.h"
#include "databasewidget.h"
#include "databasebackend.h"
#include "databaseparameters.h"
#include "schemaupdater.h"
#include "databasecopymanager.h"

namespace Digikam
{

class DatabaseCopyThread::Private
{
public:

    Private()
    {
    }

    DatabaseParameters fromDatabaseParameters;
    DatabaseParameters toDatabaseParameters;
};

DatabaseCopyThread::DatabaseCopyThread(QWidget* const parent)
    : QThread(parent), d(new Private)
{
}

DatabaseCopyThread::~DatabaseCopyThread()
{
    delete d;
}

void DatabaseCopyThread::run()
{
    m_copyManager.copyDatabases(d->fromDatabaseParameters, d->toDatabaseParameters);
}

void DatabaseCopyThread::init(const DatabaseParameters& fromDatabaseParameters, const DatabaseParameters& toDatabaseParameters)
{
    d->fromDatabaseParameters = fromDatabaseParameters;
    d->toDatabaseParameters   = toDatabaseParameters;
}

// ---------------------------------------------------------------------------

class MigrationDlg::Private
{
public:

    Private() :
        fromDatabaseWidget(0),
        toDatabaseWidget(0),
        migrateButton(0),
        cancelButton(0),
        overallStepTitle(0),
        progressBar(0),
        progressBarSmallStep(0),
        copyThread(0)
    {
    }

    DatabaseWidget*     fromDatabaseWidget;
    DatabaseWidget*     toDatabaseWidget;
    QPushButton*        migrateButton;
    QPushButton*        cancelButton;
    QLabel*             overallStepTitle;
    QProgressBar*       progressBar;
    QProgressBar*       progressBarSmallStep;
    DatabaseCopyThread* copyThread;
};

MigrationDlg::MigrationDlg(QWidget* const parent)
    : KDialog(parent), d(new Private)
{
    setupMainArea();
}

MigrationDlg::~MigrationDlg()
{
    d->copyThread->wait();
    delete d;
}

void MigrationDlg::setupMainArea()
{
    d->copyThread                      = new DatabaseCopyThread(this);
    d->fromDatabaseWidget              = new DatabaseWidget(this);
    d->toDatabaseWidget                = new DatabaseWidget(this);
    d->migrateButton                   = new QPushButton(i18n("Migrate ->"), this);
    d->cancelButton                    = new QPushButton(i18n("Cancel"), this);
    d->cancelButton->setEnabled(false);

    QGroupBox* const progressBox       = new QGroupBox(i18n("Progress Information"), this);
    QVBoxLayout* const vlay            = new QVBoxLayout(progressBox);

    d->progressBar                     = new QProgressBar(progressBox);
    d->progressBar->setTextVisible(true);
    d->progressBar->setRange(0,13);
    d->progressBarSmallStep            = new QProgressBar(progressBox);
    d->progressBarSmallStep->setTextVisible(true);

    d->overallStepTitle                = new QLabel(i18n("Step Progress"), progressBox);

    vlay->addWidget(new QLabel(i18n("Overall Progress"), progressBox));
    vlay->addWidget(d->progressBar);
    vlay->addWidget(d->overallStepTitle);
    vlay->addWidget(d->progressBarSmallStep);

    QWidget* const mainWidget = new QWidget;
    QGridLayout* const layout = new QGridLayout;
    mainWidget->setLayout(layout);

    layout->addWidget(d->fromDatabaseWidget,   0, 0, 4, 1);
    layout->addWidget(d->migrateButton,        1, 1);
    layout->addWidget(d->cancelButton,         2, 1);
    layout->addWidget(d->toDatabaseWidget,     0, 2, 4, 1);
    layout->addWidget(progressBox,             4, 0, 1, 3);

    setMainWidget(mainWidget);
    dataInit();

    // setup dialog
    setButtons(Close);

    connect(d->migrateButton, SIGNAL(clicked()),
            this, SLOT(performCopy()));

    // connect signal handlers for copy d->copyThread
    connect(&(d->copyThread->m_copyManager), SIGNAL(finished(int,QString)),
            this, SLOT(handleFinish(int,QString)));

    connect(&(d->copyThread->m_copyManager), SIGNAL(stepStarted(QString)),
            this, SLOT(handleStepStarted(QString)));

    connect(&(d->copyThread->m_copyManager), SIGNAL(smallStepStarted(int,int)),
            this, SLOT(handleSmallStepStarted(int,int)));

    connect(this, SIGNAL(closeClicked()),
            &(d->copyThread->m_copyManager), SLOT(stopProcessing()));

    connect(d->cancelButton, SIGNAL(clicked()),
            &(d->copyThread->m_copyManager), SLOT(stopProcessing()));
}

void MigrationDlg::performCopy()
{
    DatabaseParameters toDBParameters   = d->toDatabaseWidget->getDatabaseParameters();
    DatabaseParameters fromDBParameters = d->fromDatabaseWidget->getDatabaseParameters();
    d->copyThread->init(fromDBParameters, toDBParameters);

    lockInputFields();
    d->copyThread->start();
}

void MigrationDlg::dataInit()
{
    d->fromDatabaseWidget->setParametersFromSettings(ApplicationSettings::instance());
    d->toDatabaseWidget->setParametersFromSettings(ApplicationSettings::instance());
}

void MigrationDlg::unlockInputFields()
{
    d->fromDatabaseWidget->setEnabled(true);
    d->toDatabaseWidget->setEnabled(true);
    d->migrateButton->setEnabled(true);
    d->progressBar->setValue(0);
    d->progressBarSmallStep->setValue(0);

    d->cancelButton->setEnabled(false);
}

void MigrationDlg::lockInputFields()
{
    d->fromDatabaseWidget->setEnabled(false);
    d->toDatabaseWidget->setEnabled(false);
    d->migrateButton->setEnabled(false);
    d->cancelButton->setEnabled(true);
}

void MigrationDlg::handleFinish(int finishState, const QString& errorMsg)
{
    switch (finishState)
    {
        case DatabaseCopyManager::failed:
            KMessageBox::error(this, errorMsg );
            unlockInputFields();
            break;
        case DatabaseCopyManager::success:
            KMessageBox::information(this, i18n("Database copied successfully.") );
            unlockInputFields();
            break;
        case DatabaseCopyManager::canceled:
            KMessageBox::information(this, i18n("Database conversion canceled.") );
            unlockInputFields();
            break;
    }
}

void MigrationDlg::handleStepStarted(const QString& stepName)
{
    int progressBarValue = d->progressBar->value();
    d->overallStepTitle->setText(i18n("Step Progress (%1)", stepName));
    d->progressBar->setValue(++progressBarValue);
}

void MigrationDlg::handleSmallStepStarted(int currentValue, int maxValue)
{
    d->progressBarSmallStep->setMaximum(maxValue);
    d->progressBarSmallStep->setValue(currentValue);
}

}  // namespace Digikam
