/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "databasewidget.h"

// Qt includes

#include <QGridLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSqlDatabase>
#include <QSqlError>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>
#include <QTemporaryFile>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dbengineparameters.h"
#include "databaseserverstarter.h"

namespace Digikam
{

class DatabaseWidget::Private
{

public:

    Private()
    {
        databasePathLabel = 0;
        expertSettings    = 0;
    }

    QLabel*    databasePathLabel;
    QGroupBox* expertSettings;
};

DatabaseWidget::DatabaseWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setupMainArea();
}

DatabaseWidget::~DatabaseWidget()
{
    delete d;
}

void DatabaseWidget::setupMainArea()
{
    QVBoxLayout* const layout  = new QVBoxLayout();
    setLayout(layout);

    // --------------------------------------------------------

    QGroupBox* const dbPathBox = new QGroupBox(i18n("Database File Path"), this);
    QVBoxLayout* const vlay    = new QVBoxLayout(dbPathBox);
    d->databasePathLabel       = new QLabel(i18n("<p>The location where the database file will be stored on your system. "
                                                 "There is one common database file for all root albums.<br/>"
                                                 "Write access is required to be able to edit image properties.</p>"
                                                 "<p>Note: a remote file system, such as NFS, cannot be used here.</p><p></p>"),
                                            dbPathBox);
    d->databasePathLabel->setWordWrap(true);

    databasePathEdit                                 = new DFileSelector(dbPathBox);
    databasePathEdit->setFileDlgMode(QFileDialog::Directory);

    QLabel* const databaseTypeLabel                  = new QLabel(i18n("Type"));
    databaseType                                     = new QComboBox();
#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)
    QLabel* const internalServerLabel                = new QLabel(i18n("Internal Server"));
#endif
    internalServer                                   = new QCheckBox();
    QLabel* const databaseNameLabel                  = new QLabel(i18n("Core<br>Schema Name"));
    databaseName                                     = new QLineEdit();
    QLabel* const databaseNameThumbnailsLabel        = new QLabel(i18n("Thumbnails<br>Schema Name"));
    databaseNameThumbnails                           = new QLineEdit();
    QLabel* const databaseNameFaceLabel              = new QLabel(i18n("Face<br>Schema Name"));
    databaseNameFace                                 = new QLineEdit();
    QLabel* const hostNameLabel                      = new QLabel(i18n("Host Name"));
    hostName                                         = new QLineEdit();
    QLabel* const hostPortLabel                      = new QLabel(i18n("Port"));
    hostPort                                         = new QSpinBox();
    hostPort->setMaximum(65535);

    QLabel* const connectionOptionsLabel             = new QLabel(i18n("Database<br>Connection<br>Options"));
    connectionOptions                                = new QLineEdit();

    QLabel* const userNameLabel                      = new QLabel(i18n("User"));
    userName                                         = new QLineEdit();

    QLabel* const passwordLabel                      = new QLabel(i18n("Password"));
    password                                         = new QLineEdit();
    password->setEchoMode(QLineEdit::Password);

    QPushButton* const checkDatabaseConnectionButton = new QPushButton(i18n("Check DB Connection"));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)
    expertSettinglayout->addRow(internalServerLabel,         internalServer);
#endif

    expertSettinglayout->addRow(hostNameLabel,               hostName);
    expertSettinglayout->addRow(hostPortLabel,               hostPort);
    expertSettinglayout->addRow(databaseNameLabel,           databaseName);
    expertSettinglayout->addRow(databaseNameThumbnailsLabel, databaseNameThumbnails);
    expertSettinglayout->addRow(databaseNameFaceLabel,       databaseNameFace);
    expertSettinglayout->addRow(userNameLabel,               userName);
    expertSettinglayout->addRow(passwordLabel,               password);
    expertSettinglayout->addRow(connectionOptionsLabel,      connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(databaseTypeLabel);
    vlay->addWidget(databaseType);
    vlay->addWidget(d->databasePathLabel);
    vlay->addWidget(databasePathEdit);
    vlay->addWidget(d->expertSettings);
    vlay->setSpacing(0);
    vlay->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------- fill with default values ---------------------

    databaseType->addItem(i18n("SQLite"),               DbEngineParameters::SQLiteDatabaseType());

#ifdef HAVE_MYSQLSUPPORT
    databaseType->addItem(i18n("MySQL (experimental)"), DbEngineParameters::MySQLDatabaseType());
#endif

    databaseType->setToolTip(i18n("<p>Select here the type of database backend.</p>"
                                  "<p><b>SQlite</b> backend is for local database storage with a small and medium collection sizes. "
                                  "It is the default and recommended backend.</p>"
#ifdef HAVE_MYSQLSUPPORT
                                  "<p><b>MySQL</b> backend is a more robust solution especially for remote and shared database storage. "
                                  "It is also more efficient to manage huge collection sizes. "
                                  "Be careful: this one it is still in experimental stage.</p>"
#endif
                                 ));

    setDatabaseInputFields(DbEngineParameters::SQLiteDatabaseType());

    // --------------------------------------------------------

    connect(databaseType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(checkDatabaseConnection()));

#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)
    connect(internalServer, SIGNAL(stateChanged(int)),
            this, SLOT(slotHandleInternalServerCheckbox(int)));

    slotHandleInternalServerCheckbox(internalServer->checkState());
#endif
}

