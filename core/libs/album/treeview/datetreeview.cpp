/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2014      by Michael G. Hansen <mike at mghansen dot de>
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

#include "datetreeview.h"
#include "abstractalbumtreeview_p.h"

namespace Digikam
{

DateTreeView::DateTreeView(QWidget* const parent, Flags flags)
    : AbstractCountingAlbumTreeView(parent, flags)
{
    // this view should always show the inclusive counts
    disconnect(this, SIGNAL(expanded(QModelIndex)),
               this, SLOT(slotExpanded(QModelIndex)));

    disconnect(this, SIGNAL(collapsed(QModelIndex)),
               this, SLOT(slotCollapsed(QModelIndex)));

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new DateAlbumModel(this));
    }
}

void DateTreeView::setAlbumModel(DateAlbumModel* const model)
{
    AbstractCountingAlbumTreeView::setAlbumModel(model);
}

DateAlbumModel* DateTreeView::albumModel() const
{
    return static_cast<DateAlbumModel*>(m_albumModel);
}

void DateTreeView::setAlbumFilterModel(AlbumFilterModel* const filterModel)
{
    AbstractCountingAlbumTreeView::setAlbumFilterModel(filterModel);
}

DAlbum* DateTreeView::currentAlbum() const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

DAlbum* DateTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(index));
}

void DateTreeView::setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager)
{
    AbstractCountingAlbumTreeView::setCurrentAlbums(albums, selectInAlbumManager);
}

void DateTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    DAlbum* const album = AlbumManager::instance()->findDAlbum(albumId);
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

} // namespace Digikam
