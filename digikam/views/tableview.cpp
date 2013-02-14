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
      : tableViewColumnFactory(0),
        headerContextMenuActiveColumn(-1),
        actionHeaderContextMenuRemoveColumn(0)
    {
    }

    TableViewColumnFactory* tableViewColumnFactory;
    int headerContextMenuActiveColumn;
    KAction* actionHeaderContextMenuRemoveColumn;
    TableViewModel* tableViewModel;
};

class TableView::Private
{
public:
    Private()
      : treeView(0),
        imageFilterModel(0),
        imageModel(0),
        selectionModel(0),
        tableViewModel(0)
    {
    }

    TableViewTreeView*      treeView;
    ImageFilterModel*       imageFilterModel;
    ImageAlbumModel*        imageModel;
    QItemSelectionModel*    selectionModel;
    TableViewModel*         tableViewModel;
    TableViewColumnFactory* tableViewColumnFactory;
    TableViewColumnDataSource* tableViewColumnDataSource;
};

TableView::TableView(
        QItemSelectionModel* const selectionModel,
        KCategorizedSortFilterProxyModel* const imageFilterModel,
        QWidget* const parent
    )
  : QWidget(parent),
    StateSavingObject(this),
    d(new Private())
{
    d->imageFilterModel = dynamic_cast<ImageFilterModel*>(imageFilterModel);
    d->imageModel = dynamic_cast<ImageAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel = selectionModel;
    d->tableViewColumnDataSource = new TableViewColumnDataSource();
    d->tableViewColumnDataSource->sourceModel = d->imageFilterModel;
    d->tableViewColumnFactory = new TableViewColumnFactory(d->tableViewColumnDataSource, this);

    QVBoxLayout* const vbox1 = new QVBoxLayout();

    d->tableViewModel = new TableViewModel(d->tableViewColumnFactory, d->imageFilterModel, this);
    d->treeView = new TableViewTreeView(d->tableViewModel, d->tableViewColumnFactory, this);

    vbox1->addWidget(d->treeView);

    setLayout(vbox1);
}

TableView::~TableView()
{

}

void TableView::doLoadState()
{

}

void TableView::doSaveState()
{

}

TableViewTreeView::TableViewTreeView(TableViewModel* const tableViewModel, Digikam::TableViewColumnFactory*const tableViewColumnFactory, QWidget*const parent)
  : QTreeView(parent), d(new Private())
{
    d->tableViewColumnFactory = tableViewColumnFactory;
    d->tableViewModel = tableViewModel;

    d->actionHeaderContextMenuRemoveColumn = new KAction("Remove this column", this);
    connect(d->actionHeaderContextMenuRemoveColumn, SIGNAL(triggered(bool)),
            this, SLOT(slotHeaderContextMenuActionRemoveColumnTriggered()));

    header()->installEventFilter(this);

    setModel(d->tableViewModel);
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

void TableViewTreeView::showHeaderContextMenu(QEvent* const event)
{
    QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
    QHeaderView* const headerView = header();

    d->headerContextMenuActiveColumn = headerView->logicalIndexAt(e->pos());
    KMenu* const menu = new KMenu(this);

    /// @todo disable if it is the last column
    menu->addAction(d->actionHeaderContextMenuRemoveColumn);
    menu->addSeparator();

    // add actions for all columns
    QList<TableViewColumnDescription> columnDescriptions = d->tableViewColumnFactory->getColumnDescriptionList();
    for (int i = 0; i<columnDescriptions.count(); ++i)
    {
        const TableViewColumnDescription& desc = columnDescriptions.at(i);
        KAction* const action = new KAction(desc.columnTitle, menu);

        action->setData(QVariant::fromValue<TableViewColumnDescription>(desc));

        menu->addAction(action);
    }

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotHeaderContextMenuActionTriggered(QAction*)));

    menu->exec(e->globalPos());
}

void TableViewTreeView::slotHeaderContextMenuActionTriggered(QAction* triggeredAction)
{
    const QVariant actionData = triggeredAction->data();
    if (!actionData.canConvert<TableViewColumnDescription>())
    {
        return;
    }

    const TableViewColumnDescription desc = actionData.value<TableViewColumnDescription>();
    qDebug()<<"clicked: "<<desc.columnTitle;
    d->tableViewModel->addColumnAt(desc, d->headerContextMenuActiveColumn+1);
}

void TableViewTreeView::slotHeaderContextMenuActionRemoveColumnTriggered()
{
    qDebug()<<"remove column "<<d->headerContextMenuActiveColumn;
    d->tableViewModel->removeColumnAt(d->headerContextMenuActiveColumn);
}


} /* namespace Digikam */
