/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Physical Album helpers.
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
    foreach (const AlbumInfo& info, currentAlbums)
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

    foreach (PAlbum* const album, oldAlbums)
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

    foreach (PAlbum* const album, topMostOldAlbums)
    {
        // recursively removes all children and the album
        removePAlbum(album);
    }

    // sort by relative path so that parents are created before children
    std::sort(newAlbums.begin(), newAlbums.end());

    // create all new albums
    foreach (const AlbumInfo& info, newAlbums)
    {
        if (info.relativePath.isEmpty())
        {
            continue;
        }

        PAlbum* album  = 0;
        PAlbum* parent = 0;

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
                qCDebug(DIGIKAM_GENERAL_LOG) << "Could not find parent with url: "
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
    foreach (int id, d->changedPAlbums)
    {
        foreach (const AlbumInfo& info, currentAlbums)
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

    if (name.contains(QLatin1Char('/')))
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
    PAlbum* child = static_cast<PAlbum*>(parent->firstChild());

    while (child)
    {
        if (child->albumRootId() == albumRootId && child->albumPath() == albumPath)
        {
            errMsg = i18n("An existing album has the same name.");
            return 0;
        }

        child = static_cast<PAlbum*>(child->next());
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

    if (newName.contains(QLatin1Char('/')))
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

    // We use a private shortcut around collection scanner noticing our changes,
    // we rename them directly. Faster.
    ScanController::instance()->suspendCollectionScan();

    QDir dir(album->albumRootPath() + album->m_parentPath);
    bool ret = dir.rename(album->title(), newName);

    if (!ret)
    {
        ScanController::instance()->resumeCollectionScan();

        errMsg = i18n("Failed to rename Album");
        return false;
    }

    QString oldAlbumPath = album->albumPath();
    album->setTitle(newName);
    album->m_path        = newName;
    QString newAlbumPath = album->albumPath();

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

QMap<int, int> AlbumManager::getPAlbumsCount() const
{
    return d->pAlbumsCount;
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

    d->albumPathHash[PAlbumPath(album)]   = album;
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
    Album* child        = album->firstChild();
    PAlbum* toBeRemoved = 0;

    while (child)
    {
        Album* next = child->next();
        toBeRemoved = static_cast<PAlbum*>(child);

        if (toBeRemoved)
        {
            removePAlbum(toBeRemoved);
            toBeRemoved = 0;
        }

        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->albumPathHash.remove(PAlbumPath(album));
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

void AlbumManager::removeWatchedPAlbums(const PAlbum* const album)
{
    d->albumWatch->removeWatchedPAlbums(album);
}

} // namespace Digikam
