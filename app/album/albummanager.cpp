/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albummanager.moc"

// C ANSI includes

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ includes

#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstdlib>

// Qt includes

#include <QApplication>
#include <QByteArray>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QList>
#include <QMultiHash>
#include <QRadioButton>
#include <QTextCodec>
#include <QTimer>

// KDE includes

#include <kcombobox.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdirwatch.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwindowsystem.h>

// Local includes

#include "albumdb.h"
#include "album.h"
#include "applicationsettings.h"
#include "albumwatch.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "config-digikam.h"
#include "databaseaccess.h"
#include "databaseguierrorhandler.h"
#include "databaseparameters.h"
#include "databaseserverstarter.h"
#include "databasethumbnailinfoprovider.h"
#include "databaseurl.h"
#include "databasewatch.h"
#include "dio.h"
#include "facetags.h"
#include "imagelister.h"
#include "scancontroller.h"
#include "setupcollections.h"
#include "setup.h"
#include "tagscache.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnailloadthread.h"
#include "upgradedb_sqlite2tosqlite3.h"
#include "dnotificationwrapper.h"

namespace Digikam
{

class PAlbumPath
{
public:

    PAlbumPath()
        : albumRootId(-1)
    {
    }

    PAlbumPath(int albumRootId, const QString& albumPath)
        : albumRootId(albumRootId), albumPath(albumPath)
    {
    }

    PAlbumPath(PAlbum* album)
    {
        if (album->isRoot())
        {
            albumRootId = -1;
        }
        else
        {
            albumRootId = album->albumRootId();
            albumPath   = album->albumPath();
        }
    }

    bool operator==(const PAlbumPath& other) const
    {
        return other.albumRootId == albumRootId && other.albumPath == albumPath;
    }

public:

    int     albumRootId;
    QString albumPath;
};

// -----------------------------------------------------------------------------------

uint qHash(const PAlbumPath& id)
{
    return ::qHash(id.albumRootId) ^ ::qHash(id.albumPath);
}

// -----------------------------------------------------------------------------------

class AlbumManager::AlbumManagerPriv
{
public:

    AlbumManagerPriv() :
        changed(false),
        hasPriorizedDbPath(false),
        dbPort(0),
        dbInternalServer(false),
        showOnlyAvailableAlbums(false),
        albumListJob(0),
        dateListJob(0),
        tagListJob(0),
        personListJob(0),
        albumWatch(0),
        rootPAlbum(0),
        rootTAlbum(0),
        rootDAlbum(0),
        rootSAlbum(0),
        currentlyMovingAlbum(0),
        changingDB(false),
        scanPAlbumsTimer(0),
        scanTAlbumsTimer(0),
        scanSAlbumsTimer(0),
        scanDAlbumsTimer(0),
        updatePAlbumsTimer(0),
        albumItemCountTimer(0),
        tagItemCountTimer(0)
    {
    }

    bool                        changed;
    bool                        hasPriorizedDbPath;

    QString                     dbType;
    QString                     dbName;
    QString                     dbHostName;
    int                         dbPort;
    bool                        dbInternalServer;

    bool                        showOnlyAvailableAlbums;

    KIO::TransferJob*           albumListJob;
    KIO::TransferJob*           dateListJob;
    KIO::TransferJob*           tagListJob;
    KIO::TransferJob*           personListJob;

    AlbumWatch*                 albumWatch;

    PAlbum*                     rootPAlbum;
    TAlbum*                     rootTAlbum;
    DAlbum*                     rootDAlbum;
    SAlbum*                     rootSAlbum;

    QHash<int, Album*>          allAlbumsIdHash;
    QHash<PAlbumPath, PAlbum*>  albumPathHash;
    QHash<int, PAlbum*>         albumRootAlbumHash;
    Album*                      currentlyMovingAlbum;

    QMultiHash<Album*, Album**> guardedPointers;

    /** For multiple selection support **/
    QList<Album*>               currentAlbums;

    bool                        changingDB;
    QTimer*                     scanPAlbumsTimer;
    QTimer*                     scanTAlbumsTimer;
    QTimer*                     scanSAlbumsTimer;
    QTimer*                     scanDAlbumsTimer;
    QTimer*                     updatePAlbumsTimer;
    QTimer*                     albumItemCountTimer;
    QTimer*                     tagItemCountTimer;
    QSet<int>                   changedPAlbums;

    QMap<int, int>              pAlbumsCount;
    QMap<int, int>              tAlbumsCount;
    QMap<YearMonth, int>        dAlbumsCount;
    QMap<int, int>              fAlbumsCount;

public:

    QString labelForAlbumRootAlbum(const CollectionLocation& location)
    {
        QString label = location.label();

        if (label.isEmpty())
        {
            label = location.albumRootPath();
        }

        return label;
    }
};

// -----------------------------------------------------------------------------------

class ChangingDB
{
public:

    explicit ChangingDB(AlbumManager::AlbumManagerPriv* d)
        : d(d)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

    AlbumManager::AlbumManagerPriv* const d;
};

// -----------------------------------------------------------------------------------

class AlbumManagerCreator
{
public:

    AlbumManager object;
};

K_GLOBAL_STATIC(AlbumManagerCreator, creator)

// -----------------------------------------------------------------------------------

// A friend-class shortcut to circumvent accessing this from within the destructor
AlbumManager* AlbumManager::internalInstance = 0;

AlbumManager* AlbumManager::instance()
{
    return &creator->object;
}

AlbumManager::AlbumManager()
    : d(new AlbumManagerPriv)
{
    internalInstance = this;
    d->albumWatch = new AlbumWatch(this);

    // these operations are pretty fast, no need for long queuing
    d->scanPAlbumsTimer = new QTimer(this);
    d->scanPAlbumsTimer->setInterval(50);
    d->scanPAlbumsTimer->setSingleShot(true);

    connect(d->scanPAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanPAlbums()));

    d->scanTAlbumsTimer = new QTimer(this);
    d->scanTAlbumsTimer->setInterval(50);
    d->scanTAlbumsTimer->setSingleShot(true);

    connect(d->scanTAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanTAlbums()));

    d->scanSAlbumsTimer = new QTimer(this);
    d->scanSAlbumsTimer->setInterval(50);
    d->scanSAlbumsTimer->setSingleShot(true);

    connect(d->scanSAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanSAlbums()));

    d->updatePAlbumsTimer = new QTimer(this);
    d->updatePAlbumsTimer->setInterval(50);
    d->updatePAlbumsTimer->setSingleShot(true);

    connect(d->updatePAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(updateChangedPAlbums()));

    // this operation is much more expensive than the other scan methods
    d->scanDAlbumsTimer = new QTimer(this);
    d->scanDAlbumsTimer->setInterval(60 * 1000);
    d->scanDAlbumsTimer->setSingleShot(true);

    connect(d->scanDAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanDAlbumsScheduled()));

    // moderately expensive
    d->albumItemCountTimer = new QTimer(this);
    d->albumItemCountTimer->setInterval(1000);
    d->albumItemCountTimer->setSingleShot(true);

    connect(d->albumItemCountTimer, SIGNAL(timeout()),
            this, SLOT(getAlbumItemsCount()));

    // more expensive
    d->tagItemCountTimer = new QTimer(this);
    d->tagItemCountTimer->setInterval(2500);
    d->tagItemCountTimer->setSingleShot(true);

    connect(d->tagItemCountTimer, SIGNAL(timeout()),
            this, SLOT(getTagItemsCount()));
}

AlbumManager::~AlbumManager()
{
    delete d->rootPAlbum;
    delete d->rootTAlbum;
    delete d->rootDAlbum;
    delete d->rootSAlbum;

    internalInstance = 0;
    delete d;
}

void AlbumManager::cleanUp()
{
    // This is what we prefer to do before KApplication destruction

    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }

    if (d->albumListJob)
    {
        d->albumListJob->kill();
        d->albumListJob = 0;
    }

    if (d->tagListJob)
    {
        d->tagListJob->kill();
        d->tagListJob = 0;
    }

    if (d->personListJob)
    {
        d->personListJob->kill();
        d->personListJob = 0;
    }
}

bool AlbumManager::databaseEqual(const QString& dbType, const QString& dbName,
                                 const QString& dbHostName, int dbPort, bool dbInternalServer) const
{
    DatabaseParameters params = DatabaseAccess::parameters();

    return params.databaseType   == dbType          &&
           params.databaseName   == dbName          &&
           params.hostName       == dbHostName      &&
           params.port           == dbPort          &&
           params.internalServer == dbInternalServer;
}

static bool moveToBackup(const QFileInfo& info)
{
    if (info.exists())
    {
        QFileInfo backup(info.dir(), info.fileName() + "-backup-" + QDateTime::currentDateTime().toString(Qt::ISODate));
        KIO::Job* job = KIO::file_move(info.filePath(), backup.filePath(), -1, KIO::Overwrite | KIO::HideProgressInfo);

        if (!KIO::NetAccess::synchronousRun(job, 0))
        {
            KMessageBox::error(0, i18n("Failed to backup the existing database file (\"%1\"). "
                                       "Refusing to replace file without backup, using the existing file.",
                                       QDir::toNativeSeparators(info.filePath())));
            return false;
        }
    }

    return true;
}

static bool copyToNewLocation(const QFileInfo& oldFile, const QFileInfo& newFile,
                              const QString otherMessage = QString())
{
    QString message = otherMessage;

    if (message.isNull())
        message = i18n("Failed to copy the old database file (\"%1\") "
                       "to its new location (\"%2\"). "
                       "Starting with an empty database.",
                       QDir::toNativeSeparators(oldFile.filePath()), QDir::toNativeSeparators(newFile.filePath()));

    KIO::Job* job = KIO::file_copy(oldFile.filePath(), newFile.filePath(), -1,
                                   KIO::Overwrite /*| KIO::HideProgressInfo*/);

    if (!KIO::NetAccess::synchronousRun(job, 0))
    {
        KMessageBox::error(0, message);
        return false;
    }

    return true;
}

