/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Search Album helpers.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
    foreach (const SearchInfo& info, currentSearches)
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
    foreach (SAlbum* const album, oldSearches)
    {
        emit signalAlbumAboutToBeDeleted(album);
        d->allAlbumsIdHash.remove(album->globalID());
        emit signalAlbumDeleted(album);
        quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
        delete album;
        emit signalAlbumHasBeenDeleted(deletedAlbum);
    }

    // add new albums
    foreach (const SearchInfo& info, newSearches)
    {
        SAlbum* const album                   = new SAlbum(info.name, info.id);
        album->setSearch(info.type, info.query);
        emit signalAlbumAboutToBeAdded(album, d->rootSAlbum, d->rootSAlbum->lastChild());
        album->setParent(d->rootSAlbum);
        d->allAlbumsIdHash[album->globalID()] = album;
        emit signalAlbumAdded(album);
    }
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

SAlbum* AlbumManager::findSAlbum(const QString& name) const
{
    for (Album* album = d->rootSAlbum->firstChild() ;
         album ; album = album->next())
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

    for (Album* album = d->rootSAlbum->firstChild() ;
         album ; album = album->next())
    {
        if (album != 0)
        {
            SAlbum* const sAlbum = dynamic_cast<SAlbum*>(album);

            if ((sAlbum != 0) && (sAlbum->searchType() == searchType))
            {
                albums.append(sAlbum);
            }
        }
    }

    return albums;
}

SAlbum* AlbumManager::createSAlbum(const QString& name,
                                   DatabaseSearch::Type type,
                                   const QString& query)
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

bool AlbumManager::updateSAlbum(SAlbum* album,
                                const QString& changedQuery,
                                const QString& changedName,
                                DatabaseSearch::Type type)
{
    if (!album)
    {
        return false;
    }

    QString newName              = changedName.isNull()                    ? album->title()      : changedName;
    DatabaseSearch::Type newType = (type == DatabaseSearch::UndefinedType) ? album->searchType() : type;

    ChangingDB changing(d);
    CoreDbAccess().db()->updateSearch(album->id(), newType, newName, changedQuery);

    QString oldName              = album->title();

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

} // namespace Digikam
