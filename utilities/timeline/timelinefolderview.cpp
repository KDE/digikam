/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Searches dates folder view used by timeline
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

#include "timelinefolderview.moc"

// Qt includes

#include <qevent.h>

// KDE includes

#include <kdebug.h>
#include <kmenu.h>
#include <kiconloader.h>

namespace Digikam
{

class TimeLineFolderViewNewPriv
{
public:
    TimeLineFolderViewNewPriv() :
        searchModificationHelper(0)
    {
    }

    SearchModificationHelper *searchModificationHelper;

};

TimeLineFolderViewNew::TimeLineFolderViewNew(QWidget *parent,
                                             SearchModel *searchModel,
                                             SearchModificationHelper *searchModificationHelper) :
                SearchTreeView(parent, searchModel), d(new TimeLineFolderViewNewPriv)
{

    d->searchModificationHelper = searchModificationHelper;

    albumFilterModel()->listTimelineSearches();
    albumFilterModel()->setListTemporarySearches(false);
    setSortingEnabled(true);
    setSelectAlbumOnClick(true);

}

TimeLineFolderViewNew::~TimeLineFolderViewNew()
{
    delete d;
}

void TimeLineFolderViewNew::contextMenuEvent(QContextMenuEvent *event)
{

    SAlbum *album = dynamic_cast<SAlbum*> (albumFilterModel()->albumForIndex(
                    indexAt(event->pos())));
    if (!album)
    {
        kDebug() << "No album clicked, displaying no context menu";
        return;
    }

    // we need to switch to the selected album here also on right click.
    // Otherwise most of the actions in the context menu will not be enabled
    // because they are provided by the global action collection. That in turn
    // is controlled by the selected album
    slotSelectAlbum(album);

    // ensure that we are not working on the album root that doesn't allow
    // anything else than collections
    kDebug() << "context menu requested at album " << album->title();
    if (album->isRoot())
    {
        kDebug() << "returning because there is no album or album is root";
        return;
    }

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Date Searches"));
    QAction *renSearch = popmenu.addAction(SmallIcon("edit-rename"), i18n("Rename..."));
    QAction *delSearch = popmenu.addAction(SmallIcon("edit-delete"), i18n("Delete"));
    QAction *choice    = popmenu.exec(QCursor::pos());
    if (choice)
    {
        if (choice == renSearch)
        {
            d->searchModificationHelper->slotSearchRename(album);
        }
        else if (choice == delSearch)
        {
            d->searchModificationHelper->slotSearchDelete(album);
        }
    }

}

}
