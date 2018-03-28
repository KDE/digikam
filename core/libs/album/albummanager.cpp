/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "albummanager.h"

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
#include <QComboBox>
#include <QMessageBox>
#include <QIcon>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSet>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "album.h"
#include "applicationsettings.h"
#include "metadatasettings.h"
#include "metadatasynchronizer.h"
#include "albumwatch.h"
#include "imageattributeswatch.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "digikam_config.h"
#include "coredbaccess.h"
#include "coredboperationgroup.h"
#include "dbengineguierrorhandler.h"
#include "dbengineparameters.h"
#include "databaseserverstarter.h"
#include "coredbthumbinfoprovider.h"
#include "coredburl.h"
#include "coredbsearchxml.h"
#include "coredbwatch.h"
#include "dio.h"
#include "facetags.h"
#include "facetagseditor.h"
#include "imagelister.h"
#include "scancontroller.h"
#include "setupcollections.h"
#include "setup.h"
#include "tagscache.h"
#include "thumbsdbaccess.h"
#include "thumbnailloadthread.h"
#include "dnotificationwrapper.h"
#include "dbjobinfo.h"
#include "dbjobsmanager.h"
#include "dbjobsthread.h"
#include "similaritydb.h"
#include "similaritydbaccess.h"

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
        : albumRootId(albumRootId),
          albumPath(albumPath)
    {
    }

    PAlbumPath(PAlbum* const album)
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
        return (other.albumRootId == albumRootId &&
                other.albumPath   == albumPath);
    }

public:

    int     albumRootId;
    QString albumPath;
};

// -----------------------------------------------------------------------------------

uint qHash(const PAlbumPath& id)
{
    return ( ::qHash(id.albumRootId) ^ ::qHash(id.albumPath) );
}

// -----------------------------------------------------------------------------------

class AlbumManager::Private
{
public:

    Private() :
        changed(false),
        hasPriorizedDbPath(false),
        dbFakeConnection(false),
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

    bool                        dbFakeConnection;

    bool                        showOnlyAvailableAlbums;

    AlbumsDBJobsThread*         albumListJob;
    DatesDBJobsThread*          dateListJob;
    TagsDBJobsThread*           tagListJob;
    TagsDBJobsThread*           personListJob;


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

    explicit ChangingDB(AlbumManager::Private* const d)
        : d(d)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

    AlbumManager::Private* const d;
};

// -----------------------------------------------------------------------------------

class AlbumManagerCreator
{
public:

    AlbumManager object;
};

Q_GLOBAL_STATIC(AlbumManagerCreator, creator)

// -----------------------------------------------------------------------------------

// A friend-class shortcut to circumvent accessing this from within the destructor
AlbumManager* AlbumManager::internalInstance = 0;

AlbumManager* AlbumManager::instance()
{
    return &creator->object;
}

AlbumManager::AlbumManager()
    : d(new Private)
{
    qRegisterMetaType<QMap<QDateTime,int>>("QMap<QDateTime,int>");
    qRegisterMetaType<QMap<int,int>>("QMap<int,int>");
    qRegisterMetaType<QMap<QString,QMap<int,int> >>("QMap<QString,QMap<int,int> >");

    internalInstance = this;
    d->albumWatch    = new AlbumWatch(this);

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
    d->scanDAlbumsTimer->setInterval(30 * 1000);
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
    // This is what we prefer to do before Application destruction

    if (d->dateListJob)
    {
        d->dateListJob->cancel();
        d->dateListJob = 0;
    }

    if (d->albumListJob)
    {
        d->albumListJob->cancel();
        d->albumListJob = 0;
    }

    if (d->tagListJob)
    {
        d->tagListJob->cancel();
        d->tagListJob = 0;
    }

    if (d->personListJob)
    {
        d->personListJob->cancel();
        d->personListJob = 0;
    }
}

bool AlbumManager::databaseEqual(const DbEngineParameters& parameters) const
{
    DbEngineParameters params = CoreDbAccess::parameters();

    return (params == parameters);
}

static bool moveToBackup(const QFileInfo& info)
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

