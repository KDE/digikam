/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Collection helpers.
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

void AlbumManager::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (!d->rootDAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
        case CollectionImageChangeset::Deleted:
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

} // namespace Digikam