void AlbumManager::checkDatabaseDirsAfterFirstRun(const QString& dbPath, const QString& albumPath)
{
    // for bug #193522
    QDir               newDir(dbPath);
    QDir               albumDir(albumPath);
    DatabaseParameters newParams = DatabaseParameters::parametersForSQLiteDefaultFile(newDir.path());
    QFileInfo          digikam4DB(newParams.SQLiteDatabaseFile());

    if (!digikam4DB.exists())
    {
        QFileInfo digikam3DB(newDir, "digikam3.db");
        QFileInfo digikamVeryOldDB(newDir, "digikam.db");

        if (digikam3DB.exists() || digikamVeryOldDB.exists())
        {
            KGuiItem startFresh(i18n("Create New Database"), "document-new");
            KGuiItem upgrade(i18n("Upgrade Database"), "view-refresh");
            int result = KMessageBox::warningYesNo(0,
                                                   i18n("<p>You have chosen the folder \"%1\" as the place to store the database. "
                                                        "A database file from an older version of digiKam is found in this folder.</p> "
                                                        "<p>Would you like to upgrade the old database file - confirming "
                                                        "that this database file was indeed created for the pictures located in the folder \"%2\" - "
                                                        "or ignore the old file and start with a new database?</p> ",
                                                        QDir::toNativeSeparators(newDir.path()), QDir::toNativeSeparators(albumDir.path())),
                                                   i18n("Database Folder"),
                                                   upgrade, startFresh);

            if (result == KMessageBox::Yes)
            {
                // SchemaUpdater expects Album Path to point to the album root of the 0.9 db file.
                // Restore this situation.
                KSharedConfigPtr config = KGlobal::config();
                KConfigGroup group      = config->group("Album Settings");
                group.writeEntry("Album Path", albumDir.path());
                group.sync();
            }
            else if (result == KMessageBox::No)
            {
                moveToBackup(digikam3DB);
                moveToBackup(digikamVeryOldDB);
            }
        }
    }
}

