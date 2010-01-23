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
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QTimer>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kmenu.h>

// Local includes

#include "albumdragdrop.h"
#include "albummanager.h"
#include "albummodeldragdrophandler.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "contextmenuhelper.h"
#include "metadatamanager.h"
#include "tagdragdrop.h"
#include "tagmodificationhelper.h"

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

class AlbumTreeViewDelegate : public QStyledItemDelegate
{
public:
    AlbumTreeViewDelegate(AbstractAlbumTreeView *treeView = 0)
                : QStyledItemDelegate(treeView),
                  m_treeView(treeView), m_height(0)
    {
        updateHeight();
    }

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), m_height));
        return size;
    }

    void updateHeight()
    {
        int h = qMax(AlbumThumbnailLoader::instance()->thumbnailSize() + 2, m_treeView->fontMetrics().height());
        if (h % 2 > 0)
            ++h;
        setHeight(h);
    }

    void setHeight(int height)
    {
        if (m_height == height)
            return;
        m_height = height;
        emit sizeHintChanged(QModelIndex());
    }

protected:

    AbstractAlbumTreeView *m_treeView;
    int m_height;
};

class AbstractAlbumTreeViewPriv
{

public:
    AbstractAlbumTreeViewPriv() :
        configSelectionEntry("Selection"),
        configExpansionEntry("Expansion"),
        configCurrentIndexEntry("CurrentIndex"),
        configSortColumnEntry("SortColumn"),
        configSortOrderEntry("SortOrder")
    {
        delegate            = 0;
        expandOnSingleClick = false;
        selectAlbumOnClick  = false;
        selectOnContextMenu = true;
        enableContextMenu   = false;
        resizeColumnsTimer  = 0;
    }

    AlbumTreeViewDelegate *delegate;

    bool expandOnSingleClick;
    bool selectAlbumOnClick;
    bool selectOnContextMenu;
    bool enableContextMenu;

    QMap<int, State> statesByAlbumId;

    const QString configSelectionEntry;
    const QString configExpansionEntry;
    const QString configCurrentIndexEntry;
    const QString configSortColumnEntry;
    const QString configSortOrderEntry;

    QTimer       *resizeColumnsTimer;

};

AbstractAlbumTreeView::AbstractAlbumTreeView(AbstractSpecificAlbumModel *model, AlbumFilterModel *filterModel, QWidget *parent)
    : QTreeView(parent), StateSavingObject(this),
      m_albumModel(0), m_albumFilterModel(0),
      d(new AbstractAlbumTreeViewPriv)
{
    m_checkOnMiddleClick  = false;

    d->delegate = new AlbumTreeViewDelegate(this);
    setItemDelegate(d->delegate);
    setUniformRowHeights(true);

    d->resizeColumnsTimer = new QTimer(this);
    d->resizeColumnsTimer->setInterval(200);
    d->resizeColumnsTimer->setSingleShot(true);
    connect(d->resizeColumnsTimer, SIGNAL(timeout()),
            this, SLOT(adaptColumnsToContent()));

    m_albumModel       = model;

    if (filterModel)
        setAlbumFilterModel(filterModel);

    if (!m_albumModel->rootAlbum())
    {
        connect(m_albumModel, SIGNAL(rootAlbumAvailable()),
                 this, SLOT(slotRootAlbumAvailable()));
    }

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
             this, SLOT(albumSettingsChanged()));
}

AbstractAlbumTreeView::~AbstractAlbumTreeView()
{
    delete d;
}

