/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include "albummanager.moc"

// C ANSI includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ includes.

#include <clocale>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

// Qt includes.

#include <QApplication>
#include <QHash>
#include <QList>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QMultiHash>
#include <QTextCodec>
#include <QTimer>

// KDE includes.

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kdirwatch.h>
#include <kconfiggroup.h>

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albumitemhandler.h"
#include "albumsettings.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "databaseparameters.h"
#include "databasewatch.h"
#include "dio.h"
#include "imagelister.h"
#include "scancontroller.h"
#include "upgradedb_sqlite2tosqlite3.h"

namespace Digikam
{

class PAlbumPath
{
public:

    PAlbumPath()
        : albumRootId(-1)
    {
    }

    PAlbumPath(int albumRootId, const QString &albumPath)
       : albumRootId(albumRootId), albumPath(albumPath)
    {
    }

    PAlbumPath(PAlbum *album)
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

    bool operator==(const PAlbumPath &other) const
    {
        return other.albumRootId == albumRootId && other.albumPath == albumPath;
    }

    int     albumRootId;
    QString albumPath;
};

uint qHash(const PAlbumPath &id)
{
    return ::qHash(id.albumRootId) ^ ::qHash(id.albumPath);
}

class AlbumManagerPriv
{

public:

    AlbumManagerPriv()
    {
        changed            = false;
        hasPriorizedDbPath = false;
        dateListJob        = 0;
        albumListJob       = 0;
        tagListJob         = 0;
        dirWatch           = 0;
        itemHandler        = 0;
        rootPAlbum         = 0;
        rootTAlbum         = 0;
        rootDAlbum         = 0;
        rootSAlbum         = 0;
        currentAlbum       = 0;
        changingDB         = false;
        scanPAlbumsTimer   = 0;
        scanTAlbumsTimer   = 0;
        scanSAlbumsTimer   = 0;
        scanDAlbumsTimer   = 0;
        updatePAlbumsTimer = 0;
        albumItemCountTimer= 0;
        tagItemCountTimer  = 0;
    }

    bool                        changed;
    bool                        hasPriorizedDbPath;

    QString                     dbPath;

    QList<QDateTime>            dbPathModificationDateList;

    KIO::TransferJob           *albumListJob;
    KIO::TransferJob           *dateListJob;
    KIO::TransferJob           *tagListJob;

    KDirWatch                  *dirWatch;

    AlbumItemHandler           *itemHandler;

    PAlbum                     *rootPAlbum;
    TAlbum                     *rootTAlbum;
    DAlbum                     *rootDAlbum;
    SAlbum                     *rootSAlbum;

    QHash<int,Album *>          allAlbumsIdHash;
    QHash<PAlbumPath, PAlbum*>  albumPathHash;
    QHash<int, PAlbum*>         albumRootAlbumHash;

    QMultiHash<Album*, Album**> guardedPointers;

    Album                      *currentAlbum;

    bool                        changingDB;
    QTimer                     *scanPAlbumsTimer;
    QTimer                     *scanTAlbumsTimer;
    QTimer                     *scanSAlbumsTimer;
    QTimer                     *scanDAlbumsTimer;
    QTimer                     *updatePAlbumsTimer;
    QTimer                     *albumItemCountTimer;
    QTimer                     *tagItemCountTimer;
    QSet<int>                   changedPAlbums;


    QList<QDateTime> buildDirectoryModList(const QFileInfo &dbFile)
    {
        // retrieve modification dates
        QList<QDateTime> modList;
        QFileInfoList fileInfoList = dbFile.dir().entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        // build list
        foreach (const QFileInfo &info, fileInfoList)
        {
            if (info != dbFile)
                modList << info.lastModified();
        }
        return modList;
    }

    QString labelForAlbumRootAlbum(const CollectionLocation &location)
    {
        QString label = location.label();
        if (label.isEmpty())
            label = location.albumRootPath();
        return label;
    }
};

class ChangingDB
{
public:

