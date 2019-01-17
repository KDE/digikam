/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Database helpers.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "albummanager_p.h"

namespace Digikam
{

bool AlbumManager::setDatabase(const DbEngineParameters& params, bool priority, const QString& suggestedAlbumRoot)
{
    // This is to ensure that the setup does not overrule the command line.
    // TODO: there is a bug that setup is showing something different here.
    if (priority)
    {
        d->hasPriorizedDbPath = true;
    }
    else if (d->hasPriorizedDbPath)
    {
        // ignore change without priority
        return true;
    }

    // shutdown possibly running collection scans. Must call resumeCollectionScan further down.
    ScanController::instance()->cancelAllAndSuspendCollectionScan();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    d->changed = true;

    disconnect(CollectionManager::instance(), 0, this, 0);
    CollectionManager::instance()->setWatchDisabled();

    if (CoreDbAccess::databaseWatch())
    {
        disconnect(CoreDbAccess::databaseWatch(), 0, this, 0);
    }

    DatabaseServerStarter::instance()->stopServerManagerProcess();

    d->albumWatch->clear();

    cleanUp();

    d->currentAlbums.clear();
    emit signalAlbumCurrentChanged(d->currentAlbums);
    emit signalAlbumsCleared();

    d->albumPathHash.clear();
    d->allAlbumsIdHash.clear();
    d->albumRootAlbumHash.clear();

    // deletes all child albums as well
    delete d->rootPAlbum;
    delete d->rootTAlbum;
    delete d->rootDAlbum;
    delete d->rootSAlbum;

    d->rootPAlbum = 0;
    d->rootTAlbum = 0;
    d->rootDAlbum = 0;
    d->rootSAlbum = 0;

    // -- Database initialization -------------------------------------------------

    // ensure, embedded database is loaded
    qCDebug(DIGIKAM_GENERAL_LOG) << params;

    // workaround for the problem mariaDB >= 10.2 and QTBUG-63108
    if (params.isMySQL())
    {
        addFakeConnection();
    }

    if (params.internalServer)
    {
        DatabaseServerError result = DatabaseServerStarter::instance()->startServerManagerProcess(params);

        if (result.getErrorType() != DatabaseServerError::NoErrors)
        {
            QWidget* const parent = QWidget::find(0);
            QString message       = i18n("<p><b>An error occurred during the internal server start.</b></p>"
                                         "Details:\n %1", result.getErrorText());
            QApplication::changeOverrideCursor(Qt::ArrowCursor);
            QMessageBox::critical(parent, qApp->applicationName(), message);
            QApplication::changeOverrideCursor(Qt::WaitCursor);
        }
    }

    CoreDbAccess::setParameters(params, CoreDbAccess::MainApplication);

    DbEngineGuiErrorHandler* const handler = new DbEngineGuiErrorHandler(CoreDbAccess::parameters());
    CoreDbAccess::initDbEngineErrorHandler(handler);

    if (!handler->checkDatabaseConnection())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("<p>Failed to open the database. "
                                   "</p><p>You cannot use digiKam without a working database. "
                                   "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                   "Please check the database settings in the <b>configuration menu</b>.</p>"
                                  ));

