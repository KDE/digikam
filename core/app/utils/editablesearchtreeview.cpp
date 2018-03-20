/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Basic search tree view with editing functionality
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "editablesearchtreeview.h"

// Qt includes

#include <QAction>
#include <QIcon>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"

namespace Digikam
{

class EditableSearchTreeView::Private
{
public:

    Private() :
        searchModificationHelper(0),
        renameSearchAction(0),
        deleteSearchAction(0)
    {
    }

    SearchModificationHelper* searchModificationHelper;

    QAction*                  renameSearchAction;
    QAction*                  deleteSearchAction;
};

EditableSearchTreeView::EditableSearchTreeView(QWidget* const parent,
                                               SearchModel* const searchModel,
                                               SearchModificationHelper* const searchModificationHelper)
    : SearchTreeView(parent), d(new Private)
{
    setAlbumModel(searchModel);
    d->searchModificationHelper = searchModificationHelper;

    d->renameSearchAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Rename..."), this);
    d->deleteSearchAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),   i18n("Delete"),    this);

    setSortingEnabled(true);
    setSelectAlbumOnClick(true);
    setEnableContextMenu(true);
}

EditableSearchTreeView::~EditableSearchTreeView()
{
    delete d;
}

QString EditableSearchTreeView::contextMenuTitle() const
{
    return i18n("Searches");
}

void EditableSearchTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    SAlbum* const searchAlbum = dynamic_cast<SAlbum*>(album);

    // disable actions if there is no album or the album is a temporary search
    bool activate = false;

    if (searchAlbum)
    {
        activate = !searchAlbum->isTemporarySearch();
    }

    d->renameSearchAction->setEnabled(activate);
    d->deleteSearchAction->setEnabled(activate);

    cmh.addAction(d->renameSearchAction);
    cmh.addAction(d->deleteSearchAction);
}

void EditableSearchTreeView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    Album* const a            = album;
    SAlbum* const searchAlbum = dynamic_cast<SAlbum*>(a);

    if (!searchAlbum || !action)
    {
        return;
    }

    if (action == d->renameSearchAction)
    {
        d->searchModificationHelper->slotSearchRename(searchAlbum);
    }
    else if (action == d->deleteSearchAction)
    {
        d->searchModificationHelper->slotSearchDelete(searchAlbum);
    }
}

} // namespace Digikam
