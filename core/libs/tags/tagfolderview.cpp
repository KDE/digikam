/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagfolderview.h"

// Qt includes

#include <QAction>
#include <QContextMenuEvent>
#include <QQueue>
#include <QIcon>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "contextmenuhelper.h"
#include "tagmodificationhelper.h"
#include "facetags.h"

namespace Digikam
{

class TagFolderView::Private
{
public:

    Private() :
        showFindDuplicateAction(true),
        showDeleteFaceTagsAction(false),
        resetIconAction(0),
        findDuplAction(0)
    {
    }

    bool     showFindDuplicateAction;
    bool     showDeleteFaceTagsAction;

    QAction* resetIconAction;
    QAction* findDuplAction;
};

TagFolderView::TagFolderView(QWidget* const parent, TagModel* const model)
    : TagTreeView(parent), d(new Private)
{
    setAlbumModel(model);

    d->resetIconAction = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18n("Reset Tag Icon"),     this);
    d->findDuplAction  = new QAction(QIcon::fromTheme(QLatin1String("tools-wizard")), i18n("Find Duplicates..."), this);

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

void TagFolderView::setShowDeleteFaceTagsAction(bool show)
{
    d->showDeleteFaceTagsAction = show;
}

QString TagFolderView::contextMenuTitle() const
{
    return i18n("Tags");
}

void TagFolderView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    TAlbum* const tag = dynamic_cast<TAlbum*> (album);

    if (!tag)
    {
        return;
    }

    cmh.addActionNewTag(tagModificationHelper(), tag);
    cmh.addCreateTagFromAddressbookMenu();
    cmh.addAction(d->resetIconAction);
    cmh.addSeparator();

    QAction* const expandSel   = new QAction(QIcon::fromTheme(QLatin1String("format-indent-more")),
                                             i18n("Expand Selected Nodes"), this);

    cmh.addAction(expandSel, this, SLOT(slotExpandNode()), false);

    QAction* const collapseSel = new QAction(QIcon::fromTheme(QLatin1String("format-indent-more")),
                                             i18n("Collapse Selected Recursively"), this);

    cmh.addAction(collapseSel, this, SLOT(slotCollapseNode()), false);

    cmh.addSeparator();

    if (d->showFindDuplicateAction)
    {
        cmh.addAction(d->findDuplAction);
    }

    cmh.addExportMenu();
    cmh.addSeparator();
    if (d->showDeleteFaceTagsAction)
    {
        cmh.addActionDeleteFaceTag(tagModificationHelper(), tag);
        cmh.addSeparator();
    }
    else
    {
        cmh.addActionDeleteTag(tagModificationHelper(), tag);
        cmh.addSeparator();
        // If the tag is no face tag, add the option to set it as face tag.
        if (!FaceTags::isPerson(tag->id()))
        {
            cmh.addActionTagToFaceTag(tagModificationHelper(), tag);
        }
    }
    cmh.addActionEditTag(tagModificationHelper(), tag);

    connect(&cmh, SIGNAL(signalAddNewTagFromABCMenu(QString)),
            this, SLOT(slotTagNewFromABCMenu(QString)));

    d->resetIconAction->setEnabled(!tag->isRoot());
}

void TagFolderView::slotTagNewFromABCMenu(const QString& personName)
{
    TAlbum* const parent = currentAlbum();

    if (!parent)
    {
        return;
    }

    tagModificationHelper()->slotTagNew(parent, personName, QLatin1String("tag-people"));
}

void TagFolderView::slotExpandNode()
{
    //QModelIndex root                 = this->model()->index(0,0);
    QItemSelectionModel* const model = this->selectionModel();
    QModelIndexList selected         = model->selectedIndexes();

    QQueue<QModelIndex> greyNodes;

    foreach(const QModelIndex& index, selected)
    {
        greyNodes.append(index);
        expand(index);
    }

    while(!greyNodes.isEmpty())
    {
        QModelIndex current = greyNodes.dequeue();

        if(!(current.isValid()))
        {
            continue;
        }

        int it            = 0;
        QModelIndex child = current.child(it++, 0);

        while(child.isValid())
        {
            expand(child);
            greyNodes.enqueue(child);
            child = current.child(it++, 0);
        }
    }
}

