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

#include "abstractalbumtreeview_p.h"

namespace Digikam
{

AbstractAlbumTreeView::AbstractAlbumTreeView(QWidget* const parent, Flags flags)
    : QTreeView(parent),
      StateSavingObject(this),
      m_albumModel(0),
      m_albumFilterModel(0),
      m_dragDropHandler(0),
      m_checkOnMiddleClick(false),
      m_restoreCheckState(false),
      m_flags(flags),
      d(new Private)
{
    if (flags & CreateDefaultDelegate)
    {
        d->delegate = new AlbumTreeViewDelegate(this);
        setItemDelegate(d->delegate);
        setUniformRowHeights(true);
    }

    d->resizeColumnsTimer = new QTimer(this);
    d->resizeColumnsTimer->setInterval(200);
    d->resizeColumnsTimer->setSingleShot(true);

    d->contextMenuIcon  = QIcon::fromTheme(QLatin1String("digikam")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    d->contextMenuTitle = i18n("Context menu");

    connect(d->resizeColumnsTimer, SIGNAL(timeout()),
            this, SLOT(adaptColumnsToContent()));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(albumSettingsChanged()));

    connect(this, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(currentAlbumChangedForBackupSelection(Album*)));

    if (flags & CreateDefaultFilterModel)
    {
        setAlbumFilterModel(new AlbumFilterModel(this));
    }

    setSortingEnabled(true);
    albumSettingsChanged();
}

AbstractAlbumTreeView::~AbstractAlbumTreeView()
{
    delete d;
}

void AbstractAlbumTreeView::setAlbumModel(AbstractSpecificAlbumModel* const model)
{
    if (m_albumModel == model)
    {
        return;
    }

    if (m_albumModel)
    {
        disconnect(m_albumModel, 0, this, 0);
    }

    m_albumModel = model;

    if (m_albumFilterModel)
    {
        m_albumFilterModel->setSourceAlbumModel(m_albumModel);
    }

    if (m_albumModel)
    {
        if (!m_albumModel->rootAlbum())
        {
            connect(m_albumModel, SIGNAL(rootAlbumAvailable()),
                    this, SLOT(slotRootAlbumAvailable()));
        }

        if (m_albumFilterModel)
        {
            expand(m_albumFilterModel->rootAlbumIndex());
        }
    }
}

void AbstractAlbumTreeView::setAlbumFilterModel(AlbumFilterModel* const filterModel)
{
    if (filterModel == m_albumFilterModel)
    {
        return;
    }

    if (m_albumFilterModel)
    {
        disconnect(m_albumFilterModel);
    }

    if (selectionModel())
    {
        disconnect(selectionModel());
    }

    m_albumFilterModel = filterModel;
    setModel(m_albumFilterModel);

    if (m_albumFilterModel)
    {
        m_albumFilterModel->setSourceAlbumModel(m_albumModel);

        connect(m_albumFilterModel, SIGNAL(searchTextSettingsAboutToChange(bool,bool)),
                this, SLOT(slotSearchTextSettingsAboutToChange(bool,bool)));

        connect(m_albumFilterModel, SIGNAL(searchTextSettingsChanged(bool,bool)),
                this, SLOT(slotSearchTextSettingsChanged(bool,bool)));

        // NOTE: When only single selection was available, everything was
        //       implemented using currentAlbum() which was equal with selectedAlbum()
        //       after enabling multiple selection they are no longer the same
        //       and some options must use selected others only currentAlbum
        //       Now AlbumManager implementation is a little bit of mess
        //       because selected are now currentAlbums()...

        connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(slotCurrentChanged()));

        connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotSelectionChanged()));

        connect(m_albumFilterModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(adaptColumnsOnDataChange(QModelIndex,QModelIndex)));

        connect(m_albumFilterModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(adaptColumnsOnRowChange(QModelIndex,int,int)));

        connect(m_albumFilterModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(adaptColumnsOnRowChange(QModelIndex,int,int)));

        connect(m_albumFilterModel, SIGNAL(layoutChanged()),
                this, SLOT(adaptColumnsOnLayoutChange()));

        adaptColumnsToContent();

        if (m_albumModel)
        {
            expand(m_albumFilterModel->rootAlbumIndex());
        }

        //m_albumFilterModel->setDynamicSortFilter(true);
    }
}

