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

namespace Digikam
{

class TrashView::Private
{

public:

    Private()
        : model(0)
    {
    }

public:

    DTrashItemModel* model;
};

TrashView::TrashView(QWidget* parent)
    : QWidget(parent), d(new Private)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QTableView* tableView = new QTableView(this);

    d->model = new DTrashItemModel(this);
    tableView->setModel(d->model);

    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableView->verticalHeader()->setDefaultSectionSize(ThumbnailSize::Large);
    tableView->verticalHeader()->hide();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QPushButton* restoreButton = new QPushButton(i18n("Restore"));
    QPushButton* deleteButton = new QPushButton(i18n("Delete Permanently"));

    mainLayout->addWidget(tableView);
    mainLayout->addWidget(restoreButton);
    mainLayout->addWidget(deleteButton);
}

DTrashItemModel* TrashView::model()
{
    return d->model;
}

} // namespace Digikam