void AbstractAlbumTreeView::setAlbumFilterModel(AlbumFilterModel *filterModel)
{
    if (filterModel == m_albumFilterModel)
        return;

    if (m_albumFilterModel)
    {
        disconnect(m_albumFilterModel);
    }

    if (selectionModel())
    {
        disconnect(selectionModel());
    }

    m_albumFilterModel = filterModel;
    m_albumFilterModel->setSourceAlbumModel(m_albumModel);

    connect(m_albumFilterModel, SIGNAL(filterChanged()),
             this, SLOT(slotFilterChanged()));

    setModel(m_albumFilterModel);

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex &)),
             this, SLOT(slotCurrentChanged()));

    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection &)),
             this, SLOT(slotCurrentChanged()));

    connect(m_albumFilterModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(adaptColumnsOnDataChange(const QModelIndex&, const QModelIndex&)));
    connect(m_albumFilterModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(adaptColumnsOnRowChange(const QModelIndex&, int, int)));
    connect(m_albumFilterModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(adaptColumnsOnRowChange(const QModelIndex&, int, int)));
    connect(m_albumFilterModel, SIGNAL(layoutChanged()),
            this, SLOT(adaptColumnsOnLayoutChange()));
    adaptColumnsToContent();

}

AbstractSpecificAlbumModel *AbstractAlbumTreeView::albumModel() const
{
    return m_albumModel;
}

AlbumFilterModel *AbstractAlbumTreeView::albumFilterModel() const
{
    return m_albumFilterModel;
}

void AbstractAlbumTreeView::setExpandOnSingleClick(bool doThat)
{
    d->expandOnSingleClick = doThat;
}

void AbstractAlbumTreeView::setSelectAlbumOnClick(bool selectOnClick)
{
    d->selectAlbumOnClick = selectOnClick;
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

    checkExpandedState(QModelIndex());
}

void AbstractAlbumTreeView::slotRootAlbumAvailable()
{
    expand(m_albumFilterModel->rootAlbumIndex());
}

bool AbstractAlbumTreeView::checkExpandedState(const QModelIndex& index)
{
    bool anyMatch = false;

    QModelIndex source_index = m_albumFilterModel->mapToSource(index);
    AlbumFilterModel::MatchResult result = m_albumFilterModel->matchResult(source_index);
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

void AbstractAlbumTreeView::slotSelectAlbum(Album *album)
{
    setCurrentIndex(albumFilterModel()->indexForAlbum(album));
    AlbumManager::instance()->setCurrentAlbum(album);
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

    if (d->selectAlbumOnClick && e->button() == Qt::LeftButton)
    {
        QModelIndex index = indexVisuallyAt(e->pos());
        if (index.isValid())
        {
            AlbumManager::instance()->setCurrentAlbum(m_albumFilterModel->albumForIndex(index));
        }
    }

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
        drag->exec(supportedActions, Qt::IgnoreAction);
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
        Qt::DropAction action = handler->accepts(e, m_albumFilterModel->mapToSourceAlbumModel(index));
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
        if (handler->dropEvent(this, e, m_albumFilterModel->mapToSourceAlbumModel(index)))
            e->accept();
    }
}

void AbstractAlbumTreeView::doLoadState()
{

    KConfigGroup configGroup = getConfigGroup();

    kDebug() << "Loading view state from " << configGroup.name();

    const QStringList selection = configGroup.readEntry(entryName(d->configSelectionEntry),
                    QStringList());
    kDebug() << "selection: " << selection;
    foreach(const QString &key, selection)
    {
        bool validId;
        int id = key.toInt(&validId);
        if (validId)
        {
            d->statesByAlbumId[id].selected = true;
        }
    }

    const QStringList expansion = configGroup.readEntry(entryName(d->configExpansionEntry),
                    QStringList());
    kDebug() << "expansion: " << expansion;
    foreach( const QString &key, expansion )
    {
        bool validId;
        int id = key.toInt(&validId);
        if (validId)
        {
            d->statesByAlbumId[id].expanded = true;
        }
    }

    const QString key = configGroup.readEntry(entryName(d->configCurrentIndexEntry), QString());
    kDebug() << "currentIndey: " << key;
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
    kDebug() << "initial restore run with " << model()->rowCount() << " rows";
    for (int i = 0; i < model()->rowCount(); ++i)
    {
        const QModelIndex index = model()->index(i, 0);
        restoreState(index);
    }

    if (!d->statesByAlbumId.empty())
    {
        // and the watch the model for new items added
        connect(model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                         SLOT(slotFixRowsInserted(QModelIndex, int, int)), Qt::QueuedConnection);
    }

    // also restore the sorting order
    sortByColumn(configGroup.readEntry(entryName(d->configSortColumnEntry), 0),
                 (Qt::SortOrder) configGroup.readEntry(entryName(d->configSortOrderEntry), (int) Qt::AscendingOrder));

}