AbstractSpecificAlbumModel* AbstractAlbumTreeView::albumModel() const
{
    return m_albumModel;
}

AlbumFilterModel* AbstractAlbumTreeView::albumFilterModel() const
{
    return m_albumFilterModel;
}

void AbstractAlbumTreeView::setExpandOnSingleClick(const bool doThat)
{
    d->expandOnSingleClick = doThat;
}

void AbstractAlbumTreeView::setExpandNewCurrentItem(const bool doThat)
{
    d->expandNewCurrent = doThat;
}

void AbstractAlbumTreeView::setSelectAlbumOnClick(const bool selectOnClick)
{
    d->selectAlbumOnClick = selectOnClick;
}

QModelIndex AbstractAlbumTreeView::indexVisuallyAt(const QPoint& p)
{
    if (viewport()->rect().contains(p))
    {
        const QModelIndex index = indexAt(p);

        if (index.isValid() && visualRect(index).contains(p))
        {
            return index;
        }
    }

    return QModelIndex();
}

template<class A>
QList<A*> AbstractAlbumTreeView::currentAlbums()
{
    QList<A*> albums;
    const QList<Album*> currentAl = AlbumManager::instance()->currentAlbums();

    for (QList<Album*>::const_iterator it = currentAl.constBegin() ; it != currentAl.constEnd() ; ++it)
    {
        A* const item = dynamic_cast<A*>(*it);
        if (item)
        {
            albums.append(item);
        }
    }

    return albums;
}

void AbstractAlbumTreeView::slotSearchTextSettingsAboutToChange(bool searched, bool willSearch)
{
    // backup before we begin searching
    if (!searched && willSearch && d->searchBackup.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Searching started, backing up state";

        QList<int> selection, expansion;
        saveStateRecursive(QModelIndex(), selection, expansion);

        // selection is ignored here because the user may have changed this
        // while searching
        foreach (const int& expandedId, expansion)
        {
            d->searchBackup[expandedId].expanded = true;
        }

        // also backup the last selected album in case this didn't work via the
        // slot
        const QList<Album*> selList = selectedAlbums<Album>(selectionModel(),
                                                      m_albumFilterModel);
        if (!selList.isEmpty())
        {
            d->lastSelectedAlbum = selList.first();
        }
    }
}

void AbstractAlbumTreeView::slotSearchTextSettingsChanged(bool wasSearching, bool searched)
{
    // ensure that all search results are visible if there is currently a search
    // working
    if (searched)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Searched, expanding all results";
        expandMatches(QModelIndex());
    }

    // restore the tree view state if searching finished
    if (wasSearching && !searched && !d->searchBackup.isEmpty())
    {

        qCDebug(DIGIKAM_GENERAL_LOG) << "Searching finished, restoring tree view state";

        collapseAll();
        restoreStateForHierarchy(QModelIndex(), d->searchBackup);
        d->searchBackup.clear();

        if (d->lastSelectedAlbum)
        {
            setCurrentAlbums(QList<Album*>() << d->lastSelectedAlbum, false);
            // doing this twice somehow ensures that all parents are expanded
            // and we are at the right position. Maybe a hack... ;)
            scrollTo(m_albumFilterModel->indexForAlbum(d->lastSelectedAlbum));
            scrollTo(m_albumFilterModel->indexForAlbum(d->lastSelectedAlbum));
        }
    }
}

void AbstractAlbumTreeView::currentAlbumChangedForBackupSelection(Album* currentAlbum)
{
    d->lastSelectedAlbum = currentAlbum;
}

