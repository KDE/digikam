/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2014      by Michael G. Hansen <mike at mghansen dot de>
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

#include "tagtreeview.h"
#include "abstractalbumtreeview_p.h"

namespace Digikam
{

TagTreeView::TagTreeView(QWidget* const parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags),
      m_filteredModel(0)
{
    m_modificationHelper = new TagModificationHelper(this, this);
    setRootIsDecorated(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(AUTOEXPANDDELAY);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new TagModel(TagModel::IncludeRootAlbum, this));
    }

    if (flags & CreateDefaultFilterModel) // must set again!
    {
        setAlbumFilterModel(new TagPropertiesFilterModel(this), albumFilterModel());
    }
}

TagTreeView::~TagTreeView()
{
}

void TagTreeView::setAlbumFilterModel(TagPropertiesFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel)
{
    m_filteredModel = filteredModel;
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
    // hook in: source album model -> filtered model -> album filter model
    albumFilterModel()->setSourceFilterModel(m_filteredModel);
}

void TagTreeView::setAlbumModel(TagModel* const model)
{
    // changing model is not implemented
    if (m_albumModel)
    {
        return;
    }

    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    if (m_filteredModel)
    {
        m_filteredModel->setSourceAlbumModel(model);
    }

    m_dragDropHandler = albumModel()->dragDropHandler();

    if (!m_dragDropHandler)
    {
        m_dragDropHandler = new TagDragDropHandler(albumModel());
        albumModel()->setDragDropHandler(m_dragDropHandler);

        connect(albumModel()->dragDropHandler(), SIGNAL(assignTags(QList<qlonglong>,QList<int>)),
                FileActionMngr::instance(), SLOT(assignTags(QList<qlonglong>,QList<int>)));
    }

    if (m_albumModel && (m_albumModel->rootAlbumBehavior() == AbstractAlbumModel::IncludeRootAlbum))
    {
        setRootIsDecorated(false);
    }

    if (m_albumFilterModel)
    {
        expand(m_albumFilterModel->rootAlbumIndex());
    }
}

TagModel* TagTreeView::albumModel() const
{
    return static_cast<TagModel*>(m_albumModel);
}

TagPropertiesFilterModel* TagTreeView::filteredModel() const
{
    return m_filteredModel;
}

TAlbum* TagTreeView::currentAlbum() const
{
    return dynamic_cast<TAlbum*>(m_albumFilterModel->albumForIndex(currentIndex()));
}

QList<Album*> TagTreeView::selectedTags()
{
    return selectedAlbums<Album>(selectionModel(), m_filteredModel);
}

QList<TAlbum*> TagTreeView::selectedTagAlbums()
{
    return selectedAlbums<TAlbum>(selectionModel(), m_filteredModel);
}

TAlbum* TagTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<TAlbum*>(m_albumFilterModel->albumForIndex(index));
}

TagModificationHelper* TagTreeView::tagModificationHelper() const
{
    return m_modificationHelper;
}

void TagTreeView::setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager)
{
    AbstractCheckableAlbumTreeView::setCurrentAlbums(albums, selectInAlbumManager);
}

void TagTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    TAlbum* const album = AlbumManager::instance()->findTAlbum(albumId);
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

} // namespace Digikam