static bool copyToNewLocation(const QFileInfo& oldFile, const QFileInfo& newFile,
                              const QString otherMessage = QString())
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
            QMessageBox msgBox(QMessageBox::Warning,
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

            msgBox.button(QMessageBox::Yes)->setText(i18n("Upgrade Database"));
            msgBox.button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
            msgBox.button(QMessageBox::No)->setText(i18n("Create New Database"));
            msgBox.button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
            msgBox.setDefaultButton(QMessageBox::Yes);

            int result = msgBox.exec();

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
                    QMessageBox msgBox(QMessageBox::Warning,
                                       i18n("New database folder"),
                                       i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                            "A database file from an older version of digiKam is found in this folder.</p> "
                                            "<p>Would you like to upgrade the old database file, start with a new database, "
                                            "or copy the current database to this location and continue using it?</p> ",
                                            QDir::toNativeSeparators(newDir.path())),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                       qApp->activeWindow());

                    msgBox.button(QMessageBox::Yes)->setText(i18n("Upgrade Database"));
                    msgBox.button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
                    msgBox.button(QMessageBox::No)->setText(i18n("Create New Database"));
                    msgBox.button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox.button(QMessageBox::Cancel)->setText(i18n("Copy Current Database"));
                    msgBox.button(QMessageBox::Cancel)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                    msgBox.setDefaultButton(QMessageBox::Yes);

                    result = msgBox.exec();
                }
                else
                {
                    QMessageBox msgBox(QMessageBox::Warning,
                                       i18n("New database folder"),
                                       i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                            "A database file from an older version of digiKam is found in this folder.</p> "
                                            "<p>Would you like to upgrade the old database file or start with a new database?</p>",
                                            QDir::toNativeSeparators(newDir.path())),
                                       QMessageBox::Yes | QMessageBox::No,
                                       qApp->activeWindow());

                    msgBox.button(QMessageBox::Yes)->setText(i18n("Upgrade Database"));
                    msgBox.button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
                    msgBox.button(QMessageBox::No)->setText(i18n("Create New Database"));
                    msgBox.button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox.setDefaultButton(QMessageBox::Yes);

                    result = msgBox.exec();
                }

                if (result == QMessageBox::Yes)
                {
                    // CoreDbSchemaUpdater expects Album Path to point to the album root of the 0.9 db file.
                    // Restore this situation.
                    KSharedConfigPtr config = KSharedConfig::openConfig();
                    KConfigGroup group = config->group(QLatin1String("Album Settings"));
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
                    QMessageBox msgBox(QMessageBox::Warning,
                                       i18n("New database folder"),
                                       i18n("<p>You have chosen the folder \"%1\" as the new place to store the database.</p>"
                                            "<p>Would you like to copy the current database to this location "
                                            "and continue using it, or start with a new database?</p> ",
                                            QDir::toNativeSeparators(newDir.path())),
                                       QMessageBox::Yes | QMessageBox::No,
                                       qApp->activeWindow());

                    msgBox.button(QMessageBox::Yes)->setText(i18n("Create New Database"));
                    msgBox.button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox.button(QMessageBox::No)->setText(i18n("Copy Current Database"));
                    msgBox.button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                    msgBox.setDefaultButton(QMessageBox::Yes);

                    result = msgBox.exec();
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
                QMessageBox msgBox(QMessageBox::Warning,
                                   i18n("New database folder"),
                                   i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                        "There is already a database file in this location.</p> "
                                        "<p>Would you like to use this existing file as the new database, or remove it "
                                        "and copy the current database to this place?</p> ",
                                        QDir::toNativeSeparators(newDir.path())),
                                   QMessageBox::Yes | QMessageBox::No,
                                   qApp->activeWindow());

                msgBox.button(QMessageBox::Yes)->setText(i18n("Copy Current Database"));
                msgBox.button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                msgBox.button(QMessageBox::No)->setText(i18n("Use Existing File"));
                msgBox.button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
                msgBox.setDefaultButton(QMessageBox::Yes);

                result = msgBox.exec();
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

    foreach(const CollectionLocation& loc, disappearedLocations)
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
    connect(CoreDbAccess::databaseWatch(), SIGNAL(albumChange(AlbumChangeset)),
            this, SLOT(slotAlbumChange(AlbumChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(tagChange(TagChangeset)),
            this, SLOT(slotTagChange(TagChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(searchChange(SearchChangeset)),
            this, SLOT(slotSearchChange(SearchChangeset)));

    // listen to collection image changes
    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));

    // listen to image attribute changes
    connect(ImageAttributesWatch::instance(), SIGNAL(signalImageDateChanged(qlonglong)),
            d->scanDAlbumsTimer, SLOT(start()));

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
    PAlbum* const album = d->albumRootAlbumHash.value(location.id());

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
        album         = new PAlbum(location.id(), label);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Added root album called: " << album->title();

        // insert album root created into hash
        d->albumRootAlbumHash.insert(location.id(), album);
    }
}