void AbstractAlbumTreeView::restoreState(const QModelIndex &index)
{

    Album *album = albumFilterModel()->albumForIndex(index);
    if (album)
    {

        Digikam::State state = d->statesByAlbumId[album->id()];

        /*kDebug() << "Trying to restore state of album " << album->title()
                 << ": state(selected = " << state.selected
                 << ", expanded = " << state.expanded
                 << ", currentIndex = " << state.currentIndex << ")";*/
        if (state.selected)
        {
            selectionModel()->select(index, QItemSelectionModel::Select
                            | QItemSelectionModel::Rows);
        }
        if (state.expanded)
        {
            setExpanded(index, true);
        }
        if (state.currentIndex)
        {
            setCurrentIndex(index);
        }

        // remove this state so that we don't get in trouble later in case the
        // same album id is reused again
        d->statesByAlbumId.remove(album->id());

        if (d->statesByAlbumId.empty())
        {
            // disconnect if not needed anymore
            disconnect(model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                       this, SLOT(slotFixRowsInserted(QModelIndex, int, int)));
        }

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

    kDebug() << "slot rowInserted called with index = " << index
             << ", start = " << start << ", end = " << end;

    for (int i = start; i <= end; ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        restoreState(child);
    }
}

void AbstractAlbumTreeView::adaptColumnsToContent()
{
    resizeColumnToContents(0);
}

void AbstractAlbumTreeView::adaptColumnsOnDataChange(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    if (!d->resizeColumnsTimer->isActive())
        d->resizeColumnsTimer->start();
}

void AbstractAlbumTreeView::adaptColumnsOnRowChange(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    if (!d->resizeColumnsTimer->isActive())
        d->resizeColumnsTimer->start();
}

void AbstractAlbumTreeView::adaptColumnsOnLayoutChange()
{
    if (!d->resizeColumnsTimer->isActive())
        d->resizeColumnsTimer->start();
}

void AbstractAlbumTreeView::doSaveState()
{

    KConfigGroup configGroup = getConfigGroup();

    QStringList selection, expansion;
    for (int i = 0; i < model()->rowCount(); ++i)
    {
        const QModelIndex index = model()->index(i, 0);
        saveStateRecursive(index, selection, expansion);
    }

    Album *selectedAlbum = albumFilterModel()->albumForIndex(selectionModel()->currentIndex());
    QString currentIndex;
    if (selectedAlbum)
    {
        currentIndex = QString::number(selectedAlbum->id());
    }

    kDebug() << "selection: " << selection;
    kDebug() << "expansion: " << expansion;
    kDebug() << "currentIndex: " << currentIndex;

    configGroup.writeEntry(entryName(d->configSelectionEntry), selection);
    configGroup.writeEntry(entryName(d->configExpansionEntry), expansion);
    configGroup.writeEntry(entryName(d->configCurrentIndexEntry), currentIndex);
    configGroup.writeEntry(entryName(d->configSortColumnEntry), albumFilterModel()->sortColumn());
    configGroup.writeEntry(entryName(d->configSortOrderEntry), (int) albumFilterModel()->sortOrder());

}

void AbstractAlbumTreeView::saveStateRecursive(const QModelIndex &index, QStringList &selection,
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
        saveStateRecursive(child, selection, expansion);
    }
}

