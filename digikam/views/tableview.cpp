/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
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

#include "tableview.moc"

// Qt includes

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>

// KDE includes

#include <kmenu.h>
#include <kaction.h>

// local includes

/// @todo clean up includes
#include "imageposition.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "databasewatch.h"
#include "databasefields.h"
#include "digikam2kgeomap_database.h"
#include "importui.h"
#include "thumbnailloadthread.h"
#include "tableview_model.h"
#include "tableview_columnfactory.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

class TableViewTreeView::Private
{
public:
    Private()
      : headerContextMenuActiveColumn(-1),
        actionHeaderContextMenuRemoveColumn(0)
    {
    }

    int headerContextMenuActiveColumn;
    KAction* actionHeaderContextMenuRemoveColumn;
};

class TableView::Private
{
public:
    Private()
      : treeView(0),
        imageModel(0),
        selectionModel(0),
        columnProfiles()
    {
    }

    TableViewTreeView*      treeView;
    ImageAlbumModel*        imageModel;
    QItemSelectionModel*    selectionModel;
    QList<TableViewColumnProfile> columnProfiles;
};

TableView::TableView(
        QItemSelectionModel* const selectionModel,
        KCategorizedSortFilterProxyModel* const imageFilterModel,
        QWidget* const parent
    )
  : QWidget(parent),
    StateSavingObject(this),
    d(new Private()),
    s(new TableViewShared())
{
    s->thumbnailLoadThread = new ThumbnailLoadThread(this);
    s->imageFilterModel = dynamic_cast<ImageFilterModel*>(imageFilterModel);
    d->imageModel = dynamic_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel = selectionModel;
    s->columnFactory = new TableViewColumnFactory(s.data(), this);

    QVBoxLayout* const vbox1 = new QVBoxLayout();

    s->tableViewModel = new TableViewModel(s->columnFactory, s->imageFilterModel, this);
    s->sortModel = new TableViewSortFilterProxyModel(s.data(), this);
    d->treeView = new TableViewTreeView(s.data(), this);

    vbox1->addWidget(d->treeView);

    setLayout(vbox1);
}

TableView::~TableView()
{

}

void TableView::doLoadState()
{
    const KConfigGroup group = getConfigGroup();

    TableViewColumnProfile profile;
    const KConfigGroup groupCurrentProfile = group.group("Current Profile");
    /// @todo also store the QHeaderView settings into the column profile
    profile.loadSettings(groupCurrentProfile);
    s->tableViewModel->loadColumnProfile(profile);
}

void TableView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    TableViewColumnProfile profile = s->tableViewModel->getColumnProfile();
    KConfigGroup groupCurrentProfile = group.group("Current Profile");
    profile.saveSettings(groupCurrentProfile);
}

TableViewTreeView::TableViewTreeView(Digikam::TableViewShared* const tableViewShared, QWidget*const parent)
  : QTreeView(parent),
    d(new Private()),
    s(tableViewShared)
{
    s->itemDelegate = new TableViewItemDelegate(s, this);
    setItemDelegate(s->itemDelegate);

    d->actionHeaderContextMenuRemoveColumn = new KAction("Remove this column", this);
    connect(d->actionHeaderContextMenuRemoveColumn, SIGNAL(triggered(bool)),
            this, SLOT(slotHeaderContextMenuActionRemoveColumnTriggered()));

    header()->installEventFilter(this);

    setModel(s->sortModel);
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
//         s->sortModel=new TableViewSortFilterProxyModel(s,this);
//         s->sortModel->setSourceModel(s->tableViewModel);
        setModel(s->sortModel);
        return true;
    }

    return QObject::eventFilter(watched, event);
}

void TableViewTreeView::showHeaderContextMenu(QEvent* const event)
{
    QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
    QHeaderView* const headerView = header();

    d->headerContextMenuActiveColumn = headerView->logicalIndexAt(e->pos());
    KMenu* const menu = new KMenu(this);

    d->actionHeaderContextMenuRemoveColumn->setEnabled(
            s->tableViewModel->columnCount(QModelIndex())>1
        );
    menu->addAction(d->actionHeaderContextMenuRemoveColumn);
    menu->addSeparator();

    // add actions for all columns
    QList<TableViewColumnDescription> columnDescriptions = s->columnFactory->getColumnDescriptionList();
    for (int i = 0; i<columnDescriptions.count(); ++i)
    {
        const TableViewColumnDescription& desc = columnDescriptions.at(i);
        KAction* const action = new KAction(desc.columnTitle, menu);

        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(slotHeaderContextMenuAddColumn()));

        action->setData(QVariant::fromValue<TableViewColumnDescription>(desc));

        menu->addAction(action);
    }

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

TableViewItemDelegate::TableViewItemDelegate(TableViewShared* const tableViewShared, QObject* parent)
  : QItemDelegate(parent),
    s(tableViewShared)
{

}

TableViewItemDelegate::~TableViewItemDelegate()
{

}

void TableViewItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortedIndex) const
{
    const int columnIndex = sortedIndex.column();
    TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);

    const QModelIndex tableViewIndex = s->sortModel->mapToSource(sortedIndex);
    const QModelIndex sourceIndex = s->tableViewModel->toImageFilterModelIndex(tableViewIndex);

    if (!columnObject->paint(painter, option, sourceIndex))
    {
        QItemDelegate::paint(painter, option, sortedIndex);
    }
}

QSize TableViewItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortedIndex) const
{
    const int columnIndex = sortedIndex.column();
    TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);
    const QModelIndex tableViewIndex = s->sortModel->mapToSource(sortedIndex);
    const QModelIndex sourceIndex = s->tableViewModel->toImageFilterModelIndex(tableViewIndex);

    /// we have to take the maximum of all columns for the height
    /// @todo somehow cache this calculation
    const int columnCount = s->tableViewModel->columnCount(QModelIndex());
    int maxHeight = 0;
    for (int i=0; i<columnCount; ++i)
    {
        TableViewColumn* const iColumnObject = s->tableViewModel->getColumnObject(i);
        const QSize iColumnSize = iColumnObject->sizeHint(option, sourceIndex);
        if (iColumnSize.isValid())
        {
            maxHeight = qMax(maxHeight, iColumnSize.height());
        }
    }

    QSize columnSize = columnObject->sizeHint(option, sourceIndex);
    if (!columnSize.isValid())
    {
        columnSize = QItemDelegate::sizeHint(option, sortedIndex);
        /// @todo we have to incorporate the height given by QItemDelegate for the other columns, too
        maxHeight = qMax(maxHeight, columnSize.height());
    }

    return QSize(columnSize.width(), maxHeight);
}



} /* namespace Digikam */
