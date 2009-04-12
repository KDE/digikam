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

#include <QMouseEvent>

// KDE includes

#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"

namespace Digikam
{

template <class A>
static inline A* currentAlbum(QItemSelectionModel *selModel, AlbumFilterModel *filterModel)
{
    return static_cast<A*>(filterModel->albumForIndex(selModel->currentIndex()));
}

template <class A>
static QList<A*> selectedAlbums(QItemSelectionModel *selModel, AlbumFilterModel *filterModel)
{
    QList<QModelIndex> indexes = selModel->selectedIndexes();
    QList<A*> albums;
    foreach (const QModelIndex &index, indexes)
        albums << static_cast<A*>(filterModel->albumForIndex(index));
    return albums;
}

AbstractAlbumTreeView::AbstractAlbumTreeView(AbstractSpecificAlbumModel *model, QWidget *parent)
    : QTreeView(parent)
{
    m_expandOnSingleClick = true;
    m_checkOnMiddleClick  = false;

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

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex &)),
             this, SLOT(slotCurrentChanged()));

    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection &)),
             this, SLOT(slotCurrentChanged()));
}

AbstractSpecificAlbumModel *AbstractAlbumTreeView::albumModel() const
{
    return m_albumModel;
}

AlbumFilterModel *AbstractAlbumTreeView::albumFilterModel() const
{
    return m_albumFilterModel;
}

void AbstractAlbumTreeView::setSelectOnSingleClick(bool doThat)
{
    m_expandOnSingleClick = doThat;
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
        QModelIndex child = m_albumFilterModel->index(i, 0, index);
        bool childResult = checkExpandedState(child);
        anyMatch = anyMatch || childResult;
    }
    return anyMatch;
}

void AbstractAlbumTreeView::setSearchTextSettings(const SearchTextSettings& settings)
{
    m_albumFilterModel->setSearchTextSettings(settings);
}

void AbstractAlbumTreeView::slotCurrentChanged()
{
    emit currentAlbumChanged(currentAlbum<Album>(selectionModel(), m_albumFilterModel));
}

void AbstractAlbumTreeView::slotSelectionChanged()
{
    emit selectedAlbumsChanged(selectedAlbums<Album>(selectionModel(), m_albumFilterModel));
}

void AbstractAlbumTreeView::mousePressEvent(QMouseEvent *e)
{
    if (m_expandOnSingleClick && e->button() == Qt::LeftButton)
    {
        QModelIndex index = indexAt(e->pos());
        if (index.isValid())
        {
            // See B.K.O #126871: collapse/expand treeview using left mouse button single click.
            // Exception: If a newly selected item is already expanded, do not collapse on selection.
            bool expanded = isExpanded(index);
            if (index == currentIndex() || !expanded)
                setExpanded(index, !expanded);
        }
    }
    else if (m_checkOnMiddleClick && e->button() == Qt::MidButton)
    {
        Album *a = m_albumFilterModel->albumForIndex(indexAt(e->pos()));
        if (a)
            middleButtonPressed(a);
    }

    QTreeView::mousePressEvent(e);
}

void AbstractAlbumTreeView::middleButtonPressed(Album *)
{
    // reimplement if needed
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
    if (isExpanded(index))
        slotExpanded(index);
    else
        slotCollapsed(index);

    if (recurse)
    {
        int rows = m_albumFilterModel->rowCount(index);
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

AbstractCheckableAlbumTreeView::AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model, QWidget *parent)
    : AbstractCountingAlbumTreeView(model, parent)
{
    m_checkOnMiddleClick  = true;
}

AbstractCheckableAlbumModel *AbstractCheckableAlbumTreeView::checkableModel() const
{
    return static_cast<AbstractCheckableAlbumModel*>(m_albumModel);
}

void AbstractCheckableAlbumTreeView::setCheckOnMiddleClick(bool doThat)
{
    m_checkOnMiddleClick = doThat;
}

void AbstractCheckableAlbumTreeView::middleButtonPressed(Album *a)
{
    if (static_cast<AbstractCheckableAlbumModel*>(m_albumModel)->isCheckable())
        static_cast<AbstractCheckableAlbumModel*>(m_albumModel)->toggleChecked(a);
}

// --------------------------------------- //

AlbumTreeView::AlbumTreeView(QWidget *parent)
    : AbstractCheckableAlbumTreeView(new AlbumModel(AlbumModel::IncludeRootAlbum), parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalPAlbumsDirty(const QMap<int, int>&)),
             m_albumModel, SLOT(setCountMap(const QMap<int, int>&)));

    expand(m_albumFilterModel->rootAlbumIndex());
    setRootIsDecorated(false);
}

AlbumModel *AlbumTreeView::albumModel() const
{
    return static_cast<AlbumModel*>(m_albumModel);
}

TagTreeView::TagTreeView(QWidget *parent)
    : AbstractCheckableAlbumTreeView(new TagModel(AlbumModel::IncludeRootAlbum), parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(const QMap<int, int>&)),
             m_albumModel, SLOT(setCountMap(const QMap<int, int>&)));

    expand(m_albumFilterModel->rootAlbumIndex());
    setRootIsDecorated(false);
}

TagModel *TagTreeView::albumModel() const
{
    return static_cast<TagModel*>(m_albumModel);
}

SearchTreeView::SearchTreeView(QWidget *parent)
    : AbstractAlbumTreeView(new SearchModel, parent)
{
}

SearchModel *SearchTreeView::albumModel() const
{
    return static_cast<SearchModel*>(m_albumModel);
}

DateAlbumTreeView::DateAlbumTreeView(QWidget *parent)
    : AbstractCountingAlbumTreeView(new DateAlbumModel, parent)
{
    connect(AlbumManager::instance(), SIGNAL(signalDAlbumsDirty(const QMap<YearMonth, int>&)),
             m_albumModel, SLOT(setYearMonthMap(const QMap<YearMonth, int>&)));
}

DateAlbumModel *DateAlbumTreeView::albumModel() const
{
    return static_cast<DateAlbumModel*>(m_albumModel);
}

}