void AbstractAlbumTreeView::slotRootAlbumAvailable()
{
    expand(m_albumFilterModel->rootAlbumIndex());
}

bool AbstractAlbumTreeView::expandMatches(const QModelIndex& index)
{
    bool anyMatch = false;

    // expand index if a child matches
    const QModelIndex source_index             = m_albumFilterModel->mapToSource(index);
    const AlbumFilterModel::MatchResult result = m_albumFilterModel->matchResult(source_index);

    switch (result)
    {
        case AlbumFilterModel::NoMatch:

            if (index != rootIndex())
            {
                return false;
            }
            break;

        case AlbumFilterModel::ParentMatch:
            // Does not rule out additional child match, return value is unknown
            break;
        case AlbumFilterModel::DirectMatch:
            // Does not rule out additional child match, but we know we will return true
            anyMatch = true;
            break;
        case AlbumFilterModel::ChildMatch:
        case AlbumFilterModel::SpecialMatch:
            // We know already to expand, and we know already we will return true.
            anyMatch = true;
            expand(index);
            break;
    }

    // Recurse. Expand if children if have an (indirect) match
    const int rows = m_albumFilterModel->rowCount(index);

    for (int i = 0 ; i < rows ; ++i)
    {
        const QModelIndex child = m_albumFilterModel->index(i, 0, index);
        const bool childResult  = expandMatches(child);

        if (childResult)
        {
            anyMatch = true;
            // if there is a direct match _and_ a child match, do not forget to expand the parent
            expand(index);
        }
    }

    return anyMatch;
}

void AbstractAlbumTreeView::setSearchTextSettings(const SearchTextSettings& settings)
{
    m_albumFilterModel->setSearchTextSettings(settings);
}

void AbstractAlbumTreeView::setAlbumManagerCurrentAlbum(const bool set)
{
    d->setInAlbumManager = set;
}

void AbstractAlbumTreeView::setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager)
{
    if (!model())
    {
        return;
    }

    if (selectInAlbumManager && d->setInAlbumManager)
    {
        AlbumManager::instance()->setCurrentAlbums(albums);
    }

    setCurrentIndex(albumFilterModel()->indexForAlbum(albums.first()));

    QItemSelectionModel* const model = selectionModel();
    model->clearSelection();

    for (int it = 0 ; it < albums.size() ; ++it)
    {
        model->select(albumFilterModel()->indexForAlbum(albums.at(it)),
                      model->Select);
    }
}

void AbstractAlbumTreeView::slotCurrentChanged()
{
    // It seems that QItemSelectionModel::selectedIndexes() has not been updated at this point
    // and returns the previously selected items. Therefore the line below did not work.
//     QList<Album*> selected = selectedAlbums<Album>(selectionModel(),
//                                                    m_albumFilterModel);

    // Instead, we call QItemSelectionModel::currentIndex to get the current index.
    const QModelIndex cIndex = selectionModel()->currentIndex();

    if (!cIndex.isValid())
    {
        return;
    }

    Album* const cAlbum = m_albumFilterModel->albumForIndex(cIndex);

    if (!cAlbum)
    {
        return;
    }

    emit currentAlbumChanged(cAlbum);
}

void AbstractAlbumTreeView::slotSelectionChanged()
{
    /** Dead signal? Nobody listens to it **/
    //emit selectedAlbumsChanged(selectedAlbums<Album>(selectionModel(), m_albumFilterModel));
    if (d->selectAlbumOnClick)
    {
        AlbumManager::instance()->setCurrentAlbums(selectedAlbums<Album>(selectionModel(),
                                                                         m_albumFilterModel));
    }
}