        CoreDbAccess::setParameters(DbEngineParameters(), CoreDbAccess::DatabaseSlave);
        QApplication::restoreOverrideCursor();
        return true;
    }

    d->albumWatch->setDbEngineParameters(params);

    // still suspended from above
    ScanController::instance()->resumeCollectionScan();

    ScanController::Advice advice = ScanController::instance()->databaseInitialization();

    QApplication::restoreOverrideCursor();

    switch (advice)
    {
        case ScanController::Success:
            break;

        case ScanController::ContinueWithoutDatabase:
        {
            QString errorMsg = CoreDbAccess().lastError();

            if (errorMsg.isEmpty())
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                      i18n("<p>Failed to open the database. "
                                           "</p><p>You cannot use digiKam without a working database. "
                                           "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                           "Please check the database settings in the <b>configuration menu</b>.</p>"
                                          ));
            }
            else
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                      i18n("<p>Failed to open the database. Error message from database:</p>"
                                           "<p><b>%1</b></p>"
                                           "<p>You cannot use digiKam without a working database. "
                                           "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                           "Please check the database settings in the <b>configuration menu</b>.</p>",
                                           errorMsg));
            }

            return true;
        }

        case ScanController::AbortImmediately:
            return false;
    }

    // -- Locale Checking ---------------------------------------------------------

    QString currLocale = QString::fromUtf8((QTextCodec::codecForLocale()->name()));
    QString dbLocale   = CoreDbAccess().db()->getSetting(QLatin1String("Locale"));

    // guilty until proven innocent
    bool localeChanged = true;

    if (dbLocale.isNull())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No locale found in database";

        // Copy an existing locale from the settings file (used < 0.8)
        // to the database.
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(QLatin1String("General Settings"));

        if (group.hasKey(QLatin1String("Locale")))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Locale found in configfile";
            dbLocale = group.readEntry(QLatin1String("Locale"), QString());

            // this hack is necessary, as we used to store the entire
            // locale info LC_ALL (for eg: en_US.UTF-8) earlier,
            // we now save only the encoding (UTF-8)

            QString oldConfigLocale = QString::fromUtf8(::setlocale(0, 0));

            if (oldConfigLocale == dbLocale)
            {
                dbLocale = currLocale;
                localeChanged = false;
                CoreDbAccess().db()->setSetting(QLatin1String("Locale"), dbLocale);
            }
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "No locale found in config file";
            dbLocale = currLocale;

            localeChanged = false;
            CoreDbAccess().db()->setSetting(QLatin1String("Locale"), dbLocale);
        }
    }
    else
    {
        if (dbLocale == currLocale)
        {
            localeChanged = false;
        }
    }

    if (localeChanged)
    {
        // TODO it would be better to replace all yes/no confirmation dialogs with ones that has custom
        // buttons that denote the actions directly, i.e.:  ["Ignore and Continue"]  ["Adjust locale"]

        int result = QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                                 i18n("Your locale has changed since this "
                                      "album was last opened.\n"
                                      "Old locale: %1, new locale: %2\n"
                                      "If you have recently changed your locale, you need not be concerned.\n"
                                      "Please note that if you switched to a locale "
                                      "that does not support some of the filenames in your collection, "
                                      "these files may no longer be found in the collection. "
                                      "If you are sure that you want to "
                                      "continue, click 'Yes'. "
                                      "Otherwise, click 'No' and correct your "
                                      "locale setting before restarting digiKam.",
                                      dbLocale, currLocale),
                                  QMessageBox::Yes | QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            return false;
        }

        CoreDbAccess().db()->setSetting(QLatin1String("Locale"), currLocale);
    }

    // -- UUID Checking ---------------------------------------------------------

    QList<CollectionLocation> disappearedLocations = CollectionManager::instance()->checkHardWiredLocations();

    foreach (const CollectionLocation& loc, disappearedLocations)
    {
        QString locDescription;
        QStringList candidateIds, candidateDescriptions;
        CollectionManager::instance()->migrationCandidates(loc, &locDescription, &candidateIds, &candidateDescriptions);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Migration candidates for" << locDescription << ":" << candidateIds << candidateDescriptions;

        QDialog* const dialog         = new QDialog;
        QWidget* const widget         = new QWidget(dialog);
        QGridLayout* const mainLayout = new QGridLayout;
        mainLayout->setColumnStretch(1, 1);

        QLabel* const deviceIconLabel = new QLabel;
        deviceIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("drive-harddisk")).pixmap(64));
        mainLayout->addWidget(deviceIconLabel, 0, 0);

        QLabel* const mainLabel = new QLabel(i18n("<p>The collection </p><p><b>%1</b><br/>(%2)</p><p> is currently not found on your system.<br/> "
                                                  "Please choose the most appropriate option to handle this situation:</p>",
                                             loc.label(), QDir::toNativeSeparators(locDescription)));
        mainLabel->setWordWrap(true);
        mainLayout->addWidget(mainLabel, 0, 1);

        QGroupBox* const groupBox = new QGroupBox;
        mainLayout->addWidget(groupBox, 1, 0, 1, 2);

        QGridLayout* const layout = new QGridLayout;
        layout->setColumnStretch(1, 1);

        QRadioButton* migrateButton = 0;
        QComboBox* migrateChoices   = 0;

        if (!candidateIds.isEmpty())
        {
            migrateButton              = new QRadioButton;
            QLabel* const migrateLabel = new QLabel(i18n("<p>The collection is still available, but the identifier changed.<br/>"
                                                         "This can be caused by restoring a backup, changing the partition layout "
                                                         "or the file system settings.<br/>"
                                                         "The collection is now located at this place:</p>"));
            migrateLabel->setWordWrap(true);

            migrateChoices = new QComboBox;

            for (int i = 0 ; i < candidateIds.size() ; ++i)
            {
                migrateChoices->addItem(QDir::toNativeSeparators(candidateDescriptions.at(i)), candidateIds.at(i));
            }

            layout->addWidget(migrateButton,  0, 0, Qt::AlignTop);
            layout->addWidget(migrateLabel,   0, 1);
            layout->addWidget(migrateChoices, 1, 1);
        }

        QRadioButton* const isRemovableButton = new QRadioButton;
        QLabel* const isRemovableLabel        = new QLabel(i18n("The collection is located on a storage device which is not always attached. "
                                                                "Mark the collection as a removable collection."));
        isRemovableLabel->setWordWrap(true);
        layout->addWidget(isRemovableButton, 2, 0, Qt::AlignTop);
        layout->addWidget(isRemovableLabel,  2, 1);

        QRadioButton* const solveManuallyButton = new QRadioButton;
        QLabel* const solveManuallyLabel        = new QLabel(i18n("Take no action now. I would like to solve the problem "
                                                                  "later using the setup dialog"));
        solveManuallyLabel->setWordWrap(true);
        layout->addWidget(solveManuallyButton, 3, 0, Qt::AlignTop);
        layout->addWidget(solveManuallyLabel,  3, 1);

        groupBox->setLayout(layout);
        widget->setLayout(mainLayout);

        QVBoxLayout* const vbx          = new QVBoxLayout(dialog);
        QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok, dialog);
        vbx->addWidget(widget);
        vbx->addWidget(buttons);
        dialog->setLayout(vbx);
        dialog->setWindowTitle(i18n("Collection not found"));

        connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
                dialog, SLOT(accept()));

        // Default option: If there is only one candidate, default to migration.
        // Otherwise default to do nothing now.
        if (migrateButton && candidateIds.size() == 1)
        {
            migrateButton->setChecked(true);
        }
        else
        {
            solveManuallyButton->setChecked(true);
        }

        if (dialog->exec())
        {
            if (migrateButton && migrateButton->isChecked())
            {
                CollectionManager::instance()->migrateToVolume(loc, migrateChoices->itemData(migrateChoices->currentIndex()).toString());
            }
            else if (isRemovableButton->isChecked())
            {
                CollectionManager::instance()->changeType(loc, CollectionLocation::TypeVolumeRemovable);
            }
        }

        delete dialog;
    }

    // -- ---------------------------------------------------------

    // check that we have one album root
    if (CollectionManager::instance()->allLocations().isEmpty())
    {
        if (suggestedAlbumRoot.isEmpty())
        {
            Setup::execSinglePage(Setup::CollectionsPage);
        }
        else
        {
            QUrl albumRoot(QUrl::fromLocalFile(suggestedAlbumRoot));
            CollectionManager::instance()->addLocation(albumRoot, albumRoot.fileName());
            // Not needed? See bug #188959
            //ScanController::instance()->completeCollectionScan();
        }
    }

    // -- ---------------------------------------------------------

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ThumbnailLoadThread::initializeThumbnailDatabase(CoreDbAccess::parameters().thumbnailParameters(),
                                                     new ThumbsDbInfoProvider());

    DbEngineGuiErrorHandler* const thumbnailsDBHandler = new DbEngineGuiErrorHandler(ThumbsDbAccess::parameters());
    ThumbsDbAccess::initDbEngineErrorHandler(thumbnailsDBHandler);

    // Activate the similarity database.

    SimilarityDbAccess::setParameters(params.similarityParameters());

    DbEngineGuiErrorHandler* const similarityHandler = new DbEngineGuiErrorHandler(SimilarityDbAccess::parameters());
    SimilarityDbAccess::initDbEngineErrorHandler(similarityHandler);

    if (SimilarityDbAccess::checkReadyForUse(0))
    {
        qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database ready for use";
    }
    else
    {
        qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "Failed to initialize similarity database";
    }

    QApplication::restoreOverrideCursor();

    return true;
}

