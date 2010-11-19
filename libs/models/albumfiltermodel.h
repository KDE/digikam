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

#ifndef ALBUMFILTERMODEL_H
#define ALBUMFILTERMODEL_H

// Qt includes

#include <QSortFilterProxyModel>

// Local includes

#include "albummodel.h"
#include "searchtextbar.h"

namespace Digikam
{

class AlbumFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    AlbumFilterModel(QObject* parent = 0);

    void setSourceAlbumModel(AbstractAlbumModel* source);
    void setSourceAlbumModel(AlbumFilterModel* source);

    AbstractAlbumModel* sourceAlbumModel() const;
    QModelIndex mapToSourceAlbumModel(const QModelIndex& index) const;
    QModelIndex mapFromSourceAlbumModel(const QModelIndex& index) const;

    /// Convenience methods
    Album* albumForIndex(const QModelIndex& index) const;
    QModelIndex indexForAlbum(Album* album) const;
    QModelIndex rootAlbumIndex() const;

    /**
     * Returns if the currently applied filters will result in any filtering.
     *
     * @return <code>true</code> if the current selected filter could result in
     *         any filtering without checking if this really happens.
     */
    virtual bool isFiltering() const;

    /**
     * Returns the settings currently used for filtering.
     *
     * @return current settings for filtering.
     */
    SearchTextSettings searchTextSettings() const;

    enum MatchResult
    {
        /// This enum can be used as a boolean value if match/no match only is needed
        NoMatch = 0,
        TitleMatch,
        ParentMatch,
        ChildMatch,
        SpecialMatch
    };
    /**
     * Returns the MatchResult of an index of this model.
     * Never returns NoMatch for a valid index, because in this case,
     * the index would rather be filtered out.
     **/
    MatchResult matchResult(const QModelIndex& index) const;

public Q_SLOTS:

    /**
     * Accepts new settings used for filtering and applies them to the model.
     *
     * @param settings new settings to apply. An empty text will be interpreted
     *                 as no filtering
     */
    void setSearchTextSettings(const SearchTextSettings& settings);

Q_SIGNALS:

    /**
     * This signal indicates that a new SearchTextSettings arrived and is about
     * to be applied to the model.
     *
     * @param searched <code>true</code> if filtering by text was enabled before
     *                 applying the new settings
     * @param willSearch <code>true</code> if the new settings can result in
     *                   any filtering by text, else <code> false.
     */
    void searchTextSettingsAboutToChange(bool searched, bool willSearch);

    /**
     * Indicates that new search text settings were applied.
     *
     * @param wasSearching <code>true</code> if this is not a new search that
     * @param searched <code>true</code> if the new settings result in any
     *                 filtering
     */
    void searchTextSettingsChanged(bool wasSearching, bool searched);

    /**
     * Indicates that a new filter was applied to the model.
     */
    void filterChanged();

    /**
     * Indicates whether the newly applied filter results in a search result or
     * not.
     *
     * @param hasResult <code>true</code> if the new filter matches any album,
     *                  else <code>false</code>
     */
    void hasSearchResult(bool hasResult);

protected:

    /**
     * Returns if the filter matches this album (same logic as filterAcceptsRow).
     * An album matches if the search text settings are found in a parent album's title,
     * in the album's title or in a child album's title, or if it is a special album (root)
     * that is never filtered out.
     **/
    MatchResult matchResult(Album* album) const;

    /**
     * This method provides the basic match checking algorithm.
     * Return true if this single album matches the current criteria.
     * This method can be overridden to provide custom filtering.
     *
     * @param album album to tell if it matches the filter criteria or not.
     */
    virtual bool matches(Album* album) const;

    /**
     * Use setSourceAlbumModel.
     *
     * @see setSourceAlbumModel
     * @param model source model
     */
    virtual void setSourceModel(QAbstractItemModel* model);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

protected:

    SearchTextSettings m_settings;
    AlbumFilterModel*  m_chainedModel;

private:

    /**
     * Tells whether the given settings result in filtering.
     *
     * @return <code>true</code> if the provided settings result in filtering
     *         the model
     */
    bool settingsFilter(const SearchTextSettings& settings) const;

};

/**
 * Filter model for checkable album models that allows more filtering options
 * based on check state.
 */
class CheckableAlbumFilterModel : public AlbumFilterModel
{
    Q_OBJECT
public:
    CheckableAlbumFilterModel(QObject* parent = 0);

    void setSourceCheckableAlbumModel(AbstractCheckableAlbumModel* source);
    AbstractCheckableAlbumModel* sourceAlbumModel() const;

    void setFilterChecked(bool filter);
    void setFilterPartiallyChecked(bool filter);

    virtual bool isFiltering() const;

protected:

    virtual bool matches(Album* album) const;

    void setSourceAlbumModel(AbstractAlbumModel* source);

    bool m_filterChecked;
    bool m_filterPartiallyChecked;

};

/**
 * Filter model for searches that can filter by search type
 */
class SearchFilterModel : public CheckableAlbumFilterModel
{
public:

    SearchFilterModel(QObject* parent = 0);
    void setSourceSearchModel(SearchModel* source);
    SearchModel* sourceSearchModel() const;

    /** Set the DatabaseSearch::Type. */
    void setFilterSearchType(DatabaseSearch::Type);
    void listAllSearches();
    void listNormalSearches();
    void listTimelineSearches();
    void listHaarSearches();
    void listMapSearches();
    void listDuplicatesSearches();

    /** Sets if temporary search albums shall be listed */
    void setListTemporarySearches(bool list);

    virtual bool isFiltering() const;

protected:

    virtual bool matches(Album* album) const;
    void setSourceAlbumModel(AbstractAlbumModel* source);

    void setTypeFilter(int type);

    int                     m_searchType;
    bool                    m_listTemporary;
};

} // namespace Digikam

#endif // ALBUMFILTERMODEL_H