    ChangingDB(AlbumManagerPriv *d) : d(d)
    {
        d->changingDB = true;
    }
    ~ChangingDB()
    {
        d->changingDB = false;
    }
    AlbumManagerPriv *d;
};

class AlbumManagerCreator { public: AlbumManager object; };
K_GLOBAL_STATIC(AlbumManagerCreator, creator)

// A friend-class shortcut to circumvent accessing this from within the destructor
AlbumManager *AlbumManager::internalInstance = 0;

AlbumManager* AlbumManager::instance()
{
    return &creator->object;
}

AlbumManager::AlbumManager()
            : d(new AlbumManagerPriv)
{
    internalInstance = this;

    // these operations are pretty fast, no need for long queuing
    d->scanPAlbumsTimer = new QTimer(this);
    d->scanPAlbumsTimer->setInterval(50);
    d->scanPAlbumsTimer->setSingleShot(true);
    connect(d->scanPAlbumsTimer, SIGNAL(timeout()), this, SLOT(scanPAlbums()));

    d->scanTAlbumsTimer = new QTimer(this);
    d->scanTAlbumsTimer->setInterval(50);
    d->scanTAlbumsTimer->setSingleShot(true);
    connect(d->scanTAlbumsTimer, SIGNAL(timeout()), this, SLOT(scanTAlbums()));

    d->scanSAlbumsTimer = new QTimer(this);
    d->scanSAlbumsTimer->setInterval(50);
    d->scanSAlbumsTimer->setSingleShot(true);
    connect(d->scanSAlbumsTimer, SIGNAL(timeout()), this, SLOT(scanSAlbums()));

    d->updatePAlbumsTimer = new QTimer(this);
    d->updatePAlbumsTimer->setInterval(50);
    d->updatePAlbumsTimer->setSingleShot(true);
    connect(d->updatePAlbumsTimer, SIGNAL(timeout()), this, SLOT(updateChangedPAlbums()));

    // this operation is much more expensive than the other scan methods
    d->scanDAlbumsTimer = new QTimer(this);
    d->scanDAlbumsTimer->setInterval(5000);
    d->scanDAlbumsTimer->setSingleShot(true);
    connect(d->scanDAlbumsTimer, SIGNAL(timeout()), this, SLOT(scanDAlbums()));

    // moderately expensive
    d->albumItemCountTimer = new QTimer(this);
    d->albumItemCountTimer->setInterval(1000);
    d->albumItemCountTimer->setSingleShot(true);
    connect(d->albumItemCountTimer, SIGNAL(timeout()), this, SLOT(getAlbumItemsCount()));

    // more expensive
    d->tagItemCountTimer = new QTimer(this);
    d->tagItemCountTimer->setInterval(2500);
    d->tagItemCountTimer->setSingleShot(true);
    connect(d->tagItemCountTimer, SIGNAL(timeout()), this, SLOT(getTagItemsCount()));
}

AlbumManager::~AlbumManager()
{
    cleanUp();

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

    delete d->dirWatch;
    d->dirWatch = 0;
}

bool AlbumManager::databaseEqual(const QString &dbPath) const
{
    return d->dbPath == dbPath;
}

bool AlbumManager::setDatabase(const QString &dbPath, bool priority)
{
    if (dbPath.isEmpty())
        return false;

    // This is to ensure that the setup does not overrule the command line.
    // Replace with a better solution?
    if (priority)
    {
        d->hasPriorizedDbPath = true;
    }
    else if (d->hasPriorizedDbPath && !d->dbPath.isNull())
    {
        // ignore change without priority
        // true means, don't exit()
        return true;
    }

    if (d->dbPath == dbPath)
        return true;

    d->dbPath  = dbPath;
    d->changed = true;

    disconnect(CollectionManager::instance(), 0, this, 0);
    if (DatabaseAccess::databaseWatch())
        disconnect(DatabaseAccess::databaseWatch(), 0, this, 0);
    d->dbPathModificationDateList.clear();

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

    delete d->dirWatch;
    d->dirWatch = 0;

    d->currentAlbum = 0;
    emit signalAlbumCurrentChanged(0);
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

    DatabaseAccess::setParameters(Digikam::DatabaseParameters::parametersForSQLiteDefaultFile(d->dbPath),
                                  DatabaseAccess::MainApplication);

    ScanController::Advice advice = ScanController::instance()->databaseInitialization();

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
                KMessageBox::error(0, i18n("<p>Failed to open the database. "
                                        " Error message from database: %1 "
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
        kDebug(50003) << "No locale found in database" << endl;

        // Copy an existing locale from the settings file (used < 0.8)
        // to the database.
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group = config->group("General Settings");
        if (group.hasKey("Locale"))
        {
            kDebug(50003) << "Locale found in configfile" << endl;
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
            kDebug(50003) << "No locale found in config file"  << endl;
            dbLocale = currLocale;

            localeChanged = false;
            DatabaseAccess().db()->setSetting("Locale",dbLocale);
        }
    }
    else
    {
        if (dbLocale == currLocale)
            localeChanged = false;
    }

    if (localeChanged)
    {
        // TODO it would be better to replace all yes/no confirmation dialogs with ones that has custom
        // buttons that denote the actions directly, i.e.:  ["Ignore and Continue"]  ["Adjust locale"]
        int result =
            KMessageBox::warningYesNo(0,
                                      i18n("Your locale has changed since this "
                                           "album was last opened.\n"
                                           "Old Locale : %1, New Locale : %2\n"
                                           "If you changed your locale lately, this is all right.\n"
                                           "Please notice that if you switched to a locale "
                                           "that does not support some of the file names in your collection, "
                                           "these files may no longer be found in the collection. "
                                           "If you are sure that you want to "
                                           "continue, click 'Yes'. "
                                           "Otherwise, click 'No' and correct your "
                                           "locale setting before restarting digiKam.",
                                           dbLocale, currLocale));
        if (result != KMessageBox::Yes)
            exit(0);

        DatabaseAccess().db()->setSetting("Locale",currLocale);
    }

    // set an initial modification list to filter out KDirWatch signals caused by database operations
    DatabaseParameters params = DatabaseAccess::parameters();
    if (params.isSQLite())
    {
        QFileInfo dbFile(params.SQLiteDatabaseFile());
        d->dbPathModificationDateList = d->buildDirectoryModList(dbFile);
    }

    return true;
}

void AlbumManager::startScan()
{
    if (!d->changed)
        return;
    d->changed = false;

    // create dir watch
    d->dirWatch = new KDirWatch(this);
    connect(d->dirWatch, SIGNAL(dirty(const QString&)),
            this, SLOT(slotDirty(const QString&)));

    KDirWatch::Method m = d->dirWatch->internalMethod();
    QString mName("FAM");
    if (m == KDirWatch::DNotify)
        mName = QString("DNotify");
    else if (m == KDirWatch::Stat)
        mName = QString("Stat");
    else if (m == KDirWatch::INotify)
        mName = QString("INotify");
    kDebug(50003) << "KDirWatch method = " << mName << endl;

    // create root albums
    d->rootPAlbum = new PAlbum(i18n("My Albums"));
    insertPAlbum(d->rootPAlbum, 0);

    d->rootTAlbum = new TAlbum(i18n("My Tags"), 0, true);
    insertTAlbum(d->rootTAlbum, 0);

    d->rootSAlbum = new SAlbum(i18n("My Searches"), 0, true);

    d->rootDAlbum = new DAlbum(QDate(), true);

    // create albums for album roots
    QList<CollectionLocation> locations = CollectionManager::instance()->allAvailableLocations();
    foreach(const CollectionLocation location, locations)
        addAlbumRoot(location);

    // listen to location status changes
    connect(CollectionManager::instance(), SIGNAL(locationStatusChanged(const CollectionLocation &, int)),
            this, SLOT(slotCollectionLocationStatusChanged(const CollectionLocation &, int)));
    connect(CollectionManager::instance(), SIGNAL(locationPropertiesChanged(const CollectionLocation &)),
            this, SLOT(slotCollectionLocationPropertiesChanged(const CollectionLocation &)));

    // reload albums
    refresh();

    // listen to album database changes
    connect(DatabaseAccess::databaseWatch(), SIGNAL(albumChange(const AlbumChangeset &)),
            this, SLOT(slotAlbumChange(const AlbumChangeset &)));
    connect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(const TagChangeset &)),
            this, SLOT(slotTagChange(const TagChangeset &)));
    connect(DatabaseAccess::databaseWatch(), SIGNAL(searchChange(const SearchChangeset &)),
            this, SLOT(slotSearchChange(const SearchChangeset &)));
    // listen to collection image changes
    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset &)),
            this, SLOT(slotCollectionImageChange(const CollectionImageChangeset &)));
    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(const ImageTagChangeset &)),
            this, SLOT(slotImageTagChange(const ImageTagChangeset &)));

    emit signalAllAlbumsLoaded();
}

