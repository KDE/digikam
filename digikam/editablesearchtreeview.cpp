/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Basic search tree view with editing functionality
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "editablesearchtreeview.moc"

// Qt includes

#include <qevent.h>

// KDE includes

#include <kdebug.h>
#include <kmenu.h>
#include <kiconloader.h>

// Local includes

#include "contextmenuhelper.h"

namespace Digikam
{

class EditableSearchTreeViewPriv
{
public:
    EditableSearchTreeViewPriv() :
        searchModificationHelper(0)
    {
    }

    SearchModificationHelper *searchModificationHelper;

    QAction *renSearch;
    QAction *delSearch;

};

EditableSearchTreeView::EditableSearchTreeView(QWidget *parent,
                                             SearchModel *searchModel,
                                             SearchModificationHelper *searchModificationHelper) :
                SearchTreeView(parent, searchModel), d(new EditableSearchTreeViewPriv)
{

    d->searchModificationHelper = searchModificationHelper;

    d->renSearch = new QAction(SmallIcon("edit-rename"), i18n("Rename..."), this);
    d->delSearch = new QAction(SmallIcon("edit-delete"), i18n("Delete"), this);

    albumFilterModel()->listTimelineSearches();
    albumFilterModel()->setListTemporarySearches(false);
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
    return i18n("My Date Searches");
}

void EditableSearchTreeView::addCustomContextMenuActions(ContextMenuHelper &cmh, Album *album)
{

    Q_UNUSED(album);

    cmh.addAction(d->renSearch);
    cmh.addAction(d->delSearch);

}

void EditableSearchTreeView::handleCustomContextMenuAction(QAction *action, Album *album)
{

    SAlbum *searchAlbum = dynamic_cast<SAlbum*> (album);

    if (!searchAlbum || !action)
    {
        return;
    }

    if (action == d->renSearch)
    {
        d->searchModificationHelper->slotSearchRename(searchAlbum);
    }
    else if (action == d->delSearch)
    {
        d->searchModificationHelper->slotSearchDelete(searchAlbum);
    }

}

}
