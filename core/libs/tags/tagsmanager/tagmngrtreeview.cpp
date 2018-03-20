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

#include "tagmngrtreeview.h"

// Qt includes

#include <QModelIndex>
#include <QQueue>
#include <QMenu>
#include <QAction>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "tagsmanager.h"

namespace Digikam
{

class TagMngrTreeView::Private
{

public:
    Private()
    {
        tagMngr = 0;
    }

    TagsManager* tagMngr;
};

TagMngrTreeView::TagMngrTreeView(TagsManager* const parent, TagModel* const model)
                : TagFolderView(parent, model), d(new Private())
{
    d->tagMngr = parent;
    setAlbumFilterModel(new TagsManagerFilterModel(this), albumFilterModel());
    setSelectAlbumOnClick(false);
    expand(albumFilterModel()->rootAlbumIndex());
}

TagMngrTreeView::~TagMngrTreeView()
{
    delete d;
}

void TagMngrTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndexList selectedItems = selectionModel()->selectedIndexes();

    std::sort(selectedItems.begin(), selectedItems.end());
    QList<TAlbum*> items;

    foreach(const QModelIndex& mIndex, selectedItems)
    {
        TAlbum* const temp = static_cast<TAlbum*>(albumForIndex(mIndex));
        items.push_back(temp);
    }

    /**
     * Append root tag if no nodes are selected
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

    QAction* const choice = cmhelper.exec(QCursor::pos());
    Q_UNUSED(choice);
    Q_UNUSED(event);
}

void TagMngrTreeView::setAlbumFilterModel(TagsManagerFilterModel* const filteredModel,
                                          CheckableAlbumFilterModel* const filterModel)
{
    Q_UNUSED(filterModel);
    m_tfilteredModel = filteredModel;
    albumFilterModel()->setSourceFilterModel(m_tfilteredModel);
}

void TagMngrTreeView::setContexMenuItems(ContextMenuHelper& cmh, QList<TAlbum*> albums)
{
    bool isRoot = false;

    if (albums.size() == 1)
    {
        TAlbum* const tag = dynamic_cast<TAlbum*> (albums.first());

        if (!tag)
        {
            return;
        }

        if (tag->isRoot())
        {
            isRoot = true;
        }

        cmh.addActionNewTag(tagModificationHelper(), tag);
    }

    if (!isRoot)
    {
        cmh.addActionDeleteTags(tagModificationHelper(), albums);
    }
    else
    {
        /** This is a dummy action, delete is disable for root tag **/
        QAction* deleteTagsAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")),
                                                i18n("Delete Tags"), this);
        cmh.addAction(deleteTagsAction);
        deleteTagsAction->setEnabled(false);
    }

    cmh.addSeparator();

    QAction* const titleEdit     = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                               i18n("Edit Tag Title"), this);
    titleEdit->setShortcut(QKeySequence(Qt::Key_F2));

    QAction* const resetIcon     = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")),
                                               i18n("Reset Tag Icon"), this);

    QAction* const invSel        = new QAction(QIcon::fromTheme(QLatin1String("tag-reset")),
                                               i18n("Invert Selection"), this);

    QAction* const expandTree    = new QAction(QIcon::fromTheme(QLatin1String("format-indent-more")),
                                               i18n("Expand Tag Tree"), this);

    QAction* const expandSel     = new QAction(QIcon::fromTheme(QLatin1String("format-indent-more")),
                                               i18n("Expand Selected Nodes"), this);

    QAction* const delTagFromImg = new QAction(QIcon::fromTheme(QLatin1String("tag-delete")),
                                               i18n("Remove Tag from Images"), this);

    cmh.addAction(titleEdit,     d->tagMngr, SLOT(slotEditTagTitle()),       false);
    cmh.addAction(resetIcon,     d->tagMngr, SLOT(slotResetTagIcon()),       false);
    cmh.addAction(invSel,        d->tagMngr, SLOT(slotInvertSel()),          false);
    cmh.addAction(expandTree,    this,       SLOT(slotExpandTree()),         false);
    cmh.addAction(expandSel,     this ,      SLOT(slotExpandSelected()),     false);
    cmh.addAction(delTagFromImg, d->tagMngr, SLOT(slotRemoveTagsFromImgs()), false);

    if (isRoot)
    {
        titleEdit->setEnabled(false);
        resetIcon->setEnabled(false);
        delTagFromImg->setEnabled(false);
    }

    if (albums.size() != 1)
    {
        titleEdit->setEnabled(false);
    }
}

void TagMngrTreeView::slotExpandSelected()
{
    QModelIndexList list = selectionModel()->selectedIndexes();

    foreach(const QModelIndex& index, list)
    {
        expand(index);
    }
}

void TagMngrTreeView::slotExpandTree()
{
    QModelIndex root                 = this->model()->index(0, 0);
    QItemSelectionModel* const model = this->selectionModel();
    QModelIndexList selected         = model->selectedIndexes();

    QQueue<QModelIndex> greyNodes;

    greyNodes.append(root);

    while (!greyNodes.isEmpty())
    {
        QModelIndex current = greyNodes.dequeue();

        if (!(current.isValid()))
        {
            continue;
        }

        if (this->isExpanded(current))
        {
            int it            = 0;
            QModelIndex child = current.child(it++, 0);

            while (child.isValid())
            {
                if (this->isExpanded(child))
                {
                    greyNodes.enqueue(child);
                }
                else
                {
                    expand(child);
                }

                child = current.child(it++, 0);
            }
        }
        else
        {
            expand(current);
        }
    }
}

} // namespace Digikam