void AlbumManager::slotCollectionLocationStatusChanged(const CollectionLocation &location, int oldStatus)
{
    // not before initialization
    if (!d->rootPAlbum)
        return;

    if (location.status() == CollectionLocation::LocationAvailable
        && oldStatus != CollectionLocation::LocationAvailable)
    {
        addAlbumRoot(location);
        // New albums have possibly appeared
        scanPAlbums();
    }
    else if (oldStatus == CollectionLocation::LocationAvailable
             && location.status() != CollectionLocation::LocationAvailable)
    {
        removeAlbumRoot(location);
        // Albums have possibly disappeared
        scanPAlbums();
    }
}

void AlbumManager::slotCollectionLocationPropertiesChanged(const CollectionLocation &location)
{
    PAlbum *album = d->albumRootAlbumHash.value(location.id());
    if (album)
    {
        QString newLabel = d->labelForAlbumRootAlbum(location);
        kDebug() << newLabel << album->title();
        if (album->title() != newLabel)
        {
            album->setTitle(newLabel);
            emit signalAlbumRenamed(album);
        }
    }
}

void AlbumManager::addAlbumRoot(const CollectionLocation &location)
{
    if (!d->dirWatch->contains(location.albumRootPath()))
        d->dirWatch->addDir(location.albumRootPath(), KDirWatch::WatchSubDirs);

    PAlbum *album = d->albumRootAlbumHash.value(location.id());
    if (!album)
    {
        // Create a PAlbum for the Album Root.
        QString label = d->labelForAlbumRootAlbum(location);
        album = new PAlbum(location.id(), label);

        // insert album root created into hash and tree
        d->albumRootAlbumHash.insert(location.id(), album);
        insertPAlbum(album, d->rootPAlbum);
    }
}

