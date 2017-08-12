/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "LayersModel.h"

// Qt includes

#include <QDebug>
#include <QIcon>
#include <QCommonStyle>
#include <QLabel>
#include <QVariant>
#include <QList>

// Local includes

#include "LayersModelItem.h"
#include "AbstractPhoto.h"

namespace PhotoLayoutsEditor
{

LayersModel::LayersModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    root = new LayersModelItem(0, 0, this);
}

LayersModel::~LayersModel()
{
    delete root;
}

Qt::DropActions LayersModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

QModelIndex LayersModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex( row, column, parent ))
        return QModelIndex();
    LayersModelItem * parentItem = getItem(parent);
    LayersModelItem * childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row,column,childItem);
    else
        return QModelIndex();
}

QModelIndex LayersModel::parent(const QModelIndex & index) const
{
    if(!index.isValid())
        return QModelIndex();
    LayersModelItem * childItem = static_cast<LayersModelItem*>(index.internalPointer());
    LayersModelItem * parentItem = childItem->parent();
    if (parentItem == root)
        return QModelIndex();
    qDebug () << "LayersModelItem[parent]" << (long) parentItem;
    return createIndex(parentItem->row(), 0, parentItem);
}

int LayersModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;
    if (!parent.isValid())
        return root->childCount();
    else
        return static_cast<LayersModelItem*>(parent.internalPointer())->childCount();
}

int LayersModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return static_cast<LayersModelItem*>(parent.internalPointer())->columnCount();
    else
        return root->columnCount();
}

QVariant LayersModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();
    LayersModelItem * item = static_cast<LayersModelItem*>(index.internalPointer());
    switch(role)
    {
        case Qt::DecorationRole:
            if (index.column() == LayersModelItem::NameString)
                return item->data(LayersModelItem::Thumbnail);
            break;
        case Qt::DisplayRole:
            if (index.column() == LayersModelItem::NameString)
                return item->data(index.column());
            break;
        case Qt::EditRole:
            if (index.column() == LayersModelItem::NameString)
                return item->data(LayersModelItem::NameString);
            break;
        case Qt::SizeHintRole:
            return QSize(-1,50);
            break;
    }
    return QVariant();
}

bool LayersModel::setData(const QModelIndex & index, const QVariant & value, int /*role*/)
{
    if (!index.isValid() || index.column() != LayersModelItem::NameString)
        return false;
    LayersModelItem * item = static_cast<LayersModelItem*>(index.internalPointer());
    return item->setData(value, LayersModelItem::NameString);
}

Qt::ItemFlags LayersModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = Qt::ItemIsDropEnabled | this->QAbstractItemModel::flags(index);
    if (index.isValid())
    {
        if (index.column() == LayersModelItem::NameString)
            result |= Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable; // ONLY ONE COLUMN COULD HAVE ItemIsSelectable FLAG! (it simplifies model)
        else
            result &= !Qt::ItemIsEditable & !Qt::ItemIsSelectable;
        result |= Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
    }
    return result;
}

QVariant LayersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return root->data(section);
            case Qt::DecorationRole:
                if (section == 1 || section == 2)
                {
                    return root->data(section);
                }
                break;
        }
    }
    return QVariant();
}

bool LayersModel::appendItem(AbstractPhoto * item, const QModelIndex & parent)
{
    LayersModelItem * parentItem = getItem(parent);
    bool result = this->insertRow(parentItem->childCount(),parent);
    if (result)
        static_cast<LayersModelItem*>(index(parentItem->childCount()-1,0,parent).internalPointer())->setPhoto(item);
    return result;
}

bool LayersModel::insertItem(AbstractPhoto * item, int position, const QModelIndex & parent)
{
    QList<AbstractPhoto*> items;
    items << item;
    if (this->itemsToIndexes(items).count())
        return false;
    bool result = this->insertRow(position, parent);
    if (result)
        static_cast<LayersModelItem*>(index(position,0,parent).internalPointer())->setPhoto(item);
    return result;
}

bool LayersModel::prependItem(AbstractPhoto * item, const QModelIndex & parent)
{
    return insertItem(item, 0, parent);
}

bool LayersModel::appendItems(const QList<AbstractPhoto*> & items, const QModelIndex & parent)
{
    return insertItems(items, getItem(parent)->childCount(), parent);
}

bool LayersModel::insertItems(const QList<AbstractPhoto*> & items, int position, const QModelIndex & parent)
{
    foreach(AbstractPhoto* item, items)
        if (!insertItem(item, position++, parent))
            return false;
    return true;
}

bool LayersModel::prependItems(const QList<AbstractPhoto*> & items, const QModelIndex & parent)
{
    return insertItems(items, 0, parent);
}

