/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : Reimplemented QListView for Tags Manager, with support for
 *               drag-n-drop
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

#include "tagmngrlistview.h"

// Qt includes

#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "tagmngrlistmodel.h"
#include "tagmngrlistitem.h"
#include "taglist.h"

namespace Digikam
{

TagMngrListView::TagMngrListView(QWidget* const parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
}
void TagMngrListView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList list             = this->selectionModel()->selectedIndexes();
    TagMngrListModel* const tagmodel = dynamic_cast<TagMngrListModel*>(this->model());

    if(!tagmodel)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    QMimeData* const data = tagmodel->mimeData(list);

    if(!data)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no data obtained!";
        return;
    }

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(data);
    drag->exec(supportedActions, Qt::IgnoreAction);
}

QModelIndexList TagMngrListView::mySelectedIndexes()
{
    return this->selectedIndexes();
}

void TagMngrListView::dropEvent(QDropEvent *e)
{
    QModelIndex index                = indexVisuallyAt(e->pos());
    TagMngrListModel* const tagmodel = dynamic_cast<TagMngrListModel*>(this->model());

    if(!tagmodel)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    tagmodel->dropMimeData(e->mimeData(),e->dropAction(), index.row(), index.column(),index.parent());

    QList<int> toSel = tagmodel->getDragNewSelection();

    if(toSel.size() != 2)
    {
        return;
    }

    QItemSelectionModel* const model = this->selectionModel();

    model->clearSelection();
    this->setCurrentIndex(tagmodel->index(toSel.first()+1,0));

    for(int it = toSel.first()+1; it <= toSel.last(); ++it)
    {
        model->select(tagmodel->index(it,0), model->Select);
    }
}

QModelIndex TagMngrListView::indexVisuallyAt(const QPoint& p)
{
    if (viewport()->rect().contains(p))
    {
        QModelIndex index = indexAt(p);

        if (index.isValid() && visualRect(index).contains(p))
        {
            return index;
        }
    }

    return QModelIndex();
}

void TagMngrListView::contextMenuEvent(QContextMenuEvent* event)
{
    Q_UNUSED(event);

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    TagList* const tagList = dynamic_cast<TagList*>(this->parent());

    if(!tagList)
    {
        return;
    }

    QAction* const delAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18n("Delete Selected from List"),this);
    cmhelper.addAction(delAction, tagList, SLOT(slotDeleteSelected()),false);

    QModelIndexList sel = this->selectionModel()->selectedIndexes();
    if(sel.size() == 1 && sel.first().row() == 0)
        delAction->setDisabled(true);

    cmhelper.exec(QCursor::pos());
}

void TagMngrListView::slotDeleteSelected()
{
    QModelIndexList sel = this->selectionModel()->selectedIndexes();

    if(sel.isEmpty())
        return;

    TagMngrListModel* const tagmodel = dynamic_cast<TagMngrListModel*>(this->model());

    if(!tagmodel)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    foreach(const QModelIndex& index, sel)
    {
        ListItem* const item = static_cast<ListItem*>(index.internalPointer());
        tagmodel->deleteItem(item);
    }
}

} // namespace Digikam