void AlbumManager::removeAlbumRoot(const CollectionLocation &location)
{
    d->dirWatch->removeDir(location.albumRootPath());
    // retrieve and remove from hash
    PAlbum *album = d->albumRootAlbumHash.take(location.id());
    if (album)
    {
        // delete album and all its children
        removePAlbum(album);
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
    QHash<int, PAlbum *> oldAlbums;
    AlbumIterator it(d->rootPAlbum);
    while (it.current())
    {
        PAlbum* a = (PAlbum*)(*it);
        // Album root album have -1 immediately after their creation.
        // We want to recognize them as new albums then.
        if (a->id() != -1)
            oldAlbums[a->id()] = a;
        ++it;
    }

    // scan db and get a list of all albums
    QList<AlbumInfo> currentAlbums = DatabaseAccess().db()->scanAlbums();

    // sort by relative path so that parents are created before children
    qSort(currentAlbums);

    QList<AlbumInfo> newAlbums;

    // go through all the Albums and see which ones are already present
    foreach (const AlbumInfo &info, currentAlbums)
    {
        // check that location of album is available
        if (CollectionManager::instance()->locationForAlbumRootId(info.albumRootId).isAvailable())
        {
            if (oldAlbums.contains(info.id))
                oldAlbums.remove(info.id);
            else
                newAlbums << info;
        }
    }

    // now oldAlbums contains all the deleted albums and
    // newAlbums contains all the new albums

    // delete old albums, informing all frontends

    // The albums have to be removed with children being removed first,
    // removePAlbum takes care of that.
    // So we only feed it the albums from oldAlbums topmost in hierarchy.
    QSet<PAlbum *> topMostOldAlbums;
    foreach (PAlbum *album, oldAlbums)
    {
        if (!album->parent() || !oldAlbums.contains(album->parent()->id()))
            topMostOldAlbums << album;
    }

    foreach(PAlbum *album, topMostOldAlbums)
    {
        // this might look like there is memory leak here, since removePAlbum
        // doesn't delete albums and looks like child Albums don't get deleted.
        // But when the parent album gets deleted, the children are also deleted.
        removePAlbum(album);
    }

    // sort by relative path so that parents are created before children
    qSort(newAlbums);

    // create all new albums
    foreach (const AlbumInfo &info, newAlbums)
    {
        if (info.relativePath.isEmpty())
            continue;

        PAlbum *album, *parent;
        bool needInsert;
        if (info.relativePath == "/")
        {
            // Albums that represent the root directory of an album root
            // We have them as here new albums first time after their creation

            parent = d->rootPAlbum;
            album  = d->albumRootAlbumHash.value(info.albumRootId);
            needInsert = false;

            if (!album)
            {
                kError(50003) << "Did not find album root album in hash" << endl;
                continue;
            }

            // remove from hashes
            d->albumPathHash.remove(album);
            d->allAlbumsIdHash.remove(album->globalID());

            // it has been created from the collection location
            // with album root id, parentPath "/" and a name, but no album id yet.
            album->m_id = info.id;

            // update hashes after setting id
            d->albumPathHash[album] = album;
            d->allAlbumsIdHash[album->globalID()] = album;
        }
        else
        {
            // last section, no slash
            QString name = info.relativePath.section('/', -1, -1);
            // all but last sections, leading slash, no trailing slash
            QString parentPath = info.relativePath.section('/', 0, -2);

            if (parentPath.isEmpty())
                parent = d->albumRootAlbumHash.value(info.albumRootId);
            else
                parent = d->albumPathHash.value(PAlbumPath(info.albumRootId, parentPath));

            if (!parent)
            {
                kError(50003) <<  "Could not find parent with url: "
                              << parentPath << " for: " << info.relativePath << endl;
                continue;
            }

            // Create the new album
            album       = new PAlbum(info.albumRootId, parentPath, name, info.id);
            needInsert  = true;
        }

        album->m_caption  = info.caption;
        album->m_category = info.category;
        album->m_date     = info.date;

        if (info.iconAlbumRootId)
        {
            QString albumRootPath = CollectionManager::instance()->albumRootPath(info.iconAlbumRootId);
            if (!albumRootPath.isNull())
                album->m_icon = albumRootPath + info.iconRelativePath;
        }

        if (needInsert)
            insertPAlbum(album, parent);
    }

    getAlbumItemsCount();
}

void AlbumManager::updateChangedPAlbums()
{
    d->updatePAlbumsTimer->stop();

    // scan db and get a list of all albums
    QList<AlbumInfo> currentAlbums = DatabaseAccess().db()->scanAlbums();

    // Find the AlbumInfo for each id in changedPAlbums
    foreach (int id, d->changedPAlbums)
    {
        foreach (const AlbumInfo &info, currentAlbums)
        {
            if (info.id == id)
            {
                d->changedPAlbums.remove(info.id);

                PAlbum *album = findPAlbum(info.id);
                if (album)
                {
                    // Renamed?
                    if (info.relativePath != "/")
                    {
                        // last section, no slash
                        QString name = info.relativePath.section('/', -1, -1);
                        if (name != album->title())
                        {
                            album->setTitle(name);
                            updateAlbumPathHash();
                            emit signalAlbumRenamed(album);
                        }
                    }

                    // Update caption, collection, date
                    album->m_caption = info.caption;
                    album->m_category  = info.category;
                    album->m_date    = info.date;

                    // Icon changed?
                    QString icon;
                    if (info.iconAlbumRootId)
                    {
                        QString albumRootPath = CollectionManager::instance()->albumRootPath(info.iconAlbumRootId);
                        if (!albumRootPath.isNull())
                            icon = albumRootPath + info.iconRelativePath;
                    }
                    if (icon != album->m_icon)
                    {
                        album->m_icon = icon;
                        emit signalAlbumIconChanged(album);
                    }
                }
            }
        }
    }
}

void AlbumManager::getAlbumItemsCount()
{
    d->albumItemCountTimer->stop();

    if (!AlbumSettings::instance()->getShowFolderTreeViewItemsCount())
        return;

    // List albums using kioslave

    if (d->albumListJob)
    {
        d->albumListJob->kill();
        d->albumListJob = 0;
    }

    DatabaseUrl u = DatabaseUrl::albumUrl();

    d->albumListJob = ImageLister::startListJob(u);
    d->albumListJob->addMetaData("folders", "true");

    connect(d->albumListJob, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumsJobResult(KJob*)));

    connect(d->albumListJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotAlbumsJobData(KIO::Job*, const QByteArray&)));
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
        for (TagInfo::List::iterator iter = tList.begin(); iter != tList.end(); ++iter)
        {
            TagInfo info  = *iter;
            TAlbum* album = new TAlbum(info.name, info.id);
            if (info.icon.isNull())
            {
                // album image icon
                QString albumRootPath = CollectionManager::instance()->albumRootPath(info.iconAlbumRootId);
                album->m_icon         = albumRootPath + info.iconRelativePath;
            }
            else
            {
                // system icon
                album->m_icon = info.icon;
            }
            album->m_pid = info.pid;
            tagHash.insert(info.id, album);
        }
        tList.clear();

        // also add root tag
        TAlbum* rootTag = new TAlbum("root", 0, true);
        tagHash.insert(0, rootTag);

        // build tree
        for (QHash<int, TAlbum*>::iterator iter = tagHash.begin() ; iter != tagHash.end(); ++iter )
        {
            TAlbum* album = *iter;
            if (album->m_id == 0)
                continue;

            TAlbum* parent = tagHash.value(album->m_pid);
            if (parent)
            {
                album->setParent(parent);
            }
            else
            {
                kWarning(50003) << "Failed to find parent tag for tag "
                                << album->m_title
                                << " with pid "
                                << album->m_pid << endl;
            }
        }

        // now insert the items into the list. becomes sorted
        AlbumIterator it(rootTag);
        while (it.current())
        {
            TagInfo info;
            TAlbum* album = (TAlbum*)it.current();
            info.id       = album->m_id;
            info.pid      = album->m_pid;
            info.name     = album->m_title;
            info.icon     = album->m_icon;
            tList.append(info);
            ++it;
        }

        // this will also delete all child albums
        delete rootTag;
    }

    for (TagInfo::List::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TagInfo info = *it;

        // check if we have already added this tag
        if (tmap.contains(info.id))
            continue;

        // Its a new album. Find the parent of the album
        TagMap::iterator iter = tmap.find(info.pid);
        if (iter == tmap.end())
        {
            kWarning(50003) << "Failed to find parent tag for tag "
                            << info.name
                            << " with pid "
                            << info.pid << endl;
            continue;
        }

        TAlbum* parent = iter.value();

        // Create the new TAlbum
        TAlbum* album = new TAlbum(info.name, info.id, false);
        album->m_icon = info.icon;
        insertTAlbum(album, parent);

        // also insert it in the map we are doing lookup of parent tags
        tmap.insert(info.id, album);
    }

    getTagItemsCount();
}

void AlbumManager::getTagItemsCount()
{
    d->tagItemCountTimer->stop();

    if (!AlbumSettings::instance()->getShowFolderTreeViewItemsCount())
        return;

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

    connect(d->tagListJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotTagsJobData(KIO::Job*, const QByteArray&)));
}