void AlbumManager::changeDatabase(const DatabaseParameters& newParams)
{
    // if there is no file at the new place, copy old one
    DatabaseParameters params = DatabaseAccess::parameters();

    // New database type SQLITE
    if (newParams.isSQLite())
    {
        QDir newDir(newParams.getDatabaseNameOrDir());
        QFileInfo newFile(newDir, QString("digikam4.db"));

        if (!newFile.exists())
        {
            QFileInfo digikam3DB(newDir, "digikam3.db");
            QFileInfo digikamVeryOldDB(newDir, "digikam.db");

            if (digikam3DB.exists() || digikamVeryOldDB.exists())
            {
                KGuiItem copyCurrent(i18n("Copy Current Database"), "edit-copy");
                KGuiItem startFresh(i18n("Create New Database"), "document-new");
                KGuiItem upgrade(i18n("Upgrade Database"), "view-refresh");
                int result = -1;

                if (params.isSQLite())
                {
                    result = KMessageBox::warningYesNoCancel(0,
                                                             i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                                                  "A database file from an older version of digiKam is found in this folder.</p> "
                                                                  "<p>Would you like to upgrade the old database file, start with a new database, "
                                                                  "or copy the current database to this location and continue using it?</p> ",
                                                                  QDir::toNativeSeparators(newDir.path())),
                                                             i18n("New database folder"),
                                                             upgrade, startFresh, copyCurrent);
                }
                else
                {
                    result = KMessageBox::warningYesNo(0,
                                                       i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                                            "A database file from an older version of digiKam is found in this folder.</p> "
                                                            "<p>Would you like to upgrade the old database file or start with a new database?</p>",
                                                            QDir::toNativeSeparators(newDir.path())),
                                                       i18n("New database folder"),
                                                       upgrade, startFresh);
                }

                if (result == KMessageBox::Yes)
                {
                    // SchemaUpdater expects Album Path to point to the album root of the 0.9 db file.
                    // Restore this situation.
                    KSharedConfigPtr config = KGlobal::config();
                    KConfigGroup group = config->group("Album Settings");
                    group.writeEntry("Album Path", newDir.path());
                    group.sync();
                }
                else if (result == KMessageBox::No)
                {
                    moveToBackup(digikam3DB);
                    moveToBackup(digikamVeryOldDB);
                }
                else if (result == KMessageBox::Cancel)
                {
                    QDir oldDir(d->dbName);
                    QFileInfo oldFile(params.SQLiteDatabaseFile());
                    copyToNewLocation(oldFile, newFile, i18n("Failed to copy the old database file (\"%1\") "
                                                             "to its new location (\"%2\"). "
                                                             "Trying to upgrade old databases.",
                                                             QDir::toNativeSeparators(oldFile.filePath()), QDir::toNativeSeparators(newFile.filePath())));
                }
            }
            else
            {
                int result = KMessageBox::Yes;

                if (params.isSQLite())
                {
                    KGuiItem copyCurrent(i18n("Copy Current Database"), "edit-copy");
                    KGuiItem startFresh(i18n("Create New Database"), "document-new");
                    result = KMessageBox::warningYesNo(0,
                                                       i18n("<p>You have chosen the folder \"%1\" as the new place to store the database.</p>"
                                                            "<p>Would you like to copy the current database to this location "
                                                            "and continue using it, or start with a new database?</p> ",
                                                            QDir::toNativeSeparators(newDir.path())),
                                                       i18n("New database folder"),
                                                       startFresh, copyCurrent);
                }

                if (result == KMessageBox::No)
                {
                    QDir oldDir(d->dbName);
                    QFileInfo oldFile(params.SQLiteDatabaseFile());
                    copyToNewLocation(oldFile, newFile);
                }
            }
        }
        else
        {
            int result = KMessageBox::No;

            if (params.isSQLite())
            {
                KGuiItem replaceItem(i18n("Copy Current Database"), "edit-copy");
                KGuiItem useExistingItem(i18n("Use Existing File"), "document-open");
                result = KMessageBox::warningYesNo(0,
                                                   i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                                        "There is already a database file in this location.</p> "
                                                        "<p>Would you like to use this existing file as the new database, or remove it "
                                                        "and copy the current database to this place?</p> ",
                                                        QDir::toNativeSeparators(newDir.path())),
                                                   i18n("New database folder"),
                                                   replaceItem, useExistingItem);
            }

            if (result == KMessageBox::Yes)
            {
                // first backup
                if (moveToBackup(newFile))
                {
                    QDir oldDir(d->dbName);
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

bool AlbumManager::setDatabase(const DatabaseParameters& params, bool priority, const QString suggestedAlbumRoot)
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

    if (DatabaseAccess::databaseWatch())
    {
        disconnect(DatabaseAccess::databaseWatch(), 0, this, 0);
    }

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
    kDebug() << params;

    if (params.internalServer)
    {
        DatabaseServerError result = DatabaseServerStarter::startServerManagerProcess();

        if (result.getErrorType() != DatabaseServerError::NoErrors)
        {
            QWidget* parent = QWidget::find(0);
            QString message = i18n("<p><b>An error occurred during the internal server start.</b></p>"
                                   "Details:\n %1", result.getErrorText());
            QApplication::changeOverrideCursor(Qt::ArrowCursor);
            KMessageBox::error(parent, message);
            QApplication::changeOverrideCursor(Qt::WaitCursor);
        }
    }

    DatabaseAccess::setParameters(params, DatabaseAccess::MainApplication);

    DatabaseGUIErrorHandler* handler = new DatabaseGUIErrorHandler(DatabaseAccess::parameters());
    DatabaseAccess::initDatabaseErrorHandler(handler);

    if (!handler->checkDatabaseConnection())
    {
        KMessageBox::error(0, i18n("<p>Failed to open the database. "
                                   "</p><p>You cannot use digiKam without a working database. "
                                   "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                   "Please check the database settings in the <b>configuration menu</b>.</p>"
                                  ));

        DatabaseAccess::setParameters(DatabaseParameters(), DatabaseAccess::DatabaseSlave);
        QApplication::restoreOverrideCursor();
        return true;
    }

    d->albumWatch->setDatabaseParameters(params);

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
            QString errorMsg = DatabaseAccess().lastError();

            if (errorMsg.isEmpty())
            {
                KMessageBox::error(0, i18n("<p>Failed to open the database. "
                                           "</p><p>You cannot use digiKam without a working database. "
                                           "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                           "Please check the database settings in the <b>configuration menu</b>.</p>"
                                          ));
            }
            else
            {
                KMessageBox::error(0, i18n("<p>Failed to open the database. Error message from database:</p>"
                                           "<p><b>%1</b></p>"
                                           "</p><p>You cannot use digiKam without a working database. "
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

    QString currLocale(QTextCodec::codecForLocale()->name());
    QString dbLocale = DatabaseAccess().db()->getSetting("Locale");

    // guilty until proven innocent
    bool localeChanged = true;

    if (dbLocale.isNull())
    {
        kDebug() << "No locale found in database";

        // Copy an existing locale from the settings file (used < 0.8)
        // to the database.
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group = config->group("General Settings");

        if (group.hasKey("Locale"))
        {
            kDebug() << "Locale found in configfile";
            dbLocale = group.readEntry("Locale", QString());

            // this hack is necessary, as we used to store the entire
            // locale info LC_ALL (for eg: en_US.UTF-8) earlier,
            // we now save only the encoding (UTF-8)

            QString oldConfigLocale = ::setlocale(0, 0);

            if (oldConfigLocale == dbLocale)
            {
                dbLocale = currLocale;
                localeChanged = false;
                DatabaseAccess().db()->setSetting("Locale", dbLocale);
            }
        }
        else
        {
            kDebug() << "No locale found in config file";
            dbLocale = currLocale;

            localeChanged = false;
            DatabaseAccess().db()->setSetting("Locale", dbLocale);
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
        int result =
            KMessageBox::warningYesNo(0,
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
                                           dbLocale, currLocale));

        if (result != KMessageBox::Yes)
        {
            exit(0);
        }

        DatabaseAccess().db()->setSetting("Locale", currLocale);
    }

    // -- UUID Checking ---------------------------------------------------------

    QList<CollectionLocation> disappearedLocations = CollectionManager::instance()->checkHardWiredLocations();
    foreach(const CollectionLocation& loc, disappearedLocations)
    {
        QString locDescription;
        QStringList candidateIds, candidateDescriptions;
        CollectionManager::instance()->migrationCandidates(loc, &locDescription, &candidateIds, &candidateDescriptions);
        kDebug() << "Migration candidates for" << locDescription << ":" << candidateIds << candidateDescriptions;

        KDialog* dialog         = new KDialog;
        QWidget* widget         = new QWidget;
        QGridLayout* mainLayout = new QGridLayout;
        mainLayout->setColumnStretch(1, 1);

        QLabel* deviceIconLabel = new QLabel;
        deviceIconLabel->setPixmap(KIconLoader::global()->loadIcon("drive-harddisk", KIconLoader::NoGroup, KIconLoader::SizeHuge));
        mainLayout->addWidget(deviceIconLabel, 0, 0);

        QLabel* mainLabel = new QLabel(
            i18n("<p>The collection </p><p><b>%1</b><br/>(%2)</p><p> is currently not found on your system.<br/> "
                 "Please choose the most appropriate option to handle this situation:</p>",
                 loc.label(), QDir::toNativeSeparators(locDescription)));
        mainLabel->setWordWrap(true);
        mainLayout->addWidget(mainLabel, 0, 1);

        QGroupBox* groupBox = new QGroupBox;
        mainLayout->addWidget(groupBox, 1, 0, 1, 2);

        QGridLayout* layout = new QGridLayout;
        layout->setColumnStretch(1, 1);

        QRadioButton* migrateButton = 0;
        KComboBox* migrateChoices   = 0;

        if (!candidateIds.isEmpty())
        {
            migrateButton = new QRadioButton;
            QLabel* migrateLabel = new QLabel(
                i18n("<p>The collection is still available, but the identifier changed.<br/>"
                     "This can be caused by restoring a backup, changing the partition layout "
                     "or the file system settings.<br/>"
                     "The collection is now located at this place:</p>"));
            migrateLabel->setWordWrap(true);

            migrateChoices = new KComboBox;

            for (int i = 0; i < candidateIds.size(); ++i)
            {
                migrateChoices->addItem(QDir::toNativeSeparators(candidateDescriptions.at(i)), candidateIds.at(i));
            }

            layout->addWidget(migrateButton,  0, 0, Qt::AlignTop);
            layout->addWidget(migrateLabel,   0, 1);
            layout->addWidget(migrateChoices, 1, 1);
        }

        QRadioButton* isRemovableButton = new QRadioButton;
        QLabel* isRemovableLabel        = new QLabel(
            i18n("The collection is located on a storage device which is not always attached. "
                 "Mark the collection as a removable collection."));
        isRemovableLabel->setWordWrap(true);
        layout->addWidget(isRemovableButton, 2, 0, Qt::AlignTop);
        layout->addWidget(isRemovableLabel,  2, 1);

        QRadioButton* solveManuallyButton = new QRadioButton;
        QLabel* solveManuallyLabel        = new QLabel(
            i18n("Take no action now. I would like to solve the problem "
                 "later using the setup dialog"));
        solveManuallyLabel->setWordWrap(true);
        layout->addWidget(solveManuallyButton, 3, 0, Qt::AlignTop);
        layout->addWidget(solveManuallyLabel,  3, 1);

        groupBox->setLayout(layout);

        widget->setLayout(mainLayout);
        dialog->setCaption(i18n("Collection not found"));
        dialog->setMainWidget(widget);
        dialog->setButtons(KDialog::Ok);

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
            CollectionManager::instance()->addLocation(suggestedAlbumRoot);
            // Not needed? See bug #188959
            //ScanController::instance()->completeCollectionScan();
        }
    }

    // -- ---------------------------------------------------------

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ThumbnailLoadThread::initializeThumbnailDatabase(DatabaseAccess::parameters().thumbnailParameters(),
                                                     new DatabaseThumbnailInfoProvider());

    DatabaseGUIErrorHandler* const thumbnailsDBHandler = new DatabaseGUIErrorHandler(ThumbnailDatabaseAccess::parameters());
    ThumbnailDatabaseAccess::initDatabaseErrorHandler(thumbnailsDBHandler);

    QApplication::restoreOverrideCursor();

    // -- ---------------------------------------------------------

    // NOTE: Delete all Nepomuk code
//#ifdef HAVE_NEPOMUK

//    if (checkNepomukService())
//    {
//        QDBusInterface serviceInterface("org.kde.nepomuk.services.digikamnepomukservice",
//                                        "/digikamnepomukservice", "org.kde.digikam.DigikamNepomukService");
//        kDebug() << "nepomuk service available" << serviceInterface.isValid();

//        if (serviceInterface.isValid())
//        {
//            DatabaseParameters parameters = DatabaseAccess::parameters();
//            KUrl url;
//            parameters.insertInUrl(url);
//            serviceInterface.call(QDBus::NoBlock, "setDatabase", url.url());
//        }
//    }

//#endif // HAVE_NEPOMUK

    return true;
}

bool AlbumManager::checkNepomukService()
{
    bool hasNepomuk = false;
// NOTE: Delete all nepomuk code
//#ifdef HAVE_NEPOMUK
//    QDBusInterface serviceInterface("org.kde.nepomuk.services.digikamnepomukservice",
//                                    "/digikamnepomukservice", "org.kde.digikam.DigikamNepomukService");

//    // already running? (normal)
//    if (serviceInterface.isValid())
//    {
//        return true;
//    }

//    // start service
//    QDBusInterface nepomukInterface("org.kde.NepomukServer",
//                                    "/servicemanager", "org.kde.nepomuk.ServiceManager");

//    if (!nepomukInterface.isValid())
//    {
//        kDebug() << "Nepomuk server is not reachable. Cannot start Digikam Nepomuk Service";
//        return false;
//    }

//    QDBusReply<QStringList> availableServicesReply = nepomukInterface.call("availableServices");

//    if (!availableServicesReply.isValid() || !availableServicesReply.value().contains("digikamnepomukservice"))
//    {
//        kDebug() << "digikamnepomukservice is not available in NepomukServer";
//        return false;
//    }

//    /*
//        QEventLoop loop;

//        if (!connect(&nepomukInterface, SIGNAL(serviceInitialized(QString)),
//                     &loop, SLOT(quit())))
//        {
//            kDebug() << "Could not connect to Nepomuk server signal";
//            return false;
//        }

//        QTimer::singleShot(1000, &loop, SLOT(quit()));
//    */

//    kDebug() << "Trying to start up digikamnepomukservice";
//    nepomukInterface.call(QDBus::NoBlock, "startService", "digikamnepomukservice");

//    /*
//        // wait (at most 1sec) for service to start up
//        loop.exec();
//    */

//    hasNepomuk = true;
//#endif // HAVE_NEPOMUK

    return hasNepomuk;
}

void AlbumManager::startScan()
{
    if (!d->changed)
    {
        return;
    }

    d->changed = false;

    // create root albums
    d->rootPAlbum = new PAlbum(i18n("Albums"));
    insertPAlbum(d->rootPAlbum, 0);

    d->rootTAlbum = new TAlbum(i18n("Tags"), 0, true);
    insertTAlbum(d->rootTAlbum, 0);

    d->rootSAlbum = new SAlbum(i18n("Searches"), 0, true);
    emit signalAlbumAboutToBeAdded(d->rootSAlbum, 0, 0);
    d->allAlbumsIdHash[d->rootSAlbum->globalID()] = d->rootSAlbum;
    emit signalAlbumAdded(d->rootSAlbum);

    d->rootDAlbum = new DAlbum(QDate(), true);
    emit signalAlbumAboutToBeAdded(d->rootDAlbum, 0, 0);
    d->allAlbumsIdHash[d->rootDAlbum->globalID()] = d->rootDAlbum;
    emit signalAlbumAdded(d->rootDAlbum);

    // Create albums for album roots. Reuse logic implemented in the method
    foreach(const CollectionLocation& location, CollectionManager::instance()->allLocations())
    {
        handleCollectionStatusChange(location, CollectionLocation::LocationNull);
    }

    // listen to location status changes
    connect(CollectionManager::instance(), SIGNAL(locationStatusChanged(CollectionLocation,int)),
            this, SLOT(slotCollectionLocationStatusChanged(CollectionLocation,int)));

    connect(CollectionManager::instance(), SIGNAL(locationPropertiesChanged(CollectionLocation)),
            this, SLOT(slotCollectionLocationPropertiesChanged(CollectionLocation)));

    // reload albums
    refresh();

    // listen to album database changes
    connect(DatabaseAccess::databaseWatch(), SIGNAL(albumChange(AlbumChangeset)),
            this, SLOT(slotAlbumChange(AlbumChangeset)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(TagChangeset)),
            this, SLOT(slotTagChange(TagChangeset)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(searchChange(SearchChangeset)),
            this, SLOT(slotSearchChange(SearchChangeset)));

    // listen to collection image changes
    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));

    emit signalAllAlbumsLoaded();
}

void AlbumManager::slotCollectionLocationStatusChanged(const CollectionLocation& location, int oldStatus)
{
    // not before initialization
    if (!d->rootPAlbum)
    {
        return;
    }

    if (handleCollectionStatusChange(location, oldStatus))
    {
        // a change occurred. Possibly albums have appeared or disappeared
        scanPAlbums();
    }
}

/// Returns true if it added or removed an album
bool AlbumManager::handleCollectionStatusChange(const CollectionLocation& location, int oldStatus)
{
    enum Action
    {
        Add,
        Remove,
        DoNothing
    };
    Action action = DoNothing;

    switch (oldStatus)
    {
        case CollectionLocation::LocationNull:
        case CollectionLocation::LocationHidden:
        case CollectionLocation::LocationUnavailable:
        {
            switch (location.status())
            {
                case CollectionLocation::LocationNull: // not possible
                    break;
                case CollectionLocation::LocationHidden:
                    action = Remove;
                    break;
                case CollectionLocation::LocationAvailable:
                    action = Add;
                    break;
                case CollectionLocation::LocationUnavailable:
                    if (d->showOnlyAvailableAlbums)
                    {
                        action = Remove;
                    }
                    else
                    {
                        action = Add;
                    }
                    break;
                case CollectionLocation::LocationDeleted:
                    action = Remove;
                    break;
            }
            break;
        }
        case CollectionLocation::LocationAvailable:
        {
            switch (location.status())
            {
                case CollectionLocation::LocationNull:
                case CollectionLocation::LocationHidden:
                case CollectionLocation::LocationDeleted:
                    action = Remove;
                    break;
                case CollectionLocation::LocationUnavailable:
                    if (d->showOnlyAvailableAlbums)
                    {
                        action = Remove;
                    }
                    break;
                case CollectionLocation::LocationAvailable: // not possible
                    break;
            }
            break;
        }
        case CollectionLocation::LocationDeleted: // not possible
            break;
    }

    if (action == Add && !d->albumRootAlbumHash.value(location.id()))
    {
        // This is the only place where album root albums are added
        addAlbumRoot(location);
        return true;
    }
    else if (action == Remove && d->albumRootAlbumHash.value(location.id()))
    {
        removeAlbumRoot(location);
        return true;
    }
    return false;
}

void AlbumManager::slotCollectionLocationPropertiesChanged(const CollectionLocation& location)
{
    PAlbum* album = d->albumRootAlbumHash.value(location.id());

    if (album)
    {
        QString newLabel = d->labelForAlbumRootAlbum(location);

        if (album->title() != newLabel)
        {
            album->setTitle(newLabel);
            emit signalAlbumRenamed(album);
        }
    }
}

void AlbumManager::addAlbumRoot(const CollectionLocation& location)
{
    PAlbum* album = d->albumRootAlbumHash.value(location.id());

    if (!album)
    {
        // Create a PAlbum for the Album Root.
        QString label = d->labelForAlbumRootAlbum(location);
        album = new PAlbum(location.id(), label);

        // insert album root created into hash
        d->albumRootAlbumHash.insert(location.id(), album);
    }
}

void AlbumManager::removeAlbumRoot(const CollectionLocation& location)
{
    // retrieve and remove from hash
    PAlbum* album = d->albumRootAlbumHash.take(location.id());

    if (album)
    {
        // delete album and all its children
        removePAlbum(album);
    }
}

bool AlbumManager::isShowingOnlyAvailableAlbums() const
{
    return d->showOnlyAvailableAlbums;
}

void AlbumManager::setShowOnlyAvailableAlbums(bool onlyAvailable)
{
    if (d->showOnlyAvailableAlbums == onlyAvailable)
    {
        return;
    }
    d->showOnlyAvailableAlbums = onlyAvailable;
    emit signalShowOnlyAvailableAlbumsChanged(d->showOnlyAvailableAlbums);
    // We need to update the unavailable locations.
    // We assume the handleCollectionStatusChange does the right thing (even though old status == current status)
    foreach (const CollectionLocation& location, CollectionManager::instance()->allLocations())
    {
        if (location.status() == CollectionLocation::LocationUnavailable)
        {
            handleCollectionStatusChange(location, CollectionLocation::LocationUnavailable);
        }
    }
}

void AlbumManager::refresh()
{
    scanPAlbums();
    scanTAlbums();
    scanSAlbums();
    scanDAlbums();
}

void AlbumManager::prepareItemCounts()
{
    // There is no way to find out if any data we had collected
    // previously is still valid - recompute
    scanDAlbums();
    getAlbumItemsCount();
    getTagItemsCount();
}

void AlbumManager::scanPAlbums()
{
    d->scanPAlbumsTimer->stop();

    // first insert all the current normal PAlbums into a map for quick lookup
    QHash<int, PAlbum*> oldAlbums;
    AlbumIterator it(d->rootPAlbum);

    while (it.current())
    {
        PAlbum* a          = (PAlbum*)(*it);
        oldAlbums[a->id()] = a;
        ++it;
    }

    // scan db and get a list of all albums
    QList<AlbumInfo> currentAlbums = DatabaseAccess().db()->scanAlbums();

    // sort by relative path so that parents are created before children
    qSort(currentAlbums);

    QList<AlbumInfo> newAlbums;

    // go through all the Albums and see which ones are already present
    foreach(const AlbumInfo& info, currentAlbums)
    {
        // check that location of album is available
        if (d->showOnlyAvailableAlbums && !CollectionManager::instance()->locationForAlbumRootId(info.albumRootId).isAvailable())
        {
            continue;
        }
        if (oldAlbums.contains(info.id))
        {
            oldAlbums.remove(info.id);
        }
        else
        {
            newAlbums << info;
        }
    }

    // now oldAlbums contains all the deleted albums and
    // newAlbums contains all the new albums

    // delete old albums, informing all frontends

    // The albums have to be removed with children being removed first,
    // removePAlbum takes care of that.
    // So we only feed it the albums from oldAlbums topmost in hierarchy.
    QSet<PAlbum*> topMostOldAlbums;
    foreach(PAlbum* album, oldAlbums)
    {
        if (!album->parent() || !oldAlbums.contains(album->parent()->id()))
        {
            topMostOldAlbums << album;
        }
    }

    foreach(PAlbum* album, topMostOldAlbums)
    {
        // recursively removes all children and the album
        removePAlbum(album);
    }

    // sort by relative path so that parents are created before children
    qSort(newAlbums);

    // create all new albums
    foreach(const AlbumInfo& info, newAlbums)
    {
        if (info.relativePath.isEmpty())
        {
            continue;
        }

        PAlbum* album = 0, *parent = 0;

        if (info.relativePath == "/")
        {
            // Albums that represent the root directory of an album root
            // We have them as here new albums first time after their creation

            parent = d->rootPAlbum;
            album  = d->albumRootAlbumHash.value(info.albumRootId);

            if (!album)
            {
                kError() << "Did not find album root album in hash";
                continue;
            }

            // it has been created from the collection location
            // with album root id, parentPath "/" and a name, but no album id yet.
            album->m_id = info.id;
        }
        else
        {
            // last section, no slash
            QString name = info.relativePath.section('/', -1, -1);
            // all but last sections, leading slash, no trailing slash
            QString parentPath = info.relativePath.section('/', 0, -2);

            if (parentPath.isEmpty())
            {
                parent = d->albumRootAlbumHash.value(info.albumRootId);
            }
            else
            {
                parent = d->albumPathHash.value(PAlbumPath(info.albumRootId, parentPath));
            }

            if (!parent)
            {
                kError() <<  "Could not find parent with url: "
                         << QDir::toNativeSeparators(parentPath) << " for: " << QDir::toNativeSeparators(info.relativePath);
                continue;
            }

            // Create the new album
            album = new PAlbum(info.albumRootId, parentPath, name, info.id);
        }

        album->m_caption  = info.caption;
        album->m_category = info.category;
        album->m_date     = info.date;
        album->m_iconId   = info.iconId;

        insertPAlbum(album, parent);
    }

    if (!topMostOldAlbums.isEmpty() || !newAlbums.isEmpty())
    {
        emit signalAlbumsUpdated(Album::PHYSICAL);
    }

    getAlbumItemsCount();
}

void AlbumManager::updateChangedPAlbums()
{
    d->updatePAlbumsTimer->stop();

    // scan db and get a list of all albums
    QList<AlbumInfo> currentAlbums = DatabaseAccess().db()->scanAlbums();
    bool needScanPAlbums           = false;

    // Find the AlbumInfo for each id in changedPAlbums
    foreach(int id, d->changedPAlbums)
    {
        foreach(const AlbumInfo& info, currentAlbums)
        {
            if (info.id == id)
            {
                d->changedPAlbums.remove(info.id);

                PAlbum* album = findPAlbum(info.id);

                if (album)
                {
                    // Renamed?
                    if (info.relativePath != "/")
                    {
                        // Handle rename of album name
                        // last section, no slash
                        QString name       = info.relativePath.section('/', -1, -1);
                        QString parentPath = info.relativePath;
                        parentPath.chop(name.length());

                        if (parentPath != album->m_parentPath || info.albumRootId != album->albumRootId())
                        {
                            // Handle actual move operations: trigger ScanPAlbums
                            needScanPAlbums = true;
                            removePAlbum(album);
                            break;
                        }
                        else if (name != album->title())
                        {
                            album->setTitle(name);
                            updateAlbumPathHash();
                            emit signalAlbumRenamed(album);
                        }
                    }

                    // Update caption, collection, date
                    album->m_caption  = info.caption;
                    album->m_category = info.category;
                    album->m_date     = info.date;

                    // Icon changed?
                    if (album->m_iconId != info.iconId)
                    {
                        album->m_iconId = info.iconId;
                        emit signalAlbumIconChanged(album);
                    }
                }
            }
        }
    }

    if (needScanPAlbums)
    {
        scanPAlbums();
    }
}

void AlbumManager::getAlbumItemsCount()
{
    d->albumItemCountTimer->stop();

    if (!ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        return;
    }

    // List albums using kioslave

    if (d->albumListJob)
    {
        d->albumListJob->kill();
        d->albumListJob = 0;
    }

    DatabaseUrl u   = DatabaseUrl::albumUrl();
    d->albumListJob = ImageLister::startListJob(u);
    d->albumListJob->addMetaData("folders", "true");

    connect(d->albumListJob, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumsJobResult(KJob*)));

    connect(d->albumListJob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotAlbumsJobData(KIO::Job*,QByteArray)));
}

