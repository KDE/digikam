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

#include "tagmngrlistitem.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "album.h"

namespace Digikam
{

class ListItem::Private
{
public:

    Private()
    {
        parentItem = 0;
    }

    QList<ListItem*> childItems;
    QList<QVariant>  itemData;
    QList<int>       tagIds;
    QList<int>       tagsToDel;
    ListItem*        parentItem;
};

ListItem::ListItem(QList<QVariant>& data, ListItem* const parent)
    : d(new Private())
{
    d->parentItem = parent;
    d->itemData.append(data);

    data.pop_front();

    foreach(const QVariant& val, data)
    {
        d->tagIds.append(val.toInt());
    }
}

ListItem::~ListItem()
{
    qDeleteAll(d->childItems);
    delete d;
}

void ListItem::deleteChild(ListItem* const item)
{
    d->childItems.removeOne(item);
}

QList<ListItem*> ListItem::allChildren() const
{
    return d->childItems;
}

QList<int> ListItem::getTagIds() const
{
    return d->tagIds;
}

void ListItem::appendChild(ListItem* const item)
{
    d->childItems.append(item);
}

void ListItem::removeTagId(int tagId)
{
    d->tagIds.removeOne(tagId);
}

ListItem* ListItem::child(int row) const
{
    return d->childItems.value(row);
}

int ListItem::childCount() const
{
    return d->childItems.count();
}

void ListItem::deleteChild(int row)
{
    return d->childItems.removeAt(row);
}

void ListItem::removeAll()
{
    d->childItems.clear();
}

void ListItem::appendList(const QList<ListItem*>& items)
{
    d->childItems.append(items);
}

int ListItem::columnCount() const
{
    return d->itemData.count();
}

QVariant ListItem::data(int role) const
{
    switch(role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
            QString display;

            foreach(int tagId, d->tagIds)
            {
                TAlbum* const album = AlbumManager::instance()->findTAlbum(tagId);

                if(!album)
                {
                    continue;
                }

                display.append(album->title()+ QLatin1String(", "));

                if(role == Qt::DisplayRole && display.size() > 30)
                    break;
            }

            if(display.isEmpty())
                display.append(i18n("All Tags"));
            else
                display.remove(display.size()-2, 2);

            return QVariant(display);
        }
        default:
        {
            return QVariant();
        }
    }
}

void ListItem::setData(const QList<QVariant>& data)
{
    d->itemData = data;
}

ListItem* ListItem::parent() const
{
    return d->parentItem;
}

int ListItem::row() const
{
    if (d->parentItem)
    {
        return d->parentItem->allChildren().indexOf(const_cast<ListItem*>(this));
    }

    return 0;
}

ListItem* ListItem::containsItem(ListItem* const item) const
{
    // We need to compare items and not pointers

    for(int it=0; it < d->childItems.size(); ++it)
    {
        if(item->equal(d->childItems.at(it)))
        {
            return d->childItems.at(it);
        }
    }

    return 0;
}

bool ListItem::equal(ListItem* const item) const
{
    return (this->d->tagIds) == (item->getTagIds());
}

} // namespace Digikam