void AbstractAlbumTreeView::mousePressEvent(QMouseEvent* e)
{
    const QModelIndex currentBefor = currentIndex();

    QTreeView::mousePressEvent(e);

    if ((d->expandOnSingleClick || d->expandNewCurrent) && e->button() == Qt::LeftButton)
    {
        const QModelIndex index = indexVisuallyAt(e->pos());

        if (index.isValid())
        {
            if (d->expandOnSingleClick)
            {
                // See bug #126871: collapse/expand treeview using left mouse button single click.
                // Exception: If a newly selected item is already expanded, do not collapse on selection.
                const bool expanded = isExpanded(index);

                if (index == currentIndex() || !expanded)
                {
                    setExpanded(index, !expanded);
                }
            }
            else
            {
                if (currentBefor != currentIndex())
                {
                    expand(index);
                }
            }
        }
    }
    else if (m_checkOnMiddleClick && e->button() == Qt::MidButton)
    {
        Album* const a = m_albumFilterModel->albumForIndex(indexAt(e->pos()));

        if (a)
        {
            middleButtonPressed(a);
        }
    }
}

void AbstractAlbumTreeView::middleButtonPressed(Album*)
{
    // reimplement if needed
}

void AbstractAlbumTreeView::startDrag(Qt::DropActions supportedActions)
{
    const QModelIndexList indexes = selectedIndexes();

    if (indexes.count() > 0)
    {
        QMimeData* data = m_albumFilterModel->mimeData(indexes);

        if (!data)
        {
            return;
        }

        QStyleOptionViewItem option = viewOptions();
        option.rect                 = viewport()->rect();
        const QPixmap pixmap        = /*m_delegate->*/pixmapForDrag(option, indexes);
        QDrag* const drag           = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->exec(supportedActions, Qt::CopyAction);
    }
}

//TODO: Move to delegate, when we have one.
//      Copy code from image delegate for creating icons when dragging multiple items
QPixmap AbstractAlbumTreeView::pixmapForDrag(const QStyleOptionViewItem&, QList<QModelIndex> indexes)
{
    if (indexes.isEmpty())
    {
        return QPixmap();
    }

    const QVariant decoration = indexes.first().data(Qt::DecorationRole);
    return decoration.value<QPixmap>();
}