void AlbumManager::scanSAlbums()
{
    d->scanSAlbumsTimer->stop();

    // list SAlbums directly from the db
    // first insert all the current SAlbums into a map for quick lookup
    typedef QMap<int, SAlbum*> SearchMap;
    SearchMap sMap;

    AlbumIterator it(d->rootSAlbum);
    while (it.current())
    {
        SAlbum* t = (SAlbum*)(*it);
        sMap.insert(t->id(), t);
        ++it;
    }

    // Retrieve the list of searches from the database
    SearchInfo::List sList = DatabaseAccess().db()->scanSearches();

    for (SearchInfo::List::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        SearchInfo info = *it;

        // check if we have already added this search
        if (sMap.contains(info.id))
            continue;

        // Its a new album.
        SAlbum* album = new SAlbum(info.name, info.id);
        emit signalAlbumAboutToBeAdded(album, d->rootSAlbum, d->rootSAlbum->lastChild());
        album->setSearch(info.type, info.query);
        album->setParent(d->rootSAlbum);
        d->allAlbumsIdHash[album->globalID()] = album;
        emit signalAlbumAdded(album);
    }
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

    DatabaseUrl u = DatabaseUrl::dateUrl();

    d->dateListJob = ImageLister::startListJob(u);
    d->dateListJob->addMetaData("folders", "true");

    connect(d->dateListJob, SIGNAL(result(KJob*)),
            this, SLOT(slotDatesJobResult(KJob*)));

    connect(d->dateListJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotDatesJobData(KIO::Job*, const QByteArray&)));
}