void AbstractAlbumTreeView::setEnableContextMenu(bool enable)
{
    d->enableContextMenu = enable;
}

bool AbstractAlbumTreeView::showContextMenuAt(QContextMenuEvent *event, Album *albumForEvent)
{
    Q_UNUSED(event);
    return albumForEvent;
}

QPixmap AbstractAlbumTreeView::contextMenuIcon() const
{
    return SmallIcon("digikam");
}

QString AbstractAlbumTreeView::contextMenuTitle() const
{
    return i18n("Context menu");
}

void AbstractAlbumTreeView::contextMenuEvent(QContextMenuEvent *event)
{

    if (!d->enableContextMenu)
    {
        return;
    }

    Album *album = albumFilterModel()->albumForIndex(
                    indexAt(event->pos()));

    if (!showContextMenuAt(event, album))
    {
        return;
    }

    // switch to the sekected album if need
    if(d->selectOnContextMenu && album)
    {
        slotSelectAlbum(album);
    }

    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(contextMenuIcon(), contextMenuTitle());
    ContextMenuHelper cmhelper(&popmenu);

    addCustomContextMenuActions(cmhelper, album);

    QAction* choice = cmhelper.exec(QCursor::pos());
    handleCustomContextMenuAction(choice, album);

}

void AbstractAlbumTreeView::setSelectOnContextMenu(bool select)
{
    d->selectOnContextMenu = select;
}

void AbstractAlbumTreeView::addCustomContextMenuActions(ContextMenuHelper &cmh, Album *album)
{
    Q_UNUSED(cmh);
    Q_UNUSED(album);
}

void AbstractAlbumTreeView::handleCustomContextMenuAction(QAction *action, Album *album)
{
    Q_UNUSED(action);
    Q_UNUSED(album);
}

void AbstractAlbumTreeView::albumSettingsChanged()
{
    d->delegate->updateHeight();
}

// --------------------------------------- //

AbstractCountingAlbumTreeView::AbstractCountingAlbumTreeView(AbstractCountingAlbumModel *model,
                                                             AlbumFilterModel *filterModel, QWidget *parent)
    : AbstractAlbumTreeView(model, filterModel, parent)
{
    // install our own connections
    if (filterModel)
        setAlbumFilterModel(filterModel);
    setupConnections();
}

AbstractCountingAlbumTreeView::AbstractCountingAlbumTreeView(AbstractCountingAlbumModel *model, QWidget *parent)
    : AbstractAlbumTreeView(model, 0, parent)
{
    AlbumFilterModel *filterModel = new AlbumFilterModel(this);
    setAlbumFilterModel(filterModel);
    setupConnections();
}


void AbstractCountingAlbumTreeView::setupConnections()
{
    connect(this, SIGNAL(expanded(const QModelIndex &)),
             this, SLOT(slotExpanded(const QModelIndex &)));

    connect(this, SIGNAL(collapsed(const QModelIndex &)),
             this, SLOT(slotCollapsed(const QModelIndex &)));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
             this, SLOT(slotSetShowCount()));

    slotSetShowCount();
}

void AbstractCountingAlbumTreeView::setAlbumFilterModel(AlbumFilterModel *filterModel)
{
    AbstractAlbumTreeView::setAlbumFilterModel(filterModel);

    connect(m_albumFilterModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
             this, SLOT(slotRowsInserted(const QModelIndex &, int, int)));

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
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->includeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
}

void AbstractCountingAlbumTreeView::slotExpanded(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->excludeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
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

class AbstractCheckableAlbumTreeViewPriv
{
public:

    AbstractCheckableAlbumTreeViewPriv() :
        configCheckedAlbumsEntry("Checked"),
        configRestoreCheckedEntry("RestoreChecked")
    {
    }

    const QString configCheckedAlbumsEntry;
    const QString configRestoreCheckedEntry;

    QList<int> checkedAlbumIds;

};

AbstractCheckableAlbumTreeView::AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model,
                                                               CheckableAlbumFilterModel *filterModel, QWidget *parent)
    : AbstractCountingAlbumTreeView(model, filterModel, parent),
      d(new AbstractCheckableAlbumTreeViewPriv)
{
    m_checkOnMiddleClick = true;
    m_restoreCheckState  = false;
}