void AlbumManager::checkDatabaseDirsAfterFirstRun(const QString& dbPath, const QString& albumPath)
{
    // for bug #193522
    QDir               newDir(dbPath);
    QDir               albumDir(albumPath);
    DbEngineParameters newParams = DbEngineParameters::parametersForSQLiteDefaultFile(newDir.path());
    QFileInfo          digikam4DB(newParams.SQLiteDatabaseFile());

    if (!digikam4DB.exists())
    {
        QFileInfo digikam3DB(newDir, QLatin1String("digikam3.db"));
        QFileInfo digikamVeryOldDB(newDir, QLatin1String("digikam.db"));

        if (digikam3DB.exists() || digikamVeryOldDB.exists())
        {
            QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                     i18n("Database Folder"),
                     i18n("<p>You have chosen the folder \"%1\" as the place to store the database. "
                          "A database file from an older version of digiKam is found in this folder.</p> "
                          "<p>Would you like to upgrade the old database file - confirming "
                          "that this database file was indeed created for the pictures located in the folder \"%2\" - "
                          "or ignore the old file and start with a new database?</p> ",
                          QDir::toNativeSeparators(newDir.path()),
                          QDir::toNativeSeparators(albumDir.path())),
                      QMessageBox::Yes | QMessageBox::No,
                      qApp->activeWindow());

            msgBox->button(QMessageBox::Yes)->setText(i18n("Upgrade Database"));
            msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
            msgBox->button(QMessageBox::No)->setText(i18n("Create New Database"));
            msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
            msgBox->setDefaultButton(QMessageBox::Yes);

            int result = msgBox->exec();

            if (result == QMessageBox::Yes)
            {
                // CoreDbSchemaUpdater expects Album Path to point to the album root of the 0.9 db file.
                // Restore this situation.
                KSharedConfigPtr config = KSharedConfig::openConfig();
                KConfigGroup group      = config->group("Album Settings");
                group.writeEntry("Album Path", albumDir.path());
                group.sync();
            }
            else if (result == QMessageBox::No)
            {
                moveToBackup(digikam3DB);
                moveToBackup(digikamVeryOldDB);
            }

            delete msgBox;
        }
    }
}

