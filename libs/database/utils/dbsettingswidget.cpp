/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database settings widget
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

#include "dbsettingswidget.h"

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
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dwidgetutils.h"
#include "dexpanderbox.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DatabaseSettingsWidget::Private
{

public:

    Private()
    {
        dbType         = 0;
        dbPathLabel    = 0;
        expertSettings = 0;
        dbNoticeBox    = 0;
        sqlInit        = 0;
        dbNameCore     = 0;
        dbNameThumbs   = 0;
        dbNameFace     = 0;
        hostName       = 0;
        connectOpts    = 0;
        userName       = 0;
        password       = 0;
        hostPort       = 0;
        dbPathEdit     = 0;
    }

    QLineEdit*         dbNameCore;
    QLineEdit*         dbNameThumbs;
    QLineEdit*         dbNameFace;
    QLineEdit*         hostName;
    QLineEdit*         connectOpts;
    QLineEdit*         userName;
    QLineEdit*         password;

    QSpinBox*          hostPort;

    QComboBox*         dbType;
    QLabel*            dbPathLabel;
    QTextBrowser*      sqlInit;
    QGroupBox*         expertSettings;
    QGroupBox*         dbNoticeBox;
    
    DFileSelector*     dbPathEdit;
    
    DbEngineParameters orgPrms;
};

DatabaseSettingsWidget::DatabaseSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setupMainArea();
}

DatabaseSettingsWidget::~DatabaseSettingsWidget()
{
    delete d;
}

