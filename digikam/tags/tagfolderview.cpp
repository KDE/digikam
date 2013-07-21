/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
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

#include <QAction>
#include <QEvent>
#include <QContextMenuEvent>
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

class TagFolderView::TagFolderViewPriv
{
public:

    TagFolderViewPriv() :
        showFindDuplicateAction(true),
        resetIconAction(0),
        findDuplAction(0)
    {
    }

    bool      showFindDuplicateAction;

    QAction*  resetIconAction;
    QAction*  findDuplAction;
};

TagFolderView::TagFolderView(QWidget* parent, TagModel* model)
    : TagTreeView(parent), d(new TagFolderViewPriv)
{
    setAlbumModel(model);

    d->resetIconAction = new QAction(SmallIcon("view-refresh"), i18n("Reset Tag Icon"), this);
    d->findDuplAction  = new QAction(SmallIcon("tools-wizard"), i18n("Find Duplicates..."), this);

    setSortingEnabled(true);
    setSelectAlbumOnClick(true);
    setEnableContextMenu(true);
}

TagFolderView::~TagFolderView()
{
    delete d;
}

void TagFolderView::setShowFindDuplicateAction(bool show)
{
    d->showFindDuplicateAction = show;
}

QString TagFolderView::contextMenuTitle() const
{
    return i18n("My Tags");
}

void TagFolderView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    TAlbum* tag = dynamic_cast<TAlbum*> (album);

    if (!tag)
    {
        return;
    }

    cmh.addActionNewTag(tagModificationHelper(), tag);
    cmh.addCreateTagFromAddressbookMenu();
    cmh.addAction(d->resetIconAction);
    cmh.addSeparator();

    if (d->showFindDuplicateAction)
    {
        cmh.addAction(d->findDuplAction);
    }

    cmh.addExportMenu();
    cmh.addBatchMenu();
    cmh.addSeparator();
    cmh.addActionDeleteTag(tagModificationHelper(), tag);
    cmh.addSeparator();
    cmh.addActionEditTag(tagModificationHelper(), tag);

    connect(&cmh, SIGNAL(signalAddNewTagFromABCMenu(QString)),
            this, SLOT(slotTagNewFromABCMenu(QString)));

    d->resetIconAction->setEnabled(!tag->isRoot());
}

void TagFolderView::slotTagNewFromABCMenu(const QString& personName)
{
    TAlbum* parent = currentAlbum();

    if (!parent)
    {
        return;
    }

    tagModificationHelper()->slotTagNew(parent, personName, "tag-people");
}

void TagFolderView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    Album* a    = album;
    TAlbum* tag = dynamic_cast<TAlbum*> (a);

    if (!tag)
    {
        return;
    }

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

void TagFolderView::setContexMenuItems(ContextMenuHelper& cmh, QList< TAlbum* > albums)
{
    if(albums.size() == 1)
    {
        addCustomContextMenuActions(cmh, albums.first());
        return;
    }

    if (d->showFindDuplicateAction)
    {
        cmh.addAction(d->findDuplAction);
    }
    cmh.addExportMenu();
    cmh.addBatchMenu();
    cmh.addActionDeleteTags(tagModificationHelper(),albums);
    cmh.addSeparator();
}

void TagFolderView::contextMenuEvent(QContextMenuEvent* event)
{
    /**
    if (!d->enableContextMenu)
    {
        return;
    }
    */
    Album* album = albumFilterModel()->albumForIndex(indexAt(event->pos()));

    /**
    if (!showContextMenuAt(event, album))
    {
        return;
    }
    */
    // switch to the selected album if need
    /**
    if (d->selectOnContextMenu && album)
    {
        setCurrentAlbum(album);
    }
    */
    // --------------------------------------------------------
    QModelIndexList selectedItems = selectionModel()->selectedIndexes();

    qSort(selectedItems.begin(),selectedItems.end());
    QList<TAlbum*> items;

    foreach(QModelIndex mIndex, selectedItems)
    {
        TAlbum* temp = static_cast<TAlbum*>(albumForIndex(mIndex));
        kDebug() << "Adding tag" << temp->title();
        items.push_back(temp);
    }
    kDebug() << "Context Menu triggered";
    KMenu popmenu(this);
    popmenu.addTitle(contextMenuIcon(), contextMenuTitle());
    ContextMenuHelper cmhelper(&popmenu);

    setContexMenuItems(cmhelper, items);
    /**
    foreach(ContextMenuElement* const element, d->contextMenuElements)
    {
        element->addActions(this, cmhelper, album);
    }
     */
    AlbumPointer<Album> albumPointer(album);
    QAction* const choice = cmhelper.exec(QCursor::pos());
    handleCustomContextMenuAction(choice, albumPointer);
}
} // namespace Digikam
