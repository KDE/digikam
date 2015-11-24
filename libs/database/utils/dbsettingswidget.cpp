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
#include "databaseserverstarter.h"

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
    }

    QComboBox*    dbType;
    QLabel*       dbPathLabel;
    QTextBrowser* sqlInit;
    QGroupBox*    expertSettings;
    QGroupBox*    dbNoticeBox;
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

    d->dbPathLabel               = new QLabel(i18n("<p>Set here the location where the database files will be stored on your system. "
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
    dbPathEdit                                       = new DFileSelector(dbConfigBox);
    dbPathEdit->setFileDlgMode(QFileDialog::Directory);

    // --------------------------------------------------------

    QLabel* const dbNameCoreLabel                    = new QLabel(i18n("Core Db Name:"));
    dbNameCore                                       = new QLineEdit();
    dbNameCore->setPlaceholderText(i18n("Set the core database name"));
    dbNameCore->setToolTip(i18n("The core database is lead digiKam container used to store\nalbums, items, and searches metadata."));
    QLabel* const dbNameThumbnailsLabel              = new QLabel(i18n("Thumbs Db Name:"));
    dbNameThumbnails                                 = new QLineEdit();
    dbNameThumbnails->setPlaceholderText(i18n("Set the thumbnails database name"));
    dbNameThumbnails->setToolTip(i18n("The thumbnails database is used by digiKam to host\nimage thumbs with wavelets compression images.\n"
                                      "This one can use quickly a lots of space,\nespecially if you have huge collections."));
    QLabel* const dbNameFaceLabel                    = new QLabel(i18n("Face Db Name:"));
    dbNameFace                                       = new QLineEdit();
    dbNameFace->setPlaceholderText(i18n("Set the face database name"));
    dbNameFace->setToolTip(i18n("The face database is used by digiKam to host image histograms\ndedicated to faces recognition process.\n"
                                "This one can use quickly a lots of space, especially\nif you a lots of image with people faces detected and tagged."));
    QLabel* const hostNameLabel                      = new QLabel(i18n("Host Name:"));
    hostName                                         = new QLineEdit();
    hostName->setPlaceholderText(i18n("Set the host computer name"));
    hostName->setToolTip(i18n("This is the computer name running Mysql server.\nThis can be \"localhost\" for a local server, or the network computer\n"
                              "name (or IP adress) in case of remote computer."));
    QLabel* const hostPortLabel                      = new QLabel(i18n("Host Port:"));
    hostPort                                         = new QSpinBox();
    hostPort->setToolTip(i18n("Set the host computer port.\nUsually, Mysql server use port number 3306 by default"));
    hostPort->setMaximum(65535);

    QLabel* const connectionOptionsLabel             = new QLabel(i18n("Connect options:"));
    connectionOptions                                = new QLineEdit();
    connectionOptions->setPlaceholderText(i18n("Set the database connection options"));
    connectionOptions->setToolTip(i18n("Set the Mysql server connection options.\nFor advanced users only."));

    QLabel* const userNameLabel                      = new QLabel(i18n("User:"));
    userName                                         = new QLineEdit();
    userName->setPlaceholderText(i18n("Set the database account name"));
    userName->setToolTip(i18n("Set the Mysql server account name used\nby digiKam to be connected to the server."));

    QLabel* const passwordLabel                      = new QLabel(i18n("Password:"));
    password                                         = new QLineEdit();
    password->setToolTip(i18n("Set the Mysql server account password used\nby digiKam to be connected to the server."));
    password->setEchoMode(QLineEdit::Password);

    QPushButton* const checkDatabaseConnectionButton = new QPushButton(i18n("Check Database Connection"));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

    expertSettinglayout->addRow(hostNameLabel,          hostName);
    expertSettinglayout->addRow(hostPortLabel,          hostPort);
    expertSettinglayout->addRow(dbNameCoreLabel,        dbNameCore);
    expertSettinglayout->addRow(dbNameThumbnailsLabel,  dbNameThumbnails);
    expertSettinglayout->addRow(dbNameFaceLabel,        dbNameFace);
    expertSettinglayout->addRow(userNameLabel,          userName);
    expertSettinglayout->addRow(passwordLabel,          password);
    expertSettinglayout->addRow(connectionOptionsLabel, connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(typeHbox);
    vlay->addWidget(new DLineWidget(Qt::Horizontal));
    vlay->addWidget(d->dbPathLabel);
    vlay->addWidget(dbPathEdit);
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

    connect(dbNameCore, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(dbNameThumbnails, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(dbNameFace, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(userName, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

int DatabaseSettingsWidget::databaseType() const
{
    return d->dbType->currentIndex();
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
    QString newPath = dbPathEdit->lineEdit()->text();

#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        dbPathEdit->lineEdit()->setText(QDir::homePath() + QLatin1Char('/') + QDir::fromNativeSeparators(newPath));
    }

#endif

    dbPathEdit->lineEdit()->setText(QDir::toNativeSeparators(newPath));

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
            dbPathEdit->setVisible(true);
            d->expertSettings->setVisible(false);

            connect(dbPathEdit, SIGNAL(signalUrlSelected(QUrl)),
                    this, SLOT(slotChangeDatabasePath(QUrl)));

            connect(dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            d->dbNoticeBox->setVisible(false);
            break;
        }
        default: // MysqlServer
        {
            d->dbPathLabel->setVisible(false);
            dbPathEdit->setVisible(false);
            d->expertSettings->setVisible(true);

            disconnect(dbPathEdit, SIGNAL(signalUrlSelected(QUrl)),
                    this, SLOT(slotChangeDatabasePath(QUrl)));

            disconnect(dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            d->dbNoticeBox->setVisible(index == MysqlServer);
            break;
        }
    }
}

void DatabaseSettingsWidget::handleInternalServer(int index)
{
    bool internal = (index == MysqlInternal);

    hostName->setDisabled(internal);
    hostPort->setDisabled(internal);
    dbNameCore->setDisabled(internal);
    dbNameThumbnails->setDisabled(internal);
    dbNameFace->setDisabled(internal);
    userName->setDisabled(internal);
    password->setDisabled(internal);
    connectionOptions->setDisabled(internal);
    hostPort->setValue(internal ? -1 : 3306);
}

void DatabaseSettingsWidget::slotUpdateSqlInit()
{
    QString sql = QString::fromLatin1("CREATE DATABASE %1; "
                                      "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                      "FLUSH PRIVILEGES;\n")
                                      .arg(dbNameCore->text())
                                      .arg(dbNameCore->text())
                                      .arg(userName->text());

    if (dbNameThumbnails->text() != dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(dbNameThumbnails->text())
                                   .arg(dbNameThumbnails->text())
                                   .arg(userName->text());
    }

    if (dbNameFace->text() != dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(dbNameFace->text())
                                   .arg(dbNameFace->text())
                                   .arg(userName->text());
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
    QString newPath    = dbPathEdit->lineEdit()->text();

    if (!dbPathEdit->lineEdit()->text().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(originalDbPath);
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
    DbEngineParameters prm = settings->getDbEngineParameters();
    originalDbPath         = prm.getCoreDatabaseNameOrDir();
    originalDbBackend      = prm.databaseType;

    dbPathEdit->lineEdit()->setText(prm.getCoreDatabaseNameOrDir());

    if (prm.databaseType == DbEngineParameters::SQLiteDatabaseType())
    {
        d->dbType->setCurrentIndex(SQlite);
    }
#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
    else if (prm.databaseType == DbEngineParameters::MySQLDatabaseType() && prm.internalServer)
    {
        d->dbType->setCurrentIndex(MysqlInternal);
    }
#   endif
    else
    {
        d->dbType->setCurrentIndex(MysqlServer);
    }
#endif

    dbNameCore->setText(prm.databaseNameCore);
    dbNameThumbnails->setText(prm.databaseNameThumbnails);
    dbNameFace->setText(prm.databaseNameFace);
    hostName->setText(prm.hostName);
    hostPort->setValue(prm.port);
    connectionOptions->setText(prm.connectOptions);

    userName->setText(prm.userName);
    password->setText(prm.password);

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

DbEngineParameters DatabaseSettingsWidget::getDbEngineParameters() const
{
    DbEngineParameters parameters;

    if ((databaseType() == SQlite) || !(databaseType() == MysqlInternal))
    {
        parameters.connectOptions = connectionOptions->text();
        parameters.databaseType   = databaseBackend();
        parameters.hostName       = hostName->text();
        parameters.password       = password->text();
        parameters.port           = hostPort->text().toInt();
        parameters.userName       = userName->text();

        if (parameters.databaseType == DbEngineParameters::SQLiteDatabaseType())
        {
            parameters.databaseNameCore       = QDir::cleanPath(dbPathEdit->lineEdit()->text() + 
                                                QLatin1Char('/') + QLatin1String("digikam4.db"));
            parameters.databaseNameThumbnails = QDir::cleanPath(dbPathEdit->lineEdit()->text() + 
                                                QLatin1Char('/') + QLatin1String("thumbnails-digikam.db"));
            parameters.databaseNameFace       = QDir::cleanPath(dbPathEdit->lineEdit()->text() + 
                                                QLatin1Char('/') + QLatin1String("recognition.db"));
        }
        else
        {
            parameters.databaseNameCore       = dbNameCore->text();
            parameters.databaseNameThumbnails = dbNameThumbnails->text();
            parameters.databaseNameFace       = dbNameFace->text();
        }
    }
    else
    {
        parameters = DbEngineParameters::defaultParameters(databaseBackend());
        DatabaseServerStarter::startServerManagerProcess(databaseBackend());
    }

    return parameters;
}

} // namespace Digikam
