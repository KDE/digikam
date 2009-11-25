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

#include "albumtreeview.moc"

// Qt includes

#include <QMouseEvent>
#include <qscrollbar.h>

// KDE includes

#include <kdebug.h>

// Local includes

#include "albumdragdrop.h"
#include "albummanager.h"
#include "albummodeldragdrophandler.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "tagdragdrop.h"

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
    foreach (const QModelIndex& index, indexes)
        albums << static_cast<A*>(filterModel->albumForIndex(index));
    return albums;
}

struct State
{
    State() :
        selected(false), expanded(false), currentIndex(false)
    {
    }
    bool selected;
    bool expanded;
    bool currentIndex;
};

class AbstractAlbumTreeViewPriv
{

public:
    AbstractAlbumTreeViewPriv() :
        configSuffixSelection("Selection"),
        configSuffixExpansion("Expansion"),
        configSuffixCurrentIndex("CurrentIndex"),
        configSuffixSortColumn("SortColumn"),
        configSuffixSortOrder("SortOrder")
    {
        expandOnSingleClick = true;
    }

    bool expandOnSingleClick;

    QMap<int, State> statesByAlbumId;

    const QString configSuffixSelection;
    const QString configSuffixExpansion;
    const QString configSuffixCurrentIndex;
    const QString configSuffixSortColumn;
    const QString configSuffixSortOrder;

};

