/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Generic Album helpers.
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

void AlbumManager::setCurrentAlbums(const QList<Album*>& albums)
{
    if (albums.isEmpty())
        return;

    d->currentAlbums.clear();
    /**
     * Filter out the null pointers
    */
    foreach (Album* const album, albums)
    {
        if (album)
        {
            d->currentAlbums << album;
        }
    }
    /**
     * Sort is needed to identify selection correctly, ex AlbumHistory
     */
    std::sort(d->currentAlbums.begin(), d->currentAlbums.end());

    emit signalAlbumCurrentChanged(d->currentAlbums);
}

AlbumList AlbumManager::currentAlbums() const
{
    return d->currentAlbums;
}

Album* AlbumManager::findAlbum(int gid) const
{
    return d->allAlbumsIdHash.value(gid);
}

Album* AlbumManager::findAlbum(Album::Type type, int id) const
{
    return findAlbum(Album::globalID(type, id));
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

bool AlbumManager::isMovingAlbum(Album* album) const
{
    return d->currentlyMovingAlbum == album;
}

bool AlbumManager::hasDirectChildAlbumWithTitle(Album* parent, const QString& title)
{

    Album* sibling = parent->firstChild();

    while (sibling)
    {
        if (sibling->title() == title)
        {
            return true;
        }

        sibling = sibling->next();
    }

    return false;

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

void AlbumManager::updateAlbumPathHash()
{
    // Update AlbumDict. basically clear it and rebuild from scratch
    d->albumPathHash.clear();
    AlbumIterator it(d->rootPAlbum);
    PAlbum* subAlbum = 0;

    while ((subAlbum = static_cast<PAlbum*>(it.current())) != 0)
    {
        d->albumPathHash[PAlbumPath(subAlbum)] = subAlbum;
        ++it;
    }
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

    for ( ; it != d->guardedPointers.end() && it.key() == album ; ++it)
    {
        if (it.value())
        {
            *(it.value()) = 0;
        }
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

qlonglong AlbumManager::getItemFromAlbum(Album* const album, const QString& fileName)
{
    return CoreDbAccess().db()->getItemFromAlbum(album->id(), fileName);
}

} // namespace Digikam