void TagFolderView::slotCollapseNode()
{
    //QModelIndex root                 = this->model()->index(0,0);
    QItemSelectionModel* const model = this->selectionModel();
    QModelIndexList selected         = model->selectedIndexes();

    QQueue<QModelIndex> greyNodes;

    foreach(const QModelIndex& index, selected)
    {
        greyNodes.append(index);
        collapse(index);
    }

    while(!greyNodes.isEmpty())
    {
        QModelIndex current = greyNodes.dequeue();

        if(!(current.isValid()))
        {
            continue;
        }

        int it            = 0;
        QModelIndex child = current.child(it++, 0);

        while(child.isValid())
        {
            collapse(child);
            greyNodes.enqueue(child);
            child = current.child(it++, 0);
        }
    }
}
void TagFolderView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    Album* const a    = album;
    TAlbum* const tag = dynamic_cast<TAlbum*>(a);

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
        AlbumManager::instance()->updateTAlbumIcon(tag, QLatin1String("tag"), 0, errMsg);
    }
    else if (action == d->findDuplAction)
    {
        QList<TAlbum*> selected = selectedTagAlbums();

        emit signalFindDuplicates(selected);
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

    QAction* const expandSel   = new QAction(QIcon::fromTheme(QLatin1String("format-indent-more")),
                                             i18n("Expand Selected Recursively"), this);

    cmh.addAction(expandSel, this, SLOT(slotExpandNode()), false);

    QAction* const collapseSel = new QAction(QIcon::fromTheme(QLatin1String("format-indent-more")),
                                             i18n("Collapse Selected Recursively"), this);

    cmh.addAction(collapseSel, this, SLOT(slotCollapseNode()), false);
    cmh.addSeparator();
    cmh.addExportMenu();
    cmh.addSeparator();
    if (d->showDeleteFaceTagsAction)
    {
        cmh.addActionDeleteFaceTags(tagModificationHelper(), albums);
    }
    else
    {
        cmh.addActionDeleteTags(tagModificationHelper(), albums);
        // If one of the selected tags is no face tag, add the action to mark them as face tags.
        foreach (TAlbum* const tag, albums)
        {
            if (!FaceTags::isPerson(tag->id()))
            {
                cmh.addSeparator();
                cmh.addActionTagToFaceTag(tagModificationHelper(), tag);
                break;
            }
        }
    }
    cmh.addSeparator();
}

void TagFolderView::contextMenuEvent(QContextMenuEvent* event)
{
/*
    if (!d->enableContextMenu)
    {
        return;
    }
*/

    Album* const album = albumFilterModel()->albumForIndex(indexAt(event->pos()));

    if (!showContextMenuAt(event, album))
    {
        return;
    }

    // switch to the selected album if need

/*
    if (d->selectOnContextMenu && album)
    {
        setCurrentAlbum(album);
    }
*/

    // --------------------------------------------------------

    QModelIndexList selectedItems = selectionModel()->selectedIndexes();

    std::sort(selectedItems.begin(), selectedItems.end());
    QList<TAlbum*> items;

    foreach(const QModelIndex& mIndex, selectedItems)
    {
        TAlbum* const temp = static_cast<TAlbum*>(albumForIndex(mIndex));
        items.push_back(temp);
    }

    /**
     * If no item is selected append root tag
     */
    if(items.isEmpty())
    {
        QModelIndex root = this->model()->index(0, 0);
        items.append(static_cast<TAlbum*>(albumForIndex(root)));
    }

    QMenu popmenu(this);
    popmenu.addSection(contextMenuIcon(), contextMenuTitle());
    ContextMenuHelper cmhelper(&popmenu);

    setContexMenuItems(cmhelper, items);

/*
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