QString DatabaseWidget::currentDatabaseType() const
{
    return databaseType->itemData(databaseType->currentIndex()).toString();
}

void DatabaseWidget::slotChangeDatabasePath(const QUrl& result)
{
#ifdef _WIN32
    // Work around bug #189168
    QTemporaryFile temp;
    temp.setFileTemplate(result.toLocalFile(QUrl::AddTrailingSlash) + QLatin1String("XXXXXX"));
    temp.open();

    if (!result.isEmpty() && !temp.open())
#else
    QFileInfo targetPath(result.toLocalFile());

    if (!result.isEmpty() && !targetPath.isWritable())
#endif
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("You do not seem to have write access to this database folder.\n"
                                   "Without this access, the caption and tag features will not work."));
    }

    checkDBPath();
}

void DatabaseWidget::slotDatabasePathEditedDelayed()
{
    QTimer::singleShot(300, this, SLOT(slotDatabasePathEdited()));
}

void DatabaseWidget::slotDatabasePathEdited()
{
    QString newPath = databasePathEdit->lineEdit()->text();

#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        databasePathEdit->lineEdit()->setText(QDir::homePath() + QLatin1Char('/') + QDir::fromNativeSeparators(newPath));
    }

#endif

    databasePathEdit->lineEdit()->setText(QDir::toNativeSeparators(newPath));

    checkDBPath();
}

void DatabaseWidget::slotHandleDBTypeIndexChanged(int index)
{
    const QString& dbType = databaseType->itemData(index).toString();
    setDatabaseInputFields(dbType);
}

void DatabaseWidget::setDatabaseInputFields(const QString& currentIndexStr)
{
    if (currentIndexStr == QString(DbEngineParameters::SQLiteDatabaseType()))
    {
        d->databasePathLabel->setVisible(true);
        databasePathEdit->setVisible(true);
        d->expertSettings->setVisible(false);

        connect(databasePathEdit, SIGNAL(signalUrlSelected(QUrl)),
                this, SLOT(slotChangeDatabasePath(QUrl)));

        connect(databasePathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                this, SLOT(slotDatabasePathEditedDelayed()));
    }
    else
    {
        d->databasePathLabel->setVisible(false);
        databasePathEdit->setVisible(false);
        d->expertSettings->setVisible(true);

        disconnect(databasePathEdit, SIGNAL(signalUrlSelected(QUrl)),
                   this, SLOT(slotChangeDatabasePath(QUrl)));

        disconnect(databasePathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                   this, SLOT(slotDatabasePathEditedDelayed()));
    }

    adjustSize();
}

void DatabaseWidget::slotHandleInternalServerCheckbox(int enableFields)
{
    hostName->setEnabled(enableFields == Qt::Unchecked);
    hostPort->setEnabled(enableFields == Qt::Unchecked);
    databaseName->setEnabled(enableFields == Qt::Unchecked);
    databaseNameThumbnails->setEnabled(enableFields == Qt::Unchecked);
    databaseNameFace->setEnabled(enableFields == Qt::Unchecked);
    userName->setEnabled(enableFields == Qt::Unchecked);
    password->setEnabled(enableFields == Qt::Unchecked);
    connectionOptions->setEnabled(enableFields == Qt::Unchecked);

    if (enableFields == Qt::Unchecked)
    {
        hostPort->setValue(3306);
    }
    else
    {
        hostPort->setValue(-1);
    }
}

