/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Completion Box for tags
 *
 * Copyright (C) 2010       by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 1997       by Sven Radej (sven.radej@iname.com)
 * Copyright (c) 1999       by Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * Copyright (c) 1999       by Preston Brown <pbrown@kde.org>
 * Copyright (c) 2000, 2001 by Dawit Alemayehu <adawit@kde.org>
 * Copyright (c) 2000, 2001 by Carsten Pfeiffer <pfeiffer@kde.org>
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

#include "addtagscompletionbox.h"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>
#include <QScrollBar>
#include <QListWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStringListModel>
#include <QQueue>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albumfiltermodel.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumthumbnailloader.h"
#include "albumtreeview.h"
#include "tagscache.h"

namespace Digikam
{

class TagModelCompletion::Private
{
public:

    Private()
    {
        model       = 0;
        rootItem    = 0;
        parentModel = 0;
    }

    QStandardItemModel*   model;
    QStandardItem*        rootItem;
    QList<QStandardItem*> allItems;
    TagModel*             parentModel;
};

TagModelCompletion::TagModelCompletion()
    : QCompleter(),
      d(new Private)
{
    d->model = new QStandardItemModel();
    /** we might use this to speed-up completer **/
    //QCompleter::setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    QCompleter::setCaseSensitivity(Qt::CaseInsensitive);
    QCompleter::setModel(d->model);
    QCompleter::setCompletionRole(Qt::DisplayRole);
    QCompleter::setCompletionColumn(0);
}

void TagModelCompletion::setModel(TagModel* model)
{
    d->parentModel = model;

    connect(d->parentModel,SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(slotInsertRows(QModelIndex, int, int)));

    connect(d->parentModel,SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotDeleteRows(QModelIndex, int, int)));

    int i = 0;
    QQueue<QModelIndex> q;

    q.append(model->index(0,0));

    while(!q.isEmpty())
    {
        QModelIndex current = q.dequeue();
        TAlbum* const t = dynamic_cast<TAlbum*>(model->albumForIndex(current));

        if (t != NULL && !t->isInternalTag())
        {
            i++;
            QIcon icon;

            if (t->icon().isEmpty())
                icon = QIcon::fromTheme(QLatin1String("tag"));
            else
                icon = QIcon::fromTheme(t->icon());

            QStandardItem* const item = new QStandardItem(icon, t->title());
            item->setData(QVariant(t->id()), Qt::UserRole+5);
            d->allItems.append(item);
        }

        int index         = 0;
        QModelIndex child = current.child(index++,0);

        while (child.isValid())
        {
            q.append(child);
            child = current.child(index++, 0);
        }
    }

    QCompleter::setModel(d->model);
}

void TagModelCompletion::setModel(AlbumFilterModel* model)
{
    //QCompleter::setModel(d->model);
    TagModel* tagModel = dynamic_cast<TagModel*>(model);
    if(tagModel != NULL)
        setModel(tagModel);
}

TagModel* TagModelCompletion::model() const
{
    QAbstractItemModel* const model = QCompleter::model();

    if (dynamic_cast<TagModel*>(model))
    {
        return static_cast<TagModel*>(model);
    }
    else if (dynamic_cast<AlbumFilterModel*>(model))
    {
        return static_cast<TagModel*>(static_cast<AlbumFilterModel*>(model)->sourceAlbumModel());
    }

    return 0;
}

void TagModelCompletion::update(QString word)
{
    d->model->clear();
    QList<QStandardItem*> filtered;

    if(word.isEmpty())
    {
        QCompleter::complete();
        return;
    }
    for(QStandardItem* item : d->allItems)
    {
        if(item->data(Qt::DisplayRole).toString().contains(word, Qt::CaseInsensitive))
        {
            filtered.append(item->clone());
        }
    }

    QStandardItem* const insert = new QStandardItem(QIcon::fromTheme(QLatin1String("tag")),
                                                    i18n("Insert (") + word + QLatin1String(")"));
    insert->setData(-5, Qt::UserRole+5);
    insert->setData(word, Qt::UserRole+4);
    filtered.append(insert);
    d->model->invisibleRootItem()->appendRows(filtered);

    QCompleter::complete();
}

void TagModelCompletion::complete(const QRect &rect)
{
    //qDebug() << "+++ Completion +++";
    QCompleter::complete(rect);
}

void TagModelCompletion::slotInsertRows(QModelIndex index, int start, int end)
{
    for(int i = start; i <= end; i++)
    {
        TAlbum* const t = dynamic_cast<TAlbum*>(d->parentModel->albumForIndex(index.child(i,0)));

        if (t != NULL && !t->isInternalTag())
        {
            QIcon icon(index.child(i, 0).data(Qt::DecorationRole).value<QPixmap>());
            QStandardItem* const item = new QStandardItem(icon,t->title());
            item->setData(QVariant(t->id()), Qt::UserRole+5);
            //root->appendRow(item);
            d->allItems.append(item);
        }
    }

    QCompleter::setModel(d->model);
}

void TagModelCompletion::slotDeleteRows(QModelIndex index, int start, int end)
{
    if(!index.isValid())
        return;

    QList<int> removedIndexes;

    for(int i = start; i <= end; i++)
    {
        TAlbum* const t = dynamic_cast<TAlbum*>(d->parentModel->albumForIndex(index.child(i,0)));
        if(t != NULL && !t->isInternalTag())
            removedIndexes.append(t->id());
    }
    QMutableListIterator<QStandardItem*> it(d->allItems);
    while(it.hasNext())
    {
        if(removedIndexes.contains(it.next()->data(Qt::UserRole+5).toInt()))
            it.remove();
    }
}

} // namespace Digikam
