/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-05
 * Description : Tag Manager Tree View derived from TagsFolderView to implement
 *               a custom context menu and some batch view options, such as
 *               expanding multiple items
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include <QModelIndex>
#include <QQueue>

#include "kdebug.h"
#include "kmenu.h"
#include "kaction.h"

#include "tagmngrtreeview.h"
#include "contextmenuhelper.h"
#include "tagsmanager.h"


namespace Digikam {

class TagMngrTreeView::TagMngrTreeViewPriv
{

public:
    TagMngrTreeViewPriv()
    {
        tagMngr = 0;
    }

    TagsManager* tagMngr;
};

TagMngrTreeView::TagMngrTreeView(TagsManager* parent, TagModel* model)
                : TagFolderView(parent, model), d(new TagMngrTreeViewPriv())
{
    d->tagMngr = parent;
}

TagMngrTreeView::~TagMngrTreeView()
{

}
void TagMngrTreeView::contextMenuEvent(QContextMenuEvent* event)
{

    QModelIndexList selectedItems = selectionModel()->selectedIndexes();

    qSort(selectedItems.begin(),selectedItems.end());
    QList<TAlbum*> items;

    foreach(QModelIndex mIndex, selectedItems)
    {
        TAlbum* temp = static_cast<TAlbum*>(albumForIndex(mIndex));
        items.push_back(temp);
    }

    /**
     * Append root tag if no nodes are selected
     */
    if(items.isEmpty())
    {
        QModelIndex root = this->model()->index(0,0);
        items.append(static_cast<TAlbum*>(albumForIndex(root)));
    }
    KMenu popmenu(this);
    popmenu.addTitle(contextMenuIcon(), contextMenuTitle());
    ContextMenuHelper cmhelper(&popmenu);

    setContexMenuItems(cmhelper, items);

    QAction* const choice = cmhelper.exec(QCursor::pos());
    Q_UNUSED(choice);
    Q_UNUSED(event);

}

void TagMngrTreeView::setContexMenuItems(ContextMenuHelper& cmh, QList< TAlbum* > albums)
{
    if(albums.size() == 1)
    {
        TAlbum* tag = dynamic_cast<TAlbum*> (albums.first());

        if (!tag)
        {
            return;
        }

        cmh.addActionNewTag(tagModificationHelper(), tag);
    }

    cmh.addActionDeleteTags(tagModificationHelper(),albums);
    cmh.addSeparator();

    KAction* resetIcon     = new KAction(KIcon("view-refresh"),
                                         i18n("Reset tag Icon"), this);

    KAction* invSel        = new KAction(KIcon(),
                                         i18n("Invert Selection"), this);

    KAction* expandTree    = new KAction(KIcon("format-indent-more"),
                                         i18n("Expand Tag Tree"), this);

    KAction* expandSel     = new KAction(KIcon("format-indent-more"),
                                         i18n("Expand Selected Nodes"), this);

    cmh.addAction(resetIcon,d->tagMngr, SLOT(slotResetTagIcon()), false);
    cmh.addAction(invSel, d->tagMngr, SLOT(slotInvertSel()), false);
    cmh.addAction(expandTree, this, SLOT(slotExpandTree()),false);
    cmh.addAction(expandSel, this , SLOT(slotExpandSelected()), false);
}
void TagMngrTreeView::slotExpandSelected()
{
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex index, list)
    {
        expand(index);
    }
}

void TagMngrTreeView::slotExpandTree()
{
    QModelIndex root = this->model()->index(0,0);
    QItemSelectionModel* model = this->selectionModel();
    QModelIndexList selected = model->selectedIndexes();

    QQueue<QModelIndex> greyNodes;

    greyNodes.append(root);

    while(!greyNodes.isEmpty())
    {
        QModelIndex current = greyNodes.dequeue();

        if(!(current.isValid()))
        {
            continue;
        }
        int it = 0;
        QModelIndex child = current.child(it++,0);

        while(child.isValid())
        {

            if(this->isExpanded(child))
            {
                greyNodes.enqueue(child);
            }
            else
            {
                expand(child);
            }
            child = current.child(it++,0);
        }
    }
}

} // namespace Digikam
