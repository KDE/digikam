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

#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>

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

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

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

    QTreeView*              treeView;
    ImageFilterModel*       imageFilterModel;
    ImageAlbumModel*        imageModel;
    QItemSelectionModel*    selectionModel;
    TableViewModel*         tableViewModel;
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

    QVBoxLayout* const vbox1 = new QVBoxLayout();

    d->treeView = new QTreeView(this);
    d->treeView->setModel(d->imageFilterModel);

    vbox1->addWidget(d->treeView);

    setLayout(vbox1);

    d->tableViewModel = new TableViewModel(d->imageFilterModel, this);
    d->treeView->setModel(d->tableViewModel);
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


} /* namespace Digikam */