void AlbumManager::changeDatabase(const DbEngineParameters& newParams)
{
    // if there is no file at the new place, copy old one
    DbEngineParameters params = CoreDbAccess::parameters();

    // New database type SQLITE
    if (newParams.isSQLite())
    {
        DatabaseServerStarter::instance()->stopServerManagerProcess();

        QDir newDir(newParams.getCoreDatabaseNameOrDir());
        QFileInfo newFile(newDir, QLatin1String("digikam4.db"));

        if (!newFile.exists())
        {
            QFileInfo digikam3DB(newDir, QLatin1String("digikam3.db"));
            QFileInfo digikamVeryOldDB(newDir, QLatin1String("digikam.db"));

            if (digikam3DB.exists() || digikamVeryOldDB.exists())
            {
                int result = -1;

                if (params.isSQLite())
                {
                    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                             i18n("New database folder"),
                             i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                  "A database file from an older version of digiKam is found in this folder.</p> "
                                  "<p>Would you like to upgrade the old database file, start with a new database, "
                                  "or copy the current database to this location and continue using it?</p> ",
                                  QDir::toNativeSeparators(newDir.path())),
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                             qApp->activeWindow());

                    msgBox->button(QMessageBox::Yes)->setText(i18n("Upgrade Database"));
                    msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
                    msgBox->button(QMessageBox::No)->setText(i18n("Create New Database"));
                    msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox->button(QMessageBox::Cancel)->setText(i18n("Copy Current Database"));
                    msgBox->button(QMessageBox::Cancel)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                    msgBox->setDefaultButton(QMessageBox::Yes);

                    result = msgBox->exec();
                    delete msgBox;
                }
                else
                {
                    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                             i18n("New database folder"),
                             i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                  "A database file from an older version of digiKam is found in this folder.</p> "
                                  "<p>Would you like to upgrade the old database file or start with a new database?</p>",
                                  QDir::toNativeSeparators(newDir.path())),
                             QMessageBox::Yes | QMessageBox::No,
                             qApp->activeWindow());

                    msgBox->button(QMessageBox::Yes)->setText(i18n("Upgrade Database"));
                    msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
                    msgBox->button(QMessageBox::No)->setText(i18n("Create New Database"));
                    msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox->setDefaultButton(QMessageBox::Yes);

                    result = msgBox->exec();
                    delete msgBox;
                }

                if (result == QMessageBox::Yes)
                {
                    // CoreDbSchemaUpdater expects Album Path to point to the album root of the 0.9 db file.
                    // Restore this situation.
                    KSharedConfigPtr config = KSharedConfig::openConfig();
                    KConfigGroup group      = config->group(QLatin1String("Album Settings"));
                    group.writeEntry(QLatin1String("Album Path"), newDir.path());
                    group.sync();
                }
                else if (result == QMessageBox::No)
                {
                    moveToBackup(digikam3DB);
                    moveToBackup(digikamVeryOldDB);
                }
                else if (result == QMessageBox::Cancel)
                {
                    QFileInfo oldFile(params.SQLiteDatabaseFile());
                    copyToNewLocation(oldFile, newFile, i18n("Failed to copy the old database file (\"%1\") "
                                                             "to its new location (\"%2\"). "
                                                             "Trying to upgrade old databases.",
                                                             QDir::toNativeSeparators(oldFile.filePath()),
                                                             QDir::toNativeSeparators(newFile.filePath())));
                }
            }
            else
            {
                int result = QMessageBox::Yes;

                if (params.isSQLite())
                {
                    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                             i18n("New database folder"),
                             i18n("<p>You have chosen the folder \"%1\" as the new place to store the database.</p>"
                                  "<p>Would you like to copy the current database to this location "
                                  "and continue using it, or start with a new database?</p> ",
                                  QDir::toNativeSeparators(newDir.path())),
                             QMessageBox::Yes | QMessageBox::No,
                             qApp->activeWindow());

                    msgBox->button(QMessageBox::Yes)->setText(i18n("Create New Database"));
                    msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox->button(QMessageBox::No)->setText(i18n("Copy Current Database"));
                    msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                    msgBox->setDefaultButton(QMessageBox::Yes);

                    result = msgBox->exec();
                    delete msgBox;
                }

                if (result == QMessageBox::No)
                {
                    QFileInfo oldFile(params.SQLiteDatabaseFile());
                    copyToNewLocation(oldFile, newFile);
                }
            }
        }
        else
        {
            int result = QMessageBox::No;

            if (params.isSQLite())
            {
                QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                         i18n("New database folder"),
                         i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                              "There is already a database file in this location.</p> "
                              "<p>Would you like to use this existing file as the new database, or remove it "
                              "and copy the current database to this place?</p> ",
                              QDir::toNativeSeparators(newDir.path())),
                         QMessageBox::Yes | QMessageBox::No,
                         qApp->activeWindow());

                msgBox->button(QMessageBox::Yes)->setText(i18n("Copy Current Database"));
                msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                msgBox->button(QMessageBox::No)->setText(i18n("Use Existing File"));
                msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
                msgBox->setDefaultButton(QMessageBox::Yes);

                result = msgBox->exec();
                delete msgBox;
            }

            if (result == QMessageBox::Yes)
            {
                // first backup
                if (moveToBackup(newFile))
                {
                    QFileInfo oldFile(params.SQLiteDatabaseFile());

                    // then copy
                    copyToNewLocation(oldFile, newFile);
                }
            }
        }
    }

    if (setDatabase(newParams, false))
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        startScan();
        QApplication::restoreOverrideCursor();
        ScanController::instance()->completeCollectionScan();
    }
}