void AlbumManager::scanTAlbums()
{
    d->scanTAlbumsTimer->stop();

    // list TAlbums directly from the db
    // first insert all the current TAlbums into a map for quick lookup
    typedef QMap<int, TAlbum*> TagMap;
    TagMap tmap;

    tmap.insert(0, d->rootTAlbum);

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* t = (TAlbum*)(*it);
        tmap.insert(t->id(), t);
        ++it;
    }

    // Retrieve the list of tags from the database
    TagInfo::List tList = DatabaseAccess().db()->scanTags();

    // sort the list. needed because we want the tags can be read in any order,
    // but we want to make sure that we are ensure to find the parent TAlbum
    // for a new TAlbum

    {
        QHash<int, TAlbum*> tagHash;

        // insert items into a dict for quick lookup
        for (TagInfo::List::const_iterator iter = tList.constBegin(); iter != tList.constEnd(); ++iter)
        {
            TagInfo info  = *iter;
            TAlbum* album = new TAlbum(info.name, info.id);

            album->m_icon   = info.icon;
            album->m_iconId = info.iconId;
            album->m_pid    = info.pid;
            tagHash.insert(info.id, album);
        }

        tList.clear();

        // also add root tag
        TAlbum* rootTag = new TAlbum("root", 0, true);
        tagHash.insert(0, rootTag);

        // build tree
        for (QHash<int, TAlbum*>::const_iterator iter = tagHash.constBegin();
             iter != tagHash.constEnd(); ++iter)
        {
            TAlbum* album = *iter;

            if (album->m_id == 0)
            {
                continue;
            }

            TAlbum* parent = tagHash.value(album->m_pid);

            if (parent)
            {
                album->setParent(parent);
            }
            else
            {
                kWarning() << "Failed to find parent tag for tag "
                           << album->m_title
                           << " with pid "
                           << album->m_pid;
            }
        }

        // now insert the items into the list. becomes sorted
        AlbumIterator it(rootTag);

        while (it.current())
        {
            TagInfo info;
            TAlbum* album = static_cast<TAlbum*>(it.current());

            if (album)
            {
                info.id     = album->m_id;
                info.pid    = album->m_pid;
                info.name   = album->m_title;
                info.icon   = album->m_icon;
                info.iconId = album->m_iconId;
            }

            tList.append(info);
            ++it;
        }

        // this will also delete all child albums
        delete rootTag;
    }

    for (TagInfo::List::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        TagInfo info = *it;

        // check if we have already added this tag
        if (tmap.contains(info.id))
        {
            continue;
        }

        // Its a new album. Find the parent of the album
        TagMap::const_iterator iter = tmap.constFind(info.pid);

        if (iter == tmap.constEnd())
        {
            kWarning() << "Failed to find parent tag for tag "
                       << info.name
                       << " with pid "
                       << info.pid;
            continue;
        }

        TAlbum* parent = iter.value();

        // Create the new TAlbum
        TAlbum* album   = new TAlbum(info.name, info.id, false);
        album->m_icon   = info.icon;
        album->m_iconId = info.iconId;
        insertTAlbum(album, parent);

        // also insert it in the map we are doing lookup of parent tags
        tmap.insert(info.id, album);
    }

    if (!tList.isEmpty())
    {
        emit signalAlbumsUpdated(Album::TAG);
    }

    getTagItemsCount();
}

