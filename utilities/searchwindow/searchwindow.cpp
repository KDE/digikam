/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 * 
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QScrollArea>
#include <QHBoxLayout>

// KDE includes

#include <klocale.h>

// Local includes

#include "themeengine.h"
#include "searchview.h"
#include "searchwindow.h"
#include "searchxml.h"
#include "searchwindow.moc"

namespace Digikam
{

class SearchWindowPriv
{
public:

    SearchWindowPriv()
    {
        scrollArea = 0;
        searchView = 0;
        currentId  = -1;
    }

    QScrollArea *scrollArea;
    SearchView  *searchView;
    int          currentId;
};

SearchWindow::SearchWindow()
    : QWidget(0)
{
    d = new SearchWindowPriv;

    QHBoxLayout *layout = new QHBoxLayout;

    d->scrollArea = new QScrollArea(this);
    d->scrollArea->setWidgetResizable(true);

    d->searchView = new SearchView;
    d->searchView->setup();

    d->scrollArea->setWidget(d->searchView);
    d->scrollArea->setFrameStyle(QFrame::NoFrame);

    layout->addWidget(d->scrollArea);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    setVisible(false);
    setWindowTitle(i18n("Advanced Search"));
    resize(800, 600);
}

SearchWindow::~SearchWindow()
{
    delete d;
}

void SearchWindow::readSearch(int id, const QString &xml)
{
    d->currentId = id;
    d->searchView->read(xml);
}

void SearchWindow::reset()
{
    d->searchView->read(QString());
}

QString SearchWindow::search()
{
    return d->searchView->write();
}

}



