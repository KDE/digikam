/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-24
 * Description : Qt Model for Albums - filter model
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

bool AlbumFilterModel::rawMatches(Album *album) const
{
    return album->title().contains(m_settings.text, m_settings.caseSensitive);
}

AlbumFilterModel::MatchResult AlbumFilterModel::matches(Album *album) const
{
    if (!album)
        return NoMatch;

    PAlbum *palbum = dynamic_cast<PAlbum*>(album);

    if (album->isRoot() || (palbum && palbum->isAlbumRoot()))
        return SpecialMatch;

    if (rawMatches(album))
        return TitleMatch;

    // check if any of the parents match the search
    Album *parent = album->parent();
    PAlbum* pparent = palbum ? static_cast<PAlbum*>(parent) : 0;

    while (parent && !(parent->isRoot() || (pparent && pparent->isAlbumRoot()) ) )
    {
        if (rawMatches(parent))
            return ParentMatch;

        parent = parent->parent();
    }

    // check if any of the children match the search
    AlbumIterator it(album);
    while (it.current())
    {
        if (rawMatches(*it))
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

// -----------------------------------------------------------------------------

CheckableAlbumFilterModel::CheckableAlbumFilterModel(QObject *parent) :
                AlbumFilterModel(parent), m_filterChecked(false),
                m_filterPartiallyChecked(false)
{
}

void CheckableAlbumFilterModel::setSourceCheckableAlbumModel(AbstractCheckableAlbumModel *source)
{
    setSourceModel(source);
}

AbstractCheckableAlbumModel *CheckableAlbumFilterModel::sourceAlbumModel() const
{
    return dynamic_cast<AbstractCheckableAlbumModel*> (sourceModel());
}

void CheckableAlbumFilterModel::setSourceAlbumModel(AbstractAlbumModel *source)
{
    AlbumFilterModel::setSourceAlbumModel(source);
}

void CheckableAlbumFilterModel::setFilterChecked(bool filter)
{
    m_filterChecked = filter;
    invalidateFilter();
    emit filterChanged();
}

void CheckableAlbumFilterModel::setFilterPartiallyChecked(bool filter)
{
    m_filterPartiallyChecked = filter;
    invalidateFilter();
    emit filterChanged();
}

bool CheckableAlbumFilterModel::isFiltering() const
{
    return AlbumFilterModel::isFiltering() || m_filterChecked
                    || m_filterPartiallyChecked;
}

bool CheckableAlbumFilterModel::rawMatches(Album *album) const
{

    bool accepted = AlbumFilterModel::rawMatches(album);

    if (!m_filterChecked && !m_filterPartiallyChecked)
    {
        return accepted;
    }

    Qt::CheckState state = sourceAlbumModel()->checkState(album);

    bool stateAccepted = false;
    if (m_filterPartiallyChecked)
    {
        stateAccepted |= state == Qt::PartiallyChecked;
    }
    if (m_filterChecked)
    {
        stateAccepted |= state == Qt::Checked;
    }

    return accepted && stateAccepted;

}


// -----------------------------------------------------------------------------

SearchFilterModel::SearchFilterModel(QObject *parent)
            : AlbumFilterModel(parent), m_searchType(-1)
{
}

void SearchFilterModel::setSourceSearchModel(SearchModel *source)
{
    setSourceModel(source);
}

SearchModel *SearchFilterModel::sourceSearchModel() const
{
    return dynamic_cast<SearchModel*> (sourceModel());
}

void SearchFilterModel::setFilterSearchType(DatabaseSearch::Type type)
{
    setTypeFilter(type);
}

void SearchFilterModel::listNormalSearches()
{
    setTypeFilter(-1);
}

void SearchFilterModel::listAllSearches()
{
    setTypeFilter(-2);
}

void SearchFilterModel::listTimelineSearches()
{
    setTypeFilter(DatabaseSearch::TimeLineSearch);
}

void SearchFilterModel::listHaarSearches()
{
    setTypeFilter(DatabaseSearch::HaarSearch);
}

void SearchFilterModel::listMapSearches()
{
    setTypeFilter(DatabaseSearch::MapSearch);
}

void SearchFilterModel::listDuplicatesSearches()
{
    setTypeFilter(DatabaseSearch::DuplicatesSearch);
}

void SearchFilterModel::setTypeFilter(int type)
{
    m_searchType = type;
    invalidateFilter();
    emit filterChanged();
}

void SearchFilterModel::setListTemporarySearches(bool list)
{
    m_listTemporary = list;
    invalidateFilter();
    emit filterChanged();
}

bool SearchFilterModel::isFiltering() const
{
    return m_searchType != -2 || !m_listTemporary;
}

bool SearchFilterModel::rawMatches(Album *album) const
{
    SAlbum *salbum = static_cast<SAlbum*>(album);

    if (m_searchType == -1)
    {
        if (!salbum->isNormalSearch())
            return false;
    }
    else if (m_searchType == -2)
    {
    }
    else
    {
        if (salbum->searchType() != (DatabaseSearch::Type)m_searchType)
            return false;
    }

    if (!m_listTemporary && salbum->isTemporarySearch())
        return false;

    return true;
}

void SearchFilterModel::setSourceAlbumModel(AbstractAlbumModel *source)
{
    AlbumFilterModel::setSourceAlbumModel(source);
}

} // namespace Digikam
