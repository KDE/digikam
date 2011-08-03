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

    imgDatabasePathEdit     = new KUrlRequester(dbPathBox);
    imgDatabasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);

    internalServer                      = new QCheckBox(i18n("Internal Server"));

    QLabel* databaseTypeLabel           = new QLabel(i18n("Type"));
    QLabel* databaseNameLabel           = new QLabel(i18n("Schema Name"));
    QLabel* hostNameLabel               = new QLabel(i18n("Host Name"));
    QLabel* hostPortLabel               = new QLabel(i18n("Port"));
    QLabel* connectionOptionsLabel      = new QLabel(i18n("Database<br>Connection<br>Options"));
    QLabel* userNameLabel               = new QLabel(i18n("User"));
    QLabel* passwordLabel               = new QLabel(i18n("Password"));

    imgDatabaseType                        = new QComboBox();
    imgDatabaseName                        = new QLineEdit();
    imgHostName                            = new QLineEdit();
    imgHostPort                            = new QSpinBox();
    imgHostPort->setMaximum(65536);
    imgConnectionOptions                   = new QLineEdit();
    imgUserName                            = new QLineEdit();
    imgPassword                            = new QLineEdit();
    imgPassword->setEchoMode(QLineEdit::Password);
    QPushButton* imgCheckDatabaseConnectionButton = new QPushButton(i18n("Check DB Connection"));

    tmbDatabaseType                        = new QComboBox();
    tmbDatabaseName                        = new QLineEdit();
    tmbHostName                            = new QLineEdit();
    tmbHostPort                            = new QSpinBox();
    tmbHostPort->setMaximum(65536);
    tmbConnectionOptions                   = new QLineEdit();
    tmbUserName                            = new QLineEdit();
    tmbPassword                            = new QLineEdit();
    tmbPassword->setEchoMode(QLineEdit::Password);
    QPushButton* tmbCheckDatabaseConnectionButton = new QPushButton(i18n("Check DB Connection"));

    d->expertSettings                   = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* expertSettinglayout    = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

// #ifdef HAVE_INTERNALMYSQL
//     expertSettinglayout->addRow(internalServerLabel, internalServer);
// #endif // HAVE_INTERNALMYSQL
    expertSettinglayout->addRow(hostNameLabel, imgHostName);
    expertSettinglayout->addRow(hostPortLabel, imgHostPort);
    expertSettinglayout->addRow(databaseNameLabel, imgDatabaseName);
    expertSettinglayout->addRow(userNameLabel, imgUserName);
    expertSettinglayout->addRow(passwordLabel, imgPassword);
    expertSettinglayout->addRow(connectionOptionsLabel, imgConnectionOptions);
    expertSettinglayout->addWidget(imgCheckDatabaseConnectionButton);

    expertSettinglayout->addRow(hostNameLabel, tmbHostName);
    expertSettinglayout->addRow(hostPortLabel, tmbHostPort);
    expertSettinglayout->addRow(databaseNameLabel, tmbDatabaseName);
    expertSettinglayout->addRow(userNameLabel, tmbUserName);
    expertSettinglayout->addRow(passwordLabel, tmbPassword);
    expertSettinglayout->addRow(connectionOptionsLabel, tmbConnectionOptions);
    expertSettinglayout->addWidget(tmbCheckDatabaseConnectionButton);
    
    // FIXME:
    vlay->addWidget(databaseTypeLabel);
    vlay->addWidget(imgDatabaseType);
    vlay->addWidget(tmbDatabaseType);
    vlay->addWidget(d->databasePathLabel);
    vlay->addWidget(imgDatabasePathEdit);
    vlay->addWidget(tmbDatabasePathEdit);
    vlay->addWidget(d->expertSettings);
    vlay->setSpacing(0);
    vlay->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------- fill with default values ---------------------

    imgDatabaseType->addItem(i18n("SQLite"), DatabaseParameters::SQLiteDatabaseType());
    imgDatabaseType->addItem(i18n("MySQL"), DatabaseParameters::MySQLDatabaseType());
    setDatabaseInputFields(DatabaseParameters::SQLiteDatabaseType());

    tmbDatabaseType->addItem(i18n("SQLite"), DatabaseParameters::SQLiteDatabaseType());
    tmbDatabaseType->addItem(i18n("MySQL"), DatabaseParameters::MySQLDatabaseType());
    setDatabaseInputFields(DatabaseParameters::SQLiteDatabaseType());

    // --------------------------------------------------------

    adjustSize();

    // --------------------------------------------------------
    //FIXME:
    connect(imgDatabasePathEdit, SIGNAL(urlSelected(KUrl)),
            this, SLOT(slotImgChangeDatabasePath(KUrl)));

    connect(imgDatabasePathEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotImgDatabasePathEdited(QString)));

    connect(imgDatabaseType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotImgHandleDBTypeIndexChanged(int)));