AlbumList AlbumManager::allPAlbums() const
{
    AlbumList list;
    if (d->rootPAlbum)
        list.append(d->rootPAlbum);

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
        list.append(d->rootTAlbum);

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
        list.append(d->rootSAlbum);

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
        list.append(d->rootDAlbum);

    AlbumIterator it(d->rootDAlbum);
    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

void AlbumManager::setCurrentAlbum(Album *album)
{
    d->currentAlbum = album;
    emit signalAlbumCurrentChanged(album);
}

Album* AlbumManager::currentAlbum() const
{
    return d->currentAlbum;
}

PAlbum* AlbumManager::findPAlbum(const KUrl& url) const
{
    CollectionLocation location = CollectionManager::instance()->locationForUrl(url);
    if (location.isNull())
        return 0;
    return d->albumPathHash.value(PAlbumPath(location.id(), CollectionManager::instance()->album(location, url)));
}

PAlbum* AlbumManager::findPAlbum(int id) const
{
    if (!d->rootPAlbum)
        return 0;

    int gid = d->rootPAlbum->globalID() + id;

    return (PAlbum*)(d->allAlbumsIdHash.value(gid));
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    if (!d->rootTAlbum)
        return 0;

    int gid = d->rootTAlbum->globalID() + id;

    return (TAlbum*)(d->allAlbumsIdHash.value(gid));
}

SAlbum* AlbumManager::findSAlbum(int id) const
{
    if (!d->rootTAlbum)
        return 0;

    int gid = d->rootSAlbum->globalID() + id;

    return (SAlbum*)(d->allAlbumsIdHash.value(gid));
}

DAlbum* AlbumManager::findDAlbum(int id) const
{
    if (!d->rootDAlbum)
        return 0;

    int gid = d->rootDAlbum->globalID() + id;

    return (DAlbum*)(d->allAlbumsIdHash.value(gid));
}

Album* AlbumManager::findAlbum(int gid) const
{
    return d->allAlbumsIdHash.value(gid);
}

TAlbum* AlbumManager::findTAlbum(const QString &tagPath) const
{
    // handle gracefully with or without leading slash
    bool withLeadingSlash = tagPath.startsWith('/');
    AlbumIterator it(d->rootTAlbum);
    while (it.current())
    {
        TAlbum *talbum = static_cast<TAlbum *>(*it);
        if (talbum->tagPath(withLeadingSlash) == tagPath)
            return talbum;
        ++it;
    }
    return 0;

}

SAlbum* AlbumManager::findSAlbum(const QString &name) const
{
    for (Album* album = d->rootSAlbum->firstChild(); album; album = album->next())
    {
        if (album->title() == name)
            return (SAlbum*)album;
    }
    return 0;
}

void AlbumManager::addGuardedPointer(Album *album, Album **pointer)
{
    if (album)
        d->guardedPointers.insert(album, pointer);
}

void AlbumManager::removeGuardedPointer(Album *album, Album **pointer)
{
    if (album)
        d->guardedPointers.remove(album, pointer);
}

void AlbumManager::changeGuardedPointer(Album *oldAlbum, Album *album, Album **pointer)
{
    if (oldAlbum)
        d->guardedPointers.remove(oldAlbum, pointer);
    if (album)
        d->guardedPointers.insert(album, pointer);
}

void AlbumManager::invalidateGuardedPointers(Album *album)
{
    if (!album)
        return;
    QMultiHash<Album*, Album**>::iterator it = d->guardedPointers.find(album);
    for( ; it != d->guardedPointers.end() && it.key() == album; ++it)
    {
        if (it.value())
            *(it.value()) = 0;
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

PAlbum* AlbumManager::createPAlbum(const CollectionLocation &location, const QString& name,
                                   const QString& caption, const QDate& date,
                                   const QString& category,
                                   QString& errMsg)
{
    if (location.isNull() || !location.isAvailable())
    {
        errMsg = i18n("The collection location supplied is invalid or currently not available.");
        return 0;
    }

    PAlbum *album = d->albumRootAlbumHash.value(location.id());

    if (!album)
    {
        errMsg = "No album for collection location: Internal error";
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

    QString albumPath = parent->isAlbumRoot() ? ('/' + name) : (parent->albumPath() + '/' + name);
    int albumRootId   = parent->albumRootId();

    // first check if we have a sibling album with the same name
    PAlbum *child = (PAlbum *)parent->m_firstChild;
    while (child)
    {
        if (child->albumRootId() == albumRootId && child->albumPath() == albumPath)
        {
            errMsg = i18n("An existing album has the same name.");
            return 0;
        }
        child = (PAlbum *)child->m_next;
    }

    DatabaseUrl url = parent->databaseUrl();
    url.addPath(name);
    KUrl fileUrl    = url.fileUrl();

    if (!KIO::NetAccess::mkdir(fileUrl, qApp->activeWindow()))
    {
        errMsg = i18n("Failed to create directory,");
        return 0;
    }

    ChangingDB changing(d);
    int id = DatabaseAccess().db()->addAlbum(albumRootId, albumPath, caption, date, category);

    if (id == -1)
    {
        errMsg = i18n("Failed to add album to database");
        return 0;
    }

    QString parentPath;
    if (!parent->isAlbumRoot())
        parentPath = parent->albumPath();

    PAlbum *album    = new PAlbum(albumRootId, parentPath, name, id);
    album->m_caption = caption;
    album->m_category  = category;
    album->m_date    = date;

    insertPAlbum(album, parent);

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
    Album *sibling = album->m_parent->m_firstChild;
    while (sibling)
    {
        if (sibling->title() == newName)
        {
            errMsg = i18n("Another album with same name exists\n"
                          "Please choose another name");
            return false;
        }
        sibling = sibling->m_next;
    }

    QString oldAlbumPath = album->albumPath();

    KUrl u = album->fileUrl();
    u      = u.upUrl();
    u.addPath(newName);

    //TODO: Use KIO::NetAccess
    if (::rename(QFile::encodeName(album->folderPath()),
                 QFile::encodeName(u.path(KUrl::RemoveTrailingSlash))) != 0)
    {
        errMsg = i18n("Failed to rename Album");
        return false;
    }

    // now rename the album and subalbums in the database

    album->setTitle(newName);
    {
        DatabaseAccess access;
        ChangingDB changing(d);
        access.db()->renameAlbum(album->id(), album->albumRootId(), album->albumPath());

        PAlbum* subAlbum = 0;
        AlbumIterator it(album);
        while ((subAlbum = static_cast<PAlbum*>(it.current())) != 0)
        {
            access.db()->renameAlbum(subAlbum->id(), subAlbum->albumRootId(), subAlbum->albumPath());
            ++it;
        }
    }

    updateAlbumPathHash();
    emit signalAlbumRenamed(album);

    return true;
}

void AlbumManager::updateAlbumPathHash()
{
    // Update AlbumDict. basically clear it and rebuild from scratch
    {
        d->albumPathHash.clear();
        AlbumIterator it(d->rootPAlbum);
        PAlbum* subAlbum = 0;
        while ((subAlbum = (PAlbum*)it.current()) != 0)
        {
            d->albumPathHash[subAlbum] = subAlbum;
            ++it;
        }
    }

}

bool AlbumManager::updatePAlbumIcon(PAlbum *album, qlonglong iconID, QString& errMsg)
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
        QString iconRelativePath;
        int iconAlbumRootId;
        if (access.db()->getAlbumIcon(album->id(), &iconAlbumRootId, &iconRelativePath))
        {
            QString albumRootPath = CollectionManager::instance()->albumRootPath(iconAlbumRootId);
            album->m_icon = albumRootPath + iconRelativePath;
        }
        else
            album->m_icon = QString();
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
    Album *child = parent->m_firstChild;
    while (child)
    {
        if (child->title() == name)
        {
            errMsg = i18n("Tag name already exists");
            return 0;
        }
        child = child->m_next;
    }

    ChangingDB changing(d);
    int id = DatabaseAccess().db()->addTag(parent->id(), name, iconkde, 0);
    if (id == -1)
    {
        errMsg = i18n("Failed to add tag to database");
        return 0;
    }

    TAlbum *album = new TAlbum(name, id, false);
    album->m_icon = iconkde;

    insertTAlbum(album, parent);

    return album;
}

AlbumList AlbumManager::findOrCreateTAlbums(const QStringList &tagPaths)
{
    // find tag ids for tag paths in list, create if they don't exist
    QList<int> tagIDs = DatabaseAccess().db()->getTagsFromTagPaths(tagPaths, true);

    // create TAlbum objects for the newly created tags
    scanTAlbums();

    AlbumList resultList;

    for (QList<int>::iterator it = tagIDs.begin() ; it != tagIDs.end() ; ++it)
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

    return true;
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
    Album *sibling = album->m_parent->m_firstChild;
    while (sibling)
    {
        if (sibling->title() == name)
        {
            errMsg = i18n("Another tag with same name exists\n"
                          "Please choose another name");
            return false;
        }
        sibling = sibling->m_next;
    }

    ChangingDB changing(d);
    DatabaseAccess().db()->setTagName(album->id(), name);
    album->setTitle(name);
    emit signalAlbumRenamed(album);

    return true;
}

bool AlbumManager::moveTAlbum(TAlbum* album, TAlbum *newParent, QString &errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot move root tag");
        return false;
    }

    ChangingDB changing(d);
    DatabaseAccess().db()->setTagParentID(album->id(), newParent->id());
    album->parent()->removeChild(album);
    album->setParent(newParent);

    emit signalTAlbumMoved(album, newParent);

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
        int albumRootId;
        if (access.db()->getTagIcon(album->id(), &albumRootId, &albumRelativePath, &iconKDE))
        {
            if (iconKDE.isEmpty())
            {
                QString albumRootPath = CollectionManager::instance()->albumRootPath(albumRootId);
                album->m_icon = albumRootPath + albumRelativePath;
            }
            else
            {
                album->m_icon = iconKDE;
            }
        }
        else
            album->m_icon = QString();
    }

    emit signalAlbumIconChanged(album);

    return true;
}

AlbumList AlbumManager::getRecentlyAssignedTags() const
{
    QList<int> tagIDs = DatabaseAccess().db()->getRecentlyAssignedTags();

    AlbumList resultList;

    for (QList<int>::iterator it = tagIDs.begin() ; it != tagIDs.end() ; ++it)
    {
        resultList.append(findTAlbum(*it));
    }

    return resultList;
}

QStringList AlbumManager::tagPaths(const QList<int> &tagIDs, bool leadingSlash) const
{
    QStringList tagPaths;

    for (QList<int>::const_iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
    {
        TAlbum *album = findTAlbum(*it);
        if (album)
        {
            tagPaths.append(album->tagPath(leadingSlash));
        }
    }

    return tagPaths;
}

QStringList AlbumManager::tagNames(const QList<int> &tagIDs) const
{
    QStringList tagNames;

    foreach(int id, tagIDs)
    {
        TAlbum *album = findTAlbum(id);
        if (album)
        {
            tagNames << album->title();
        }
    }

    return tagNames;
}

SAlbum* AlbumManager::createSAlbum(const QString &name, DatabaseSearch::Type type, const QString &query)
{
    // first iterate through all the search albums and see if there's an existing
    // SAlbum with same name. (Remember, SAlbums are arranged in a flat list)
    SAlbum *album = findSAlbum(name);
    ChangingDB changing(d);
    if (album)
    {
        album->setSearch(type, query);
        DatabaseAccess access;
        access.db()->updateSearch(album->id(), album->m_type, name, query);
        return album;
    }

    int id = DatabaseAccess().db()->addSearch(type, name, query);

    if (id == -1)
        return 0;

    album = new SAlbum(name, id);
    emit signalAlbumAboutToBeAdded(album, d->rootSAlbum, d->rootSAlbum->lastChild());
    album->setSearch(type, query);
    album->setParent(d->rootSAlbum);

    d->allAlbumsIdHash.insert(album->globalID(), album);
    emit signalAlbumAdded(album);

    return album;
}

bool AlbumManager::updateSAlbum(SAlbum* album, const QString &changedQuery,
                                const QString &changedName, DatabaseSearch::Type type)
{
    if (!album)
        return false;

    QString newName = changedName.isNull() ? album->title() : changedName;
    DatabaseSearch::Type newType = (type == DatabaseSearch::UndefinedType) ? album->type() : type;

    ChangingDB changing(d);
    DatabaseAccess().db()->updateSearch(album->id(), newType, newName, changedQuery);

    QString oldName = album->title();

    album->setSearch(newType, changedQuery);
    album->setTitle(newName);
    if (oldName != album->title())
        emit signalAlbumRenamed(album);

    return true;
}

bool AlbumManager::deleteSAlbum(SAlbum* album)
{
    if (!album)
        return false;

    emit signalAlbumAboutToBeDeleted(album);

    ChangingDB changing(d);
    DatabaseAccess().db()->deleteSearch(album->id());

    d->allAlbumsIdHash.remove(album->globalID());
    emit signalAlbumDeleted(album);
    delete album;
    emit signalAlbumHasBeenDeleted(album);

    return true;
}

void AlbumManager::insertPAlbum(PAlbum *album, PAlbum *parent)
{
    if (!album)
        return;

    emit signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : 0);

    if (parent)
        album->setParent(parent);

    d->albumPathHash[album]  = album;
    d->allAlbumsIdHash[album->globalID()] = album;

    emit signalAlbumAdded(album);
}

void AlbumManager::removePAlbum(PAlbum *album)
{
    if (!album)
        return;

    // remove all children of this album
    Album* child = album->m_firstChild;
    while (child)
    {
        Album *next = child->m_next;
        removePAlbum((PAlbum*)child);
        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->albumPathHash.remove(album);
    d->allAlbumsIdHash.remove(album->globalID());

    DatabaseUrl url = album->databaseUrl();

    if (album == d->currentAlbum)
    {
        d->currentAlbum = 0;
        emit signalAlbumCurrentChanged(0);
    }

    emit signalAlbumDeleted(album);
    delete album;
    emit signalAlbumHasBeenDeleted(album);
}

void AlbumManager::insertTAlbum(TAlbum *album, TAlbum *parent)
{
    if (!album)
        return;

    emit signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : 0);

    if (parent)
        album->setParent(parent);

    d->allAlbumsIdHash.insert(album->globalID(), album);

    emit signalAlbumAdded(album);
}

void AlbumManager::removeTAlbum(TAlbum *album)
{
    if (!album)
        return;

    // remove all children of this album
    Album* child = album->m_firstChild;
    while (child)
    {
        Album *next = child->m_next;
        removeTAlbum((TAlbum*)child);
        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->allAlbumsIdHash.remove(album->globalID());

    if (album == d->currentAlbum)
    {
        d->currentAlbum = 0;
        emit signalAlbumCurrentChanged(0);
    }

    emit signalAlbumDeleted(album);
    delete album;
    emit signalAlbumHasBeenDeleted(album);
}

void AlbumManager::notifyAlbumDeletion(Album *album)
{
    invalidateGuardedPointers(album);
}

void AlbumManager::emitAlbumItemsSelected(bool val)
{
    emit signalAlbumItemsSelected(val);
}

void AlbumManager::setItemHandler(AlbumItemHandler *handler)
{
    d->itemHandler = handler;
}

AlbumItemHandler* AlbumManager::getItemHandler()
{
    return d->itemHandler;
}

void AlbumManager::refreshItemHandler(const KUrl::List& itemList)
{
    if (itemList.empty())
        d->itemHandler->refresh();
    else
        d->itemHandler->refreshItems(itemList);
}

void AlbumManager::slotAlbumsJobResult(KJob* job)
{
    d->albumListJob = 0;

    if (job->error())
    {
        kWarning(50003) << k_funcinfo << "Failed to list albums" << endl;
        return;
    }
}

void AlbumManager::slotAlbumsJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    QMap<int, int> albumsStatMap;
    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);
    ds >> albumsStatMap;

    emit signalPAlbumsDirty(albumsStatMap);
}

