/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : Trash view
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "trashview.h"

// Qt includes

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QHeaderView>
#include <QListView>

// KDE includes

#include "klocalizedstring.h"

// Local includes

#include "dtrashiteminfo.h"
#include "dtrashitemmodel.h"
#include "thumbnailsize.h"
#include "iojobsmanager.h"
#include "iojobsthread.h"

namespace Digikam
{

class TrashView::Private
{

public:

    Private()
        : model(0),
          mainLayout(0),
          tableView(0),
          restoreButton(0),
          deleteButton(0)
    {
    }

public:

    DTrashItemModel* model;
    QVBoxLayout*     mainLayout;
    QTableView*      tableView;
    QPushButton*     restoreButton;
    QPushButton*     deleteButton;
    IOJobsThread*    itemsLoadingThread;
};

TrashView::TrashView(QWidget* parent)
    : QWidget(parent), d(new Private)
{
    d->mainLayout = new QVBoxLayout(this);

    d->tableView = new QTableView(this);

    d->model = new DTrashItemModel(this);

    d->tableView->setModel(d->model);
    d->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    d->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    d->tableView->verticalHeader()->setDefaultSectionSize(ThumbnailSize::Large);
    d->tableView->verticalHeader()->hide();
    d->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    d->restoreButton = new QPushButton(i18n("Restore"));
    d->deleteButton  = new QPushButton(i18n("Delete Permanently"));

    d->mainLayout->addWidget(d->tableView);
    d->mainLayout->addWidget(d->restoreButton);
    d->mainLayout->addWidget(d->deleteButton);
}

TrashView::~TrashView()
{
    delete d;
}

DTrashItemModel* TrashView::model()
{
    return d->model;
}

void TrashView::showTrashItemsForCollection(const QString& collectionPath)
{
    d->model->clearCurrentData();
    d->itemsLoadingThread = IOJobsManager::instance()->startDTrashItemsListingForCollection(collectionPath);

    connect(d->itemsLoadingThread, SIGNAL(collectionTrashItemInfo(DTrashItemInfo)),
            d->model, SLOT(append(DTrashItemInfo)));
}

} // namespace Digikam
