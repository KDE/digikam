/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "databasewidget.moc"

// Qt includes

#include <QGridLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSqlDatabase>
#include <QSqlError>
#include <QLabel>
#include <QGroupBox>

// KDE includes

#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <ktemporaryfile.h>

// Local includes

#include "config-digikam.h"
#include "databaseparameters.h"
#include "databaseserverstarter.h"

namespace Digikam
{

class DatabaseWidget::DatabaseWidgetPriv
{

public:

    DatabaseWidgetPriv()
    {
        databasePathLabel = 0;
        expertSettings    = 0;
    }

    QLabel*    databasePathLabel;
    QGroupBox* expertSettings;
};

DatabaseWidget::DatabaseWidget(QWidget* parent, const QString & title)
    : QWidget(parent), d(new DatabaseWidgetPriv)
{
    setupMainArea(title);
}

DatabaseWidget::~DatabaseWidget()
{
    delete d;
}

void DatabaseWidget::setupMainArea(const QString & title)
{
    setAutoFillBackground(false);

    QVBoxLayout* layout  = new QVBoxLayout();
    setLayout(layout);

    // --------------------------------------------------------

    QGroupBox* dbPathBox = new QGroupBox(title, this);
    QVBoxLayout* vlay    = new QVBoxLayout(dbPathBox);
    d->databasePathLabel = new QLabel(i18n("<p>The location where the database file will be stored on your system. "
                                           "There is one common database file for all root albums.<br/>"
                                           "Write access is required to be able to edit image properties.</p>"
                                           "<p>Note: a remote file system, such as NFS, cannot be used here.</p><p></p>"),
                                      dbPathBox);
    d->databasePathLabel->setWordWrap(true);
    d->databasePathLabel->setFont(KGlobalSettings::smallestReadableFont());

    databasePathEdit     = new KUrlRequester(dbPathBox);
    databasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);

    QLabel* databaseTypeLabel           = new QLabel(i18n("Type"));
    databaseType                        = new QComboBox();
    //fr QLabel* internalServerLabel         = new QLabel(i18n("Internal Server"));
    //fr internalServer                      = new QCheckBox();
    QLabel* databaseNameLabel           = new QLabel(i18n("Schema Name"));
    databaseName                        = new QLineEdit();
    //fr QLabel* databaseNameThumbnailsLabel = new QLabel(i18n("Thumbnails<br>Schema Name"));
    //fr databaseNameThumbnails              = new QLineEdit();
    QLabel* hostNameLabel               = new QLabel(i18n("Host Name"));
    hostName                            = new QLineEdit();
    QLabel* hostPortLabel               = new QLabel(i18n("Port"));
    hostPort                            = new QSpinBox();
    hostPort->setMaximum(65536);

    QLabel* connectionOptionsLabel      = new QLabel(i18n("Database<br>Connection<br>Options"));
    connectionOptions                   = new QLineEdit();

    QLabel* userNameLabel               = new QLabel(i18n("User"));
    userName                            = new QLineEdit();

    QLabel* passwordLabel               = new QLabel(i18n("Password"));
    password                            = new QLineEdit();
    password->setEchoMode(QLineEdit::Password);

    QPushButton* checkDatabaseConnectionButton = new QPushButton(i18n("Check DB Connection"));