void AlbumManager::slotTagsJobResult(KJob* job)
{
    d->tagListJob = 0;

    if (job->error())
    {
        kWarning(50003) << k_funcinfo << "Failed to list tags" << endl;
        return;
    }
}

void AlbumManager::slotTagsJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    QMap<int, int> tagsStatMap;
    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);
    ds >> tagsStatMap;

    emit signalTAlbumsDirty(tagsStatMap);
}

void AlbumManager::slotDatesJobResult(KJob* job)
{
    d->dateListJob = 0;

    if (job->error())
    {
        kWarning(50003) << "Failed to list dates" << endl;
        return;
    }

    emit signalAllDAlbumsLoaded();
}

void AlbumManager::slotDatesJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    // insert all the DAlbums into a qmap for quick access
    QMap<QDate, DAlbum*> mAlbumMap;
    QMap<int, DAlbum*>   yAlbumMap;

    AlbumIterator it(d->rootDAlbum);
    while (it.current())
    {
        DAlbum* a = (DAlbum*)(*it);
        if (a->range() == DAlbum::Month)
            mAlbumMap.insert(a->date(), a);
        else
            yAlbumMap.insert(a->date().year(), a);
        ++it;
    }

    QMap<QDateTime, int> datesStatMap;
    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);
    ds >> datesStatMap;

    QMap<YearMonth, int> yearMonthMap;
    for ( QMap<QDateTime, int>::iterator it = datesStatMap.begin();
          it != datesStatMap.end(); ++it )
    {
        YearMonth yearMonth = YearMonth(it.key().date().year(), it.key().date().month());

        QMap<YearMonth, int>::iterator it2 = yearMonthMap.find(yearMonth);
        if ( it2 == yearMonthMap.end() )
        {
            yearMonthMap.insert( yearMonth, *it );
        }
        else
        {
            *it2 += *it;
        }
    }

    int year, month;
    for ( QMap<YearMonth, int>::iterator iter = yearMonthMap.begin();
          iter != yearMonthMap.end(); ++iter )
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
        DAlbum *yAlbum = 0;
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
        DAlbum *mAlbum = new DAlbum(md);
        emit signalAlbumAboutToBeAdded(mAlbum, yAlbum, yAlbum->lastChild());
        mAlbum->setParent(yAlbum);
        d->allAlbumsIdHash.insert(mAlbum->globalID(), mAlbum);
        emit signalAlbumAdded(mAlbum);
    }

    // Now the items contained in the maps are the ones which
    // have been deleted.
    for (QMap<QDate, DAlbum*>::iterator it = mAlbumMap.begin();
         it != mAlbumMap.end(); ++it)
    {
        DAlbum* album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        delete album;
        emit signalAlbumHasBeenDeleted(album);
    }

    for (QMap<int, DAlbum*>::iterator it = yAlbumMap.begin();
         it != yAlbumMap.end(); ++it)
    {
        DAlbum* album = it.value();
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        delete album;
        emit signalAlbumHasBeenDeleted(album);
    }

    emit signalDAlbumsDirty(yearMonthMap);
    emit signalDatesMapDirty(datesStatMap);
}

