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
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include <kdebug.h>
#include "tagmngrlistview.h"
#include "tagmngrlistmodel.h"


TagMngrListView::TagMngrListView(QWidget *parent) :
    QListView(parent)
{
}
void TagMngrListView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList list = this->selectionModel()->selectedIndexes();

    TagMngrListModel* tagmodel = dynamic_cast<TagMngrListModel*>(this->model());
    QMimeData* data = tagmodel->mimeData(list);

    if(!data)
    {
        kDebug() << "Error! no data obtained!";
        return;
    }

    QDrag* drag = new QDrag(this);
    drag->setMimeData(data);
    drag->exec(supportedActions, Qt::IgnoreAction);
}
void TagMngrListView::dragEnterEvent(QDragEnterEvent *event)
{
    setState(DraggingState);
    event->accept();
}

void TagMngrListView::dropEvent(QDropEvent *e)
{
    qDebug() << e->mimeData();

    QModelIndex index = indexVisuallyAt(e->pos());
    TagMngrListModel* tagmodel = dynamic_cast<TagMngrListModel*>(this->model());

    tagmodel->dropMimeData(e->mimeData(),e->dropAction(),index.row(),
                           index.column(),index.parent());

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