void AlbumManager::getTagItemsCount()
{
    d->tagItemCountTimer->stop();

    if (!ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        return;
    }

    // List tags using kioslave

    if (d->tagListJob)
    {
        d->tagListJob->kill();
        d->tagListJob = 0;
    }

    DatabaseUrl u = DatabaseUrl::fromTagIds(QList<int>());
    d->tagListJob = ImageLister::startListJob(u);
    d->tagListJob->addMetaData("folders", "true");

    connect(d->tagListJob, SIGNAL(result(KJob*)),
            this, SLOT(slotTagsJobResult(KJob*)));

    connect(d->tagListJob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotTagsJobData(KIO::Job*,QByteArray)));

    if (d->personListJob)
    {
        d->personListJob->kill();
        d->personListJob = 0;
    }

    d->personListJob = ImageLister::startListJob(u);
    d->personListJob->addMetaData("facefolders", "true");

    connect(d->personListJob, SIGNAL(result(KJob*)),
            this, SLOT(slotPeopleJobResult(KJob*)));

    connect(d->personListJob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotPeopleJobData(KIO::Job*,QByteArray)));
}

void AlbumManager::scanSAlbums()
{
    d->scanSAlbumsTimer->stop();

    // list SAlbums directly from the db
    // first insert all the current SAlbums into a map for quick lookup
    QMap<int, SAlbum*> oldSearches;

    AlbumIterator it(d->rootSAlbum);

    while (it.current())
    {
        SAlbum* search            = (SAlbum*)(*it);
        oldSearches[search->id()] = search;
        ++it;
    }

    // scan db and get a list of all albums
    QList<SearchInfo> currentSearches = DatabaseAccess().db()->scanSearches();

    QList<SearchInfo> newSearches;

    // go through all the Albums and see which ones are already present
    foreach(const SearchInfo& info, currentSearches)
    {
        if (oldSearches.contains(info.id))
        {
            SAlbum* album = oldSearches[info.id];

            if (info.name != album->title()      ||
                info.type != album->searchType() ||
                info.query != album->query())
            {
                QString oldName = album->title();

                album->setSearch(info.type, info.query);
                album->setTitle(info.name);

                if (oldName != album->title())
                {
                    emit signalAlbumRenamed(album);
                }

                emit signalSearchUpdated(album);
            }

            oldSearches.remove(info.id);
        }
        else
        {
            newSearches << info;
        }
    }

    // remove old albums that have been deleted
    foreach(SAlbum* album, oldSearches)
    {
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        delete album;
        emit signalAlbumHasBeenDeleted(album);
    }

    // add new albums
    foreach(const SearchInfo& info, newSearches)
    {
        SAlbum* album = new SAlbum(info.name, info.id);
        album->setSearch(info.type, info.query);
        emit signalAlbumAboutToBeAdded(album, d->rootSAlbum, d->rootSAlbum->lastChild());
        album->setParent(d->rootSAlbum);
        d->allAlbumsIdHash[album->globalID()] = album;
        emit signalAlbumAdded(album);
    }
}

void AlbumManager::scanDAlbumsScheduled()
{
    // Avoid a cycle of killing a job which takes longer than the timer interval
    if (d->dateListJob)
    {
        d->scanDAlbumsTimer->start();
        return;
    }
    scanDAlbums();
}

void AlbumManager::scanDAlbums()
{
    d->scanDAlbumsTimer->stop();

    // List dates using kioslave:
    // The kioslave has a special mode listing the dates
    // for which there are images in the DB.

    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }

    DatabaseUrl u  = DatabaseUrl::dateUrl();
    d->dateListJob = ImageLister::startListJob(u);
    d->dateListJob->addMetaData("folders", "true");

    connect(d->dateListJob, SIGNAL(result(KJob*)),
            this, SLOT(slotDatesJobResult(KJob*)));

    connect(d->dateListJob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotDatesJobData(KIO::Job*,QByteArray)));
}