void AlbumManager::slotAlbumChange(const AlbumChangeset &changeset)
{
    if (d->changingDB || !d->rootPAlbum)
        return;

    switch(changeset.operation())
    {
        case AlbumChangeset::Added:
        case AlbumChangeset::Deleted:
            if (!d->scanPAlbumsTimer->isActive())
                d->scanPAlbumsTimer->start();
            break;
        case AlbumChangeset::Renamed:
        case AlbumChangeset::PropertiesChanged:
            // mark for rescan
            d->changedPAlbums << changeset.albumId();
            if (!d->updatePAlbumsTimer->isActive())
                d->updatePAlbumsTimer->start();
            break;
        case AlbumChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotTagChange(const TagChangeset &changeset)
{
    if (d->changingDB || !d->rootTAlbum)
        return;

    switch(changeset.operation())
    {
        case TagChangeset::Added:
        case TagChangeset::Deleted:
        case TagChangeset::Reparented:
            if (!d->scanTAlbumsTimer->isActive())
                d->scanTAlbumsTimer->start();
            break;
        case TagChangeset::Renamed:
        case TagChangeset::IconChanged:
            //TODO
            break;
        case TagChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotSearchChange(const SearchChangeset &changeset)
{
    if (d->changingDB || !d->rootSAlbum)
        return;

    switch(changeset.operation())
    {
        case SearchChangeset::Added:
        case SearchChangeset::Deleted:
            if (!d->scanSAlbumsTimer->isActive())
                d->scanSAlbumsTimer->start();
            break;
        case SearchChangeset::Changed:
            break;
        case SearchChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotCollectionImageChange(const CollectionImageChangeset &changeset)
{
    if (!d->rootDAlbum)
        return;

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
            if (!d->scanDAlbumsTimer->isActive())
                d->scanDAlbumsTimer->start();
            if (!d->albumItemCountTimer->isActive())
                d->albumItemCountTimer->start();
            break;
        default:
            break;
    }
}

void AlbumManager::slotImageTagChange(const ImageTagChangeset &changeset)
{
    if (!d->rootTAlbum)
        return;
    switch (changeset.operation())
    {
        case ImageTagChangeset::Added:
        case ImageTagChangeset::Removed:
        case ImageTagChangeset::RemovedAll:
            if (!d->tagItemCountTimer->isActive())
                d->tagItemCountTimer->start();
            break;
        default:
            break;
    }
}

void AlbumManager::slotDirty(const QString& path)
{
    kDebug(50003) << "AlbumManager::slotDirty" << path << endl;

    // Filter out dirty signals triggered by changes on the database file
    DatabaseParameters params = DatabaseAccess::parameters();
    if (params.isSQLite())
    {
        QDir dir(path);
        QFileInfo dbFile(params.SQLiteDatabaseFile());

        // is the signal for the directory containing the database file?
        if (dbFile.dir() == dir)
        {
            // retrieve modification dates
            QList<QDateTime> modList = d->buildDirectoryModList(dbFile);

            // check for equality
            if (modList == d->dbPathModificationDateList)
            {
                kDebug(50003) << "Filtering out db-file-triggered dir watch signal" << endl;
                // we can skip the signal
                return;
            }

            // set new list
            d->dbPathModificationDateList = modList;
        }
    }

    ScanController::instance()->scheduleCollectionScan(path);
}

}  // namespace Digikam
