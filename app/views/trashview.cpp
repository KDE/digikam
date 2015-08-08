/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-07
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

// KDE includes

#include "klocalizedstring.h"

namespace Digikam
{

TrashView::TrashView(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QTableView* tableView = new QTableView(this);

    QPushButton* restoreButton = new QPushButton(i18n("Restore"));
    QPushButton* deleteButton = new QPushButton(i18n("Delete Permanently"));

    mainLayout->addWidget(tableView);
    mainLayout->addWidget(restoreButton);
    mainLayout->addWidget(deleteButton);

}

} // namespace Digikam
