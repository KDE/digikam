/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2011 by Gilles Caulier<caulier dot gilles at gmail dot com>
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

#include "config-digikam.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "databasewidget.h"
#include "databasebackend.h"
#include "databaseparameters.h"
#include "schemaupdater.h"
#include "databasecopymanager.h"

namespace Digikam
{

class DatabaseCopyThread::DatabaseCopyThreadPriv
{
public:

    DatabaseCopyThreadPriv()
    {
    }

    DatabaseParameters fromDatabaseParameters;
    DatabaseParameters toDatabaseParameters;
};

DatabaseCopyThread::DatabaseCopyThread(QWidget* parent)
    : QThread(parent), d(new DatabaseCopyThreadPriv)
{
}

DatabaseCopyThread::~DatabaseCopyThread()
{
    delete d;
}

void DatabaseCopyThread::run()
{
    copyManager.copyDatabases(d->fromDatabaseParameters, d->toDatabaseParameters);
}

void DatabaseCopyThread::init(DatabaseParameters fromDatabaseParameters, DatabaseParameters toDatabaseParameters)
{
    d->fromDatabaseParameters = fromDatabaseParameters;
    d->toDatabaseParameters   = toDatabaseParameters;
}

// ---------------------------------------------------------------------------

class MigrationDlg::MigrationDlgPriv
{
public:

    MigrationDlgPriv() :
        // Img database
        fromImgDatabaseWidget(0),
        toImgDatabaseWidget(0),
        migrateImgButton(0),
        cancelImgButton(0),
#ifdef USE_THUMBS_DB
        // Thumb database
        fromThumbDatabaseWidget(0),
        toThumbDatabaseWidget(0),
        migrateThumbButton(0),
        cancelThumbButton(0),
#endif
        // Global
        overallStepTitle(0),
        progressBar(0),
        progressBarSmallStep(0),
        sameDatabase(0),
        internalFromServer(0),
#ifdef HAVE_INTERNALMYSQL
        internalToServer(0),
#endif
        copyThread(0)
    {
    }
    // Img database
    DatabaseWidget*     fromImgDatabaseWidget;
    DatabaseWidget*     toImgDatabaseWidget;
    QPushButton*        migrateImgButton;
    QPushButton*        cancelImgButton;
#ifdef USE_THUMBS_DB
    // Thumb database
    DatabaseWidget*     fromThumbDatabaseWidget;
    DatabaseWidget*     toThumbDatabaseWidget;
    QPushButton*        migrateThumbButton;
    QPushButton*        cancelThumbButton;
#endif
    // Global
    QLabel*             overallStepTitle;
    QProgressBar*       progressBar;
    QProgressBar*       progressBarSmallStep;
    QCheckBox*          sameDatabase;
    QCheckBox*          internalFromServer;
#ifdef HAVE_INTERNALMYSQL
    QCheckBox*          internalToServer;
#endif
    DatabaseCopyThread* copyThread;
};

MigrationDlg::MigrationDlg(QWidget* parent)
    : KDialog(parent), d(new MigrationDlgPriv)
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
    d->fromImgDatabaseWidget           = new DatabaseWidget(this, i18n("current images DB"));
    d->toImgDatabaseWidget             = new DatabaseWidget(this, i18n("dest. images DB"));
    d->migrateImgButton                = new QPushButton(i18n("Migrate ->"), this);
    d->cancelImgButton                 = new QPushButton(i18n("Cancel"), this);
    d->cancelImgButton->setEnabled(false);
#ifdef USE_THUMBS_DB
    d->fromThumbDatabaseWidget         = new DatabaseWidget(this, i18n("current thumbnail DB"));
    d->toThumbDatabaseWidget           = new DatabaseWidget(this, i18n("dest. thumbnail DB"));
    d->migrateThumbButton              = new QPushButton(i18n("Migrate ->"), this);
    d->cancelThumbButton               = new QPushButton(i18n("Cancel"), this);
    d->cancelThumbButton->setEnabled(false);
#endif
    d->sameDatabase                    = new QCheckBox(i18n("Same database"), this);
    d->internalFromServer              = new QCheckBox(i18n("Internal server"), this);
#ifdef HAVE_INTERNALMYSQL
    d->internalToServer                = new QCheckBox(i18n("Internal server"), this);
#endif

    QGroupBox* progressBox             = new QGroupBox(i18n("Progress Information"), this);
    QVBoxLayout* vlay                  = new QVBoxLayout(progressBox);

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

    QWidget* mainWidget = new QWidget;
    QGridLayout* layout = new QGridLayout;
    mainWidget->setLayout(layout);

    layout->addWidget(d->internalFromServer,       0, 0);
#ifdef HAVE_INTERNALMYSQL
    layout->addWidget(d->internalToServer,         0, 2);
#endif