    d->expertSettings                   = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* expertSettinglayout    = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

//fr #ifdef HAVE_INTERNALMYSQL
//fr     expertSettinglayout->addRow(internalServerLabel, internalServer);
//fr #endif // HAVE_INTERNALMYSQL
    expertSettinglayout->addRow(hostNameLabel, hostName);
    expertSettinglayout->addRow(hostPortLabel, hostPort);
    expertSettinglayout->addRow(databaseNameLabel, databaseName);
    //fr expertSettinglayout->addRow(databaseNameThumbnailsLabel, databaseNameThumbnails);
    expertSettinglayout->addRow(userNameLabel, userName);
    expertSettinglayout->addRow(passwordLabel, password);
    expertSettinglayout->addRow(connectionOptionsLabel, connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(databaseTypeLabel);
    vlay->addWidget(databaseType);
    vlay->addWidget(d->databasePathLabel);
    vlay->addWidget(databasePathEdit);
    vlay->addWidget(d->expertSettings);
    vlay->setSpacing(0);
    vlay->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------- fill with default values ---------------------

    databaseType->addItem(i18n("SQLite"), DatabaseParameters::SQLiteDatabaseType());
    databaseType->addItem(i18n("MySQL"), DatabaseParameters::MySQLDatabaseType());
    setDatabaseInputFields(DatabaseParameters::SQLiteDatabaseType());

    // --------------------------------------------------------

    adjustSize();

    // --------------------------------------------------------

    connect(databasePathEdit, SIGNAL(urlSelected(KUrl)),
            this, SLOT(slotChangeDatabasePath(KUrl)));

    connect(databasePathEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotDatabasePathEdited(QString)));

    connect(databaseType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

//fr #ifdef HAVE_INTERNALMYSQL
//fr     connect(internalServer, SIGNAL(stateChanged(int)),
//fr             this, SLOT(slotHandleInternalServerCheckbox(int)));
//fr #endif // HAVE_INTERNALMYSQL

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(checkDatabaseConnection()));
}

QString DatabaseWidget::currentDatabaseType() const
{
    return databaseType->itemData(databaseType->currentIndex()).toString();
}

void DatabaseWidget::slotChangeDatabasePath(const KUrl& result)
{
#ifdef _WIN32
    // Work around B.K.O #189168
    KTemporaryFile temp;
    temp.setPrefix(result.toLocalFile(KUrl::AddTrailingSlash));
    temp.open();

    if (!result.isEmpty() && !temp.open())
#else
    QFileInfo targetPath(result.toLocalFile());

    if (!result.isEmpty() && !targetPath.isWritable())
#endif
    {
        KMessageBox::information(0, i18n("You do not seem to have write access to this database folder.\n"
                                         "Without this access, the caption and tag features will not work."));
    }

    checkDBPath();
}

void DatabaseWidget::slotDatabasePathEdited(const QString& newPath)
{
#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        databasePathEdit->setUrl(QString(QDir::homePath() + QLatin1Char('/') + newPath));
    }

#endif

    checkDBPath();
}

void DatabaseWidget::slotHandleDBTypeIndexChanged(int index)
{
    const QString& dbType = databaseType->itemData(index).toString();
    setDatabaseInputFields(dbType);
}

void DatabaseWidget::setDatabaseInputFields(const QString& currentIndexStr)
{
    if (currentIndexStr == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        d->databasePathLabel->setVisible(true);
        databasePathEdit->setVisible(true);
        d->expertSettings->setVisible(false);
    }
    else
    {
        d->databasePathLabel->setVisible(false);
        databasePathEdit->setVisible(false);
        d->expertSettings->setVisible(true);
    }

    adjustSize();
}

//fr void DatabaseWidget::slotHandleInternalServerCheckbox(int enableFields)
//fr {
//fr     hostName->setEnabled(enableFields == Qt::Unchecked);
//fr     hostPort->setEnabled(enableFields == Qt::Unchecked);
//fr     databaseName->setEnabled(enableFields == Qt::Unchecked);
//fr     //fr databaseNameThumbnails->setEnabled(enableFields == Qt::Unchecked);
//fr     userName->setEnabled(enableFields == Qt::Unchecked);
//fr     password->setEnabled(enableFields == Qt::Unchecked);
//fr     connectionOptions->setEnabled(enableFields == Qt::Unchecked);
//fr }

