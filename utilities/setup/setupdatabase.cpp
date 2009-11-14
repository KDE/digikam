/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class SetupDatabasePriv
{
public:

    SetupDatabasePriv()
    {
        mainDialog       = 0;
        databasePathEdit = 0;
    }

    KUrlRequester           *databasePathEdit;
    QString                  originalDbPath;
    QString                  originalDbType;
    QLabel                  *databasePathLabel;
    QComboBox               *databaseType;
    QLineEdit               *databaseName;
    QLineEdit               *hostName;
    QSpinBox                *hostPort;
    QLineEdit               *connectionOptions;

    QLineEdit               *userName;

    QLineEdit               *password;

    QGroupBox               *expertSettings;

    KPageDialog             *mainDialog;
};

SetupDatabase::SetupDatabase(KPageDialog* dialog, QWidget* parent)
                : QScrollArea(parent), d(new SetupDatabasePriv)
{
    d->mainDialog  = dialog;
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *dbPathBox      = new QGroupBox(i18n("Database File Path"), panel);
    QVBoxLayout *vlay         = new QVBoxLayout(dbPathBox);
    d->databasePathLabel = new QLabel(i18n("<p>The location where the database file will be stored on your system. "
                                                "There is one common database file for all root albums.<br/>"
                                                "Write access is required to be able to edit image properties.</p>"
                                                "<p>Note: a remote file system, such as NFS, cannot be used here.</p><p></p>"),
                                           dbPathBox);
    d->databasePathLabel->setWordWrap(true);
    d->databasePathLabel->setFont(KGlobalSettings::smallestReadableFont());

    d->databasePathEdit = new KUrlRequester(dbPathBox);
    d->databasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);

    QLabel *databaseTypeLabel        = new QLabel(i18n("Type"));
    d->databaseType                  = new QComboBox();
    QLabel *databaseNameLabel        = new QLabel(i18n("Name"));
    d->databaseName                  = new QLineEdit();
    QLabel *hostNameLabel            = new QLabel(i18n("Host Name"));
    d->hostName                      = new QLineEdit();
    QLabel *hostPortLabel            = new QLabel(i18n("Port"));
    d->hostPort                      = new QSpinBox();
    d->hostPort->setMaximum(65536);

    QLabel *connectionOptionsLabel   = new QLabel(i18n("Database Connection Options"));
    d->connectionOptions             = new QLineEdit();

    QLabel *userNameLabel            = new QLabel(i18n("User"));
    d->userName                      = new QLineEdit();

    QLabel *passwordLabel            = new QLabel(i18n("Password"));
    d->password                      = new QLineEdit();
    d->password->setEchoMode(QLineEdit::Password);

    QPushButton *checkDatabaseConnectionButton = new QPushButton(i18n("Check DB Connection"));

    d->expertSettings = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout *expertSettinglayout = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

    vlay->addWidget(databaseTypeLabel);
    vlay->addWidget(d->databaseType);

    vlay->addWidget(d->databasePathLabel);
    vlay->addWidget(d->databasePathEdit);

    expertSettinglayout->addRow(hostNameLabel, d->hostName);
    expertSettinglayout->addRow(hostPortLabel, d->hostPort);
    expertSettinglayout->addRow(databaseNameLabel, d->databaseName);
    expertSettinglayout->addRow(userNameLabel, d->userName);
    expertSettinglayout->addRow(passwordLabel, d->password);
    expertSettinglayout->addRow(connectionOptionsLabel, d->connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(d->expertSettings);

    vlay->setSpacing(0);
    vlay->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------- fill with default values ---------------------
    d->databaseType->addItem("QSQLITE");
    d->databaseType->addItem("QMYSQL");

    setDatabaseInputFields("QSQLITE");

    // --------------------------------------------------------

    readSettings();
    adjustSize();

    // --------------------------------------------------------

    connect(d->databasePathEdit, SIGNAL(urlSelected(const KUrl&)),
            this, SLOT(slotChangeDatabasePath(const KUrl&)));

    connect(d->databasePathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDatabasePathEdited(const QString&)));

    connect(d->databaseType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setDatabaseInputFields(const QString&)));

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()), this, SLOT(checkDatabaseConnection()));

}