void AbstractAlbumTreeView::dragEnterEvent(QDragEnterEvent* e)
{
    AlbumModelDragDropHandler* const handler = m_albumModel->dragDropHandler();

    if (handler && handler->acceptsMimeData(e->mimeData()))
    {
        setState(DraggingState);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void AbstractAlbumTreeView::dragMoveEvent(QDragMoveEvent* e)
{
    QTreeView::dragMoveEvent(e);
    AlbumModelDragDropHandler* const handler = m_albumModel->dragDropHandler();

    if (handler)
    {
        const QModelIndex index     = indexVisuallyAt(e->pos());
        const Qt::DropAction action = handler->accepts(e, m_albumFilterModel->mapToSourceAlbumModel(index));

        if (action == Qt::IgnoreAction)
        {
            e->ignore();
        }
        else
        {
            e->setDropAction(action);
            e->accept();
        }
    }
}

void AbstractAlbumTreeView::dragLeaveEvent(QDragLeaveEvent* e)
{
    QTreeView::dragLeaveEvent(e);
}

void AbstractAlbumTreeView::dropEvent(QDropEvent* e)
{
    QTreeView::dropEvent(e);
    AlbumModelDragDropHandler* const handler = m_albumModel->dragDropHandler();

    if (handler)
    {
        const QModelIndex index = indexVisuallyAt(e->pos());

        if (handler->dropEvent(this, e, m_albumFilterModel->mapToSourceAlbumModel(index)))
        {
            e->accept();
        }
    }
}

bool AbstractAlbumTreeView::viewportEvent(QEvent* event)
{
    return QTreeView::viewportEvent(event);
}

QList<Album*> AbstractAlbumTreeView::selectedItems()
{
    return selectedAlbums<Album>(selectionModel(), m_albumFilterModel);
}

void AbstractAlbumTreeView::doLoadState()
{
    KConfigGroup configGroup = getConfigGroup();

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Loading view state from " << this << configGroup.name() << objectName();

    // extract the selection from the config
    const QStringList selection = configGroup.readEntry(entryName(d->configSelectionEntry), QStringList());
    //qCDebug(DIGIKAM_GENERAL_LOG) << "selection: " << selection;

    foreach (const QString& key, selection)
    {
        bool validId;
        const int id = key.toInt(&validId);

        if (validId)
        {
            d->statesByAlbumId[id].selected = true;
        }
    }

    // extract expansion state from config
    const QStringList expansion = configGroup.readEntry(entryName(d->configExpansionEntry), QStringList());
    //qCDebug(DIGIKAM_GENERAL_LOG) << "expansion: " << expansion;

    // If no expansion was done, at least expand the root albums
    if (expansion.isEmpty())
    {
        QList<AlbumRootInfo> roots = CoreDbAccess().db()->getAlbumRoots();

        foreach (const AlbumRootInfo& info, roots)
        {
            int albumId = CoreDbAccess().db()->getAlbumForPath(info.id, QLatin1String("/"), false);

            if (albumId != -1)
            {
                d->statesByAlbumId[albumId].expanded = true;
            }
        }
    }
    else
    {
        foreach (const QString& key, expansion)
        {
            bool validId;
            const int id = key.toInt(&validId);

            if (validId)
            {
                d->statesByAlbumId[id].expanded = true;
            }
        }
    }

    // extract current index from config
    const QString key = configGroup.readEntry(entryName(d->configCurrentIndexEntry), QString());
    //qCDebug(DIGIKAM_GENERAL_LOG) << "currentIndex: " << key;
    bool validId;
    const int id      = key.toInt(&validId);

    if (validId)
    {
        d->statesByAlbumId[id].currentIndex = true;
    }

/*
    for (QMap<int, Digikam::State>::iterator it = d->statesByAlbumId.begin(); it
        != d->statesByAlbumId.end(); ++it)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "id = " << it.key() << ": recovered state (selected = "
        << it.value().selected << ", expanded = "
        << it.value().expanded << ", currentIndex = "
        << it.value().currentIndex << ")";
    }
*/

    // initial restore run, for everything already loaded
    //qCDebug(DIGIKAM_GENERAL_LOG) << "initial restore run with " << model()->rowCount() << " rows";
    restoreStateForHierarchy(QModelIndex(), d->statesByAlbumId);

    // also restore the sorting order
    sortByColumn(configGroup.readEntry(entryName(d->configSortColumnEntry), 0),
                 (Qt::SortOrder) configGroup.readEntry(entryName(d->configSortOrderEntry), (int)Qt::AscendingOrder));

    // use a timer to scroll to the first possible selected album
    QTimer::singleShot(200, this, SLOT(scrollToSelectedAlbum()));
}

void AbstractAlbumTreeView::restoreStateForHierarchy(const QModelIndex& index, const QMap<int, Digikam::State>& stateStore)
{
    restoreState(index, stateStore);

    // do a recursive call of the state restoration
    for (int i = 0 ; i < model()->rowCount(index) ; ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        restoreStateForHierarchy(child, stateStore);
    }
}

void AbstractAlbumTreeView::restoreState(const QModelIndex& index, const QMap<int, Digikam::State>& stateStore)
{
    Album* const album = albumFilterModel()->albumForIndex(index);

    if (album && stateStore.contains(album->id()))
    {

        Digikam::State state = stateStore.value(album->id());

/*
        qCDebug(DIGIKAM_GENERAL_LOG) << "Trying to restore state of album " << album->title() << "(" <<album->id() << ")"
                 << ": state(selected = " << state.selected
                 << ", expanded = " << state.expanded
                 << ", currentIndex = " << state.currentIndex << ")" << this;
*/
        // Block signals to prevent that the searches started when the last
        // selected index is restored when loading the GUI
        selectionModel()->blockSignals(true);

        if (state.selected)
        {
            //qCDebug(DIGIKAM_GENERAL_LOG) << "Selecting" << album->title();
            selectionModel()->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }

        // Restore expansion state but ensure that the root album is always expanded
        if (!album->isRoot())
        {
            setExpanded(index, state.expanded);
        }
        else
        {
            setExpanded(index, true);
        }

        // restore the current index
        if (state.currentIndex)
        {
            //qCDebug(DIGIKAM_GENERAL_LOG) << "Setting current index" << album->title() << "(" << album->id() << ")";
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }

        selectionModel()->blockSignals(false);
    }
}

void AbstractAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);

    if (!d->statesByAlbumId.isEmpty())
    {
        //qCDebug(DIGIKAM_GENERAL_LOG) << "slot rowInserted called with index = " << index
        //         << ", start = " << start << ", end = " << end << "remaining ids" << d->statesByAlbumId.keys();

        // Restore state for parent a second time - expansion can only be restored if there are children
        restoreState(parent, d->statesByAlbumId);

        for (int i = start ; i <= end ; ++i)
        {
            const QModelIndex child = model()->index(i, 0, parent);
            restoreState(child, d->statesByAlbumId);
        }
    }
}

