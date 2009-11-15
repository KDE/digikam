/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
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

#include "databasewidget.h"

// KDE includes
#include <klocale.h>
#include <kglobalsettings.h>

// QT includes
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <KMessageBox>
#include <QSqlDatabase>
#include <qsqlerror.h>

namespace Digikam
{
    DatabaseWidget::DatabaseWidget(QWidget* parent): QWidget(parent)
    {
        setupMainArea();
    }

    DatabaseWidget::~DatabaseWidget()
    {

    }

    void DatabaseWidget::setupMainArea()
    {
        setAutoFillBackground(false);

        QVBoxLayout *layout = new QVBoxLayout();
        setLayout(layout);

        // --------------------------------------------------------

        QGroupBox *dbPathBox      = new QGroupBox(i18n("Database File Path"), this);
        QVBoxLayout *vlay         = new QVBoxLayout(dbPathBox);
        databasePathLabel = new QLabel(i18n("<p>The location where the database file will be stored on your system. "
                                                    "There is one common database file for all root albums.<br/>"
                                                    "Write access is required to be able to edit image properties.</p>"
                                                    "<p>Note: a remote file system, such as NFS, cannot be used here.</p><p></p>"),
                                               dbPathBox);
        databasePathLabel->setWordWrap(true);
        databasePathLabel->setFont(KGlobalSettings::smallestReadableFont());

        databasePathEdit = new KUrlRequester(dbPathBox);
        databasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);

        QLabel *databaseTypeLabel        = new QLabel(i18n("Type"));
        databaseType                  = new QComboBox();
        QLabel *databaseNameLabel        = new QLabel(i18n("Name"));
        databaseName                  = new QLineEdit();
        QLabel *hostNameLabel            = new QLabel(i18n("Host Name"));
        hostName                      = new QLineEdit();
        QLabel *hostPortLabel            = new QLabel(i18n("Port"));
        hostPort                      = new QSpinBox();
        hostPort->setMaximum(65536);

        QLabel *connectionOptionsLabel   = new QLabel(i18n("Database Connection Options"));
        connectionOptions             = new QLineEdit();

        QLabel *userNameLabel            = new QLabel(i18n("User"));
        userName                      = new QLineEdit();

        QLabel *passwordLabel            = new QLabel(i18n("Password"));
        password                      = new QLineEdit();
        password->setEchoMode(QLineEdit::Password);

        QPushButton *checkDatabaseConnectionButton = new QPushButton(i18n("Check DB Connection"));

        expertSettings = new QGroupBox();
        expertSettings->setFlat(true);
        QFormLayout *expertSettinglayout = new QFormLayout();
        expertSettings->setLayout(expertSettinglayout);

        vlay->addWidget(databaseTypeLabel);
        vlay->addWidget(databaseType);

        vlay->addWidget(databasePathLabel);
        vlay->addWidget(databasePathEdit);

        expertSettinglayout->addRow(hostNameLabel, hostName);
        expertSettinglayout->addRow(hostPortLabel, hostPort);
        expertSettinglayout->addRow(databaseNameLabel, databaseName);
        expertSettinglayout->addRow(userNameLabel, userName);
        expertSettinglayout->addRow(passwordLabel, password);
        expertSettinglayout->addRow(connectionOptionsLabel, connectionOptions);

        expertSettinglayout->addWidget(checkDatabaseConnectionButton);

        vlay->addWidget(expertSettings);

        vlay->setSpacing(0);
        vlay->setMargin(KDialog::spacingHint());

        // --------------------------------------------------------

        layout->setMargin(0);
        layout->setSpacing(KDialog::spacingHint());
        layout->addWidget(dbPathBox);
        layout->addStretch();

        // --------- fill with default values ---------------------
        databaseType->addItem("QSQLITE");
        databaseType->addItem("QMYSQL");

        setDatabaseInputFields("QSQLITE");

        // --------------------------------------------------------
        adjustSize();

        // --------------------------------------------------------
        connect(databasePathEdit, SIGNAL(urlSelected(const KUrl&)),
                this, SLOT(slotChangeDatabasePath(const KUrl&)));

        connect(databasePathEdit, SIGNAL(textChanged(const QString&)),
                this, SLOT(slotDatabasePathEdited(const QString&)));

        connect(databaseType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setDatabaseInputFields(const QString&)));

        connect(checkDatabaseConnectionButton, SIGNAL(clicked()), this, SLOT(checkDatabaseConnection()));

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
            databasePathEdit->setUrl(QDir::homePath() + '/' + newPath);
        }
    #endif

        checkDBPath();
    }
    void DatabaseWidget::setDatabaseInputFields(const QString& currentIndexStr){
        if (currentIndexStr=="QSQLITE"){
            databasePathLabel->setVisible(true);
            databasePathEdit->setVisible(true);

            expertSettings->setVisible(false);
        }else{
            databasePathLabel->setVisible(false);
            databasePathEdit->setVisible(false);

            expertSettings->setVisible(true);
        }
        adjustSize();
    }

    void DatabaseWidget::checkDatabaseConnection(){
        QString databaseID("ConnectionTest");
        QSqlDatabase testDatabase = QSqlDatabase::addDatabase(databaseType->currentText(), databaseID);
        testDatabase.setHostName(hostName->text());
        testDatabase.setPort(hostPort->text().toInt());
        testDatabase.setUserName(userName->text());
        testDatabase.setPassword(password->text());
        testDatabase.setConnectOptions(connectionOptions->text());

        bool result = testDatabase.open();
        if (result == true){
            KMessageBox::information(0, i18n("Database connection test successful."), i18n("Database connection test"));
        }else{
            KMessageBox::error(0, i18n("Database connection test was not successful. <p>Error was: %1</p>", testDatabase.lastError().text()), i18n("Database connection test") );
        }
        testDatabase.close();
        QSqlDatabase::removeDatabase(databaseID);
    }

    void DatabaseWidget::checkDBPath()
    {
        bool dbOk          = false;
        bool pathUnchanged = true;
        QString newPath    = databasePathEdit->url().toLocalFile();
        if (!databasePathEdit->url().path().isEmpty())
        {
            QDir dbDir(newPath);
            QDir oldDir(originalDbPath);
            dbOk          = dbDir.exists();
            pathUnchanged = (dbDir == oldDir);
        }
        //TODO create an Enable button slot, if the path is vald
//        d->mainDialog->enableButtonOk(dbOk);
    }
}
