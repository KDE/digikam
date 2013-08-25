/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : List View Item for List View Model
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

#include <QStringList>

#include "tagmngrlistitem.h"

ListItem::ListItem(const QList<QVariant> &data, ListItem *parent)
{
    parentItem = parent;
    itemData = data;
}

ListItem::~ListItem()
{
    qDeleteAll(childItems);
}

void ListItem::appendChild(ListItem *item)
{
    childItems.append(item);
}

ListItem *ListItem::child(int row)
{
    return childItems.value(row);
}

int ListItem::childCount() const
{
    return childItems.count();
}

void ListItem::deleteChild(int row)
{
    return childItems.removeAt(row);
}

void ListItem::removeAll()
{
    childItems.clear();
}

void ListItem::appendList(QList<ListItem*> items)
{
    childItems.append(items);
}

int ListItem::columnCount() const
{
    return itemData.count();
}

QVariant ListItem::data(int column) const
{
    return itemData.value(column);
}

void ListItem::setData(QList<QVariant> &data)
{
    itemData = data;
}

ListItem *ListItem::parent()
{
    return parentItem;
}

int ListItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ListItem*>(this));

    return 0;
}