void AbstractAlbumTreeView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    QTreeView::rowsAboutToBeRemoved(parent, start, end);

    // Clean up map if album id is reused for a new album
    if (!d->statesByAlbumId.isEmpty())
    {
        for (int i = start ; i <= end ; ++i)
        {
            const QModelIndex child = model()->index(i, 0, parent);
            Album* const album      = albumModel()->albumForIndex(child);

            if (album)
            {
                d->statesByAlbumId.remove(album->id());
            }
        }
    }
}

void AbstractAlbumTreeView::adaptColumnsToContent()
{
    resizeColumnToContents(0);
}

void AbstractAlbumTreeView::scrollToSelectedAlbum()
{
    const QModelIndexList selected = selectedIndexes();

    if (!selected.isEmpty())
    {
        scrollTo(selected.first(), PositionAtCenter);
        horizontalScrollBar()->setValue(0);
    }
}

void AbstractAlbumTreeView::expandEverything(const QModelIndex& index)
{
    for (int row = 0 ; row < albumFilterModel()->rowCount(index) ; ++row)
    {
        const QModelIndex rowIndex = albumFilterModel()->index(row, 0, index);
        expand(rowIndex);
        expandEverything(rowIndex);
    }
}

void AbstractAlbumTreeView::adaptColumnsOnDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    if (!d->resizeColumnsTimer->isActive())
    {
        d->resizeColumnsTimer->start();
    }
}

void AbstractAlbumTreeView::adaptColumnsOnRowChange(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    if (!d->resizeColumnsTimer->isActive())
    {
        d->resizeColumnsTimer->start();
    }
}

void AbstractAlbumTreeView::adaptColumnsOnLayoutChange()
{
    if (!d->resizeColumnsTimer->isActive())
    {
        d->resizeColumnsTimer->start();
    }
}

void AbstractAlbumTreeView::doSaveState()
{
    KConfigGroup configGroup = getConfigGroup();

    QList<int> selection, expansion;

    for (int i = 0 ; i < model()->rowCount() ; ++i)
    {
        const QModelIndex index = model()->index(i, 0);
        saveStateRecursive(index, selection, expansion);
    }

    Album* const selectedAlbum = albumFilterModel()->albumForIndex(selectionModel()->currentIndex());
    QString currentIndex;

    if (selectedAlbum)
    {
        currentIndex = QString::number(selectedAlbum->id());
    }

    configGroup.writeEntry(entryName(d->configSelectionEntry),    selection);
    configGroup.writeEntry(entryName(d->configExpansionEntry),    expansion);
    configGroup.writeEntry(entryName(d->configCurrentIndexEntry), currentIndex);
    configGroup.writeEntry(entryName(d->configSortColumnEntry),   albumFilterModel()->sortColumn());

    // A dummy way to force the tree view to resort if the album sort role changed

    if (ApplicationSettings::instance()->getAlbumSortChanged())
    {
        if (int(albumFilterModel()->sortOrder()) == 0)
        {
            configGroup.writeEntry(entryName(d->configSortOrderEntry), 1);
        }
        else
        {
            configGroup.writeEntry(entryName(d->configSortOrderEntry), 0);
        }
    }
    else
    {
        configGroup.writeEntry(entryName(d->configSortOrderEntry), int(albumFilterModel()->sortOrder()));
    }
}