void DatabaseWidget::checkDatabaseConnection()
{
    // TODO : if check DB connection operations can be threaded, use DBusyDlg dialog there...

    qApp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID(QLatin1String("ConnectionTest"));
    QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(currentDatabaseType(), databaseID);
    DbEngineParameters parameters = getDbEngineParameters();
    testDatabase.setHostName(parameters.hostName);
    testDatabase.setPort(parameters.port);
    testDatabase.setUserName(parameters.userName);
    testDatabase.setPassword(parameters.password);
    testDatabase.setConnectOptions(parameters.connectOptions);

    qApp->restoreOverrideCursor();

    bool result = testDatabase.open();

    if (result)
    {
        QMessageBox::critical(qApp->activeWindow(), i18n("Database connection test"),
                              i18n("Database connection test successful."));
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), i18n("Database connection test"),
                              i18n("Database connection test was not successful. <p>Error was: %1</p>",
                                   testDatabase.lastError().text()) );
    }

    testDatabase.close();
    QSqlDatabase::removeDatabase(databaseID);
}

void DatabaseWidget::checkDBPath()
{
/*
    bool dbOk          = false;
    bool pathUnchanged = true;
*/
    QString newPath    = databasePathEdit->lineEdit()->text();

    if (!databasePathEdit->lineEdit()->text().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(originalDbPath);
/*
        dbOk          = dbDir.exists();
        pathUnchanged = (dbDir == oldDir);
*/
    }

    //TODO create an Enable button slot, if the path is vald
    //d->mainDialog->enableButtonOk(dbOk);
}

void DatabaseWidget::setParametersFromSettings(const ApplicationSettings* const settings)
{
    originalDbPath = settings->getDatabaseFilePath();
    originalDbType = settings->getDatabaseType();
    databasePathEdit->lineEdit()->setText(settings->getDatabaseFilePath());

#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)
    internalServer->setChecked(settings->getInternalDatabaseServer());
#else
    internalServer->setChecked(false);
#endif
    databaseName->setText(settings->getDatabaseName());
    databaseNameThumbnails->setText(settings->getDatabaseNameThumbnails());
    databaseNameFace->setText(settings->getDatabaseNameFace());
    hostName->setText(settings->getDatabaseHostName());
    hostPort->setValue(settings->getDatabasePort());
    connectionOptions->setText(settings->getDatabaseConnectoptions());

    userName->setText(settings->getDatabaseUserName());
    password->setText(settings->getDatabasePassword());

    // Now set the type according the database type from the settings.
    // If no item is found, ignore the setting.

    for (int i = 0; i < databaseType->count(); ++i)
    {
        //qCDebug(DIGIKAM_WIDGETS_LOG) << "Comparing comboboxentry on index ["<< i <<"] [" << databaseType->itemData(i)
        //                             << "] with ["<< settings->getDatabaseType() << "]";

        if (databaseType->itemData(i).toString() == settings->getDatabaseType())
        {
            databaseType->setCurrentIndex(i);
        }
    }
}

DbEngineParameters DatabaseWidget::getDbEngineParameters()
{
    DbEngineParameters parameters;

    if (currentDatabaseType() == QString(DbEngineParameters::SQLiteDatabaseType()) || !internalServer->isChecked())
    {
        parameters.connectOptions = connectionOptions->text();
        parameters.databaseType   = currentDatabaseType();
        parameters.hostName       = hostName->text();
        parameters.password       = password->text();
        parameters.port           = hostPort->text().toInt();
        parameters.userName       = userName->text();

        if (parameters.databaseType == QString(DbEngineParameters::SQLiteDatabaseType()))
        {
            parameters.databaseName           = QDir::cleanPath(databasePathEdit->lineEdit()->text() + 
                                                QLatin1Char('/') + QLatin1String("digikam4.db"));
            parameters.databaseNameThumbnails = parameters.databaseName;
            parameters.databaseNameFace       = parameters.databaseName;
        }
        else
        {
            parameters.databaseName           = databaseName->text();
            parameters.databaseNameThumbnails = databaseNameThumbnails->text();
            parameters.databaseNameFace       = databaseNameFace->text();
        }
    }
    else
    {
        parameters = DbEngineParameters::defaultParameters(currentDatabaseType());
        DatabaseServerStarter::startServerManagerProcess(currentDatabaseType());
    }

    return parameters;
}

} // namespace Digikam
