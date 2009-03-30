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

#ifndef ALBUMFILTERMODEL_H
#define ALBUMFILTERMODEL_H

// Qt includes.

#include <QSortFilterProxyModel>

// Local includes.

#include "albummodel.h"
#include "searchtextbar.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_MODEL_EXPORT AlbumFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    AlbumFilterModel(QObject *parent = 0);

    void setSourceAlbumModel(AbstractAlbumModel *source);

    AbstractAlbumModel *sourceAlbumModel() const;

    Album *albumForIndex(const QModelIndex &index) const;
    QModelIndex indexForAlbum(Album *album) const;
    QModelIndex rootAlbumIndex() const;

    /// Returns if the set search text settings will result in any filtering
    bool isFiltering() const;
    SearchTextSettings searchTextSettings() const;

    /** Returns if the filter matches this album (same logic as filterAcceptsRow).
        An album matches if the search text settings are found in a parent album's title,
        in the album's title or in a child album's title, or if it is a special album (root)
        that is never filtered out. */
    enum MatchResult
    {
        /// Can use as bool value if match/no match only is needed
        NoMatch = 0,
        TitleMatch,
        ParentMatch,
        ChildMatch,
        SpecialMatch
    };
    MatchResult matches(Album *album) const;
    MatchResult matches(const QModelIndex &index) const;

public Q_SLOTS:

    void setSearchTextSettings(const SearchTextSettings& settings);

Q_SIGNALS:

    void filterChanged();

protected:

    // use setSourceAlbumModel please
    virtual void setSourceModel(QAbstractItemModel* model);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    SearchTextSettings m_settings;
};

}

#endif