AlbumList AlbumManager::allPAlbums() const
{
    AlbumList list;

    if (d->rootPAlbum)
    {
        list.append(d->rootPAlbum);
    }

    AlbumIterator it(d->rootPAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

AlbumList AlbumManager::allTAlbums() const
{
    AlbumList list;

    if (d->rootTAlbum)
    {
        list.append(d->rootTAlbum);
    }

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

AlbumList AlbumManager::allSAlbums() const
{
    AlbumList list;

    if (d->rootSAlbum)
    {
        list.append(d->rootSAlbum);
    }

    AlbumIterator it(d->rootSAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

AlbumList AlbumManager::allDAlbums() const
{
    AlbumList list;

    if (d->rootDAlbum)
    {
        list.append(d->rootDAlbum);
    }

    AlbumIterator it(d->rootDAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

void AlbumManager::setCurrentAlbums(QList<Album*> albums)
{
    if(albums.isEmpty())
        return;

    QList<Album*> filtered;
    /**
     * Filter out the null pointers
    */
    Q_FOREACH(Album *album, albums)
    {
        if(album != 0) {
            filtered.append(album);
        }
    }

    albums = filtered;

    /**
     * Sort is needed to identify selection correctly, ex AlbumHistory
     */
    qSort(albums.begin(),albums.end());
    d->currentAlbums.clear();
    d->currentAlbums+=albums;

    emit signalAlbumCurrentChanged(d->currentAlbums);
}

AlbumList AlbumManager::currentAlbums() const
{
    return d->currentAlbums;
}

PAlbum* AlbumManager::currentPAlbum() const
{
    /**
     * Temporary fix, to return multiple items,
     * iterate and cast each element
     */
    if(!d->currentAlbums.isEmpty())
        return dynamic_cast<PAlbum*>(d->currentAlbums.first());
    else
        return 0;
}

QList<TAlbum*> AlbumManager::currentTAlbums() const
{
    /**
     * This method is not yet used
     */
    QList<TAlbum*> talbums;
    QList<Album*>::iterator it;
    for(it = d->currentAlbums.begin(); it != d->currentAlbums.end(); ++it)
    {
        TAlbum* temp = dynamic_cast<TAlbum*>(*it);
        if(temp)
            talbums.push_back(temp);
    }
    return talbums;
}

PAlbum* AlbumManager::findPAlbum(const KUrl& url) const
{
    CollectionLocation location = CollectionManager::instance()->locationForUrl(url);

    if (location.isNull())
    {
        return 0;
    }

    return d->albumPathHash.value(PAlbumPath(location.id(), CollectionManager::instance()->album(location, url)));
}

PAlbum* AlbumManager::findPAlbum(int id) const
{
    if (!d->rootPAlbum)
    {
        return 0;
    }

    int gid = d->rootPAlbum->globalID() + id;

    return static_cast<PAlbum*>((d->allAlbumsIdHash.value(gid)));
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    if (!d->rootTAlbum)
    {
        return 0;
    }

    int gid = d->rootTAlbum->globalID() + id;

    return static_cast<TAlbum*>((d->allAlbumsIdHash.value(gid)));
}

SAlbum* AlbumManager::findSAlbum(int id) const
{
    if (!d->rootSAlbum)
    {
        return 0;
    }

    int gid = d->rootSAlbum->globalID() + id;

    return static_cast<SAlbum*>((d->allAlbumsIdHash.value(gid)));
}

DAlbum* AlbumManager::findDAlbum(int id) const
{
    if (!d->rootDAlbum)
    {
        return 0;
    }

    int gid = d->rootDAlbum->globalID() + id;

    return static_cast<DAlbum*>((d->allAlbumsIdHash.value(gid)));
}

Album* AlbumManager::findAlbum(int gid) const
{
    return d->allAlbumsIdHash.value(gid);
}

TAlbum* AlbumManager::findTAlbum(const QString& tagPath) const
{
    // handle gracefully with or without leading slash
    bool          withLeadingSlash = tagPath.startsWith('/');
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* talbum = static_cast<TAlbum*>(*it);

        if (talbum->tagPath(withLeadingSlash) == tagPath)
        {
            return talbum;
        }

        ++it;
    }

    return 0;

}

SAlbum* AlbumManager::findSAlbum(const QString& name) const
{
    for (Album* album = d->rootSAlbum->firstChild(); album; album = album->next())
    {
        if (album->title() == name)
        {
            return dynamic_cast<SAlbum*>(album);
        }
    }

    return 0;
}

void AlbumManager::addGuardedPointer(Album* album, Album** pointer)
{
    if (album)
    {
        d->guardedPointers.insert(album, pointer);
    }
}

void AlbumManager::removeGuardedPointer(Album* album, Album** pointer)
{
    if (album)
    {
        d->guardedPointers.remove(album, pointer);
    }
}

void AlbumManager::changeGuardedPointer(Album* oldAlbum, Album* album, Album** pointer)
{
    if (oldAlbum)
    {
        d->guardedPointers.remove(oldAlbum, pointer);
    }

    if (album)
    {
        d->guardedPointers.insert(album, pointer);
    }
}

void AlbumManager::invalidateGuardedPointers(Album* album)
{
    if (!album)
    {
        return;
    }

    QMultiHash<Album*, Album**>::iterator it = d->guardedPointers.find(album);

    for (; it != d->guardedPointers.end() && it.key() == album; ++it)
    {
        if (it.value())
        {
            *(it.value()) = 0;
        }
    }
}

PAlbum* AlbumManager::createPAlbum(const QString& albumRootPath, const QString& name,
                                   const QString& caption, const QDate& date,
                                   const QString& category,
                                   QString& errMsg)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRootPath);
    return createPAlbum(location, name, caption, date, category, errMsg);
}

PAlbum* AlbumManager::createPAlbum(const CollectionLocation& location, const QString& name,
                                   const QString& caption, const QDate& date,
                                   const QString& category,
                                   QString& errMsg)
{
    if (location.isNull() || !location.isAvailable())
    {
        errMsg = i18n("The collection location supplied is invalid or currently not available.");
        return 0;
    }

    PAlbum* album = d->albumRootAlbumHash.value(location.id());

    if (!album)
    {
        errMsg = i18n("No album for collection location: Internal error");
        return 0;
    }

    return createPAlbum(album, name, caption, date, category, errMsg);
}

PAlbum* AlbumManager::createPAlbum(PAlbum*        parent,
                                   const QString& name,
                                   const QString& caption,
                                   const QDate&   date,
                                   const QString& category,
                                   QString&       errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for album.");
        return 0;
    }

    // sanity checks
    if (name.isEmpty())
    {
        errMsg = i18n("Album name cannot be empty.");
        return 0;
    }

    if (name.contains("/"))
    {
        errMsg = i18n("Album name cannot contain '/'.");
        return 0;
    }

    if (parent->isRoot())
    {
        errMsg = i18n("createPAlbum does not accept the root album as parent.");
        return 0;
    }

    QString albumPath = parent->isAlbumRoot() ? QString(QLatin1Char('/') + name) : QString(parent->albumPath() + QLatin1Char('/') + name);
    int albumRootId   = parent->albumRootId();

    // first check if we have a sibling album with the same name
    PAlbum* child     = static_cast<PAlbum*>(parent->m_firstChild);

    while (child)
    {
        if (child->albumRootId() == albumRootId && child->albumPath() == albumPath)
        {
            errMsg = i18n("An existing album has the same name.");
            return 0;
        }

        child = static_cast<PAlbum*>(child->m_next);
    }

    DatabaseUrl url = parent->databaseUrl();
    url.addPath(name);
    KUrl fileUrl    = url.fileUrl();

    if (!KIO::NetAccess::mkdir(fileUrl, qApp->activeWindow()))
    {
        errMsg = i18n("Failed to create directory.");
        return 0;
    }

    ChangingDB changing(d);
    int        id = DatabaseAccess().db()->addAlbum(albumRootId, albumPath, caption, date, category);

    if (id == -1)
    {
        errMsg = i18n("Failed to add album to database");
        return 0;
    }

    QString parentPath;

    if (!parent->isAlbumRoot())
    {
        parentPath = parent->albumPath();
    }

    PAlbum* album     = new PAlbum(albumRootId, parentPath, name, id);
    album->m_caption  = caption;
    album->m_category = category;
    album->m_date     = date;

    insertPAlbum(album, parent);
    emit signalAlbumsUpdated(Album::PHYSICAL);

    return album;
}

bool AlbumManager::renamePAlbum(PAlbum* album, const QString& newName,
                                QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootPAlbum)
    {
        errMsg = i18n("Cannot rename root album");
        return false;
    }

    if (album->isAlbumRoot())
    {
        errMsg = i18n("Cannot rename album root album");
        return false;
    }

    if (newName.contains("/"))
    {
        errMsg = i18n("Album name cannot contain '/'");
        return false;
    }

    // first check if we have another sibling with the same name
    if (hasDirectChildAlbumWithTitle(album->m_parent, newName))
    {
        errMsg = i18n("Another album with the same name already exists.\n"
                      "Please choose another name.");
        return false;
    }

    QString oldAlbumPath = album->albumPath();
    KUrl oldUrl          = album->fileUrl();
    album->setTitle(newName);
    album->m_path        = newName;
    KUrl newUrl          = album->fileUrl();
    QString newAlbumPath = album->albumPath();

    // We use a private shortcut around collection scanner noticing our changes,
    // we rename them directly. Faster.
    ScanController::instance()->suspendCollectionScan();

    KIO::Job* job = KIO::rename(oldUrl, newUrl, KIO::HideProgressInfo);

    if (!KIO::NetAccess::synchronousRun(job, 0))
    {
        errMsg = i18n("Failed to rename Album");
        return false;
    }

    // now rename the album and subalbums in the database
    {
        DatabaseAccess access;
        ChangingDB changing(d);
        access.db()->renameAlbum(album->id(), album->albumRootId(), album->albumPath());

        PAlbum* subAlbum = 0;
        AlbumIterator it(album);

        while ((subAlbum = static_cast<PAlbum*>(it.current())) != 0)
        {
            subAlbum->m_parentPath = newAlbumPath + subAlbum->m_parentPath.mid(oldAlbumPath.length());
            access.db()->renameAlbum(subAlbum->id(), album->albumRootId(), subAlbum->albumPath());
            ++it;
        }
    }

    updateAlbumPathHash();
    emit signalAlbumRenamed(album);

    ScanController::instance()->resumeCollectionScan();

    return true;
}

void AlbumManager::updateAlbumPathHash()
{
    // Update AlbumDict. basically clear it and rebuild from scratch
    d->albumPathHash.clear();
    AlbumIterator it(d->rootPAlbum);
    PAlbum* subAlbum = 0;

    while ((subAlbum = static_cast<PAlbum*>(it.current())) != 0)
    {
        d->albumPathHash[subAlbum] = subAlbum;
        ++it;
    }

}

bool AlbumManager::updatePAlbumIcon(PAlbum* album, qlonglong iconID, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootPAlbum)
    {
        errMsg = i18n("Cannot edit root album");
        return false;
    }

    {
        DatabaseAccess access;
        ChangingDB changing(d);
        access.db()->setAlbumIcon(album->id(), iconID);
        album->m_iconId = iconID;
    }

    emit signalAlbumIconChanged(album);

    return true;
}

TAlbum* AlbumManager::createTAlbum(TAlbum* parent, const QString& name,
                                   const QString& iconkde, QString& errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for tag");
        return 0;
    }

    // sanity checks
    if (name.isEmpty())
    {
        errMsg = i18n("Tag name cannot be empty");
        return 0;
    }

    if (name.contains("/"))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return 0;
    }

    // first check if we have another album with the same name
    if (hasDirectChildAlbumWithTitle(parent, name))
    {
        errMsg = i18n("Tag name already exists");
        return 0;
    }

    ChangingDB changing(d);
    int id = DatabaseAccess().db()->addTag(parent->id(), name, iconkde, 0);

    if (id == -1)
    {
        errMsg = i18n("Failed to add tag to database");
        return 0;
    }

    TAlbum* album = new TAlbum(name, id, false);
    album->m_icon = iconkde;

    insertTAlbum(album, parent);

    TAlbum* personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    emit signalAlbumsUpdated(Album::TAG);

    return album;
}

