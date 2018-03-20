/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view
 *
 * Copyright (C) 2005      by Renchi Raju <renchi at pooh dot tam dot uiuc dot edu>
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009      by Johannes Wienke <languitar at semipol dot de>
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

#include "searchfolderview.h"

// Qt includes

#include <QAction>
#include <QIcon>

// Local includes

#include "contextmenuhelper.h"

namespace Digikam
{

class NormalSearchTreeView::Private
{
public:

    Private() :
        newAction(0),
        editAction(0)
    {
    }

    QAction* newAction;
    QAction* editAction;
};

NormalSearchTreeView::NormalSearchTreeView(QWidget* const parent,
                                           SearchModel* const searchModel,
                                           SearchModificationHelper* const searchModificationHelper)
    : EditableSearchTreeView(parent, searchModel, searchModificationHelper),
      d(new Private)
{

    d->newAction  = new QAction(QIcon::fromTheme(QLatin1String("document-new")), i18nc("Create new search",    "New..."),  this);
    d->editAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")),    i18nc("Edit selected search", "Edit..."), this);
}

NormalSearchTreeView::~NormalSearchTreeView()
{
    delete d;
}

void NormalSearchTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    cmh.addAction(d->newAction);
    cmh.addSeparator();

    EditableSearchTreeView::addCustomContextMenuActions(cmh, album);

    SAlbum* const salbum = dynamic_cast<SAlbum*>(album);

    d->editAction->setEnabled(salbum);
    cmh.addAction(d->editAction);
}

void NormalSearchTreeView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    Album* a             = album;
    SAlbum* const salbum = dynamic_cast<SAlbum*>(a);

    if (action == d->newAction && salbum)
    {
        emit newSearch();
    }
    else if (action == d->editAction && salbum)
    {
        emit editSearch(salbum);
    }
    else
    {
        EditableSearchTreeView::handleCustomContextMenuAction(action, album);
    }
}

} // namespace Digikam