void DatabaseSettingsWidget::setupMainArea()
{
    QVBoxLayout* const layout = new QVBoxLayout();
    setLayout(layout);

    // --------------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGroupBox* const dbConfigBox    = new QGroupBox(i18n("Database Configuration"), this);
    QVBoxLayout* const vlay         = new QVBoxLayout(dbConfigBox);

    DHBox* const typeHbox           = new DHBox();
    QLabel* const databaseTypeLabel = new QLabel(typeHbox);
    d->dbType                       = new QComboBox(typeHbox);
    databaseTypeLabel->setText(i18n("Type:"));

    // --------------------------------------------------------

    d->dbPathLabel = new QLabel(i18n("<p>Set here the location where the database files will be stored on your system. "
                                     "There are 3 databases : one for all collections properties, "
                                     "one to store compressed thumbnails, "
                                     "and one to store faces recognition metadata.<br/>"
                                     "Write access is required to be able to edit image properties.</p>"
                                     "<p>Databases are digiKam core engines. Take a care to use a place hosted by a fast "
                                     "hardware (as SSD) with enough free space especially for thumbnails database.</p>"
                                     "<p>Note: a remote file system such as NFS, cannot be used here. "
                                     "For performance reasons, it's also recommended to not use a removable media.</p>"
                                     "<p></p>"), dbConfigBox);
    d->dbPathLabel->setWordWrap(true);
    d->dbPathEdit  = new DFileSelector(dbConfigBox);
    d->dbPathEdit->setFileDlgMode(QFileDialog::Directory);

    // --------------------------------------------------------

    QLabel* const dbNameCoreLabel                    = new QLabel(i18n("Core Db Name:"));
    d->dbNameCore                                    = new QLineEdit();
    d->dbNameCore->setPlaceholderText(i18n("Set the core database name"));
    d->dbNameCore->setToolTip(i18n("The core database is lead digiKam container used to store\nalbums, items, and searches metadata."));
    QLabel* const dbNameThumbsLabel                  = new QLabel(i18n("Thumbs Db Name:"));
    d->dbNameThumbs                                  = new QLineEdit();
    d->dbNameThumbs->setPlaceholderText(i18n("Set the thumbnails database name"));
    d->dbNameThumbs->setToolTip(i18n("The thumbnails database is used by digiKam to host\nimage thumbs with wavelets compression images.\n"
                                      "This one can use quickly a lots of space,\nespecially if you have huge collections."));
    QLabel* const dbNameFaceLabel                    = new QLabel(i18n("Face Db Name:"));
    d->dbNameFace                                    = new QLineEdit();
    d->dbNameFace->setPlaceholderText(i18n("Set the face database name"));
    d->dbNameFace->setToolTip(i18n("The face database is used by digiKam to host image histograms\ndedicated to faces recognition process.\n"
                                "This one can use quickly a lots of space, especially\nif you a lots of image with people faces detected and tagged."));
    QLabel* const hostNameLabel                      = new QLabel(i18n("Host Name:"));
    d->hostName                                      = new QLineEdit();
    d->hostName->setPlaceholderText(i18n("Set the host computer name"));
    d->hostName->setToolTip(i18n("This is the computer name running Mysql server.\nThis can be \"localhost\" for a local server, or the network computer\n"
                              "name (or IP address) in case of remote computer."));
    QLabel* const hostPortLabel                      = new QLabel(i18n("Host Port:"));
    d->hostPort                                      = new QSpinBox();
    d->hostPort->setToolTip(i18n("Set the host computer port.\nUsually, Mysql server use port number 3306 by default"));
    d->hostPort->setMaximum(65535);

    QLabel* const connectOptsLabel                   = new QLabel(i18n("Connect options:"));
    d->connectOpts                                   = new QLineEdit();
    d->connectOpts->setPlaceholderText(i18n("Set the database connection options"));
    d->connectOpts->setToolTip(i18n("Set the Mysql server connection options.\nFor advanced users only."));

    QLabel* const userNameLabel                      = new QLabel(i18n("User:"));
    d->userName                                      = new QLineEdit();
    d->userName->setPlaceholderText(i18n("Set the database account name"));
    d->userName->setToolTip(i18n("Set the Mysql server account name used\nby digiKam to be connected to the server."));

    QLabel* const passwordLabel                      = new QLabel(i18n("Password:"));
    d->password                                      = new QLineEdit();
    d->password->setToolTip(i18n("Set the Mysql server account password used\nby digiKam to be connected to the server."));
    d->password->setEchoMode(QLineEdit::Password);

    QPushButton* const checkDatabaseConnectionButton = new QPushButton(i18n("Check Database Connection"));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

    expertSettinglayout->addRow(hostNameLabel,     d->hostName);
    expertSettinglayout->addRow(hostPortLabel,     d->hostPort);
    expertSettinglayout->addRow(dbNameCoreLabel,   d->dbNameCore);
    expertSettinglayout->addRow(dbNameThumbsLabel, d->dbNameThumbs);
    expertSettinglayout->addRow(dbNameFaceLabel,   d->dbNameFace);
    expertSettinglayout->addRow(userNameLabel,     d->userName);
    expertSettinglayout->addRow(passwordLabel,     d->password);
    expertSettinglayout->addRow(connectOptsLabel,  d->connectOpts);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(typeHbox);
    vlay->addWidget(new DLineWidget(Qt::Horizontal));
    vlay->addWidget(d->dbPathLabel);
    vlay->addWidget(d->dbPathEdit);
    vlay->addWidget(d->expertSettings);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    // --------------------------------------------------------

    d->dbNoticeBox           = new QGroupBox(i18n("Database Server Instructions"), this);
    QVBoxLayout* const vlay2 = new QVBoxLayout(d->dbNoticeBox);
    QLabel* const notice     = new QLabel(i18n("<p>digiKam expects the above database and user account to already exists. "
                                               "This user also require full access to the database.<br>"
                                               "If your database is not already set up, you can use the following SQL commands "
                                               "(after replacing the password with the correct one)."), d->dbNoticeBox);
    notice->setWordWrap(true);

    d->sqlInit = new QTextBrowser(d->dbNoticeBox);
    d->sqlInit->setOpenExternalLinks(false);
    d->sqlInit->setOpenLinks(false);
    d->sqlInit->setReadOnly(false);

    vlay2->addWidget(notice);
    vlay2->addWidget(d->sqlInit);
    vlay2->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay2->setSpacing(spacing);

    // --------------------------------------------------------

    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addWidget(dbConfigBox);
    layout->addWidget(d->dbNoticeBox);
    layout->addStretch();

    // --------- fill with default values ---------------------

    d->dbType->addItem(i18n("SQLite"),                        SQlite);

#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
    d->dbType->addItem(i18n("MySQL Internal (experimental)"), MysqlInternal);
#   endif

    d->dbType->addItem(i18n("MySQL Server (experimental)"),   MysqlServer);
#endif

    d->dbType->setToolTip(i18n("<p>Select here the type of database backend.</p>"
                               "<p><b>SQlite</b> backend is for local database storage with a small or medium collection sizes. "
                               "It is the default and recommended backend.</p>"
#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
                               "<p><b>MySQL Internal</b> backend is for local database storage with huge collection sizes. "
                               "Be careful: this one still in experimental stage.</p>"
#   endif

                               "<p><b>MySQL Server</b> backend is a more robust solution especially for remote and shared database storage. "
                               "It is also more efficient to manage huge collection sizes. "
                               "Be careful: this one still in experimental stage.</p>"
#endif
                              ));

    // --------------------------------------------------------

    connect(d->dbType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(checkDatabaseConnection()));

    connect(d->dbNameCore, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameThumbs, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameFace, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->userName, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

int DatabaseSettingsWidget::databaseType() const
{
    return d->dbType->currentIndex();
}

QString DatabaseSettingsWidget::databasePath() const
{
    return d->dbPathEdit->lineEdit()->text();
}

DbEngineParameters DatabaseSettingsWidget::orgDatabasePrm() const
{
    return d->orgPrms;
}

QString DatabaseSettingsWidget::databaseBackend() const
{
    switch(databaseType())
    {
        case MysqlInternal:
        case MysqlServer:
        {
            return DbEngineParameters::MySQLDatabaseType();
        }
        default: // SQlite
        {
            return DbEngineParameters::SQLiteDatabaseType();
        }
    }
}

void DatabaseSettingsWidget::slotChangeDatabasePath(const QUrl& result)
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

void DatabaseSettingsWidget::slotDatabasePathEditedDelayed()
{
    QTimer::singleShot(300, this, SLOT(slotDatabasePathEdited()));
}

void DatabaseSettingsWidget::slotDatabasePathEdited()
{
    QString newPath = databasePath();

#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        d->dbPathEdit->lineEdit()->setText(QDir::homePath() + QLatin1Char('/') + QDir::fromNativeSeparators(newPath));
    }

#endif

    d->dbPathEdit->lineEdit()->setText(QDir::toNativeSeparators(newPath));

    checkDBPath();
}

void DatabaseSettingsWidget::slotHandleDBTypeIndexChanged(int index)
{
    setDatabaseInputFields(index);
    handleInternalServer(index);
    slotUpdateSqlInit();
}

void DatabaseSettingsWidget::setDatabaseInputFields(int index)
{
    switch(index)
    {
        //case MysqlInternal:
        case SQlite:
        {
            d->dbPathLabel->setVisible(true);
            d->dbPathEdit->setVisible(true);
            d->expertSettings->setVisible(false);

            connect(d->dbPathEdit, SIGNAL(signalUrlSelected(QUrl)),
                    this, SLOT(slotChangeDatabasePath(QUrl)));

            connect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            d->dbNoticeBox->setVisible(false);
            break;
        }
        default: // MysqlServer
        {
            d->dbPathLabel->setVisible(false);
            d->dbPathEdit->setVisible(false);
            d->expertSettings->setVisible(true);

            disconnect(d->dbPathEdit, SIGNAL(signalUrlSelected(QUrl)),
                       this, SLOT(slotChangeDatabasePath(QUrl)));

            disconnect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                       this, SLOT(slotDatabasePathEditedDelayed()));

            d->dbNoticeBox->setVisible(index == MysqlServer);
            break;
        }
    }
}

