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

#include <klocale.h>
#include <kdebug.h>

#include "tagmngrlistitem.h"
#include "albummanager.h"
#include "album.h"


namespace Digikam {

ListItem::ListItem(QList<QVariant> &data, ListItem *parent)
{
    parentItem = parent;
    itemData.append(data);

    data.pop_front();

    foreach(QVariant val, data)
    {
        tagIds.append(val.toInt());
    }
}

ListItem::~ListItem()
{
    qDeleteAll(childItems);
}

void ListItem::appendChild(ListItem *item)
{
    childItems.append(item);
}

void ListItem::removeTagId(int tagId)
{
    tagIds.removeOne(tagId);
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

QVariant ListItem::data(int role) const
{
    switch(role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
            QString display;
            foreach(int tagId, tagIds)
            {
                TAlbum* album = AlbumManager::instance()->findTAlbum(tagId);
                if(!album)
                {
                    continue;
                }
                display.append(album->title()+ ", ");
                if(role == Qt::DisplayRole && display.size() > 30)
                    break;
            }
            if(display.isEmpty())
                display.append(i18n("All Tags"));
            else
                display.remove(display.size()-2, 2);
            return QVariant(display);
        }
        case Qt::BackgroundRole:
        {
            return itemData.first();
        }
        default:
            return QVariant();
    }
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

ListItem* ListItem::containsItem(ListItem* item)
{
    /** We need to compare items and not pointers **/
    for(int it=0; it < childItems.size(); ++it)
    {
        if(item->equal(childItems.at(it)))
        {
            return childItems.at(it);
        }
    }
    return NULL;
}

bool ListItem::equal(ListItem* item)
{
    return (this->tagIds) == (item->getTagIds());
}
} // namespace Digikam