void DatabaseWidget::checkDatabaseConnection()
{
    // TODO : if chek DB connection operations can be threaded, use DBusyDlg dialog there...

    kapp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID("ConnectionTest");
    QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(currentDatabaseType(), databaseID);
    DatabaseParameters parameters = getDatabaseParameters();
    testDatabase.setHostName(parameters.hostName);
    testDatabase.setPort(parameters.port);
    testDatabase.setUserName(parameters.userName);
    testDatabase.setPassword(parameters.password);
    testDatabase.setConnectOptions(parameters.connectOptions);

    kapp->restoreOverrideCursor();

    bool result = testDatabase.open();

    if (result)
    {
        KMessageBox::information(0, i18n("Database connection test successful."), i18n("Database connection test"));
    }
    else
    {
        KMessageBox::error(0, i18n("Database connection test was not successful. <p>Error was: %1</p>",
                                   testDatabase.lastError().text()), i18n("Database connection test") );
    }

    testDatabase.close();
    QSqlDatabase::removeDatabase(databaseID);
}

void DatabaseWidget::checkDBPath()
{
//    bool dbOk          = false;
//    bool pathUnchanged = true;
    QString newPath    = databasePathEdit->url().toLocalFile();

    if (!databasePathEdit->url().path().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(originalDbPath);
//        dbOk          = dbDir.exists();
//        pathUnchanged = (dbDir == oldDir);
    }

    //TODO create an Enable button slot, if the path is vald
    //d->mainDialog->enableButtonOk(dbOk);
}

void DatabaseWidget::setParametersFromSettings(const AlbumSettings* settings)
{
    originalDbPath = settings->getDatabaseFilePath();
    originalDbType = settings->getDatabaseType();
    databasePathEdit->setUrl(settings->getDatabaseFilePath());

//fr #ifdef HAVE_INTERNALMYSQL
//fr     internalServer->setChecked(settings->getInternalDatabaseServer());
//fr #else
//fr     internalServer->setChecked(false);
//fr #endif // HAVE_INTERNALMYSQL
    databaseName->setText(settings->getDatabaseName());
    //fr databaseNameThumbnails->setText(settings->getDatabaseNameThumbnails());
    hostName->setText(settings->getDatabaseHostName());
    hostPort->setValue(settings->getDatabasePort());
    connectionOptions->setText(settings->getDatabaseConnectoptions());

    userName->setText(settings->getDatabaseUserName());

    password->setText(settings->getDatabasePassword());

    /* Now set the type according the database type from the settings.
     * If no item is found, ignore the setting.
     */
    for (int i=0; i<databaseType->count(); ++i)
    {
        //kDebug(50003) << "Comparing comboboxentry on index ["<< i <<"] [" << databaseType->itemData(i)
        //            << "] with ["<< settings->getDatabaseType() << "]";
        if (databaseType->itemData(i).toString() == settings->getDatabaseType())
        {
            databaseType->setCurrentIndex(i);
        }
    }
}

DatabaseParameters DatabaseWidget::getDatabaseParameters()
{
    DatabaseParameters parameters;

    //fr if (currentDatabaseType() == QString(DatabaseParameters::SQLiteDatabaseType()) || !internalServer->isChecked())
    if (currentDatabaseType() == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        parameters.connectOptions = connectionOptions->text();
        parameters.databaseType   = currentDatabaseType();
        parameters.hostName       = hostName->text();
        parameters.password       = password->text();
        parameters.port           = hostPort->text().toInt();
        parameters.userName       = userName->text();

        if (parameters.databaseType == QString(DatabaseParameters::SQLiteDatabaseType()))
        {
            parameters.databaseName = QDir::cleanPath(databasePathEdit->url().toLocalFile() + '/' + "digikam4.db");
            //fr parameters.databaseNameThumbnails = parameters.databaseName;
            parameters.databaseNameThumbnails = parameters.databaseName; //fr
        }
        else
        {
            parameters.databaseName = databaseName->text();
            //fr parameters.databaseNameThumbnails = databaseNameThumbnails->text();
            parameters.databaseNameThumbnails = parameters.databaseName; //fr
        }
    }
    else
    {
        parameters = DatabaseParameters::defaultParameters(currentDatabaseType());
        DatabaseServerStarter::startServerManagerProcess(currentDatabaseType());
    }

    return parameters;
}

} // namespace Digikam
