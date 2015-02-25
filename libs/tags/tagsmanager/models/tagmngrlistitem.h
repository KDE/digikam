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

// Qt includes

#include <QList>
#include <QVariant>

namespace Digikam
{

class ListItem : public QObject
{
    Q_OBJECT

public:

    ListItem(QList<QVariant>& data, ListItem* const parent = 0);
    ~ListItem();

    void appendChild(ListItem* const child);
    void removeTagId(int tagId);
    void deleteChild(int row);
    void setData(const QList<QVariant>& data);
    void removeAll();
    void appendList(const QList<ListItem*>& items);
    void deleteChild(ListItem* const item);

    ListItem* child(int row)         const;
    QVariant data(int column)        const;
    ListItem* parent()               const;
    int childCount()                 const;
    int columnCount()                const;
    int row()                        const;
    QList<int> getTagIds()           const;
    QList<ListItem*> allChildren()   const;
    bool equal(ListItem* const item) const;

    /**
     * @brief containsItem  - search child items if contains a ListItem with
     *                        the same data as item
     * @param item          - ListItem pointer for which we should search if there
     *                        is a similar item
     * @return              - NULL if no similar item was found and a valid ListItem
     *                        if a ListItem with the same data was found
     */
    ListItem* containsItem(ListItem* const item) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // LISTITEM_H