    layout->addWidget(d->fromImgDatabaseWidget,    1, 0, 3, 1);
    layout->addWidget(d->migrateImgButton,         2, 1);
    layout->addWidget(d->cancelImgButton,          3, 1);
    layout->addWidget(d->toImgDatabaseWidget,      1, 2, 3, 1);

    layout->addWidget(d->sameDatabase,             5, 2);

#ifdef USE_THUMBS_DB
    layout->addWidget(d->fromThumbDatabaseWidget,  6, 0, 3, 1);
    layout->addWidget(d->migrateThumbButton,       7, 1);
    layout->addWidget(d->cancelThumbButton,        8, 1);
    layout->addWidget(d->toThumbDatabaseWidget,    6, 2, 3, 1);
#endif

    layout->addWidget(progressBox,                10, 0, 1, 3);

    setMainWidget(mainWidget);
    dataInit();

    // setup dialog
    setButtons(Close);

    connect(d->migrateImgButton, SIGNAL(clicked()),
            this, SLOT(performImgCopy()));

    connect(d->cancelImgButton, SIGNAL(clicked()),
            &(d->copyThread->copyManager), SLOT(stopProcessing()));

#ifdef USE_THUMBS_DB
    connect(d->migrateThumbButton, SIGNAL(clicked()),
            this, SLOT(performThumbCopy()));

    connect(d->cancelThumbButton, SIGNAL(clicked()),
            &(d->copyThread->copyManager), SLOT(stopProcessing()));
#endif

    // connect signal handlers for copy d->copyThread
    connect(&(d->copyThread->copyManager), SIGNAL(finished(int,QString)),
            this, SLOT(handleFinish(int,QString)));

    connect(&(d->copyThread->copyManager), SIGNAL(stepStarted(QString)),
            this, SLOT(handleStepStarted(QString)));

    connect(&(d->copyThread->copyManager), SIGNAL(smallStepStarted(int,int)),
            this, SLOT(handleSmallStepStarted(int,int)));

    connect(this, SIGNAL(closeClicked()),
            &(d->copyThread->copyManager), SLOT(stopProcessing()));

}

void MigrationDlg::performImgCopy()
{
    KMessageBox::information(this, i18n("TODO: performImgCopy") );
    return;
    DatabaseParameters toDBParameters   = d->toImgDatabaseWidget->getDatabaseParameters();
    DatabaseParameters fromDBParameters = d->fromImgDatabaseWidget->getDatabaseParameters();
    d->copyThread->init(fromDBParameters, toDBParameters);

    lockInputFields();
    d->copyThread->start();
}

#ifdef USE_THUMBS_DB
void MigrationDlg::performThumbCopy()
{
    KMessageBox::information(this, i18n("TODO: performThumbCopy") );
    return;
    DatabaseParameters toDBParameters   = d->toThumbDatabaseWidget->getDatabaseParameters();
    DatabaseParameters fromDBParameters = d->fromThumbDatabaseWidget->getDatabaseParameters();
    d->copyThread->init(fromDBParameters, toDBParameters);

    lockInputFields();
    d->copyThread->start();
}
#endif

void MigrationDlg::dataInit()
{
    d->fromImgDatabaseWidget->setParametersFromSettings(AlbumSettings::instance());
    d->toImgDatabaseWidget->setParametersFromSettings(AlbumSettings::instance());
#ifdef USE_THUMBS_DB
    //fr TODO: change database name
    d->fromThumbDatabaseWidget->setParametersFromSettings(AlbumSettings::instance());
    d->toThumbDatabaseWidget->setParametersFromSettings(AlbumSettings::instance());
#endif
}

void MigrationDlg::unlockInputFields()
{
    d->fromImgDatabaseWidget->setEnabled(true);
    d->toImgDatabaseWidget->setEnabled(true);
    d->migrateImgButton->setEnabled(true);

#ifdef USE_THUMBS_DB
    d->fromThumbDatabaseWidget->setEnabled(true);
    d->toThumbDatabaseWidget->setEnabled(true);
    d->migrateThumbButton->setEnabled(true);
#endif

    d->progressBar->setValue(0);
    d->progressBarSmallStep->setValue(0);

    d->cancelImgButton->setEnabled(false);
#ifdef USE_THUMBS_DB
    d->cancelThumbButton->setEnabled(false);
#endif
}

void MigrationDlg::lockInputFields()
{
    d->fromImgDatabaseWidget->setEnabled(false);
    d->toImgDatabaseWidget->setEnabled(false);
    d->migrateImgButton->setEnabled(false);

#ifdef USE_THUMBS_DB
    d->fromThumbDatabaseWidget->setEnabled(false);
    d->toThumbDatabaseWidget->setEnabled(false);
    d->migrateThumbButton->setEnabled(false);
#endif

    d->cancelImgButton->setEnabled(true);
#ifdef USE_THUMBS_DB
    d->cancelThumbButton->setEnabled(true);
#endif
}

void MigrationDlg::handleFinish(int finishState, QString errorMsg)
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
