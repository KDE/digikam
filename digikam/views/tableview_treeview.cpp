/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Tree view subelement
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tableview_treeview.moc"

// Qt includes

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>

// KDE includes

#include <kaction.h>
#include <kdebug.h>
#include <klinkitemselectionmodel.h>
#include <kmenu.h>

// local includes

/// @todo clean up includes
#include "contextmenuhelper.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "tableview_column_configuration_dialog.h"
#include "tableview_columnfactory.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "tableview_treeview_delegate.h"
#include "thumbnailsize.h"

namespace Digikam
{

class TableViewTreeView::Private
{
public:
    Private()
      : headerContextMenuActiveColumn(-1),
        actionHeaderContextMenuRemoveColumn(0),
        actionHeaderContextMenuConfigureColumn(0),
        dragDropThumbnailSize()
    {
    }

    int headerContextMenuActiveColumn;
    KAction* actionHeaderContextMenuRemoveColumn;
    KAction* actionHeaderContextMenuConfigureColumn;
    ThumbnailSize dragDropThumbnailSize;
};

TableViewTreeView::TableViewTreeView(TableViewShared* const tableViewShared, QWidget* const parent)
  : QTreeView(parent),
    d(new Private()),
    s(tableViewShared)
{
    s->itemDelegate = new TableViewItemDelegate(s, this);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setItemDelegate(s->itemDelegate);
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setAllColumnsShowFocus(true);
    setDragEnabled(true);
    setAcceptDrops(true);
//     viewport()->setAcceptDrops(true);

    d->actionHeaderContextMenuRemoveColumn = new KAction(KIcon("edit-table-delete-column"), i18n("Remove this column"), this);
    connect(d->actionHeaderContextMenuRemoveColumn, SIGNAL(triggered(bool)),
            this, SLOT(slotHeaderContextMenuActionRemoveColumnTriggered()));

    d->actionHeaderContextMenuConfigureColumn = new KAction(KIcon("configure"), i18n("Configure this column"), this);
    connect(d->actionHeaderContextMenuConfigureColumn, SIGNAL(triggered(bool)),
            this, SLOT(slotHeaderContextMenuConfigureColumn()));

    header()->installEventFilter(this);

    setModel(s->tableViewModel);
    setSelectionModel(s->tableViewSelectionModel);
    setSortingEnabled(true);
}

TableViewTreeView::~TableViewTreeView()
{

}

bool TableViewTreeView::eventFilter(QObject* watched, QEvent* event)
{
    QHeaderView* const headerView = header();

    if ( (watched==headerView) && (event->type()==QEvent::ContextMenu) )
    {
        showHeaderContextMenu(event);

        return true;
    }

    return QObject::eventFilter(watched, event);
}

void TableViewTreeView::addColumnDescriptionsToMenu(const QList<TableViewColumnDescription>& columnDescriptions, KMenu* const menu)
{
    for (int i = 0; i<columnDescriptions.count(); ++i)
    {
        const TableViewColumnDescription& desc = columnDescriptions.at(i);
        KAction* const action                  = new KAction(desc.columnTitle, menu);

        if (!desc.columnIcon.isEmpty())
        {
            action->setIcon(KIcon(desc.columnIcon));
        }

        if (desc.subColumns.isEmpty())
        {
            connect(action, SIGNAL(triggered(bool)),
                    this, SLOT(slotHeaderContextMenuAddColumn()));

            action->setData(QVariant::fromValue<TableViewColumnDescription>(desc));
        }
        else
        {
            KMenu* const subMenu = new KMenu(menu);
            addColumnDescriptionsToMenu(desc.subColumns, subMenu);

            action->setMenu(subMenu);
        }

        menu->addAction(action);
    }
}

void TableViewTreeView::showHeaderContextMenu(QEvent* const event)
{
    QContextMenuEvent* const e    = static_cast<QContextMenuEvent*>(event);
    QHeaderView* const headerView = header();

    d->headerContextMenuActiveColumn          = headerView->logicalIndexAt(e->pos());
    const TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(d->headerContextMenuActiveColumn);
    KMenu* const menu                         = new KMenu(this);

    d->actionHeaderContextMenuRemoveColumn->setEnabled(s->tableViewModel->columnCount(QModelIndex())>1);
    menu->addAction(d->actionHeaderContextMenuRemoveColumn);
    const bool columnCanConfigure = columnObject->getColumnFlags().testFlag(TableViewColumn::ColumnHasConfigurationWidget);
    d->actionHeaderContextMenuConfigureColumn->setEnabled(columnCanConfigure);
    menu->addAction(d->actionHeaderContextMenuConfigureColumn);
    menu->addSeparator();

    // add actions for all columns
    QList<TableViewColumnDescription> columnDescriptions = s->columnFactory->getColumnDescriptionList();
    addColumnDescriptionsToMenu(columnDescriptions, menu);

    menu->exec(e->globalPos());
}

void TableViewTreeView::slotHeaderContextMenuAddColumn()
{
    QAction* const triggeredAction = qobject_cast<QAction*>(sender());

    const QVariant actionData = triggeredAction->data();
    if (!actionData.canConvert<TableViewColumnDescription>())
    {
        return;
    }

    const TableViewColumnDescription desc = actionData.value<TableViewColumnDescription>();
    qDebug()<<"clicked: "<<desc.columnTitle;
    s->tableViewModel->addColumnAt(desc, d->headerContextMenuActiveColumn+1);
}

void TableViewTreeView::slotHeaderContextMenuActionRemoveColumnTriggered()
{
    qDebug()<<"remove column "<<d->headerContextMenuActiveColumn;
    s->tableViewModel->removeColumnAt(d->headerContextMenuActiveColumn);
}

void TableViewTreeView::slotHeaderContextMenuConfigureColumn()
{
    TableViewConfigurationDialog* const configurationDialog = new TableViewConfigurationDialog(s, d->headerContextMenuActiveColumn, this);
    const int result                                        = configurationDialog->exec();

    if (result!=QDialog::Accepted)
    {
        return;
    }

    const TableViewColumnConfiguration newConfiguration = configurationDialog->getNewConfiguration();
    s->tableViewModel->getColumnObject(d->headerContextMenuActiveColumn)->setConfiguration(newConfiguration);
}

AbstractItemDragDropHandler* TableViewTreeView::dragDropHandler() const
{
    kDebug()<<s->imageModel->dragDropHandler();
    return s->imageModel->dragDropHandler();
}

QModelIndex TableViewTreeView::mapIndexForDragDrop(const QModelIndex& index) const
{
    // "index" is a TableViewModel index.
    // We are using the drag-drop-handler of ImageModel, thus
    // we have to convert it to an index of ImageModel.

    // map to ImageModel
    const QModelIndex imageModelIndex = s->tableViewModel->toImageModelIndex(index);

    return imageModelIndex;
}

QPixmap TableViewTreeView::pixmapForDrag(const QList< QModelIndex >& indexes) const
{
    const QModelIndex& firstIndex = indexes.at(0);
    const ImageInfo info          = s->tableViewModel->imageInfo(firstIndex);
    const QString path            = info.filePath();

    QPixmap thumbnailPixmap;
    /// @todo The first thumbnail load always fails. We have to add thumbnail pre-generation
    ///       like in ImageModel. Getting thumbnails from ImageModel does not help, because it
    ///       does not necessarily prepare them the same way.
    /// @todo Make a central drag-drop thumbnail generator?
    if (!s->thumbnailLoadThread->find(path, thumbnailPixmap, d->dragDropThumbnailSize.size()))
    {
        /// @todo better default pixmap?
        thumbnailPixmap.fill();
    }

    /// @todo Decorate the pixmap like the other drag-drop implementations?
    /// @todo Write number of images onto the pixmap
    return thumbnailPixmap;

//     const QModelIndex& firstIndex = indexes.at(0);
//     const QModelIndex& imageModelIndex = s->sortModel->toImageModelIndex(firstIndex);
//     ImageModel* const imageModel = s->imageFilterModel->sourceImageModel();
//
//     /// @todo Determine how other views choose the size
//     const QSize thumbnailSize(60, 60);
//
//     imageModel->setData(imageModelIndex, qMax(thumbnailSize.width(), thumbnailSize.height()), ImageModel::ThumbnailRole);
//     QVariant thumbnailData = imageModel->data(imageModelIndex, ImageModel::ThumbnailRole);
//     imageModel->setData(imageModelIndex, QVariant(), ImageModel::ThumbnailRole);
//
//     QPixmap thumbnailPixmap = thumbnailData.value<QPixmap>();
//
//     /// @todo Write number of images onto the pixmap
//     return thumbnailPixmap;
}

Album* TableViewTreeView::albumAt(const QPoint& pos) const
{
    Q_UNUSED(pos)

    ImageAlbumModel* const albumModel = qobject_cast<ImageAlbumModel*>(s->imageModel);

    if (albumModel)
    {
        return albumModel->currentAlbum();
    }

    return 0;
}

void TableViewTreeView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        const int delta = event->delta();

        if (delta > 0)
        {
            emit(signalZoomInStep());
        }
        else if (delta < 0)
        {
            emit(signalZoomOutStep());
        }

        event->accept();
        return;
    }

    QTreeView::wheelEvent(event);
}

} /* namespace Digikam */