AlbumList AlbumManager::findOrCreateTAlbums(const QStringList& tagPaths)
{
    // find tag ids for tag paths in list, create if they don't exist
    QList<int> tagIDs = TagsCache::instance()->getOrCreateTags(tagPaths);

    // create TAlbum objects for the newly created tags
    scanTAlbums();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        resultList.append(findTAlbum(*it));
    }

    return resultList;
}

bool AlbumManager::deleteTAlbum(TAlbum* album, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot delete Root Tag");
        return false;
    }

    {
        DatabaseAccess access;
        ChangingDB changing(d);
        access.db()->deleteTag(album->id());

        Album* subAlbum = 0;
        AlbumIterator it(album);

        while ((subAlbum = it.current()) != 0)
        {
            access.db()->deleteTag(subAlbum->id());
            ++it;
        }
    }

    removeTAlbum(album);
    emit signalAlbumsUpdated(Album::TAG);

    return true;
}

bool AlbumManager::hasDirectChildAlbumWithTitle(Album* parent, const QString& title)
{

    Album* sibling = parent->m_firstChild;

    while (sibling)
    {
        if (sibling->title() == title)
        {
            return true;
        }

        sibling = sibling->m_next;
    }

    return false;

}

bool AlbumManager::renameTAlbum(TAlbum* album, const QString& name,
                                QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    if (name.contains("/"))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return false;
    }

    // first check if we have another sibling with the same name
    if (hasDirectChildAlbumWithTitle(album->m_parent, name))
    {
        errMsg = i18n("Another tag with the same name already exists.\n"
                      "Please choose another name.");
        return false;
    }

    ChangingDB changing(d);
    DatabaseAccess().db()->setTagName(album->id(), name);
    album->setTitle(name);
    emit signalAlbumRenamed(album);

    return true;
}

bool AlbumManager::moveTAlbum(TAlbum* album, TAlbum* newParent, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (!newParent)
    {
        errMsg = i18n("Attempt to move TAlbum to nowhere");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot move root tag");
        return false;
    }

    if (hasDirectChildAlbumWithTitle(newParent, album->title()))
    {
        errMsg = i18n("Another tag with the same name already exists.\n"
                      "Please rename the tag before moving it.");
        return false;
    }

    d->currentlyMovingAlbum = album;
    emit signalAlbumAboutToBeMoved(album);

    emit signalAlbumAboutToBeDeleted(album);

    if (album->parent())
    {
        album->parent()->removeChild(album);
    }

    album->setParent(0);
    emit signalAlbumDeleted(album);
    emit signalAlbumHasBeenDeleted(album);

    emit signalAlbumAboutToBeAdded(album, newParent, newParent->lastChild());
    ChangingDB changing(d);
    DatabaseAccess().db()->setTagParentID(album->id(), newParent->id());
    album->setParent(newParent);
    emit signalAlbumAdded(album);

    emit signalAlbumMoved(album);
    emit signalAlbumsUpdated(Album::TAG);
    d->currentlyMovingAlbum = 0;

    TAlbum* personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    return true;
}

bool AlbumManager::updateTAlbumIcon(TAlbum* album, const QString& iconKDE,
                                    qlonglong iconID, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such tag");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    {
        DatabaseAccess access;
        ChangingDB changing(d);
        access.db()->setTagIcon(album->id(), iconKDE, iconID);
        QString albumRelativePath, iconKDE;
        album->m_icon   = iconKDE;
        album->m_iconId = iconID;
    }

    emit signalAlbumIconChanged(album);

    return true;
}

AlbumList AlbumManager::getRecentlyAssignedTags(bool includeInternal) const
{
    QList<int> tagIDs = DatabaseAccess().db()->getRecentlyAssignedTags();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        TAlbum* album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            resultList.append(album);
        }
    }

    return resultList;
}

QStringList AlbumManager::tagPaths(const QList<int>& tagIDs, bool leadingSlash, bool includeInternal) const
{
    QStringList tagPaths;

    for (QList<int>::const_iterator it = tagIDs.constBegin(); it != tagIDs.constEnd(); ++it)
    {
        TAlbum* album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagPaths.append(album->tagPath(leadingSlash));
        }
    }

    return tagPaths;
}

QStringList AlbumManager::tagNames(const QList<int>& tagIDs, bool includeInternal) const
{
    QStringList tagNames;

    foreach(int id, tagIDs)
    {
        TAlbum* album = findTAlbum(id);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagNames << album->title();
        }
    }

    return tagNames;
}

QHash<int, QString> AlbumManager::tagPaths(bool leadingSlash, bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->tagPath(leadingSlash));
        }

        ++it;
    }

    return hash;
}

QHash<int, QString> AlbumManager::tagNames(bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->title());
        }

        ++it;
    }

    return hash;
}

QList< int > AlbumManager::subTags(int tagId, bool recursive)
{
    TAlbum* album = this->findTAlbum(tagId);
    return album->childAlbumIds(recursive);
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property)
{
    AlbumList list;

    QList<int> ids = TagsCache::instance()->tagsWithProperty(property);
    foreach(int id, ids)
    {
        TAlbum* album = findTAlbum(id);

        if (album)
        {
            list << album;
        }
    }

    return list;
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property, const QString& value)
{
    AlbumList list;

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        if (static_cast<TAlbum*>(*it)->property(property) == value)
        {
            list << *it;
        }

        ++it;
    }

    return list;
}

QHash<int, QString> AlbumManager::albumTitles() const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootPAlbum);

    while (it.current())
    {
        PAlbum* a = (PAlbum*)(*it);
        hash.insert(a->id(), a->title());
        ++it;
    }

    return hash;
}

SAlbum* AlbumManager::createSAlbum(const QString& name, DatabaseSearch::Type type, const QString& query)
{
    // first iterate through all the search albums and see if there's an existing
    // SAlbum with same name. (Remember, SAlbums are arranged in a flat list)
    SAlbum* album = findSAlbum(name);
    ChangingDB changing(d);

    if (album)
    {
        updateSAlbum(album, query, name, type);
        return album;
    }

    int id = DatabaseAccess().db()->addSearch(type, name, query);

    if (id == -1)
    {
        return 0;
    }

    album = new SAlbum(name, id);
    emit signalAlbumAboutToBeAdded(album, d->rootSAlbum, d->rootSAlbum->lastChild());
    album->setSearch(type, query);
    album->setParent(d->rootSAlbum);

    d->allAlbumsIdHash.insert(album->globalID(), album);
    emit signalAlbumAdded(album);

    return album;
}

bool AlbumManager::updateSAlbum(SAlbum* album, const QString& changedQuery,
                                const QString& changedName, DatabaseSearch::Type type)
{
    if (!album)
    {
        return false;
    }

    QString newName              = changedName.isNull()                    ? album->title()      : changedName;
    DatabaseSearch::Type newType = (type == DatabaseSearch::UndefinedType) ? album->searchType() : type;

    ChangingDB changing(d);
    DatabaseAccess().db()->updateSearch(album->id(), newType, newName, changedQuery);

    QString oldName = album->title();

    album->setSearch(newType, changedQuery);
    album->setTitle(newName);

    if (oldName != album->title())
    {
        emit signalAlbumRenamed(album);
    }

    if(!d->currentAlbums.isEmpty())
        if (d->currentAlbums.first() == album)
        {
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }

    return true;
}

bool AlbumManager::deleteSAlbum(SAlbum* album)
{
    if (!album)
    {
        return false;
    }

    emit signalAlbumAboutToBeDeleted(album);

    ChangingDB changing(d);
    DatabaseAccess().db()->deleteSearch(album->id());

    d->allAlbumsIdHash.remove(album->globalID());
    emit signalAlbumDeleted(album);
    delete album;
    emit signalAlbumHasBeenDeleted(album);

    return true;
}

QMap<int, int> AlbumManager::getPAlbumsCount() const
{
    return d->pAlbumsCount;
}

QMap<int, int> AlbumManager::getTAlbumsCount() const
{
    return d->tAlbumsCount;
}

QMap<YearMonth, int> AlbumManager::getDAlbumsCount() const
{
    return d->dAlbumsCount;
}

QMap<int, int> AlbumManager::getFaceCount() const
{
    return d->fAlbumsCount;
}

bool AlbumManager::isMovingAlbum(Album* album) const
{
    return d->currentlyMovingAlbum == album;
}

void AlbumManager::insertPAlbum(PAlbum* album, PAlbum* parent)
{
    if (!album)
    {
        return;
    }

    emit signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : 0);

    if (parent)
    {
        album->setParent(parent);
    }

    d->albumPathHash[album]  = album;
    d->allAlbumsIdHash[album->globalID()] = album;

    emit signalAlbumAdded(album);
}

void AlbumManager::removePAlbum(PAlbum* album)
{
    if (!album)
    {
        return;
    }

    // remove all children of this album
    Album* child        = album->m_firstChild;
    PAlbum* toBeRemoved = 0;

    while (child)
    {
        Album* next = child->m_next;
        toBeRemoved = static_cast<PAlbum*>(child);

        if (toBeRemoved)
        {
            removePAlbum(toBeRemoved);
            toBeRemoved = 0;
        }

        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->albumPathHash.remove(album);
    d->allAlbumsIdHash.remove(album->globalID());

    DatabaseUrl url = album->databaseUrl();

    if(!d->currentAlbums.isEmpty())
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }

    if (album->isAlbumRoot())
    {
        d->albumRootAlbumHash.remove(album->albumRootId());
    }

    emit signalAlbumDeleted(album);
    delete album;
    emit signalAlbumHasBeenDeleted(album);
}