#ifdef HAVE_INTERNALMYSQL
    connect(internalServer, SIGNAL(stateChanged(int)),
            this, SLOT(slotHandleInternalServerCheckbox(int)));
#endif

    connect(imgCheckDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(slotImgCheckDatabaseConnection()));

    // --------------------------------------------------------
    //FIXME:
    connect(imgDatabasePathEdit, SIGNAL(urlSelected(KUrl)),
            this, SLOT(slotTmbChangeDatabasePath(KUrl)));

    connect(imgDatabasePathEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotTmbDatabasePathEdited(QString)));

    connect(imgDatabaseType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTmbHandleDBTypeIndexChanged(int)));

    connect(imgCheckDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(slotTmbCheckDatabaseConnection()));
}

QString DatabaseWidget::imgCurrentDatabaseType() const
{
    return imgDatabaseType->itemData(imgDatabaseType->currentIndex()).toString();
}

QString DatabaseWidget::tmbCurrentDatabaseType() const
{
    return tmbDatabaseType->itemData(tmbDatabaseType->currentIndex()).toString();
}

void DatabaseWidget::slotImgChangeDatabasePath(const KUrl& result)
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

void DatabaseWidget::slotImgDatabasePathEdited(const QString& newPath)
{
#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        imgDatabasePathEdit->setUrl(QString(QDir::homePath() + QLatin1Char('/') + newPath));
    }

#endif

    checkDBPath();
}

void DatabaseWidget::slotImgHandleDBTypeIndexChanged(int index)
{
    const QString& dbType = imgDatabaseType->itemData(index).toString();
    setDatabaseInputFields(dbType);
}

void DatabaseWidget::slotTmbChangeDatabasePath(const KUrl& result)
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

void DatabaseWidget::slotTmbDatabasePathEdited(const QString& newPath)
{
#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        tmbDatabasePathEdit->setUrl(QString(QDir::homePath() + QLatin1Char('/') + newPath));
    }

#endif

    checkDBPath();
}

void DatabaseWidget::slotTmbHandleDBTypeIndexChanged(int index)
{
    const QString& dbType = tmbDatabaseType->itemData(index).toString();
    setDatabaseInputFields(dbType);
}

void DatabaseWidget::setDatabaseInputFields(const QString& currentIndexStr)
{
    if (currentIndexStr == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        d->databasePathLabel->setVisible(true);
        imgDatabasePathEdit->setVisible(true);
        d->expertSettings->setVisible(false);
    }
    else
    {
        d->databasePathLabel->setVisible(false);
        imgDatabasePathEdit->setVisible(false);
        d->expertSettings->setVisible(true);
    }

    adjustSize();
}

void DatabaseWidget::slotHandleInternalServerCheckbox(int enableFields)
{
    //FIXME:
    imgHostName->setEnabled(enableFields == Qt::Unchecked);
    imgHostPort->setEnabled(enableFields == Qt::Unchecked);
    imgDatabaseName->setEnabled(enableFields == Qt::Unchecked);
    //fr databaseNameThumbnails->setEnabled(enableFields == Qt::Unchecked);
    imgUserName->setEnabled(enableFields == Qt::Unchecked);
    imgPassword->setEnabled(enableFields == Qt::Unchecked);
    imgConnectionOptions->setEnabled(enableFields == Qt::Unchecked);
}