bool AlbumManager::databaseEqual(const DbEngineParameters& parameters) const
{
    DbEngineParameters params = CoreDbAccess::parameters();

    return (params == parameters);
}

void AlbumManager::addFakeConnection()
{
    if (!d->dbFakeConnection)
    {
        // workaround for the problem mariaDB >= 10.2 and QTBUG-63108
        QSqlDatabase::addDatabase(QLatin1String("QMYSQL"), QLatin1String("FakeConnection"));
        d->dbFakeConnection = true;
    }
}

void AlbumManager::removeFakeConnection()
{
    if (d->dbFakeConnection)
    {
        QSqlDatabase::removeDatabase(QLatin1String("FakeConnection"));
    }
}

bool AlbumManager::moveToBackup(const QFileInfo& info)
{
    if (info.exists())
    {
        QFileInfo backup(info.dir(), info.fileName() + QLatin1String("-backup-") + QDateTime::currentDateTime().toString(Qt::ISODate));

        bool ret = QDir().rename(info.filePath(), backup.filePath());

        if (!ret)
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                  i18n("Failed to backup the existing database file (\"%1\"). "
                                       "Refusing to replace file without backup, using the existing file.",
                                       QDir::toNativeSeparators(info.filePath())));
            return false;
        }
    }

    return true;
}

bool AlbumManager::copyToNewLocation(const QFileInfo& oldFile,
                                     const QFileInfo& newFile,
                                     const QString otherMessage)
{
    QString message = otherMessage;

    if (message.isNull())
    {
        message = i18n("Failed to copy the old database file (\"%1\") "
                       "to its new location (\"%2\"). "
                       "Starting with an empty database.",
                       QDir::toNativeSeparators(oldFile.filePath()),
                       QDir::toNativeSeparators(newFile.filePath()));
    }

    bool ret = QFile::copy(oldFile.filePath(), newFile.filePath());

    if (!ret)
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), message);
        return false;
    }

    return true;
}

} // namespace Digikam
