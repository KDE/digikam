/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albumtreeview.h"
#include "albumtreeview.moc"

// Qt includes

// KDE includes

#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"

namespace Digikam
{

AbstractAlbumTreeView::AbstractAlbumTreeView(AbstractSpecificAlbumModel *model, QWidget *parent)
    : QTreeView(parent)
{
    m_albumModel       = model;
    m_albumModel->setParent(this); // cannot be done before QObject constructor of this is called
    m_albumFilterModel = new AlbumFilterModel(this);

    connect(m_albumFilterModel, SIGNAL(filterChanged()),
             this, SLOT(slotFilterChanged()));

    if (!m_albumModel->rootAlbum())
        connect(m_albumModel, SIGNAL(rootAlbumAvailable()),
                 this, SLOT(slotRootAlbumAvailable()));

    m_albumFilterModel->setSourceAlbumModel(m_albumModel);
    setModel(m_albumFilterModel);
}

AbstractSpecificAlbumModel *AbstractAlbumTreeView::albumModel() const
{
    return m_albumModel;
}

AlbumFilterModel *AbstractAlbumTreeView::albumFilterModel() const
{
    return m_albumFilterModel;
}

void AbstractAlbumTreeView::slotFilterChanged()
{
    if (!m_albumFilterModel->isFiltering())
    {
        // Returning from search: collapse all, expand to current album
        collapseAll();
        Album *currentAlbum = AlbumManager::instance()->currentAlbum();
        if (currentAlbum)
        {
            QModelIndex current = m_albumFilterModel->indexForAlbum(currentAlbum);
            expand(current);
            scrollTo(current);
        }
        else
        {
            // expand only root
            expand(m_albumFilterModel->rootAlbumIndex());
        }
        return;
    }

    bool hasAnyMatch = checkExpandedState(QModelIndex());
    emit filteringDone(hasAnyMatch);
}

void AbstractAlbumTreeView::slotRootAlbumAvailable()
{
    expand(m_albumFilterModel->rootAlbumIndex());
}

bool AbstractAlbumTreeView::checkExpandedState(const QModelIndex &index)
{
    bool anyMatch = false;

    AlbumFilterModel::MatchResult result = m_albumFilterModel->matches(index);
    if (result == AlbumFilterModel::ChildMatch)
        expand(index);
    anyMatch = result;

    int rows = m_albumFilterModel->rowCount(index);
    for (int i=0; i<rows; ++i)
    {
        QModelIndex child = model()->index(i, 0, index);
        bool childResult = checkExpandedState(child);
        anyMatch = anyMatch || childResult;
    }
    return anyMatch;
}

void AbstractAlbumTreeView::setSearchTextSettings(const SearchTextSettings& settings)
{
    m_albumFilterModel->setSearchTextSettings(settings);
}

// --------------------------------------- //

AbstractCountingAlbumTreeView::AbstractCountingAlbumTreeView(AbstractCountingAlbumModel *model, QWidget *parent)
    : AbstractAlbumTreeView(model, parent)
{
    connect(this, SIGNAL(expanded(const QModelIndex &)),
             this, SLOT(slotExpanded(const QModelIndex &)));

    connect(this, SIGNAL(collapsed(const QModelIndex &)),
             this, SLOT(slotCollapsed(const QModelIndex &)));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
             this, SLOT(slotSetShowCount()));

    connect(m_albumFilterModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
             this, SLOT(slotRowsInserted(const QModelIndex &, int, int)));

    slotSetShowCount();

    // Initialize expanded/collapsed showCount state
    updateShowCountState(QModelIndex(), true);
}

void AbstractCountingAlbumTreeView::updateShowCountState(const QModelIndex &index, bool recurse)
{
    kDebug() << "  updating" << index << isExpanded(index);
    if (isExpanded(index))
        slotExpanded(index);
    else
        slotCollapsed(index);

    if (recurse)
    {
        int rows = m_albumFilterModel->rowCount(index);
        kDebug() << "recursing" << index << rows;
        for (int i=0; i<rows; ++i)
            updateShowCountState(m_albumFilterModel->index(i, 0, index), true);
    }
}

void AbstractCountingAlbumTreeView::slotCollapsed(const QModelIndex &index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->includeChildrenCount(m_albumFilterModel->mapToSource(index));
}

void AbstractCountingAlbumTreeView::slotExpanded(const QModelIndex &index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->excludeChildrenCount(m_albumFilterModel->mapToSource(index));
}

void AbstractCountingAlbumTreeView::slotSetShowCount()
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->setShowCount(AlbumSettings::instance()->getShowFolderTreeViewItemsCount());
}

void AbstractCountingAlbumTreeView::slotRowsInserted(const QModelIndex &parent, int start, int end)
{
    // initialize showCount state when items are added
    for (int i=start; i<=end; ++i)
        updateShowCountState(m_albumFilterModel->index(i, 0, parent), false);
}

// --------------------------------------- //

AlbumTreeView::AlbumTreeView(QWidget *parent)
    : AbstractCountingAlbumTreeView(new AlbumModel(AlbumModel::IncludeRootAlbum), parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalPAlbumsDirty(const QMap<int, int>&)),
             m_albumModel, SLOT(setCountMap(const QMap<int, int>&)));

    expand(m_albumFilterModel->rootAlbumIndex());
}

TagTreeView::TagTreeView(QWidget *parent)
    : AbstractCountingAlbumTreeView(new TagModel(AlbumModel::IncludeRootAlbum), parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(const QMap<int, int>&)),
             m_albumModel, SLOT(setCountMap(const QMap<int, int>&)));

    expand(m_albumFilterModel->rootAlbumIndex());
}

SearchTreeView::SearchTreeView(QWidget *parent)
    : AbstractAlbumTreeView(new SearchModel, parent)
{
}

DateAlbumTreeView::DateAlbumTreeView(QWidget *parent)
    : AbstractCountingAlbumTreeView(new DateAlbumModel, parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalDAlbumsDirty(const QMap<YearMonth, int>&)),
             m_albumModel, SLOT(setYearMonthMap(const QMap<YearMonth, int>&)));
}

}
