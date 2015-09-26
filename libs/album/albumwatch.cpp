/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-11-07
 * Description : Directory watch interface
 *
 * Copyright (C) 2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albumwatch.h"

// Qt includes

#include <QDateTime>
#include <QDBusConnection>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "databaseparameters.h"
#include "kinotify.h"
#include "scancontroller.h"

namespace Digikam
{

enum Mode
{
    InotifyMode,
    QFSWatcherMode
};

class AlbumWatch::Private
{
public:

    explicit Private(AlbumWatch* const q)
        : mode(InotifyMode),
          inotify(0),
          dirWatch(0),
          connectedToKIO(false),
          q(q)
    {
    }

    bool isInotifyMode() const
    {
        return (mode == InotifyMode);
    }

    void             determineMode();
    bool             inBlackList(const QString& path) const;
    bool             inDirWatchParametersBlackList(const QFileInfo& info, const QString& path);
    QList<QDateTime> buildDirectoryModList(const QFileInfo& dbFile);

public:

    Mode                mode;

    KInotify*           inotify;
    QFileSystemWatcher* dirWatch;
    QStringList         dirWatchAddedDirs;
    bool                connectedToKIO;

    DatabaseParameters  params;
    QStringList         fileNameBlackList;
    QList<QDateTime>    dbPathModificationDateList;