void AbstractAlbumTreeView::saveStateRecursive(const QModelIndex& index, QList<int>& selection, QList<int>& expansion)
{
    Album* const album = albumFilterModel()->albumForIndex(index);

    if (album)
    {
        const int id = album->id();

        if (selectionModel()->isSelected(index))
        {
            selection.append(id);
        }

        if (isExpanded(index))
        {
            expansion.append(id);
        }
    }

    for (int i = 0 ; i < model()->rowCount(index) ; ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        saveStateRecursive(child, selection, expansion);
    }
}

void AbstractAlbumTreeView::setEnableContextMenu(const bool enable)
{
    d->enableContextMenu = enable;
}

bool AbstractAlbumTreeView::showContextMenuAt(QContextMenuEvent* event, Album* albumForEvent)
{
    Q_UNUSED(event);
    return albumForEvent;
}

void AbstractAlbumTreeView::setContextMenuIcon(const QPixmap& pixmap)
{
    d->contextMenuIcon = pixmap;
}

void AbstractAlbumTreeView::setContextMenuTitle(const QString& title)
{
    d->contextMenuTitle = title;
}

QPixmap AbstractAlbumTreeView::contextMenuIcon() const
{
    return d->contextMenuIcon;
}

QString AbstractAlbumTreeView::contextMenuTitle() const
{
    return d->contextMenuTitle;
}

void AbstractAlbumTreeView::addContextMenuElement(ContextMenuElement* element)
{
    d->contextMenuElements << element;
}

void AbstractAlbumTreeView::removeContextMenuElement(ContextMenuElement* element)
{
    d->contextMenuElements.removeAll(element);
}

QList<AbstractAlbumTreeView::ContextMenuElement*> AbstractAlbumTreeView::contextMenuElements() const
{
    return d->contextMenuElements;
}

void AbstractAlbumTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    if (!d->enableContextMenu)
    {
        return;
    }

    Album* const album = albumFilterModel()->albumForIndex(indexAt(event->pos()));

    if (!album)
    {
        return;
    }

    if (album->isTrashAlbum())
    {
        // For the moment, disabling context menu for trash.
        // TODO : customize it.
        return;
    }

    if (!showContextMenuAt(event, album))
    {
        return;
    }

    // switch to the selected album if need
    if (d->selectOnContextMenu)
    {
        setCurrentAlbums(QList<Album*>() << album);
    }

    // --------------------------------------------------------

    QMenu* const popmenu = new QMenu(this);
    popmenu->addSection(contextMenuIcon(), contextMenuTitle());
    ContextMenuHelper cmhelper(popmenu);

    addCustomContextMenuActions(cmhelper, album);

    foreach (ContextMenuElement* const element, d->contextMenuElements)
    {
        element->addActions(this, cmhelper, album);
    }

    AlbumPointer<Album> albumPointer(album);
    QAction* const choice = cmhelper.exec(QCursor::pos());
    handleCustomContextMenuAction(choice, albumPointer);
}

void AbstractAlbumTreeView::setSelectOnContextMenu(const bool select)
{
    d->selectOnContextMenu = select;
}

void AbstractAlbumTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    Q_UNUSED(cmh);
    Q_UNUSED(album);
}

void AbstractAlbumTreeView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    Q_UNUSED(action);
    Q_UNUSED(album);
}

void AbstractAlbumTreeView::albumSettingsChanged()
{
    setFont(ApplicationSettings::instance()->getTreeViewFont());

    if (d->delegate)
    {
        d->delegate->updateHeight();
    }
}

} // namespace Digikam