void AlbumManager::insertTAlbum(TAlbum* album, TAlbum* parent)
{
    if (!album)
    {
        return;
    }

    emit signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : 0);

    if (parent)
    {
        album->setParent(parent);
    }

    d->allAlbumsIdHash.insert(album->globalID(), album);

    emit signalAlbumAdded(album);
}

void AlbumManager::removeTAlbum(TAlbum* album)
{
    if (!album)
    {
        return;
    }

    // remove all children of this album
    Album* child        = album->m_firstChild;
    TAlbum* toBeRemoved = 0;

    while (child)
    {
        Album* next = child->m_next;
        toBeRemoved = static_cast<TAlbum*>(child);

        if (toBeRemoved)
        {
            removeTAlbum(toBeRemoved);
            toBeRemoved = 0;
        }

        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->allAlbumsIdHash.remove(album->globalID());

    if(!d->currentAlbums.isEmpty())
    {
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    emit signalAlbumDeleted(album);
    delete album;
    emit signalAlbumHasBeenDeleted(album);
}

void AlbumManager::notifyAlbumDeletion(Album* album)
{
    invalidateGuardedPointers(album);
}

void AlbumManager::slotAlbumsJobResult(KJob* job)
{
    d->albumListJob = 0;

    if (job->error())
    {
        kWarning() << k_funcinfo << "Failed to list albums";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), job->errorString(),
                             0, i18n("digiKam"));
    }
}

void AlbumManager::slotAlbumsJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }

    QMap<int, int> albumsStatMap;
    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);
    ds >> albumsStatMap;

    d->pAlbumsCount = albumsStatMap;
    emit signalPAlbumsDirty(albumsStatMap);
}

void AlbumManager::slotPeopleJobResult(KJob* job)
{
    d->personListJob = 0;

    if (job->error())
    {
        kWarning() << k_funcinfo << "Failed to list face tags";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), job->errorString(),
                             0, i18n("digiKam"));
    }
}

void AlbumManager::slotPeopleJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }

    QMap<QString, QMap<int, int> > facesStatMap;
    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);
    ds >> facesStatMap;

    // For now, we only use the sum of confirmed and unconfirmed faces
    d->fAlbumsCount.clear();
    typedef QMap<int, int> IntIntMap;

    foreach(const IntIntMap& counts, facesStatMap)
    {
        QMap<int, int>::const_iterator it;

        for (it = counts.begin(); it != counts.end(); ++it)
        {
            d->fAlbumsCount[it.key()] += it.value();
        }
    }

    emit signalFaceCountsDirty(d->fAlbumsCount);
}

void AlbumManager::slotTagsJobResult(KJob* job)
{
    d->tagListJob = 0;

    if (job->error())
    {
        kWarning() << k_funcinfo << "Failed to list tags";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), job->errorString(),
                             0, i18n("digiKam"));
    }
}

void AlbumManager::slotTagsJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }

    QMap<int, int> tagsStatMap;
    QByteArray     di(data);
    QDataStream    ds(&di, QIODevice::ReadOnly);
    ds >> tagsStatMap;

    d->tAlbumsCount = tagsStatMap;
    emit signalTAlbumsDirty(tagsStatMap);
}

void AlbumManager::slotDatesJobResult(KJob* job)
{
    d->dateListJob = 0;

    if (job->error())
    {
        kWarning() << "Failed to list dates";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), job->errorString(),
                             0, i18n("digiKam"));
    }

    emit signalAllDAlbumsLoaded();
}

void AlbumManager::slotDatesJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty() || !d->rootDAlbum)
    {
        return;
    }

    // insert all the DAlbums into a qmap for quick access
    QMap<QDate, DAlbum*> mAlbumMap;
    QMap<int, DAlbum*>   yAlbumMap;

    AlbumIterator it(d->rootDAlbum);

    while (it.current())
    {
        DAlbum* a = (DAlbum*)(*it);

        if (a->range() == DAlbum::Month)
        {
            mAlbumMap.insert(a->date(), a);
        }
        else
        {
            yAlbumMap.insert(a->date().year(), a);
        }

        ++it;
    }

    QMap<QDateTime, int> datesStatMap;
    QByteArray           di(data);
    QDataStream          ds(&di, QIODevice::ReadOnly);
    ds >> datesStatMap;

    QMap<YearMonth, int> yearMonthMap;

    for (QMap<QDateTime, int>::const_iterator it = datesStatMap.constBegin(); it != datesStatMap.constEnd(); ++it)
    {
        YearMonth yearMonth = YearMonth(it.key().date().year(), it.key().date().month());

        QMap<YearMonth, int>::iterator it2 = yearMonthMap.find(yearMonth);

        if (it2 == yearMonthMap.end())
        {
            yearMonthMap.insert(yearMonth, *it);
        }
        else
        {
            *it2 += *it;
        }
    }

    int year, month;

    for (QMap<YearMonth, int>::const_iterator iter = yearMonthMap.constBegin();
         iter != yearMonthMap.constEnd(); ++iter)
    {
        year  = iter.key().first;
        month = iter.key().second;

        QDate md(year, month, 1);

        // Do we already have this Month album
        if (mAlbumMap.contains(md))
        {
            // already there. remove Month album from map
            mAlbumMap.remove(md);

            if (yAlbumMap.contains(year))
            {
                // already there. remove from map
                yAlbumMap.remove(year);
            }

            continue;
        }

        // Check if Year Album already exist.
        DAlbum* yAlbum = 0;
        AlbumIterator it(d->rootDAlbum);

        while (it.current())
        {
            DAlbum* a = (DAlbum*)(*it);

            if (a->date() == QDate(year, 1, 1) && a->range() == DAlbum::Year)
            {
                yAlbum = a;
                break;
            }

            ++it;
        }

        // If no, create Year album.
        if (!yAlbum)
        {
            yAlbum = new DAlbum(QDate(year, 1, 1), false, DAlbum::Year);
            emit signalAlbumAboutToBeAdded(yAlbum, d->rootDAlbum, d->rootDAlbum->lastChild());
            yAlbum->setParent(d->rootDAlbum);
            d->allAlbumsIdHash.insert(yAlbum->globalID(), yAlbum);
            emit signalAlbumAdded(yAlbum);
        }

        // Create Month album
        DAlbum* mAlbum = new DAlbum(md);
        emit signalAlbumAboutToBeAdded(mAlbum, yAlbum, yAlbum->lastChild());
        mAlbum->setParent(yAlbum);
        d->allAlbumsIdHash.insert(mAlbum->globalID(), mAlbum);
        emit signalAlbumAdded(mAlbum);
    }

    // Now the items contained in the maps are the ones which
    // have been deleted.
    for (QMap<QDate, DAlbum*>::const_iterator it = mAlbumMap.constBegin();
         it != mAlbumMap.constEnd(); ++it)
    {
        DAlbum* album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        delete album;
        emit signalAlbumHasBeenDeleted(album);
    }

    for (QMap<int, DAlbum*>::const_iterator it = yAlbumMap.constBegin();
         it != yAlbumMap.constEnd(); ++it)
    {
        DAlbum* album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        delete album;
        emit signalAlbumHasBeenDeleted(album);
    }

    d->dAlbumsCount = yearMonthMap;
    emit signalDAlbumsDirty(yearMonthMap);
    emit signalDatesMapDirty(datesStatMap);
}

void AlbumManager::slotAlbumChange(const AlbumChangeset& changeset)
{
    if (d->changingDB || !d->rootPAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case AlbumChangeset::Added:
        case AlbumChangeset::Deleted:

            if (!d->scanPAlbumsTimer->isActive())
            {
                d->scanPAlbumsTimer->start();
            }

            break;

        case AlbumChangeset::Renamed:
        case AlbumChangeset::PropertiesChanged:
            // mark for rescan
            d->changedPAlbums << changeset.albumId();

            if (!d->updatePAlbumsTimer->isActive())
            {
                d->updatePAlbumsTimer->start();
            }

            break;

        case AlbumChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotTagChange(const TagChangeset& changeset)
{
    if (d->changingDB || !d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case TagChangeset::Added:
        case TagChangeset::Deleted:
        case TagChangeset::Reparented:

            if (!d->scanTAlbumsTimer->isActive())
            {
                d->scanTAlbumsTimer->start();
            }

            break;

        case TagChangeset::Renamed:
        case TagChangeset::IconChanged:
            /**
             * @todo what happens here?
             */
            break;

        case TagChangeset::PropertiesChanged:
        {
            TAlbum* tag = findTAlbum(changeset.tagId());

            if (tag)
            {
                emit signalTagPropertiesChanged(tag);
            }

            break;
        }

        case TagChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotSearchChange(const SearchChangeset& changeset)
{
    if (d->changingDB || !d->rootSAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case SearchChangeset::Added:
        case SearchChangeset::Deleted:

            if (!d->scanSAlbumsTimer->isActive())
            {
                d->scanSAlbumsTimer->start();
            }

            break;

        case SearchChangeset::Changed:
            if(!d->currentAlbums.isEmpty())
            {
                Album* currentAlbum = d->currentAlbums.first();

                if (currentAlbum && currentAlbum->type() == Album::SEARCH
                    && currentAlbum->id() == changeset.searchId())
                {
                    // the pointer is the same, but the contents changed
                    emit signalAlbumCurrentChanged(d->currentAlbums);
                }
            }
            break;

        case SearchChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (!d->rootDAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:

            if (!d->scanDAlbumsTimer->isActive())
            {
                d->scanDAlbumsTimer->start();
            }

            if (!d->albumItemCountTimer->isActive())
            {
                d->albumItemCountTimer->start();
            }

            break;

        default:
            break;
    }
}

void AlbumManager::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (!d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case ImageTagChangeset::Added:
        case ImageTagChangeset::Removed:
        case ImageTagChangeset::RemovedAll:

            if (!d->tagItemCountTimer->isActive())
            {
                d->tagItemCountTimer->start();
            }

            break;

        default:
            break;
    }
}

}  // namespace Digikam
