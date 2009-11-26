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
    TagFolderViewNewPriv() :
        model(0),
        tagModificationHelper(0),
        resetIconAction(0),
        findDuplAction(0),
        selectOnContextMenu(true)
    {

    }

    TagModel *model;
    TagModificationHelper *tagModificationHelper;

    QAction *resetIconAction;
    QAction *findDuplAction;

    bool selectOnContextMenu;
};

TagFolderViewNew::TagFolderViewNew(QWidget *parent, TagModel *model,
                                   TagModificationHelper *tagModificationHelper) :
    TagTreeView(model, tagModificationHelper, parent), d(new TagFolderViewNewPriv)
{

    d->model = model;
    d->tagModificationHelper = tagModificationHelper;

    d->resetIconAction = new QAction(SmallIcon("view-refresh"), i18n("Reset Tag Icon"), this);
    d->findDuplAction = new QAction(SmallIcon("tools-wizard"), i18n("Find Duplicates..."), this);

    setSortingEnabled(true);
    setSelectAlbumOnClick(true);

}

TagFolderViewNew::~TagFolderViewNew()
{
    delete d;
}

void TagFolderViewNew::setSelectOnContextMenu(bool select)
{
    d->selectOnContextMenu = select;
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
    if(d->selectOnContextMenu)
    {
        slotSelectAlbum(tag);
    }

    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("tag_new");
    cmhelper.addCreateTagFromAddressbookMenu();
    addCustomContextMenuActions(cmhelper, tag);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("tag_delete");
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("tag_edit");

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAddNewTagFromABCMenu(const QString&)),
            this, SLOT(slotTagNewFromABCMenu(const QString&)));

    QAction* choice = cmhelper.exec(QCursor::pos());
    handleCustomContextMenuAction(choice, tag);

}

void TagFolderViewNew::addCustomContextMenuActions(ContextMenuHelper &cmh, TAlbum *tag)
{
    cmh.addAction(d->resetIconAction);
    cmh.addAction(d->findDuplAction);

    d->resetIconAction->setEnabled(!tag->isRoot());
}

void TagFolderViewNew::slotTagNewFromABCMenu(const QString &personName)
{

    TAlbum *parent = currentAlbum();
    if (!parent)
    {
        return;
    }

    d->tagModificationHelper->slotTagNew(parent, personName, "tag-people");

}

void TagFolderViewNew::handleCustomContextMenuAction(QAction *action, TAlbum *tag)
{

    if (!tag || !action)
    {
        return;
    }

    if (action == d->resetIconAction)
    {
        QString errMsg;
        AlbumManager::instance()->updateTAlbumIcon(tag, "tag", 0, errMsg);
    }
    else if (action == d->findDuplAction)
    {
        emit signalFindDuplicatesInAlbum(tag);
    }

}

}

