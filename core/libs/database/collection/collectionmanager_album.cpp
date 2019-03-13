/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management - location helpers.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "collectionmanager_p.h"

namespace Digikam
{

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    CoreDbAccess access;
    QStringList list;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {
            list << location->albumRootPath();
        }
    }

    return list;
}

QString CollectionManager::albumRootPath(int id)
{
    CoreDbAccess access;
    CollectionLocation* const location = d->locations.value(id);

    if (location && location->status() == CollectionLocation::LocationAvailable)
    {
        return location->albumRootPath();
    }

    return QString();
}

QString CollectionManager::albumRootLabel(int id)
{
    CoreDbAccess access;
    CollectionLocation* const location = d->locations.value(id);

    if (location && location->status() == CollectionLocation::LocationAvailable)
    {
        return location->label();
    }

    return QString();
}

QUrl CollectionManager::albumRoot(const QUrl& fileUrl)
{
    return QUrl::fromLocalFile(albumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile()));
}

QString CollectionManager::albumRootPath(const QUrl& fileUrl)
{
    return albumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::albumRootPath(const QString& givenPath)
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        QString rootPath = location->albumRootPath();
        QString filePath = QDir::fromNativeSeparators(givenPath);

        if (!rootPath.isEmpty() && filePath.startsWith(rootPath))
        {
            // see also bug #221155 for extra checks
            if (filePath == rootPath || filePath.startsWith(rootPath + QLatin1Char('/')))
            {
                return location->albumRootPath();
            }
        }
    }

    return QString();
}

bool CollectionManager::isAlbumRoot(const QUrl& fileUrl)
{
    return isAlbumRoot(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

bool CollectionManager::isAlbumRoot(const QString& filePath)
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (filePath == location->albumRootPath())
        {
            return true;
        }
    }

    return false;
}

QString CollectionManager::album(const QUrl& fileUrl)
{
    return album(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::album(const QString& filePath)
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        QString absolutePath = location->albumRootPath();

        if (absolutePath.isEmpty())
        {
            continue;
        }

        QString firstPart = filePath.left(absolutePath.length());

        if (firstPart == absolutePath && filePath.mid(absolutePath.length(), 1) == QLatin1String("/"))
        {
            if (filePath == absolutePath ||
                (filePath.length() == absolutePath.length() + 1 && filePath.right(1) == QLatin1String("/")))
            {
                return QLatin1String("/");
            }
            else
            {
                QString album = filePath.mid(absolutePath.length());

                if (album.endsWith(QLatin1Char('/')))
                {
                    album.chop(1);
                }

                return album;
            }
        }
    }

    return QString();
}

QString CollectionManager::album(const CollectionLocation& location, const QUrl& fileUrl)
{
    return album(location, fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::album(const CollectionLocation& location, const QString& filePath)
{
    if (location.isNull())
    {
        return QString();
    }

    QString absolutePath = location.albumRootPath();

    if (filePath == absolutePath)
    {
        return QLatin1String("/");
    }
    else
    {
        QString album = filePath.mid(absolutePath.length());

        if (album.endsWith(QLatin1Char('/')))
        {
            album.chop(1);
        }

        return album;
    }
}

QUrl CollectionManager::oneAlbumRoot()
{
    return QUrl::fromLocalFile(oneAlbumRootPath());
}

QString CollectionManager::oneAlbumRootPath()
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {
            return location->albumRootPath();
        }
    }

    return QString();
}

void CollectionManager::slotAlbumRootChange(const AlbumRootChangeset& changeset)
{
    if (d->changingDB)
    {
        return;
    }

    switch (changeset.operation())
    {
        case AlbumRootChangeset::Added:
        case AlbumRootChangeset::Deleted:
            updateLocations();
            break;

        case AlbumRootChangeset::PropertiesChanged:
            // label has changed
        {
            CollectionLocation toBeEmitted;
            {
                CoreDbAccess access;
                AlbumRootLocation* const location = d->locations.value(changeset.albumRootId());

                if (location)
                {
                    QList<AlbumRootInfo> infos = access.db()->getAlbumRoots();

                    foreach(const AlbumRootInfo& info, infos)
                    {
                        if (info.id == location->id())
                        {
                            location->setLabel(info.label);
                            toBeEmitted = *location;
                            break;
                        }
                    }
                }
            }

            if (!toBeEmitted.isNull())
            {
                emit locationPropertiesChanged(toBeEmitted);
            }
        }
        break;

        case AlbumRootChangeset::Unknown:
            break;
    }
}

} // namespace Digikam
