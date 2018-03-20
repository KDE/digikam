/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2014-2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

// Local includes

#include "applicationsettings.h"
#include "applicationsettings_p.h"

namespace Digikam
{

void ApplicationSettings::setTreeViewIconSize(int val)
{
    d->treeThumbnailSize = val;
}

int ApplicationSettings::getTreeViewIconSize() const
{
    return ((d->treeThumbnailSize < 8) || (d->treeThumbnailSize > 48)) ? 48 : d->treeThumbnailSize;
}

void ApplicationSettings::setTreeViewFont(const QFont& font)
{
    d->treeviewFont = font;
}

QFont ApplicationSettings::getTreeViewFont() const
{
    return d->treeviewFont;
}

void ApplicationSettings::setAlbumSortRole(const ApplicationSettings::AlbumSortRole role)
{
    d->albumSortRole = role;
}

ApplicationSettings::AlbumSortRole ApplicationSettings::getAlbumSortRole() const
{
    return d->albumSortRole;
}

void ApplicationSettings::setRecurseAlbums(bool val)
{
    d->recursiveAlbums = val;
    emit recurseSettingsChanged();
}

bool ApplicationSettings::getRecurseAlbums() const
{
    return d->recursiveAlbums;
}

void ApplicationSettings::setRecurseTags(bool val)
{
    d->recursiveTags = val;
    emit recurseSettingsChanged();
}

bool ApplicationSettings::getRecurseTags() const
{
    return d->recursiveTags;
}

void ApplicationSettings::setShowFolderTreeViewItemsCount(bool val)
{
    d->showFolderTreeViewItemsCount = val;
}

bool ApplicationSettings::getShowFolderTreeViewItemsCount() const
{
    return d->showFolderTreeViewItemsCount;
}

void ApplicationSettings::setAlbumSortChanged(bool val)
{
    d->albumSortChanged = val;
}

bool ApplicationSettings::getAlbumSortChanged() const
{
    return d->albumSortChanged;
}

void ApplicationSettings::setAlbumCategoryNames(const QStringList& list)
{
    d->albumCategoryNames = list;
}

QStringList ApplicationSettings::getAlbumCategoryNames() const
{
    return d->albumCategoryNames;
}

bool ApplicationSettings::addAlbumCategoryName(const QString& name) const
{
    if (d->albumCategoryNames.contains(name))
    {
        return false;
    }

    d->albumCategoryNames.append(name);
    return true;
}

bool ApplicationSettings::delAlbumCategoryName(const QString& name) const
{
    uint count = d->albumCategoryNames.removeAll(name);
    return (count > 0) ? true : false;
}

}  // namespace Digikam
