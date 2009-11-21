/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagfolderview.moc"

// Qt includes
#include <qaction.h>
#include <qevent.h>

// KDE includes
#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes
#include "albummanager.h"
#include "contextmenuhelper.h"
#include "tagmodificationhelper.h"

namespace Digikam
{

class TagFolderViewNewPriv
{
public:
    TagModel *model;
    TagModificationHelper *tagModificationHelper;
};

TagFolderViewNew::TagFolderViewNew(QWidget *parent, TagModel *model,
                                   TagModificationHelper *tagModificationHelper) :
    TagTreeView(model, parent), d(new TagFolderViewNewPriv)
{

    d->model = model;
    d->tagModificationHelper = tagModificationHelper;

    setSortingEnabled(true);

    connect(this, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(slotTagSelected(const QModelIndex&)));

    connect(this, SIGNAL(assignTags(int, const QList<int>&)),
            d->tagModificationHelper, SLOT(slotAssignTags(int, const QList<int>&)),
            Qt::QueuedConnection);

}

TagFolderViewNew::~TagFolderViewNew()
{
    delete d;
}

void TagFolderViewNew::contextMenuEvent(QContextMenuEvent *event)
{

    // TODO update, copied from album folder view...
    TAlbum *tag = dynamic_cast<TAlbum*> (albumFilterModel()->albumForIndex(
                    indexAt(event->pos())));
    if (!tag)
    {
        kDebug() << "No tag album clicked, displaying no context menu";
        return;
    }

    // we need to switch to the selected album here also on right click.
    // Otherwise most of the actions in the context menu will not be enabled
    // because they are provided by the global action collection. That in turn
    // is controlled by the selected album
    slotSelectAlbum(tag);

    // temporary actions --------------------------------------

    QAction *resetIconAction = new QAction(SmallIcon("view-refresh"), i18n("Reset Tag Icon"), this);
    QAction *findDuplAction  = new QAction(SmallIcon("tools-wizard"), i18n("Find Duplicates..."), this);

    if (tag->isRoot())
    {
        resetIconAction->setEnabled(false);
    }
    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("tag_new");
    cmhelper.addCreateTagFromAddressbookMenu();
    cmhelper.addAction(resetIconAction);
    cmhelper.addAction(findDuplAction);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("tag_delete");
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("tag_edit");

    // special action handling --------------------------------

    // TODO update, what is this?
    //connect(&cmhelper, SIGNAL(signalAddNewTagFromABCMenu(const QString&)),
    //        this, SLOT(slotTagNewFromABCMenu(const QString&)));

    QAction* choice = cmhelper.exec(QCursor::pos());
    if (choice)
    {
        if (choice == resetIconAction)
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(tag, "tag", 0, errMsg);
        }
        else if (choice == findDuplAction)
        {
            emit signalFindDuplicatesInAlbum(tag);
        }
    }
}

void TagFolderViewNew::slotTagSelected(const QModelIndex &index)
{
    AlbumManager::instance()->setCurrentAlbum(albumForIndex(index));
}

// TODO copied code from albumfolderview
void TagFolderViewNew::slotSelectAlbum(Album *album)
{
    kDebug() << "Selecting album " << album;
    setCurrentIndex(albumFilterModel()->mapFromSource(
                    albumModel()->indexForAlbum(album)));
    AlbumManager::instance()->setCurrentAlbum(album);
}

}