AbstractAlbumTreeView::AbstractAlbumTreeView(AbstractSpecificAlbumModel *model, QWidget *parent)
    : QTreeView(parent), d(new AbstractAlbumTreeViewPriv)
{
    m_checkOnMiddleClick  = false;

    m_albumModel       = model;
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

AbstractAlbumTreeView::~AbstractAlbumTreeView()
{
    delete d;
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
    d->expandOnSingleClick = doThat;
}

QModelIndex AbstractAlbumTreeView::indexVisuallyAt(const QPoint& p)
{
    if (viewport()->rect().contains(p)) {
        QModelIndex index = indexAt(p);
        if (index.isValid() && visualRect(index).contains(p))
            return index;
    }
    return QModelIndex();
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

bool AbstractAlbumTreeView::checkExpandedState(const QModelIndex& index)
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
    if (d->expandOnSingleClick && e->button() == Qt::LeftButton)
    {
        QModelIndex index = indexVisuallyAt(e->pos());
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

void AbstractAlbumTreeView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.count() > 0) {
        QMimeData *data = m_albumFilterModel->mimeData(indexes);
        if (!data)
            return;
        QStyleOptionViewItem option = viewOptions();
        option.rect = viewport()->rect();
        QPixmap pixmap = /*m_delegate->*/pixmapForDrag(option, indexes);
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->exec(supportedActions);
    }
}

//TODO: Move to delegate, when we have one.
//      Copy code from image delegate for creating icons when dragging multiple items
QPixmap AbstractAlbumTreeView::pixmapForDrag(const QStyleOptionViewItem&, QList<QModelIndex> indexes)
{
    if (indexes.isEmpty())
        return QPixmap();

    QVariant decoration = indexes.first().data(Qt::DecorationRole);
    return decoration.value<QPixmap>();
}

void AbstractAlbumTreeView::dragEnterEvent(QDragEnterEvent *e)
{
    AlbumModelDragDropHandler *handler = m_albumModel->dragDropHandler();
    if (handler && handler->acceptsMimeData(e->mimeData()))
        e->accept();
    else
        e->ignore();
}

void AbstractAlbumTreeView::dragMoveEvent(QDragMoveEvent *e)
{
    QTreeView::dragMoveEvent(e);
    AlbumModelDragDropHandler *handler = m_albumModel->dragDropHandler();
    if (handler)
    {
        QModelIndex index = indexVisuallyAt(e->pos());
        Qt::DropAction action = handler->accepts(e, m_albumFilterModel->mapToSource(index));
        if (action == Qt::IgnoreAction)
            e->ignore();
        else
        {
            e->setDropAction(action);
            e->accept();
        }
    }
}

void AbstractAlbumTreeView::dragLeaveEvent(QDragLeaveEvent * e)
{
    QTreeView::dragLeaveEvent(e);
}

void AbstractAlbumTreeView::dropEvent(QDropEvent *e)
{
    QTreeView::dropEvent(e);
    AlbumModelDragDropHandler *handler = m_albumModel->dragDropHandler();
    if (handler)
    {
        QModelIndex index = indexVisuallyAt(e->pos());
        if (handler->dropEvent(this, e, m_albumFilterModel->mapToSource(index)))
            e->accept();
    }
}

void AbstractAlbumTreeView::loadViewState(KConfigGroup &configGroup, QString prefix)
{

    kDebug() << "Loading view state from " << configGroup.name();

    const QStringList selection = configGroup.readEntry(prefix + d->configSuffixSelection,
                    QStringList());
    foreach(const QString &key, selection)
    {
        bool validId;
        int id = key.toInt(&validId);
        if (validId)
        {
            d->statesByAlbumId[id].selected = true;
        }
    }

    const QStringList expansion = configGroup.readEntry(prefix + d->configSuffixExpansion,
                    QStringList());
    foreach( const QString &key, expansion )
    {
        bool validId;
        int id = key.toInt(&validId);
        if (validId)
        {
            d->statesByAlbumId[id].expanded = true;
        }
    }

    const QString key = configGroup.readEntry(prefix + d->configSuffixCurrentIndex, QString());
    bool validId;
    const int id = key.toInt(&validId);
    if (validId)
    {
        d->statesByAlbumId[id].currentIndex = true;
    }

    for (QMap<int, Digikam::State>::iterator it = d->statesByAlbumId.begin(); it
                    != d->statesByAlbumId.end(); ++it)
    {
        kDebug() << "id = " << it.key() << ": recovered state (selected = "
                 << it.value().selected << ", expanded = "
                 << it.value().expanded << ", currentIndex = "
                 << it.value().currentIndex << ")";
    }


    // initial restore run, for everything already loaded
    kDebug() << "initial restore run";
    for (int i = 0; i < model()->rowCount(); ++i)
    {
        const QModelIndex index = model()->index(i, 0);
        restoreState(index);
    }

    // TODO do we really need this. I would say this code should only be
    // called if the albums are loaded completely. Otherwise you have to think
    // about what happens every time you pop up an album.

    // and the watch the model for new items added
    //connect(model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
    //                 SLOT(slotFixRowsInserted(QModelIndex, int, int)), Qt::QueuedConnection);

    // also restore the sorting order
    sortByColumn(configGroup.readEntry(prefix + d->configSuffixSortColumn, 0),
                 (Qt::SortOrder) configGroup.readEntry(prefix + d->configSuffixSortOrder, (int) Qt::AscendingOrder));

}

void AbstractAlbumTreeView::restoreState(const QModelIndex &index)
{

    Album *album = albumFilterModel()->albumForIndex(index);
    if (album)
    {

        Digikam::State state = d->statesByAlbumId[album->id()];

        kDebug() << "Trying to restore state of album " << album->title()
                 << ": state(selected = " << state.selected
                 << ", expanded = " << state.expanded
                 << ", currentIndex = " << state.currentIndex << ")";
        if (state.selected)
            selectionModel()->select(index, QItemSelectionModel::Select
                            | QItemSelectionModel::Rows);
        if (state.expanded)
            setExpanded(index, true);
        if (state.currentIndex)
            setCurrentIndex(index);
    }
    else
    {
        kError() << "got an invalid album for the index";
    }

    // do a recursive call of the state restoration
    for (int i = 0; i < model()->rowCount(index); ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        restoreState(child);
    }

}

void AbstractAlbumTreeView::slotFixRowsInserted(const QModelIndex &index, int start, int end)
{

    kDebug() << "slot rowInserted called";

    for (int i = start; i <= end; ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        restoreState(child);
    }
}

void AbstractAlbumTreeView::saveViewState(KConfigGroup &configGroup, QString prefix)
{

    QStringList selection, expansion;
    for (int i = 0; i < model()->rowCount(); ++i)
    {
        const QModelIndex index = model()->index(i, 0);
        saveState(index, selection, expansion);
    }

    Album *selectedAlbum = albumFilterModel()->albumForIndex(selectionModel()->currentIndex());
    QString currentIndex;
    if (selectedAlbum)
    {
        currentIndex = QString::number(selectedAlbum->id());
    }

    configGroup.writeEntry(prefix + d->configSuffixSelection, selection);
    configGroup.writeEntry(prefix + d->configSuffixExpansion, expansion);
    configGroup.writeEntry(prefix + d->configSuffixCurrentIndex, currentIndex);
    configGroup.writeEntry(prefix + d->configSuffixSortColumn, albumFilterModel()->sortColumn());
    configGroup.writeEntry(prefix + d->configSuffixSortOrder, (int) albumFilterModel()->sortOrder());

}

void AbstractAlbumTreeView::saveState(const QModelIndex &index, QStringList &selection,
                QStringList &expansion)
{
    const QString cfgKey = QString::number(albumFilterModel()->albumForIndex(
                    index)->id());
    if (selectionModel()->isSelected(index))
        selection.append(cfgKey);
    if (isExpanded(index))
        expansion.append(cfgKey);
    for (int i = 0; i < model()->rowCount(index); ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        saveState(child, selection, expansion);
    }
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

void AbstractCountingAlbumTreeView::updateShowCountState(const QModelIndex& index, bool recurse)
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

void AbstractCountingAlbumTreeView::slotCollapsed(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->includeChildrenCount(m_albumFilterModel->mapToSource(index));
}

void AbstractCountingAlbumTreeView::slotExpanded(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->excludeChildrenCount(m_albumFilterModel->mapToSource(index));
}

void AbstractCountingAlbumTreeView::slotSetShowCount()
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->setShowCount(AlbumSettings::instance()->getShowFolderTreeViewItemsCount());
}

void AbstractCountingAlbumTreeView::slotRowsInserted(const QModelIndex& parent, int start, int end)
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

AlbumTreeView::AlbumTreeView(AlbumModel *model, QWidget *parent)
    : AbstractCheckableAlbumTreeView(model, parent)
{
    albumModel()->setDragDropHandler(new AlbumDragDropHandler(albumModel()));

    connect(AlbumManager::instance(), SIGNAL(signalPAlbumsDirty(const QMap<int, int>&)),
             m_albumModel, SLOT(setCountMap(const QMap<int, int>&)));

    expand(m_albumFilterModel->rootAlbumIndex());
    setRootIsDecorated(false);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(300);
}

AlbumTreeView::~AlbumTreeView()
{
}

AlbumModel *AlbumTreeView::albumModel() const
{
    return dynamic_cast<AlbumModel*>(m_albumModel);
}

PAlbum* AlbumTreeView::currentAlbum() const
{
    return dynamic_cast<PAlbum*> (m_albumModel->albumForIndex(
                    m_albumFilterModel->mapToSource(currentIndex())));
}

PAlbum *AlbumTreeView::albumForIndex(const QModelIndex &index) const
{
    return dynamic_cast<PAlbum*> (m_albumModel->albumForIndex(
                    m_albumFilterModel->mapToSource(index)));
}

TagTreeView::TagTreeView(TagModel *model, QWidget *parent)
    : AbstractCheckableAlbumTreeView(model, parent)
{
    albumModel()->setDragDropHandler(new TagDragDropHandler(albumModel()));

    connect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(const QMap<int, int>&)),
             m_albumModel, SLOT(setCountMap(const QMap<int, int>&)));

    expand(m_albumFilterModel->rootAlbumIndex());
    setRootIsDecorated(false);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(300);
}

TagModel *TagTreeView::albumModel() const
{
    return static_cast<TagModel*>(m_albumModel);
}

TAlbum* TagTreeView::currentAlbum() const
{
    return dynamic_cast<TAlbum*> (m_albumModel->albumForIndex(
                    m_albumFilterModel->mapToSource(currentIndex())));
}

TAlbum *TagTreeView::albumForIndex(const QModelIndex &index) const
{
    return dynamic_cast<TAlbum*> (m_albumModel->albumForIndex(
                    m_albumFilterModel->mapToSource(index)));
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