SetupDatabase::~SetupDatabase()
{
    delete d;
}

void SetupDatabase::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    if (d->databaseType->currentText()=="QSQLITE"){
        QString newPath = d->databasePathEdit->url().path();
        QDir oldDir(d->originalDbPath);
        QDir newDir(newPath);


        if (oldDir != newDir || d->databaseType->currentText()!=d->originalDbType)
        {
            settings->setDatabaseType(d->databaseType->currentText());
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
        settings->setDatabaseType(d->databaseType->currentText());
        settings->setDatabaseName(d->databaseName->text());
        settings->setDatabaseConnectoptions(d->connectionOptions->text());
        settings->setDatabaseHostName(d->hostName->text());
        settings->setDatabasePort(d->hostPort->text().toInt());
        settings->setDatabaseUserName(d->userName->text());
        settings->setDatabasePassword(d->password->text());
        settings->saveSettings();
    }
}

void SetupDatabase::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->originalDbPath = settings->getDatabaseFilePath();
    d->originalDbType = settings->getDatabaseType();
    d->databasePathEdit->setUrl(settings->getDatabaseFilePath());

    d->databaseName->setText(settings->getDatabaseName());
    d->hostName->setText(settings->getDatabaseHostName());
    d->hostPort->setValue(settings->getDatabasePort());
    d->connectionOptions->setText(settings->getDatabaseConnectoptions());

    d->userName->setText(settings->getDatabaseUserName());

    d->password->setText(settings->getDatabasePassword());

    /* Now set the type according the database type from the settings.
     * If no item is found, ignore the setting.
     */
    for (int i=0; i<d->databaseType->count(); i++){
        kDebug(50003) << "Comparing comboboxentry on index ["<< i <<"] [" << d->databaseType->itemText(i) << "] with ["<< settings->getDatabaseType() << "]";
        if (d->databaseType->itemText(i)==settings->getDatabaseType()){
            d->databaseType->setCurrentIndex(i);
            setDatabaseInputFields(d->databaseType->itemText(i));
        }
    }
}

void SetupDatabase::setDatabaseInputFields(const QString& currentIndexStr){
    if (currentIndexStr=="QSQLITE"){
        d->databasePathLabel->setVisible(true);
        d->databasePathEdit->setVisible(true);

        d->expertSettings->setVisible(false);
    }else{
        d->databasePathLabel->setVisible(false);
        d->databasePathEdit->setVisible(false);

        d->expertSettings->setVisible(true);
    }
}

void SetupDatabase::checkDatabaseConnection(){
    QString databaseID("ConnectionTest");
    QSqlDatabase testDatabase = QSqlDatabase::addDatabase(d->databaseType->currentText(), databaseID);
    testDatabase.setHostName(d->hostName->text());
    testDatabase.setPort(d->hostPort->text().toInt());
    testDatabase.setUserName(d->userName->text());
    testDatabase.setPassword(d->password->text());
    testDatabase.setConnectOptions(d->connectionOptions->text());

    bool result = testDatabase.open();
    if (result == true){
        KMessageBox::information(0, i18n("Database connection test successful."), i18n("Database connection test"));
    }else{
        KMessageBox::error(0, i18n("Database connection test was not successful. <p>Error was: %1</p>", testDatabase.lastError().text()), i18n("Database connection test") );
    }
    testDatabase.close();
    QSqlDatabase::removeDatabase(databaseID);
}

void SetupDatabase::slotChangeDatabasePath(const KUrl& result)
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

void SetupDatabase::slotDatabasePathEdited(const QString& newPath)
{
#ifndef _WIN32
    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        d->databasePathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }
#endif

    checkDBPath();
}

void SetupDatabase::checkDBPath()
{
    bool dbOk          = false;
    bool pathUnchanged = true;
    QString newPath    = d->databasePathEdit->url().toLocalFile();
    if (!d->databasePathEdit->url().path().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(d->originalDbPath);
        dbOk          = dbDir.exists();
        pathUnchanged = (dbDir == oldDir);
    }

    d->mainDialog->enableButtonOk(dbOk);
}

}  // namespace Digikam