    AlbumWatch* const   q;
};

void AlbumWatch::Private::determineMode()
{
    if (KInotify::available())
    {
        mode = InotifyMode;
    }
    else
    {
        mode = QFSWatcherMode;
    }
}

bool AlbumWatch::Private::inBlackList(const QString& path) const
{
    // Filter out dirty signals triggered by changes on the database file
    foreach(const QString& bannedFile, fileNameBlackList)
    {
        if (path.endsWith(bannedFile))
        {
            return true;
        }
    }

    return false;
}

bool AlbumWatch::Private::inDirWatchParametersBlackList(const QFileInfo& info, const QString& path)
{
    if (params.isSQLite())
    {
        QDir dir;

        if (info.isDir())
        {
            dir = QDir(path);
        }
        else
        {
            dir = info.dir();
        }

        QFileInfo dbFile(params.SQLiteDatabaseFile());

        // Workaround for broken KDirWatch in KDE 4.2.4
        if (path.startsWith(dbFile.filePath()))
        {
            return true;
        }

        // is the signal for the directory containing the database file?
        if (dbFile.dir() == dir)
        {
            // retrieve modification dates
            QList<QDateTime> modList = buildDirectoryModList(dbFile);

            // check for equality
            if (modList == dbPathModificationDateList)
            {
                //qCDebug(DIGIKAM_GENERAL_LOG) << "Filtering out db-file-triggered dir watch signal";
                // we can skip the signal
                return true;
            }

            // set new list
            dbPathModificationDateList = modList;
        }
    }

    return false;
}

// -------------------------------------------------------------------------------------

AlbumWatch::AlbumWatch(AlbumManager* const parent)
    : QObject(parent),
      d(new Private(this))
{
    d->determineMode();

    if (d->isInotifyMode())
    {
        connectToKInotify();
    }
    else
    {
        connectToQFSWatcher();
        connectToKIO();
    }

    connect(parent, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(parent, SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));
}

AlbumWatch::~AlbumWatch()
{
    delete d;
}

void AlbumWatch::clear()
{
    if (d->dirWatch)
    {
        foreach(const QString& addedDirectory, d->dirWatchAddedDirs)
        {
            d->dirWatch->removePath(addedDirectory);
        }

        d->dirWatchAddedDirs.clear();
    }

    if (d->connectedToKIO)
    {
        QDBusConnection::sessionBus().disconnect(QString(), QString(), QLatin1String("org.kde.KDirNotify"), QLatin1String("FileMoved"),    0, 0);
        QDBusConnection::sessionBus().disconnect(QString(), QString(), QLatin1String("org.kde.KDirNotify"), QLatin1String("FilesAdded"),   0, 0);
        QDBusConnection::sessionBus().disconnect(QString(), QString(), QLatin1String("org.kde.KDirNotify"), QLatin1String("FilesRemoved"), 0, 0);

        d->connectedToKIO = false;
    }

    if (d->inotify)
    {
        d->inotify->removeAllWatches();
    }
}

void AlbumWatch::setDatabaseParameters(const DatabaseParameters& params)
{
    d->params = params;

    d->fileNameBlackList.clear();

    // filter out notifications caused by database operations
    if (params.isSQLite())
    {
        d->fileNameBlackList << QLatin1String("thumbnails-digikam.db") << QLatin1String("thumbnails-digikam.db-journal");

        QFileInfo dbFile(params.SQLiteDatabaseFile());
        d->fileNameBlackList << dbFile.fileName() << dbFile.fileName() + QLatin1String("-journal");

        // ensure this is done after setting up the black list
        d->dbPathModificationDateList = d->buildDirectoryModList(dbFile);
    }
}

void AlbumWatch::slotAlbumAdded(Album* a)
{
    if (a->isRoot() || a->type() != Album::PHYSICAL)
    {
        return;
    }

    PAlbum* const album         = static_cast<PAlbum*>(a);
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(album->albumRootId());

    if (!location.isAvailable())
    {
        return;
    }

    QString dir = album->folderPath();

    if (dir.isEmpty())
    {
        return;
    }

    if (d->isInotifyMode())
    {
        d->inotify->watchDirectory(dir);
    }
    else
    {
        if (!d->dirWatch->directories().contains(dir))
        {
            d->dirWatchAddedDirs << dir;
            d->dirWatch->addPath(dir);
        }
    }
}

void AlbumWatch::slotAlbumAboutToBeDeleted(Album* a)
{
    if (a->isRoot() || a->type() != Album::PHYSICAL)
    {
        return;
    }

    PAlbum* const album = static_cast<PAlbum*>(a);
    QString dir         = album->folderPath();

    if (dir.isEmpty())
    {
        return;
    }

    if (d->isInotifyMode())
    {
        d->inotify->removeDirectory(dir);
    }
    else
    {
        d->dirWatch->removePath(album->folderPath());
    }
}

void AlbumWatch::rescanDirectory(const QString& dir)
{
//    qCDebug(DIGIKAM_GENERAL_LOG) << "Detected change, triggering rescan of directory" << dir;
    ScanController::instance()->scheduleCollectionScanRelaxed(dir);
}

// -- KInotify ----------------------------------------------------------------------------------

void AlbumWatch::connectToKInotify()
{
    if (d->inotify)
    {
        return;
    }

    d->inotify = new KInotify(this);

    qCDebug(DIGIKAM_GENERAL_LOG) << "AlbumWatch use KInotify";    
    
    connect( d->inotify, SIGNAL(movedFrom(QString)),
             this, SLOT(slotFileMoved(QString)) );

    connect( d->inotify, SIGNAL(movedTo(QString)),
             this, SLOT(slotFileMoved(QString)) );

/*
    connect( d->inotify, SIGNAL(moved(QString,QString)),
             this, SLOT(slotFileMoved(QString,QString)) );
*/

    connect( d->inotify, SIGNAL(deleted(QString,bool)),
             this, SLOT(slotFileDeleted(QString,bool)) );

    connect( d->inotify, SIGNAL(created(QString,bool)),
             this, SLOT(slotFileCreated(QString,bool)) );

    connect( d->inotify, SIGNAL(closedWrite(QString)),
             this, SLOT(slotFileClosedAfterWrite(QString)) );

    connect( d->inotify, SIGNAL(watchUserLimitReached()),
             this, SLOT(slotInotifyWatchUserLimitReached()) );
}

/*
 * Note that moved(QString,QString) is only emitted if both source and target are watched!
 * This does not apply for moving to trash, or files moved from/to non-collection directories
void AlbumWatch::slotFileMoved(const QString& from, const QString& to)
{
    // we could add a copyOrMoveHint here...but identical-file detection seems to work well
    rescanPath(from);
    rescanPath(to);
}*/

void AlbumWatch::slotFileMoved(const QString& path)
{
    // both movedTo and movedFrom are connected to this slot
    // moved(QString,QString) is ignored, with it the information about pairing of moves
    rescanPath(path);
}

void AlbumWatch::slotFileDeleted(const QString& path, bool isDir)
{
    Q_UNUSED(isDir);
    rescanPath(path);
}

void AlbumWatch::slotFileCreated(const QString& path, bool isDir)
{
    if (isDir)
    {
        rescanPath(path);
    }
    // for files, rely on ClosedAfterWrite only,
    // which always comes after create if the operation has finished
}

void AlbumWatch::slotFileClosedAfterWrite(const QString& path)
{
    rescanPath(path);
}

void AlbumWatch::slotInotifyWatchUserLimitReached()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Reached inotify limit";
}

