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

#ifndef LISTITEM_H
#define LISTITEM_H

#include <QList>
#include <QVariant>

namespace Digikam {

class ListItem
{
public:
    ListItem(QList<QVariant> &data, ListItem *parent = 0);
    ~ListItem();

    void appendChild(ListItem *child);

    void removeTagId(int tagId);

    ListItem *child(int row);
    int childCount() const;
    void deleteChild(int row);
    void deleteChild(ListItem* item) { childItems.removeOne(item); }
    void removeAll();
    void appendList(QList<ListItem*> items);
    int columnCount() const;
    QVariant data(int column) const;
    void setData(QList<QVariant> &data);
    int row() const;
    ListItem *parent();

    QList<ListItem*> allChildren() { return childItems; }
    QList<int> getTagIds() const { return tagIds; }
    bool containsItem(ListItem* item);
    bool equal(ListItem* item);

private:

    QList<ListItem*> childItems;
    QList<QVariant> itemData;
    QList<int> tagIds;
    QList<int> tagsToDel;
    ListItem *parentItem;
};

} // namespace Digikam
#endif
