/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A view to display a list of images.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#include "gpsimagelist.h"

// Qt includes

#include <QDrag>
#include <QHeaderView>
#include <QWheelEvent>
#include <QMenu>
#include <QAction>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "gpsimageitemdelegate.h"
#include "gpsimagelistdragdrophandler.h"

namespace Digikam
{

class GPSImageList::Private
{
public:

    Private()
    : editEnabled(true),
      dragEnabled(false),
      model(0),
      selectionModel(0),
      itemDelegate(0),
      imageSortProxyModel(0),
      dragDropHandler(0)
    {
    }

    bool                      editEnabled;
    bool                      dragEnabled;
    GPSImageModel*            model;
    QItemSelectionModel*      selectionModel;
    GPSImageItemDelegate*     itemDelegate;
    GPSImageSortProxyModel*   imageSortProxyModel;
    ImageListDragDropHandler* dragDropHandler;
};

GPSImageList::GPSImageList(QWidget* const parent)
    : QTreeView(parent),
      d(new Private())
{
    header()->setSectionsMovable(true);
    setUniformRowHeights(true);
    setRootIsDecorated(false);
    setAlternatingRowColors(true);

    d->itemDelegate = new GPSImageItemDelegate(this, this);
    setItemDelegate(d->itemDelegate);
    setThumbnailSize(60);
    slotUpdateActionsEnabled();

    header()->installEventFilter(this);
}

GPSImageList::~GPSImageList()
{
    delete d;
}

void GPSImageList::startDrag(Qt::DropActions supportedActions)
{
    if (!d->dragDropHandler)
    {
        QTreeView::startDrag(supportedActions);
        return;
    }

    // NOTE: read the selected indices from the source selection model, not our selection model,
    // which is for the sorted model!
    const QList<QModelIndex> selectedIndicesFromModel = d->selectionModel->selectedIndexes();
    QList<QPersistentModelIndex> selectedIndices;

    for (int i = 0 ; i < selectedIndicesFromModel.count() ; ++i)
    {
        selectedIndices << selectedIndicesFromModel.at(i);
    }

    QMimeData* const dragMimeData = d->dragDropHandler->createMimeData(selectedIndices);

    if (!dragMimeData)
        return;

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(dragMimeData);
    drag->exec(Qt::CopyAction);
}

void GPSImageList::setModelAndSelectionModel(GPSImageModel* const model, QItemSelectionModel* const selectionModel)
{
    d->model               = model;
    d->selectionModel      = selectionModel;
    d->imageSortProxyModel = new GPSImageSortProxyModel(d->model, d->selectionModel);
    setModel(d->imageSortProxyModel);

    connect(d->model, SIGNAL(signalThumbnailForIndexAvailable(QPersistentModelIndex,QPixmap)),
            this, SLOT(slotThumbnailFromModel(QPersistentModelIndex,QPixmap)));

    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotInternalTreeViewImageActivated(QModelIndex)));

    if (d->imageSortProxyModel->mappedSelectionModel())
        setSelectionModel(d->imageSortProxyModel->mappedSelectionModel());
}

void GPSImageList::setDragDropHandler(ImageListDragDropHandler* const dragDropHandler)
{
    d->dragDropHandler = dragDropHandler;
}

GPSImageModel* GPSImageList::getModel() const
{
    return d->model;
}

void GPSImageList::setThumbnailSize(const int size)
{
    d->itemDelegate->setThumbnailSize(size);
    setColumnWidth(GPSImageItem::ColumnThumbnail, size);
}

void GPSImageList::slotIncreaseThumbnailSize()
{
    // TODO: pick reasonable limits and make sure we stay on multiples of 5
    const int currentThumbnailSize = d->itemDelegate->getThumbnailSize();

    if (currentThumbnailSize < 200)
        setThumbnailSize(currentThumbnailSize + 5);
}