void DatabaseWidget::slotImgCheckDatabaseConnection()
{
    // TODO : if chek DB connection operations can be threaded, use DBusyDlg dialog there...
    //FIXME:

    kapp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID("ConnectionTest");
    QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(imgCurrentDatabaseType(), databaseID);
    DatabaseParameters parameters = getDatabaseParameters();
    testDatabase.setHostName(parameters.imgHostName);
    testDatabase.setPort(parameters.imgPort);
    testDatabase.setUserName(parameters.imgUserName);
    testDatabase.setPassword(parameters.imgPassword);
    testDatabase.setConnectOptions(parameters.imgConnectOptions);

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


void DatabaseWidget::slotTmbCheckDatabaseConnection()
{
    // TODO : if chek DB connection operations can be threaded, use DBusyDlg dialog there...
    //FIXME:

    kapp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID("ConnectionTest");
    QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(tmbCurrentDatabaseType(), databaseID);
    DatabaseParameters parameters = getDatabaseParameters();
    testDatabase.setHostName(parameters.tmbHostName);
    testDatabase.setPort(parameters.tmbPort);
    testDatabase.setUserName(parameters.tmbUserName);
    testDatabase.setPassword(parameters.tmbPassword);
    testDatabase.setConnectOptions(parameters.tmbConnectOptions);

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
    QString newPath    = imgDatabasePathEdit->url().toLocalFile();

    if (!imgDatabasePathEdit->url().path().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(imgOriginalDbPath);
    }

    if (!tmbDatabasePathEdit->url().path().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(tmbOriginalDbPath);
    }
}

void DatabaseWidget::setParametersFromSettings(const AlbumSettings* settings)
{

#ifdef HAVE_INTERNALMYSQL
    internalServer->setChecked(settings->getInternalDatabaseServer());
#else
    internalServer->setChecked(false);
#endif

    imgOriginalDbPath = settings->getDatabaseFilePath();
    imgOriginalDbType = settings->getDatabaseType();
    imgDatabasePathEdit->setUrl(settings->getDatabaseFilePath());

    imgDatabaseName->setText(settings->getDatabaseName());
    //fr databaseNameThumbnails->setText(settings->getDatabaseNameThumbnails());
    imgHostName->setText(settings->getDatabaseHostName());
    imgHostPort->setValue(settings->getDatabasePort());
    imgConnectionOptions->setText(settings->getDatabaseConnectoptions());

    imgUserName->setText(settings->getDatabaseUserName());

    imgPassword->setText(settings->getDatabasePassword());

    /* Now set the type according the database type from the settings.
     * If no item is found, ignore the setting.
     */
    for (int i=0; i<imgDatabaseType->count(); ++i)
    {
        //kDebug(50003) << "Comparing comboboxentry on index ["<< i <<"] [" << imgDatabaseType->itemData(i)
        //            << "] with ["<< settings->getImgDatabaseType() << "]";
        if (imgDatabaseType->itemData(i).toString() == settings->getDatabaseType())
        {
            imgDatabaseType->setCurrentIndex(i);
        }
    }
}

DatabaseParameters DatabaseWidget::getDatabaseParameters()
{
    DatabaseParameters parameters;

    //fr if (currentDatabaseType() == QString(DatabaseParameters::SQLiteDatabaseType()) || !internalServer->isChecked())
    if (imgCurrentDatabaseType() == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        //FIXME:
        parameters.imgConnectOptions = imgConnectionOptions->text();
        parameters.imgDatabaseType   = imgCurrentDatabaseType();
        parameters.imgHostName       = imgHostName->text();
        parameters.imgPassword       = imgPassword->text();
        parameters.imgPort           = imgHostPort->text().toInt();
        parameters.imgUserName       = imgUserName->text();

        if (parameters.imgDatabaseType == QString(DatabaseParameters::SQLiteDatabaseType()))
        {
            parameters.imgDatabaseName = QDir::cleanPath(imgDatabasePathEdit->url().toLocalFile() + '/' + "digikam4.db");
            parameters.tmbDatabaseName = QDir::cleanPath(tmbDatabasePathEdit->url().toLocalFile() + '/' + "thumbnails-digikam.db");
        }
        else
        {
            parameters.imgDatabaseName = imgDatabaseName->text();
            parameters.tmbDatabaseName = tmbDatabaseName->text();
        }
    }
    else
    {
        // FIXME:
        parameters = DatabaseParameters::defaultParameters(imgCurrentDatabaseType());
        DatabaseServerStarter::startServerManagerProcess(imgCurrentDatabaseType());
    }

    return parameters;
}

} // namespace Digikam
