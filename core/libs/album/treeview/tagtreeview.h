/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_TAG_TREE_VIEW_H
#define DIGIKAM_TAG_TREE_VIEW_H

// Local includes

#include "abstractcheckablealbumtreeview.h"

namespace Digikam
{

class TagTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT

public:

    explicit TagTreeView(QWidget* const parent = nullptr, Flags flags = DefaultFlags);
    ~TagTreeView();

    TagModel* albumModel() const;

    /// Contains only the tags filtered by properties - prefer to albumModel()
    TagPropertiesFilterModel* filteredModel() const;

    /**
     * @brief currentAlbum     - even if multiple selection is enabled current
     *                           Album can be only one, the last clicked item
     *                          if you need selected items, see selectedAlbums()
     *                          It's NOT the same as AlbumManager::currentAlbums()
     */
    TAlbum* currentAlbum() const;

    /**
     * @brief selectedTags - return a list of all selected items in tag model
     */
    QList<Album*> selectedTags();
    QList<TAlbum*> selectedTagAlbums();

    TAlbum* albumForIndex(const QModelIndex& index) const;
    TagModificationHelper* tagModificationHelper()  const;

    void setAlbumFilterModel(TagPropertiesFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel);
    void setAlbumModel(TagModel* const model);

public Q_SLOTS:

    void setCurrentAlbums(const QList<Album*>& tags, bool selectInAlbumManager = true);
    void setCurrentAlbum(int tagId, bool selectInAlbumManager = true);

Q_SIGNALS:

    void assignTags(int tagId, const QList<int>& imageIDs);

protected:

    TagPropertiesFilterModel* m_filteredModel;
    TagModificationHelper*    m_modificationHelper;
};

} // namespace Digikam

#endif // DIGIKAM_TAG_TREE_VIEW_H