void GPSImageList::slotDecreaseThumbnailSize()
{
    const int currentThumbnailSize = d->itemDelegate->getThumbnailSize();

    if (currentThumbnailSize > 30)
        setThumbnailSize(currentThumbnailSize - 5);
}

void GPSImageList::wheelEvent(QWheelEvent* we)
{
    if ((we->modifiers() & Qt::ControlModifier) == 0)
    {
        QTreeView::wheelEvent(we);
        return;
    }

    we->accept();

    if (we->delta() > 0)
    {
        slotIncreaseThumbnailSize();
    }
    else
    {
        slotDecreaseThumbnailSize();
    }
}

void GPSImageList::slotThumbnailFromModel(const QPersistentModelIndex& index, const QPixmap& /*pixmap*/)
{
    // TODO: verify that the size corresponds to the size of our thumbnails!
    update(d->imageSortProxyModel->mapFromSource(index));
}

void GPSImageList::saveSettingsToGroup(KConfigGroup* const group)
{
    group->writeEntry("Image List Thumbnail Size", d->itemDelegate->getThumbnailSize());
    group->writeEntry("Header State",              header()->saveState());
}

void GPSImageList::readSettingsFromGroup(const KConfigGroup* const group)
{
    setThumbnailSize(group->readEntry("Image List Thumbnail Size", 60));

    const QByteArray headerState = group->readEntry("Header State", QByteArray());

    if (!headerState.isEmpty())
    {
        header()->restoreState(headerState);
    }
    else
    {
        // by default, hide the advanced columns:
        header()->setSectionHidden(GPSImageItem::ColumnDOP,         true);
        header()->setSectionHidden(GPSImageItem::ColumnFixType,     true);
        header()->setSectionHidden(GPSImageItem::ColumnNSatellites, true);
    }
}

QItemSelectionModel* GPSImageList::getSelectionModel() const
{
    return d->selectionModel;
}

void GPSImageList::slotInternalTreeViewImageActivated(const QModelIndex& index)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << index << d->imageSortProxyModel->mapToSource(index);
    emit(signalImageActivated(d->imageSortProxyModel->mapToSource(index)));
}

GPSImageSortProxyModel* GPSImageList::getSortProxyModel() const
{
    return d->imageSortProxyModel;
}

void GPSImageList::setEditEnabled(const bool state)
{
    d->editEnabled = state;
    slotUpdateActionsEnabled();
}

void GPSImageList::setDragEnabled(const bool state)
{
    d->dragEnabled = state;
    slotUpdateActionsEnabled();
}

void GPSImageList::slotUpdateActionsEnabled()
{
    QTreeView::setDragEnabled(d->dragEnabled && d->editEnabled);

    if (d->dragEnabled && d->editEnabled)
    {
        QTreeView::setDragDropMode(QAbstractItemView::DragOnly);
    }
}

bool GPSImageList::eventFilter(QObject* watched, QEvent* event)
{
    QHeaderView* const headerView = header();

    if (!d->model || (watched != headerView) || (event->type() != QEvent::ContextMenu))
        return QWidget::eventFilter(watched, event);

    QMenu* const menu               = new QMenu(this);

    // add action for all the columns
    for (int i = 0 ; i < d->model->columnCount() ; ++i)
    {
        const QString columnName    = d->model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        const bool isVisible        = !headerView->isSectionHidden(i);

        QAction* const columnAction = new QAction(columnName, menu);
        columnAction->setCheckable(true);
        columnAction->setChecked(isVisible);
        columnAction->setData(i);

        menu->addAction(columnAction);
    }

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotColumnVisibilityActionTriggered(QAction*)));

    QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
    menu->exec(e->globalPos());

    return true;
}

void GPSImageList::slotColumnVisibilityActionTriggered(QAction* action)
{
    const int columnNumber     = action->data().toInt();
    const bool columnIsVisible = action->isChecked();

    header()->setSectionHidden(columnNumber, !columnIsVisible);
}

} // namespace Digikam