void AlbumWatch::rescanPath(const QString& path)
{
    if (d->inBlackList(path))
    {
        return;
    }

    QUrl url(path);
    rescanDirectory(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path());
}

// -- QFileSystemWatcher ---------------------------------------------------------------------------------------

QList<QDateTime> AlbumWatch::Private::buildDirectoryModList(const QFileInfo& dbFile)
{
    // retrieve modification dates
    QList<QDateTime> modList;
    QFileInfoList    fileInfoList = dbFile.dir().entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    // build list
    foreach(const QFileInfo& info, fileInfoList)
    {
        // ignore digikam4.db and journal and other temporary files
        if (!fileNameBlackList.contains(info.fileName()))
        {
            modList << info.lastModified();
        }
    }

    return modList;
}

void AlbumWatch::slotQFSWatcherDirty(const QString& path)
{
    if (d->inBlackList(path))
    {
        return;
    }

    QFileInfo info(path);

    if (d->inDirWatchParametersBlackList(info, path))
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "QFileSystemWatcher detected change at" << path;

    if (info.isDir())
    {
        rescanDirectory(path);
    }
    else
    {
        rescanDirectory(info.path());
    }
}

void AlbumWatch::connectToQFSWatcher()
{
    if (d->dirWatch)
    {
        return;
    }

    d->dirWatch = new QFileSystemWatcher(this);

    qCDebug(DIGIKAM_GENERAL_LOG) << "AlbumWatch use QFileSystemWatcher";

    connect(d->dirWatch, SIGNAL(directoryChanged(QString)),
            this, SLOT(slotDirWatchDirty(QString)));
    
    connect(d->dirWatch, SIGNAL(fileChanged(QString)),
            this, SLOT(slotDirWatchDirty(QString)));
}

// -- KIO -----------------------------------------------------------------------------------------

void AlbumWatch::connectToKIO()
{
    if (d->connectedToKIO)
    {
        return;
    }

    QDBusConnection::sessionBus().connect(QString(), QString(), QLatin1String("org.kde.KDirNotify"), QLatin1String("FileMoved"),
                                          this, SLOT(slotKioFileMoved(QString,QString)));

    QDBusConnection::sessionBus().connect(QString(), QString(), QLatin1String("org.kde.KDirNotify"), QLatin1String("FilesAdded"),
                                          this, SLOT(slotKioFilesAdded(QString)));

    QDBusConnection::sessionBus().connect(QString(), QString(), QLatin1String("org.kde.KDirNotify"), QLatin1String("FilesRemoved"),
                                          this, SLOT(slotKioFilesDeleted(QStringList)));

    d->connectedToKIO = true;
}

void AlbumWatch::slotKioFileMoved(const QString& urlFrom, const QString& urlTo)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << urlFrom << urlTo;
    handleKioNotification(QUrl(urlFrom));
    handleKioNotification(QUrl(urlTo));
}

void AlbumWatch::slotKioFilesDeleted(const QStringList& urls)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << urls;

    foreach(const QString& url, urls)
    {
        handleKioNotification(QUrl(url));
    }
}

void AlbumWatch::slotKioFilesAdded(const QString& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << url;
    handleKioNotification(QUrl(url));
}

void AlbumWatch::handleKioNotification(const QUrl& url)
{
    if (url.isLocalFile())
    {
        QString path = url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path();

        //qCDebug(DIGIKAM_GENERAL_LOG) << path << !CollectionManager::instance()->albumRootPath(path).isEmpty();
        // check path is in our collection
        if (CollectionManager::instance()->albumRootPath(path).isNull())
        {
            return;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "QFileSystemWatcher detected file change at" << path;

        rescanDirectory(path);
    }
    else
    {
        DatabaseUrl dbUrl(url);

        if (dbUrl.isAlbumUrl())
        {
            QString path = dbUrl.fileUrl().adjusted(QUrl::RemoveFilename).path();
            qCDebug(DIGIKAM_GENERAL_LOG) << "QFileSystemWatcher detected file change at" << path;
            rescanDirectory(path);
        }
    }
}

} // namespace Digikam
