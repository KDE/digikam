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

#include "abstractcountingalbumtreeview.h"
#include "abstractalbumtreeview_p.h"

namespace Digikam
{

AbstractCountingAlbumTreeView::AbstractCountingAlbumTreeView(QWidget* const parent, Flags flags)
    : AbstractAlbumTreeView(parent, flags & ~CreateDefaultFilterModel)
{
    if (flags & CreateDefaultFilterModel)
    {
        setAlbumFilterModel(new AlbumFilterModel(this));
    }

    connect(this, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotExpanded(QModelIndex)));

    connect(this, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotCollapsed(QModelIndex)));

    if (flags & ShowCountAccordingToSettings)
    {
        connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
                this, SLOT(setShowCountFromSettings()));
    }
}

void AbstractCountingAlbumTreeView::setAlbumModel(AbstractCountingAlbumModel* const model)
{
    AbstractAlbumTreeView::setAlbumModel(model);

    if (m_flags & ShowCountAccordingToSettings)
    {
        setShowCountFromSettings();
    }
}

void AbstractCountingAlbumTreeView::setAlbumFilterModel(AlbumFilterModel* const filterModel)
{
    AbstractAlbumTreeView::setAlbumFilterModel(filterModel);

    // Initialize expanded/collapsed showCount state
    updateShowCountState(QModelIndex(), true);
}

void AbstractCountingAlbumTreeView::updateShowCountState(const QModelIndex& index, bool recurse)
{
    if (isExpanded(index))
    {
        slotExpanded(index);
    }
    else
    {
        slotCollapsed(index);
    }

    if (recurse)
    {
        const int rows = m_albumFilterModel->rowCount(index);

        for (int i = 0 ; i < rows ; ++i)
        {
            updateShowCountState(m_albumFilterModel->index(i, 0, index), true);
        }
    }
}

void AbstractCountingAlbumTreeView::slotCollapsed(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->includeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
}

void AbstractCountingAlbumTreeView::slotExpanded(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->excludeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
}

void AbstractCountingAlbumTreeView::setShowCountFromSettings()
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->setShowCount(ApplicationSettings::instance()->getShowFolderTreeViewItemsCount());
}

void AbstractCountingAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    AbstractAlbumTreeView::rowsInserted(parent, start, end);

    // initialize showCount state when items are added
    for (int i = start ; i <= end ; ++i)
    {
        updateShowCountState(m_albumFilterModel->index(i, 0, parent), false);
    }
}

} // namespace Digikam