AbstractCheckableAlbumTreeView::AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model, QWidget *parent)
    : AbstractCountingAlbumTreeView(model, 0, parent),
      d(new AbstractCheckableAlbumTreeViewPriv)
{
    m_checkOnMiddleClick = true;
    m_restoreCheckState  = false;

    CheckableAlbumFilterModel *filterModel = new CheckableAlbumFilterModel(this);
    setAlbumFilterModel(filterModel);
}

AbstractCheckableAlbumTreeView::~AbstractCheckableAlbumTreeView()
{
    delete d;
}

AbstractCheckableAlbumModel *AbstractCheckableAlbumTreeView::checkableModel() const
{
    return dynamic_cast<AbstractCheckableAlbumModel*>(m_albumModel);
}

CheckableAlbumFilterModel *AbstractCheckableAlbumTreeView::checkableAlbumFilterModel() const
{
    return dynamic_cast<CheckableAlbumFilterModel*> (m_albumFilterModel);
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

bool AbstractCheckableAlbumTreeView::isRestoreCheckState() const
{
    return m_restoreCheckState;
}

void AbstractCheckableAlbumTreeView::setRestoreCheckState(bool restore)
{
    m_restoreCheckState = restore;
}

void AbstractCheckableAlbumTreeView::doLoadState()
{
    AbstractCountingAlbumTreeView::doLoadState();

    KConfigGroup group = getConfigGroup();

    m_restoreCheckState = group.readEntry(entryName(d->configRestoreCheckedEntry), false);

    if (!m_restoreCheckState || !checkableModel()->isCheckable())
    {
        return;
    }

    QStringList checkedAlbums = group.readEntry(entryName(
                    d->configCheckedAlbumsEntry), QStringList());

    d->checkedAlbumIds.clear();
    foreach(const QString &albumId, checkedAlbums)
    {
        bool ok;
        int id = albumId.toInt(&ok);
        if (ok)
        {
            d->checkedAlbumIds << id;
        }
    }

    // initially sync with the albums that are already in the model
    for (int i = 0; i < checkableModel()->rowCount(); ++i)
    {
        const QModelIndex index = checkableModel()->index(i, 0);
        restoreCheckState(index);
    }

    // wait for missing albums in the background
    if (!d->checkedAlbumIds.empty())
    {
        // and the watch the model for new items added
        connect(checkableModel(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                this,  SLOT(slotRowsAddedCheckState(QModelIndex, int, int)), Qt::QueuedConnection);
    }

}

void AbstractCheckableAlbumTreeView::slotRowsAddedCheckState(QModelIndex index, int start, int end)
{
    for (int i = start; i <= end; ++i)
    {
        const QModelIndex child = checkableModel()->index(i, 0, index);
        restoreCheckState(child);
    }
}

void AbstractCheckableAlbumTreeView::restoreCheckState(const QModelIndex &index)
{

    Album *album = checkableModel()->albumForIndex(index);
    if (album)
    {

        if (d->checkedAlbumIds.contains(album->id()))
        {
            checkableModel()->setCheckState(album, Qt::Checked);
            d->checkedAlbumIds.removeOne(album->id());
            if (d->checkedAlbumIds.empty())
            {
                // disconnect if not needed anymore
                disconnect(model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                           this, SLOT(slotRowsAddedCheckState(QModelIndex, int, int)));
            }
        }

    }

    // recurse children
    for (int i = 0; i < checkableModel()->rowCount(index); ++i)
    {
        const QModelIndex child = checkableModel()->index(i, 0, index);
        restoreCheckState(child);
    }

}

void AbstractCheckableAlbumTreeView::doSaveState()
{
    AbstractCountingAlbumTreeView::doSaveState();

    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configRestoreCheckedEntry), m_restoreCheckState);

    if (!m_restoreCheckState || !checkableModel()->isCheckable())
    {
        return;
    }

    QList<Album*> checkedAlbums = checkableModel()->checkedAlbums();
    QStringList checkedIds;
    foreach(Album* album, checkedAlbums)
    {
        checkedIds << QString::number(album->id());
    }

    group.writeEntry(entryName(d->configCheckedAlbumsEntry), checkedIds);

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
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

PAlbum *AlbumTreeView::albumForIndex(const QModelIndex &index) const
{
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(index));
}