void DatabaseSettingsWidget::handleInternalServer(int index)
{
    bool internal = (index == MysqlInternal);

    d->hostName->setDisabled(internal);
    d->hostPort->setDisabled(internal);
    d->dbNameCore->setDisabled(internal);
    d->dbNameThumbs->setDisabled(internal);
    d->dbNameFace->setDisabled(internal);
    d->userName->setDisabled(internal);
    d->password->setDisabled(internal);
    d->connectOpts->setDisabled(internal);
    d->hostPort->setValue(internal ? -1 : 3306);
}

void DatabaseSettingsWidget::slotUpdateSqlInit()
{
    QString sql = QString::fromLatin1("CREATE DATABASE %1; "
                                      "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                      "FLUSH PRIVILEGES;\n")
                                      .arg(d->dbNameCore->text())
                                      .arg(d->dbNameCore->text())
                                      .arg(d->userName->text());

    if (d->dbNameThumbs->text() != d->dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(d->dbNameThumbs->text())
                                   .arg(d->dbNameThumbs->text())
                                   .arg(d->userName->text());
    }

    if (d->dbNameFace->text() != d->dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(d->dbNameFace->text())
                                   .arg(d->dbNameFace->text())
                                   .arg(d->userName->text());
    }

    d->sqlInit->setText(sql);
}

void DatabaseSettingsWidget::checkDatabaseConnection()
{
    // TODO : if check DB connection operations can be threaded, use DBusyDlg dialog there...

    qApp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID(QLatin1String("ConnectionTest"));
    QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(databaseBackend(), databaseID);
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
        QMessageBox::information(qApp->activeWindow(), i18n("Database connection test"),
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

void DatabaseSettingsWidget::checkDBPath()
{
/*
    bool dbOk          = false;
    bool pathUnchanged = true;
*/
    QString newPath    = databasePath();

    if (!newPath.isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(orgDatabasePrm().getCoreDatabaseNameOrDir());
/*
        dbOk          = dbDir.exists();
        pathUnchanged = (dbDir == oldDir);
*/
    }

    //TODO create an Enable button slot, if the path is valid
    //d->mainDialog->enableButtonOk(dbOk);
}

void DatabaseSettingsWidget::setParametersFromSettings(const ApplicationSettings* const settings)
{
    d->orgPrms = settings->getDbEngineParameters();

    if (d->orgPrms.databaseType == DbEngineParameters::SQLiteDatabaseType())
    {
        d->dbPathEdit->lineEdit()->setText(d->orgPrms.getCoreDatabaseNameOrDir());
        d->dbType->setCurrentIndex(SQlite);
    }
#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
    else if (d->orgPrms.databaseType == DbEngineParameters::MySQLDatabaseType() && d->orgPrms.internalServer)
    {
        d->dbType->setCurrentIndex(MysqlInternal);
    }
#   endif
    else
    {
        d->dbType->setCurrentIndex(MysqlServer);
    }
#endif

    d->dbNameCore->setText(d->orgPrms.databaseNameCore);
    d->dbNameThumbs->setText(d->orgPrms.databaseNameThumbnails);
    d->dbNameFace->setText(d->orgPrms.databaseNameFace);
    d->hostName->setText(d->orgPrms.hostName);
    d->hostPort->setValue(d->orgPrms.port);
    d->connectOpts->setText(d->orgPrms.connectOptions);

    d->userName->setText(d->orgPrms.userName);
    d->password->setText(d->orgPrms.password);

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

DbEngineParameters DatabaseSettingsWidget::getDbEngineParameters() const
{
    DbEngineParameters prm;

    switch(databaseType())
    {
        case SQlite:
            prm = DbEngineParameters::parametersForSQLiteDefaultFile(databasePath());
            break;
            
        case MysqlInternal:
            prm = DbEngineParameters::defaultParameters(databaseBackend());
            //DatabaseServerStarter::startServerManagerProcess(databaseBackend());
            break;

        default: // MysqlServer
            prm.internalServer         = false;
            prm.databaseType           = databaseBackend();
            prm.databaseNameCore       = d->dbNameCore->text();
            prm.databaseNameThumbnails = d->dbNameThumbs->text();
            prm.databaseNameFace       = d->dbNameFace->text();
            prm.connectOptions         = d->connectOpts->text();
            prm.hostName               = d->hostName->text();
            prm.port                   = d->hostPort->text().toInt();
            prm.userName               = d->userName->text();
            prm.password               = d->password->text();
            break;
    }
    
    return prm;
}

} // namespace Digikam
