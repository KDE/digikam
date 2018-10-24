/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_SEARCH_TREE_VIEW_H
#define DIGIKAM_SEARCH_TREE_VIEW_H

// Local includes

#include "abstractcheckablealbumtreeview.h"

namespace Digikam
{

class SearchTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT

public:

    explicit SearchTreeView(QWidget* const parent = 0, Flags flags = DefaultFlags);
    ~SearchTreeView();

    /// Note: not filtered by search type
    SearchModel* albumModel()          const;

    /// Contains only the searches with appropriate type - prefer to albumModel()
    SearchFilterModel* filteredModel() const;
    SAlbum* currentAlbum()             const;

    void setAlbumModel(SearchModel* const model);
    void setAlbumFilterModel(SearchFilterModel* const filteredModel, CheckableAlbumFilterModel* const model);

public Q_SLOTS:

    void setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager = true);
    void setCurrentAlbum(int searchId, bool selectInAlbumManager = true);

protected:

    SearchFilterModel* m_filteredModel;
};

} // namespace Digikam

#endif // DIGIKAM_SEARCH_TREE_VIEW_H
