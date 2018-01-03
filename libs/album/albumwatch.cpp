/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-11-07
 * Description : Directory watch interface
 *
 * Copyright (C) 2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QFileSystemWatcher>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "dbengineparameters.h"
#include "scancontroller.h"

namespace Digikam
{

class AlbumWatch::Private
{
public:

    Private() :
        dirWatch(0)
    {
    }

    bool             inBlackList(const QString& path) const;
    bool             inDirWatchParametersBlackList(const QFileInfo& info, const QString& path);
    QList<QDateTime> buildDirectoryModList(const QFileInfo& dbFile) const;

public:

    QFileSystemWatcher* dirWatch;

    DbEngineParameters  params;
    QStringList         fileNameBlackList;
    QList<QDateTime>    dbPathModificationDateList;
};

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

QList<QDateTime> AlbumWatch::Private::buildDirectoryModList(const QFileInfo& dbFile) const
{
    // Retrieve modification dates

    QList<QDateTime> modList;
    QFileInfoList    fileInfoList = dbFile.dir().entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    // Build the list

    foreach(const QFileInfo& info, fileInfoList)
    {
        // Ignore digikam4.db and journal and other temporary files

        if (!fileNameBlackList.contains(info.fileName()))
        {
            modList << info.lastModified();
        }
    }

    return modList;
}

// -------------------------------------------------------------------------------------

AlbumWatch::AlbumWatch(AlbumManager* const parent)
    : QObject(parent),
      d(new Private)
{
    d->dirWatch = new QFileSystemWatcher(this);

    qCDebug(DIGIKAM_GENERAL_LOG) << "AlbumWatch use QFileSystemWatcher";

    connect(d->dirWatch, SIGNAL(directoryChanged(QString)),
            this, SLOT(slotQFSWatcherDirty(QString)));

    connect(d->dirWatch, SIGNAL(fileChanged(QString)),
            this, SLOT(slotQFSWatcherDirty(QString)));

    connect(parent, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(parent, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(parent, SIGNAL(signalAlbumNewPath(Album*)),
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
    if (d->dirWatch && !d->dirWatch->directories().isEmpty())
    {
        d->dirWatch->removePaths(d->dirWatch->directories());
    }
}

void AlbumWatch::removeWatchedPAlbums(const PAlbum* const album)
{
    if (!album)
    {
        return;
    }

    foreach(const QString& dir, d->dirWatch->directories())
    {
        if (dir.startsWith(album->folderPath()))
        {
            d->dirWatch->removePath(dir);
        }
    }
}

void AlbumWatch::setDbEngineParameters(const DbEngineParameters& params)
{
    d->params = params;

    d->fileNameBlackList.clear();

    // filter out notifications caused by database operations
    if (params.isSQLite())
    {
        d->fileNameBlackList << QLatin1String("thumbnails-digikam.db") << QLatin1String("thumbnails-digikam.db-journal");
        d->fileNameBlackList << QLatin1String("recognition.db") << QLatin1String("recognition.db-journal");

        QFileInfo dbFile(params.SQLiteDatabaseFile());
        d->fileNameBlackList << dbFile.fileName() << dbFile.fileName() + QLatin1String("-journal");

        // ensure this is done after setting up the black list
        d->dbPathModificationDateList = d->buildDirectoryModList(dbFile);
    }
}

void AlbumWatch::slotAlbumAdded(Album* a)
{
    if (a->isRoot() || a->isTrashAlbum() || a->type() != Album::PHYSICAL)
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

    d->dirWatch->addPath(dir);
}

void AlbumWatch::slotAlbumAboutToBeDeleted(Album* a)
{
    if (a->isRoot() || a->isTrashAlbum() || a->type() != Album::PHYSICAL)
    {
        return;
    }

    PAlbum* const album = static_cast<PAlbum*>(a);
    QString dir         = album->folderPath();

    if (dir.isEmpty())
    {
        return;
    }

    d->dirWatch->removePath(dir);
}

void AlbumWatch::rescanDirectory(const QString& dir)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Detected change, triggering rescan of" << dir;

    ScanController::instance()->scheduleCollectionScanRelaxed(dir);
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

    if (info.isDir())
    {
        rescanDirectory(path);
    }
    else
    {
        rescanDirectory(info.path());
    }
}

} // namespace Digikam