bool LayersModel::insertRows(int position, int count, const QModelIndex  & parent)
{
    LayersModelItem * parentItem = getItem(parent);
    if (position > parentItem->childCount())
        return false;
    beginInsertRows(parent,position,position+count-1);
    bool result = true;
    for (;count;--count)
        result &= parentItem->insertChildren(position, new LayersModelItem(0, 0, this));
    endInsertRows();
    emit layoutChanged();
    return result;
}

LayersModelItem * LayersModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<LayersModelItem*>(index.internalPointer());
    return root;
}

QModelIndexList LayersModel::itemsToIndexes(const QList<AbstractPhoto*> & items) const
{
    QModelIndexList indexes;
    QModelIndex temp;
    foreach(AbstractPhoto* item, items)
    {
        temp = findIndex(item);
        if (temp.isValid())
            indexes.append(temp);
    }
    return indexes;
}

QList<AbstractPhoto*> LayersModel::indexesToItems(const QModelIndexList & indexes) const
{
    QList<AbstractPhoto*> items;
    foreach(QModelIndex index, indexes)
        items.append(getItem(index)->photo());
    return items;
}

QModelIndex LayersModel::findIndex(AbstractPhoto * item, const QModelIndex & parent) const
{
    if (item)
    {
        QModelIndex temp;
        LayersModelItem * parentItem = getItem(parent);
        int rows = parentItem->childCount();
        for (int i = 0; i < rows; ++i)
        {
            temp = index(i,LayersModelItem::NameString,parent);
            if (!temp.isValid())
                continue;
            if (static_cast<LayersModelItem*>(temp.internalPointer())->photo() == item)
                return temp;
            temp = findIndex(item, temp);
            if (temp.isValid())
                return temp;
        }
    }
    return QModelIndex();
}

QModelIndex LayersModel::findIndex(LayersModelItem * item, const QModelIndex & parent) const
{
    QModelIndex temp;
    LayersModelItem * parentItem = getItem(parent);
    int rows = parentItem->childCount();
    for (int i = 0; i < rows; ++i)
    {
        temp = index(i,0,parent);
        if (!temp.isValid())
            continue;
        if (static_cast<LayersModelItem*>(temp.internalPointer()) == item)
            return temp;
        temp = findIndex(item, temp);
        if (temp.isValid())
            return temp;
    }
    return QModelIndex();
}

bool LayersModel::removeRows(int row, int count, const QModelIndex & parent)
{
    LayersModelItem * parentItem = getItem(parent);
    if (row >= parentItem->childCount() || row+count > parentItem->childCount())
        return false;
    beginRemoveRows(parent, row, row+count-1);
    bool result = parentItem->removeChildren(row, count);
    endRemoveRows();
    emit layoutChanged();
    return result;
}

bool LayersModel::moveRows(int sourcePosition,
                           const QModelIndex & sourceParent,
                           int destPosition,
                           const QModelIndex & destinationParent)
{
    return moveRows(sourcePosition, 1, sourceParent, destPosition, destinationParent);
}

bool LayersModel::moveRows(const QModelIndex & sourceIndex,
                           const QModelIndex & sourdeParent,
                           const QModelIndex & destinationIndex,
                           const QModelIndex & destinationParent)
{
    return moveRows(sourceIndex.row(), 1, sourdeParent, destinationIndex.row()+1, destinationParent);
}

bool LayersModel::moveRows(int sourcePosition,
                           int sourceCount,
                           const QModelIndex & sourceParent,
                           int destPosition,
                           const QModelIndex & destinationParent)
{
    LayersModelItem * srcItem = getItem(sourceParent);
    LayersModelItem * destItem = getItem(destinationParent);
    if (    sourceCount                                          &&
            sourcePosition < srcItem->childCount()               &&
            sourcePosition+sourceCount <= srcItem->childCount()  &&
            destPosition <= destItem->childCount()               &&
            ((srcItem != destItem) || (srcItem == destItem && sourcePosition != destPosition && sourcePosition != destPosition-1)) &&
            sourcePosition >= 0                                  &&
            destPosition >= 0)
    {
        beginMoveRows(sourceParent, sourcePosition, sourcePosition+sourceCount-1, destinationParent, destPosition);
        bool result = srcItem->moveChildren(sourcePosition, sourceCount, destItem, destPosition);
        endMoveRows();
        emit layoutChanged();
        return result;
    }
    return false;
}

bool LayersModel::moveRows(const QModelIndex & start1,
                           const QModelIndex & end1,
                           const QModelIndex & parent1,
                           const QModelIndex & start2,
                           const QModelIndex & parent2)
{
    return moveRows(start1.row(), end1.row(), parent1, start2.row(), parent2);
}

void LayersModel::updateModel(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
    emit dataChanged(topLeft, bottomRight);
}

} // namespace PhotoLayoutsEditor
