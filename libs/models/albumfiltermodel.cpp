/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-24
 * Description : Qt Model for Albums - filter model
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albumfiltermodel.h"
#include "albumfiltermodel.moc"

// Qt includes

#include <QSortFilterProxyModel>

// KDE includes

#include <kdebug.h>
#include <kstringhandler.h>

// Local includes

#include "albummodel.h"

namespace Digikam
{

AlbumFilterModel::AlbumFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void AlbumFilterModel::setSearchTextSettings(const SearchTextSettings& settings)
{
    m_settings = settings;
    invalidateFilter();
    emit filterChanged();

    // find out if this setting has some results or not
    int validRows = 0;
    // for every collection we got
    for(int i = 0; i < rowCount(rootAlbumIndex()); ++i)
    {
        QModelIndex collectionIndex = index(i, 0, rootAlbumIndex());
        // count the number of rows
        validRows += rowCount(collectionIndex);
    }
    bool hasResult = validRows > 0;
    kDebug() << "new search text settings: " << settings.text
             << ": hasResult = " << hasResult << ", validRows = "
             << validRows;
    emit hasSearchResult(hasResult);
}

bool AlbumFilterModel::isFiltering() const
{
    return !m_settings.text.isEmpty();
}

SearchTextSettings AlbumFilterModel::searchTextSettings() const
{
    return m_settings;
}

void AlbumFilterModel::setSourceAlbumModel(AbstractAlbumModel *source)
{
    setSourceModel(source);
}

void AlbumFilterModel::setSourceModel(QAbstractItemModel* model)
{
    // made it protected, only setSourceAlbumModel is public
    QSortFilterProxyModel::setSourceModel(model);
}

AbstractAlbumModel *AlbumFilterModel::sourceAlbumModel() const
{
    return static_cast<AbstractAlbumModel*>(sourceModel());
}

Album *AlbumFilterModel::albumForIndex(const QModelIndex& index) const
{
    return sourceAlbumModel()->albumForIndex(mapToSource(index));
}

QModelIndex AlbumFilterModel::indexForAlbum(Album *album) const
{
    return mapFromSource(sourceAlbumModel()->indexForAlbum(album));
}

QModelIndex AlbumFilterModel::rootAlbumIndex() const
{
    return mapFromSource(sourceAlbumModel()->rootAlbumIndex());
}

AlbumFilterModel::MatchResult AlbumFilterModel::matches(const QModelIndex& index) const
{
    return matches(sourceAlbumModel()->albumForIndex(mapToSource(index)));
}

AlbumFilterModel::MatchResult AlbumFilterModel::matches(Album *album) const
{
    if (!album)
        return NoMatch;

    PAlbum *palbum = dynamic_cast<PAlbum*>(album);

    if (album->isRoot() || (palbum && palbum->isAlbumRoot()))
        return SpecialMatch;

    if (album->title().contains(m_settings.text, m_settings.caseSensitive))
        return TitleMatch;

    // check if any of the parents match the search
    Album *parent = album->parent();
    PAlbum* pparent = palbum ? static_cast<PAlbum*>(parent) : 0;

    while (parent && !(parent->isRoot() || (pparent && pparent->isAlbumRoot()) ) )
    {
        if (parent->title().contains(m_settings.text, m_settings.caseSensitive))
            return ParentMatch;

        parent = parent->parent();
    }

    // check if any of the children match the search
    AlbumIterator it(album);
    while (it.current())
    {
        if ((*it)->title().contains(m_settings.text, m_settings.caseSensitive))
            return ChildMatch;
        ++it;
    }

    return NoMatch;
}

bool AlbumFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceAlbumModel()->index(source_row, 0, source_parent);
    Album *album = sourceAlbumModel()->albumForIndex(index);
    if (!album)
        return false;
    MatchResult result = matches(album);
    return result;
}

bool AlbumFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QVariant valLeft = left.data(sortRole());
    QVariant valRight = right.data(sortRole());

    if (valLeft.type() == QVariant::String && valRight.type() == QVariant::String)
        return KStringHandler::naturalCompare(valLeft.toString(), valRight.toString(), sortCaseSensitivity()) < 0;
    else
        return QSortFilterProxyModel::lessThan(left, right);
}

} // namespace Digikam
