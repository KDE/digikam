/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : List View Model with support for mime data and drag-n-drop
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

#include "tagmngrlistmodel.h"

// Qt includes

#include <QStringList>
#include <QSize>
#include <QBrush>
#include <QMimeData>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "tagmngrlistitem.h"

namespace Digikam
{

class TagMngrListModel::Private
{
public:

    Private()
    {
        rootItem = 0;
    }

    ListItem*  rootItem;
    QList<int> dragNewSelection;
};

TagMngrListModel::TagMngrListModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d(new Private())
{
    QList<QVariant> rootData;
    rootData << QLatin1String("Quick List");
    d->rootItem = new ListItem(rootData);
}

TagMngrListModel::~TagMngrListModel()
{
    delete d->rootItem;
    delete d;
}

ListItem* TagMngrListModel::addItem(QList<QVariant> values)
{
    emit layoutAboutToBeChanged();
    ListItem* const item = new ListItem(values, d->rootItem);

    /** containsItem will return a valid pointer if item with the same
     *  values is already added to it's children list.
     */
    ListItem* const existingItem = d->rootItem->containsItem(item);

    if(!existingItem)
    {
        d->rootItem->appendChild(item);
        emit layoutChanged();
        return item;
    }
    else
    {
        delete item;
        return existingItem;
    }
}

QList<ListItem*> TagMngrListModel::allItems() const
{
    return d->rootItem->allChildren();
}

QList<int> TagMngrListModel::getDragNewSelection() const
{
    return d->dragNewSelection;
}

void TagMngrListModel::deleteItem(ListItem* const item)
{
    if(!item)
        return;

    emit layoutAboutToBeChanged();
    d->rootItem->deleteChild(item);
    emit layoutChanged();

}

int TagMngrListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Qt::DropActions TagMngrListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TagMngrListModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/vnd.text.list");
    return types;
}

bool TagMngrListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(role);
    ListItem* const parent = static_cast<ListItem*>(index.internalPointer());

    if(!parent)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No node found";
        return false;
    }

    QList<QVariant> itemDa;
    itemDa << value;
    parent->appendChild(new ListItem(itemDa,parent));

    return true;
}

QMimeData* TagMngrListModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* const mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach(const QModelIndex& index, indexes)
    {
        if(index.isValid())
        {
            stream << index.row();
        }
    }

    mimeData->setData(QLatin1String("application/vnd.text.list"), encodedData);
    return mimeData;
}

bool TagMngrListModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                    int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if(action == Qt::IgnoreAction)
        return true;

    if(!(data->hasFormat(QLatin1String("application/vnd.text.list"))))
        return false;

    QByteArray       encodedData = data->data(QLatin1String("application/vnd.text.list"));
    QDataStream      stream(&encodedData, QIODevice::ReadOnly);
    QList<ListItem*> newItems;
    QList<ListItem*> finalItems;
    QList<int>       toRemove;

    int itemPoz;
    int temp = 0;

    while(!stream.atEnd())
    {
        stream >> itemPoz;
        newItems << d->rootItem->child(itemPoz);

        if(itemPoz < row)
        {
            temp++;
        }

        toRemove.append(itemPoz);
    }

    row -= temp;
    emit layoutAboutToBeChanged();

    for(QList<int>::iterator itr = toRemove.end() -1 ; itr != toRemove.begin() -1 ; --itr)
    {
        d->rootItem->deleteChild(*itr);
    }

    emit layoutChanged();

    for(int it = 0; it < d->rootItem->childCount(); it++)
    {
        finalItems.append(d->rootItem->child(it));

        if(it == row)
        {
            finalItems.append(newItems);
            /** After drag-n-drop selection is messed up, store the interval were
             *  new items are and TagsMngrListView will update seelction
             */
            d->dragNewSelection.clear();
            d->dragNewSelection << row;
            d->dragNewSelection << row + newItems.size();
        }
    }

    d->rootItem->removeAll();
    d->rootItem->appendList(finalItems);

    emit layoutChanged();

    return true;
}

QVariant TagMngrListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::SizeHintRole)
        return QSize(30,30);

    if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    ListItem* const item = static_cast<ListItem*>(index.internalPointer());
    return item->data(role);
}

Qt::ItemFlags TagMngrListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable
                             | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant TagMngrListModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant(i18n("Quick Access List"));

    return QVariant();
}

QModelIndex TagMngrListModel::index(int row, int column, const QModelIndex& parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ListItem* parentItem = 0;

    if (!parent.isValid())
        parentItem = d->rootItem;
    else
        parentItem = static_cast<ListItem*>(parent.internalPointer());

    ListItem* const childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TagMngrListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ListItem* const childItem  = static_cast<ListItem*>(index.internalPointer());
    ListItem* const parentItem = childItem->parent();

    if (parentItem == d->rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TagMngrListModel::rowCount(const QModelIndex &parent) const
{
    ListItem* parentItem = 0;

    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = d->rootItem;
    else
        parentItem = static_cast<ListItem*>(parent.internalPointer());

    return parentItem->childCount();
}

} // namespace Digikam