// --------------------------------------- //

TagTreeView::TagTreeView(TagModel *model, QWidget *parent)
    : AbstractCheckableAlbumTreeView(model, parent)
{
    m_modificationHelper = new TagModificationHelper(this, this);

    albumModel()->setDragDropHandler(new TagDragDropHandler(albumModel()));

    connect(albumModel()->dragDropHandler(), SIGNAL(assignTags(const QList<int>&, const QList<int>&)),
            MetadataManager::instance(), SLOT(assignTags(const QList<int>&, const QList<int>&)));

    connect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(const QMap<int, int>&)),
             m_albumModel, SLOT(setCountMap(const QMap<int, int>&)));

    expand(m_albumFilterModel->rootAlbumIndex());
    if (m_albumModel->rootAlbumBehavior() == AbstractAlbumModel::IncludeRootAlbum)
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
    return dynamic_cast<TAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

TAlbum *TagTreeView::albumForIndex(const QModelIndex &index) const
{
    return dynamic_cast<TAlbum*> (m_albumFilterModel->albumForIndex(index));
}

TagModificationHelper *TagTreeView::tagModificationHelper() const
{
    return m_modificationHelper;
}

// --------------------------------------- //

SearchTreeView::SearchTreeView(QWidget *parent, SearchModel *searchModel)
    : AbstractCheckableAlbumTreeView(searchModel, parent)
{
    m_filteredModel = new SearchFilterModel(this);
    m_filteredModel->setSourceSearchModel(searchModel);

    AlbumFilterModel *albumFilterModel = new AlbumFilterModel(this);
    setAlbumFilterModel(albumFilterModel);
    albumFilterModel->setSourceAlbumModel(m_filteredModel);

    expand(m_albumFilterModel->rootAlbumIndex());
    setRootIsDecorated(false);
}

SearchModel *SearchTreeView::albumModel() const
{
    return static_cast<SearchModel*>(m_albumModel);
}

SearchFilterModel *SearchTreeView::filteredModel() const
{
    return m_filteredModel;
}

SAlbum* SearchTreeView::currentAlbum() const
{
    return dynamic_cast<SAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

void SearchTreeView::slotSelectSAlbum(SAlbum *salbum)
{
    slotSelectAlbum(salbum);
}

// --------------------------------------- //

DateAlbumTreeView::DateAlbumTreeView(QWidget *parent, DateAlbumModel *dateAlbumModel)
    : AbstractCountingAlbumTreeView(dateAlbumModel, parent)
{
    // this view should always show the inclusive counts
    disconnect(this, SIGNAL(expanded(const QModelIndex &)),
               this, SLOT(slotExpanded(const QModelIndex &)));
    disconnect(this, SIGNAL(collapsed(const QModelIndex &)),
               this, SLOT(slotCollapsed(const QModelIndex &)));
}

DateAlbumModel *DateAlbumTreeView::albumModel() const
{
    return static_cast<DateAlbumModel*>(m_albumModel);
}

DAlbum* DateAlbumTreeView::currentAlbum() const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

DAlbum *DateAlbumTreeView::albumForIndex(const QModelIndex &index) const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(index));
}

}