void AlbumManager::removeAlbumRoot(const CollectionLocation& location)
{
    // retrieve and remove from hash
    PAlbum* const album = d->albumRootAlbumHash.take(location.id());

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
        PAlbum* const a    = (PAlbum*)(*it);
        oldAlbums[a->id()] = a;
        ++it;
    }

    // scan db and get a list of all albums
    QList<AlbumInfo> currentAlbums = CoreDbAccess().db()->scanAlbums();

    // sort by relative path so that parents are created before children
    std::sort(currentAlbums.begin(), currentAlbums.end());

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

    foreach(PAlbum* const album, oldAlbums)
    {
        if (album->isTrashAlbum())
        {
            continue;
        }

        if (!album->parent() || !oldAlbums.contains(album->parent()->id()))
        {
            topMostOldAlbums << album;
        }
    }

    foreach(PAlbum* const album, topMostOldAlbums)
    {
        // recursively removes all children and the album
        removePAlbum(album);
    }

    // sort by relative path so that parents are created before children
    std::sort(newAlbums.begin(), newAlbums.end());

    // create all new albums
    foreach(const AlbumInfo& info, newAlbums)
    {
        if (info.relativePath.isEmpty())
        {
            continue;
        }

        PAlbum* album = 0, *parent = 0;

        if (info.relativePath == QLatin1String("/"))
        {
            // Albums that represent the root directory of an album root
            // We have them as here new albums first time after their creation

            parent = d->rootPAlbum;
            album  = d->albumRootAlbumHash.value(info.albumRootId);

            if (!album)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Did not find album root album in hash";
                continue;
            }

            // it has been created from the collection location
            // with album root id, parentPath "/" and a name, but no album id yet.
            album->m_id = info.id;
        }
        else
        {
            // last section, no slash
            QString name = info.relativePath.section(QLatin1Char('/'), -1, -1);
            // all but last sections, leading slash, no trailing slash
            QString parentPath = info.relativePath.section(QLatin1Char('/'), 0, -2);

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
                qCDebug(DIGIKAM_GENERAL_LOG) <<  "Could not find parent with url: "
                                             << parentPath << " for: "
                                             << info.relativePath;
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

        if (album->isAlbumRoot())
        {
            // Inserting virtual Trash PAlbum for AlbumsRootAlbum using special constructor
            PAlbum* trashAlbum = new PAlbum(album->title(), album->id());
            insertPAlbum(trashAlbum, album);
        }
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
    QList<AlbumInfo> currentAlbums = CoreDbAccess().db()->scanAlbums();
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
                    if (info.relativePath != QLatin1String("/"))
                    {
                        // Handle rename of album name
                        // last section, no slash
                        QString name       = info.relativePath.section(QLatin1Char('/'), -1, -1);
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

    if (d->albumListJob)
    {
        d->albumListJob->cancel();
        d->albumListJob = 0;
    }

    AlbumsDBJobInfo jInfo;
    jInfo.setFoldersJob();
    d->albumListJob = DBJobsManager::instance()->startAlbumsJobThread(jInfo);

    connect(d->albumListJob, SIGNAL(finished()),
            this, SLOT(slotAlbumsJobResult()));

    connect(d->albumListJob, SIGNAL(foldersData(QMap<int,int>)),
            this, SLOT(slotAlbumsJobData(QMap<int,int>)));
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
    TagInfo::List tList = CoreDbAccess().db()->scanTags();

    // sort the list. needed because we want the tags can be read in any order,
    // but we want to make sure that we are ensure to find the parent TAlbum
    // for a new TAlbum

    {
        QHash<int, TAlbum*> tagHash;

        // insert items into a dict for quick lookup
        for (TagInfo::List::const_iterator iter = tList.constBegin() ; iter != tList.constEnd() ; ++iter)
        {
            TagInfo info        = *iter;
            TAlbum* const album = new TAlbum(info.name, info.id);

            album->m_icon   = info.icon;
            album->m_iconId = info.iconId;
            album->m_pid    = info.pid;
            tagHash.insert(info.id, album);
        }

        tList.clear();

        // also add root tag
        TAlbum* const rootTag = new TAlbum(QLatin1String("root"), 0, true);
        tagHash.insert(0, rootTag);

        // build tree
        for (QHash<int, TAlbum*>::const_iterator iter = tagHash.constBegin() ; iter != tagHash.constEnd() ; ++iter)
        {
            TAlbum* album = *iter;

            if (album->m_id == 0)
            {
                continue;
            }

            TAlbum* const parent = tagHash.value(album->m_pid);

            if (parent)
            {
                album->setParent(parent);
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                           << album->m_title
                           << " with pid "
                           << album->m_pid;
            }
        }

        tagHash.clear();

        // now insert the items into the list. becomes sorted
        AlbumIterator it(rootTag);

        while (it.current())
        {
            TagInfo info;
            TAlbum* const album = static_cast<TAlbum*>(it.current());

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

    for (TagInfo::List::const_iterator it = tList.constBegin() ; it != tList.constEnd() ; ++it)
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
            qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                                           << info.name
                                           << " with pid "
                                           << info.pid;
            continue;
        }

        TAlbum* const parent = iter.value();

        // Create the new TAlbum
        TAlbum* const album = new TAlbum(info.name, info.id, false);
        album->m_icon       = info.icon;
        album->m_iconId     = info.iconId;
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

    tagItemsCount();
    personItemsCount();
}

void AlbumManager::tagItemsCount()
{
    if (d->tagListJob)
    {
        d->tagListJob->cancel();
        d->tagListJob = 0;
    }

    TagsDBJobInfo jInfo;
    jInfo.setFoldersJob();

    d->tagListJob = DBJobsManager::instance()->startTagsJobThread(jInfo);

    connect(d->tagListJob, SIGNAL(finished()),
            this, SLOT(slotTagsJobResult()));

    connect(d->tagListJob, SIGNAL(foldersData(QMap<int,int>)),
            this, SLOT(slotTagsJobData(QMap<int,int>)));
}

void AlbumManager::personItemsCount()
{
    if (d->personListJob)
    {
        d->personListJob->cancel();
        d->personListJob = 0;
    }

    TagsDBJobInfo jInfo;
    jInfo.setFaceFoldersJob();

    d->personListJob = DBJobsManager::instance()->startTagsJobThread(jInfo);

    connect(d->personListJob, SIGNAL(finished()),
            this, SLOT(slotPeopleJobResult()));

    connect(d->personListJob, SIGNAL(faceFoldersData(QMap<QString,QMap<int,int> >)),
            this, SLOT(slotPeopleJobData(QMap<QString,QMap<int,int> >)));
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
        SAlbum* const search      = (SAlbum*)(*it);
        oldSearches[search->id()] = search;
        ++it;
    }

    // scan db and get a list of all albums
    QList<SearchInfo> currentSearches = CoreDbAccess().db()->scanSearches();

    QList<SearchInfo> newSearches;

    // go through all the Albums and see which ones are already present
    foreach(const SearchInfo& info, currentSearches)
    {
        if (oldSearches.contains(info.id))
        {
            SAlbum* const album = oldSearches[info.id];

            if (info.name  != album->title()      ||
                info.type  != album->searchType() ||
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
    foreach(SAlbum* const album, oldSearches)
    {
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
        delete album;
        emit signalAlbumHasBeenDeleted(deletedAlbum);
    }

    // add new albums
    foreach(const SearchInfo& info, newSearches)
    {
        SAlbum* const album = new SAlbum(info.name, info.id);
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

    if (d->dateListJob)
    {
        d->dateListJob->cancel();
        d->dateListJob = 0;
    }

    DatesDBJobInfo jInfo;
    jInfo.setFoldersJob();
    d->dateListJob = DBJobsManager::instance()->startDatesJobThread(jInfo);

    connect(d->dateListJob, SIGNAL(finished()),
            this, SLOT(slotDatesJobResult()));

    connect(d->dateListJob, SIGNAL(foldersData(QMap<QDateTime,int>)),
            this, SLOT(slotDatesJobData(QMap<QDateTime, int>)));
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
    if (albums.isEmpty())
        return;

    QList<Album*> filtered;

    /**
     * Filter out the null pointers
    */
    foreach(Album* const album, albums)
    {
        if (album != 0)
        {
            filtered.append(album);
        }
    }

    albums = filtered;

    /**
     * Sort is needed to identify selection correctly, ex AlbumHistory
     */
    std::sort(albums.begin(), albums.end());
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
    if (!d->currentAlbums.isEmpty())
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

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        TAlbum* const temp = dynamic_cast<TAlbum*>(*it);

        if (temp)
            talbums.push_back(temp);
    }

    return talbums;
}

PAlbum* AlbumManager::findPAlbum(const QUrl& url) const
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

Album* AlbumManager::findAlbum(Album::Type type, int id) const
{
    return findAlbum(Album::globalID(type, id));
}

TAlbum* AlbumManager::findTAlbum(const QString& tagPath) const
{
    // handle gracefully with or without leading slash
    bool          withLeadingSlash = tagPath.startsWith(QLatin1Char('/'));
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const talbum = static_cast<TAlbum*>(*it);

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
    for (Album* album = d->rootSAlbum->firstChild() ; album ; album = album->next())
    {
        if (album->title() == name)
        {
            return dynamic_cast<SAlbum*>(album);
        }
    }

    return 0;
}

QList<SAlbum*> AlbumManager::findSAlbumsBySearchType(int searchType) const
{
    QList<SAlbum*> albums;
    for (Album* album = d->rootSAlbum->firstChild() ; album ; album = album->next())
    {
        if (album != 0)
        {
            SAlbum* sAlbum = dynamic_cast<SAlbum*>(album);
            if ((sAlbum != 0) && (sAlbum->searchType() == searchType))
            {
                albums.append(sAlbum);
            }
        }
    }
    return albums;
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

    if (name.contains(QLatin1String("/")))
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
    PAlbum* child = static_cast<PAlbum*>(parent->m_firstChild);

    while (child)
    {
        if (child->albumRootId() == albumRootId && child->albumPath() == albumPath)
        {
            errMsg = i18n("An existing album has the same name.");
            return 0;
        }

        child = static_cast<PAlbum*>(child->m_next);
    }

    CoreDbUrl url   = parent->databaseUrl();
    url             = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + name);
    QUrl fileUrl    = url.fileUrl();

    bool ret = QDir().mkdir(fileUrl.toLocalFile());

    if (!ret)
    {
        errMsg = i18n("Failed to create directory '%1'", fileUrl.toString()); // TODO add tags?
        return 0;
    }

    ChangingDB changing(d);
    int        id = CoreDbAccess().db()->addAlbum(albumRootId, albumPath, caption, date, category);

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

    PAlbum* const album = new PAlbum(albumRootId, parentPath, name, id);
    album->m_caption    = caption;
    album->m_category   = category;
    album->m_date       = date;

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

    if (newName.contains(QLatin1String("/")))
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

    d->albumWatch->removeWatchedPAlbums(album);

    QString oldAlbumPath = album->albumPath();
    QUrl oldUrl          = album->fileUrl();
    album->setTitle(newName);
    album->m_path        = newName;
    QUrl newUrl          = album->fileUrl();
    QString newAlbumPath = album->albumPath();

    // We use a private shortcut around collection scanner noticing our changes,
    // we rename them directly. Faster.
    ScanController::instance()->suspendCollectionScan();

    bool ret = QDir().rename(oldUrl.toLocalFile(), newUrl.toLocalFile());

    if (!ret)
    {
        ScanController::instance()->resumeCollectionScan();

        errMsg = i18n("Failed to rename Album");
        return false;
    }

    // now rename the album and subalbums in the database
    {
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->renameAlbum(album->id(), album->albumRootId(), album->albumPath());

        PAlbum* subAlbum = 0;
        AlbumIterator it(album);

        while ((subAlbum = static_cast<PAlbum*>(it.current())) != 0)
        {
            subAlbum->m_parentPath = newAlbumPath + subAlbum->m_parentPath.mid(oldAlbumPath.length());
            access.db()->renameAlbum(subAlbum->id(), album->albumRootId(), subAlbum->albumPath());
            emit signalAlbumNewPath(subAlbum);
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
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->setAlbumIcon(album->id(), iconID);
        album->m_iconId = iconID;
    }

    emit signalAlbumIconChanged(album);

    return true;
}

qlonglong AlbumManager::getItemFromAlbum(PAlbum* album, const QString& fileName)
{
    return CoreDbAccess().db()->getItemFromAlbum(album->id(),fileName);
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

    if (name.contains(QLatin1String("/")))
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
    int id = CoreDbAccess().db()->addTag(parent->id(), name, iconkde, 0);

    if (id == -1)
    {
        errMsg = i18n("Failed to add tag to database");
        return 0;
    }

    TAlbum* const album = new TAlbum(name, id, false);
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

bool AlbumManager::deleteTAlbum(TAlbum* album, QString& errMsg, bool askUser)
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

    QList<qlonglong> imageIds;

    if (askUser)
    {
        imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id());
    }

    {
        CoreDbAccess access;
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

    if (askUser)
    {
        askUserForWriteChangedTAlbumToFiles(imageIds);
    }

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

    if (name.contains(QLatin1String("/")))
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
    CoreDbAccess().db()->setTagName(album->id(), name);
    album->setTitle(name);
    emit signalAlbumRenamed(album);

    askUserForWriteChangedTAlbumToFiles(album);

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
        QMessageBox msgBox(QMessageBox::Warning,
                           qApp->applicationName(),
                           i18n("Another tag with the same name already exists.\n"
                                "Do you want to merge the tags?"),
                           QMessageBox::Yes | QMessageBox::No,
                           qApp->activeWindow());

        if (msgBox.exec() == QMessageBox::Yes)
        {
            if (album->m_firstChild)
            {
                errMsg = i18n("Only a tag without children can be merged!");
                return false;
            }

            TAlbum* const mergeTag = findTAlbum(newParent->tagPath() +
                                                QLatin1Char('/') +
                                                album->title());
            if (!mergeTag)
            {
                errMsg = i18n("No such album");
                return false;
            }

            int oldId   = album->id();
            int mergeId = mergeTag->id();

            if (oldId == mergeId)
            {
                return true;
            }

            QApplication::setOverrideCursor(Qt::WaitCursor);
            QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(oldId);

            CoreDbOperationGroup group;
            group.setMaximumTime(200);

            foreach(const qlonglong& imageId, imageIds)
            {
                QList<FaceTagsIface> facesList = FaceTagsEditor().databaseFaces(imageId);
                bool foundFace                 = false;

                foreach(const FaceTagsIface& face, facesList)
                {
                    if (face.tagId() == oldId)
                    {
                        foundFace = true;
                        FaceTagsEditor().removeFace(face);
                        FaceTagsEditor().add(imageId, mergeId, face.region(), false);
                    }
                }

                if (!foundFace)
                {
                    ImageInfo info(imageId);
                    info.removeTag(oldId);
                    info.setTag(mergeId);
                    group.allowLift();
                }
            }

            QApplication::restoreOverrideCursor();

            if (!deleteTAlbum(album, errMsg, false))
            {
                return false;
            }

            askUserForWriteChangedTAlbumToFiles(imageIds);

            return true;
        }
        else
        {
            return true;
        }
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
    emit signalAlbumHasBeenDeleted(reinterpret_cast<quintptr>(album));

    emit signalAlbumAboutToBeAdded(album, newParent, newParent->lastChild());
    ChangingDB changing(d);
    CoreDbAccess().db()->setTagParentID(album->id(), newParent->id());
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

    askUserForWriteChangedTAlbumToFiles(album);

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
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->setTagIcon(album->id(), iconKDE, iconID);
        album->m_icon   = iconKDE;
        album->m_iconId = iconID;
    }

    emit signalAlbumIconChanged(album);

    return true;
}

AlbumList AlbumManager::getRecentlyAssignedTags(bool includeInternal) const
{
    QList<int> tagIDs = CoreDbAccess().db()->getRecentlyAssignedTags();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        TAlbum* const album = findTAlbum(*it);

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

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
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
        TAlbum* const album = findTAlbum(id);

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
        TAlbum* const t = (TAlbum*)(*it);

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
        TAlbum* const t = (TAlbum*)(*it);

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
    TAlbum* const album = this->findTAlbum(tagId);
    return album->childAlbumIds(recursive);
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property)
{
    AlbumList list;

    QList<int> ids = TagsCache::instance()->tagsWithProperty(property);

    foreach(int id, ids)
    {
        TAlbum* const album = findTAlbum(id);

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
        PAlbum* const a = (PAlbum*)(*it);
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

    int id = CoreDbAccess().db()->addSearch(type, name, query);

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
    CoreDbAccess().db()->updateSearch(album->id(), newType, newName, changedQuery);

    QString oldName = album->title();

    album->setSearch(newType, changedQuery);
    album->setTitle(newName);

    if (oldName != album->title())
    {
        emit signalAlbumRenamed(album);
    }

    if (!d->currentAlbums.isEmpty())
    {
        if (d->currentAlbums.first() == album)
        {
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }
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
    CoreDbAccess().db()->deleteSearch(album->id());

    d->allAlbumsIdHash.remove(album->globalID());
    emit signalAlbumDeleted(album);
    quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
    delete album;
    emit signalAlbumHasBeenDeleted(deletedAlbum);

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

    CoreDbUrl url = album->databaseUrl();

    if (!d->currentAlbums.isEmpty())
    {
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    if (album->isAlbumRoot())
    {
        d->albumRootAlbumHash.remove(album->albumRootId());
    }

    emit signalAlbumDeleted(album);
    quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
    delete album;
    emit signalAlbumHasBeenDeleted(deletedAlbum);
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

    if (!d->currentAlbums.isEmpty())
    {
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    emit signalAlbumDeleted(album);
    quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
    delete album;
    emit signalAlbumHasBeenDeleted(deletedAlbum);
}

void AlbumManager::notifyAlbumDeletion(Album* album)
{
    invalidateGuardedPointers(album);
}

void AlbumManager::slotAlbumsJobResult()
{
    if (!d->albumListJob)
    {
        return;
    }

    if (d->albumListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list albums";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->albumListJob->errorsList().first(),
                             0, i18n("digiKam"));
    }

    d->albumListJob = 0;
}

void AlbumManager::slotAlbumsJobData(const QMap<int, int> &albumsStatMap)
{
    if (albumsStatMap.isEmpty())
    {
        return;
    }

    d->pAlbumsCount = albumsStatMap;
    emit signalPAlbumsDirty(albumsStatMap);
}

void AlbumManager::slotPeopleJobResult()
{
    if (!d->personListJob)
    {
        return;
    }

    if (d->personListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list face tags";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->personListJob->errorsList().first(),
                             0, i18n("digiKam"));
    }

    d->personListJob = 0;
}

void AlbumManager::slotPeopleJobData(const QMap<QString,QMap<int,int> >& facesStatMap)
{
    if (facesStatMap.isEmpty())
    {
        return;
    }

    // For now, we only use the sum of confirmed and unconfirmed faces
    d->fAlbumsCount.clear();
    typedef QMap<int, int> IntIntMap;

    foreach(const IntIntMap& counts, facesStatMap)
    {
        QMap<int, int>::const_iterator it;

        for (it = counts.begin() ; it != counts.end() ; ++it)
        {
            d->fAlbumsCount[it.key()] += it.value();
        }
    }

    emit signalFaceCountsDirty(d->fAlbumsCount);
}

void AlbumManager::slotTagsJobResult()
{
    if (!d->tagListJob)
    {
        return;
    }

    if (d->tagListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list face tags";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->personListJob->errorsList().first(),
                             0, i18n("digiKam"));
    }

    d->tagListJob = 0;
}

void AlbumManager::slotTagsJobData(const QMap<int,int>& tagsStatMap)
{
    if (tagsStatMap.isEmpty())
    {
        return;
    }

    d->tAlbumsCount = tagsStatMap;
    emit signalTAlbumsDirty(tagsStatMap);
}

void AlbumManager::slotDatesJobResult()
{
    if (!d->dateListJob)
    {
        return;
    }

    if (d->dateListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list dates";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->dateListJob->errorsList().first(),
                             0, i18n("digiKam"));
    }

    d->dateListJob = 0;
    emit signalAllDAlbumsLoaded();
}

void AlbumManager::slotDatesJobData(const QMap<QDateTime, int>& datesStatMap)
{
    if (datesStatMap.isEmpty() || !d->rootDAlbum)
    {
        return;
    }

    // insert all the DAlbums into a qmap for quick access
    QMap<QDate, DAlbum*> mAlbumMap;
    QMap<int, DAlbum*>   yAlbumMap;

    AlbumIterator it(d->rootDAlbum);

    while (it.current())
    {
        DAlbum* const a = (DAlbum*)(*it);

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

    QMap<YearMonth, int> yearMonthMap;

    for (QMap<QDateTime, int>::const_iterator it = datesStatMap.constBegin() ; it != datesStatMap.constEnd() ; ++it)
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

    for (QMap<YearMonth, int>::const_iterator iter = yearMonthMap.constBegin() ; iter != yearMonthMap.constEnd() ; ++iter)
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
            DAlbum* const a = (DAlbum*)(*it);

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
    for (QMap<QDate, DAlbum*>::const_iterator it = mAlbumMap.constBegin() ; it != mAlbumMap.constEnd() ; ++it)
    {
        DAlbum* const album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
        delete album;
        emit signalAlbumHasBeenDeleted(deletedAlbum);
    }

    for (QMap<int, DAlbum*>::const_iterator it = yAlbumMap.constBegin() ; it != yAlbumMap.constEnd() ; ++it)
    {
        DAlbum* const album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
        delete album;
        emit signalAlbumHasBeenDeleted(deletedAlbum);
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
        case TagChangeset::Moved:
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
            if (!d->currentAlbums.isEmpty())
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
        // Add properties changed.
        // Reason: in people sidebar, the images are not
        // connected with the ImageTag table but by
        // ImageTagProperties entries.
        // Thus, the count of entries in face tags are not
        // updated. This adoption should fix the problem.
        case ImageTagChangeset::PropertiesChanged:

            if (!d->tagItemCountTimer->isActive())
            {
                d->tagItemCountTimer->start();
            }

            break;

        default:
            break;
    }
}

void AlbumManager::slotImagesDeleted(const QList<qlonglong>& imageIds)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Got image deletion notification from ImageViewUtilities for " << imageIds.size() << " images.";

    QSet<SAlbum*> sAlbumsToUpdate;
    QSet<qlonglong> deletedImages = imageIds.toSet();

    QList<SAlbum*> sAlbums = findSAlbumsBySearchType(DatabaseSearch::DuplicatesSearch);

    foreach(SAlbum* const sAlbum, sAlbums)
    {
        // Read the search query XML and save the image ids
        SearchXmlReader reader(sAlbum->query());
        SearchXml::Element element;
        QSet<qlonglong> images;

        while ((element = reader.readNext()) != SearchXml::End)
        {
            if ((element == SearchXml::Field) && (reader.fieldName().compare(QLatin1String("imageid")) == 0))
            {
                images = reader.valueToLongLongList().toSet();
            }
        }

        // If the deleted images are part of the SAlbum,
        // mark the album as ready for deletion and the images as ready for rescan.
#if QT_VERSION >= 0x050600
        if (images.intersects(deletedImages))
#else
        if (images.intersect(deletedImages).isEmpty())
#endif
        {
            sAlbumsToUpdate.insert(sAlbum);
        }
    }

    if (!sAlbumsToUpdate.isEmpty())
    {
        emit signalUpdateDuplicatesAlbums(sAlbumsToUpdate.toList(), deletedImages.toList());
    }
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(TAlbum* const album)
{
    QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id());
    askUserForWriteChangedTAlbumToFiles(imageIds);
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(const QList<qlonglong>& imageIds)
{
    MetadataSettings* const settings = MetadataSettings::instance();

    if ((!settings->settings().saveTags &&
         !settings->settings().saveFaceTags) || imageIds.isEmpty())
    {
        return;
    }

    if (imageIds.count() > 100)
    {
        QMessageBox msgBox(QMessageBox::Warning,
                           qApp->applicationName(),
                           i18n("This operation can take a long time in the background.\n"
                                "Do you want to write the metadata to %1 files now?",
                                imageIds.count()),
                           QMessageBox::Yes | QMessageBox::No,
                           qApp->activeWindow());

        if (msgBox.exec() != QMessageBox::Yes)
        {
            return;
        }
    }

    ImageInfoList infos(imageIds);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(infos, MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

void AlbumManager::removeWatchedPAlbums(const PAlbum* const album)
{
    d->albumWatch->removeWatchedPAlbums(album);
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

}  // namespace Digikam
