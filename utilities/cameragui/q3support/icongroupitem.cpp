/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-24
 * Description : icons group item.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "icongroupitem.h"

// C++ includes

#include <cstdlib>

// Qt includes

#include <QPixmap>
#include <QPalette>
#include <QPainter>

// Local includes

#include "iconview.h"
#include "iconitem.h"

namespace Digikam
{

class IconGroupItem::IconGroupItemPriv
{
public:

    IconGroupItemPriv()
    {
        view      = 0;
        firstItem = 0;
        lastItem  = 0;
        y         = 0;
        count     = 0;
        clearing  = false;
    }

    IconView*      view;

    IconItem*      firstItem;
    IconItem*      lastItem;

    int            y;
    int            count;
    bool           clearing;

    struct SortableItem
    {
        IconItem* item;
    };
};

IconGroupItem::IconGroupItem(IconView* parent)
    : d(new IconGroupItemPriv)
{
    d->view = parent;
    m_next  = 0;
    m_prev  = 0;

    parent->insertGroup(this);
}

IconGroupItem::~IconGroupItem()
{
    clear(false);
    d->view->takeGroup(this);
    delete d;
}

IconView* IconGroupItem::iconView() const
{
    return d->view;
}

IconGroupItem* IconGroupItem::nextGroup() const
{
    return m_next;
}

IconGroupItem* IconGroupItem::prevGroup() const
{
    return m_prev;
}

QRect IconGroupItem::rect() const
{
    QRect r = d->view->bannerRect();
    r.translate(0, d->y);
    return r;
}

int IconGroupItem::y() const
{
    return d->y;
}

IconItem* IconGroupItem::firstItem() const
{
    return d->firstItem;
}

IconItem* IconGroupItem::lastItem() const
{
    return d->lastItem;
}

void IconGroupItem::insertItem(IconItem* item)
{
    if (!item)
    {
        return;
    }

    if (!d->firstItem)
    {
        d->firstItem = item;
        d->lastItem  = item;
        item->m_prev = 0;
        item->m_next = 0;
    }
    else
    {
        d->lastItem->m_next = item;
        item->m_prev        = d->lastItem;
        item->m_next        = 0;
        d->lastItem         = item;
    }

    d->count++;
    d->view->insertItem(item);
}

void IconGroupItem::takeItem(IconItem* item)
{
    if (!item)
    {
        return;
    }

    // take item triggers update
    d->view->takeItem(item);
    d->count--;

    if (item == d->firstItem)
    {
        d->firstItem = d->firstItem->m_next;

        if (d->firstItem)
        {
            d->firstItem->m_prev = 0;
        }
        else
        {
            d->firstItem = d->lastItem = 0;
        }
    }
    else if (item == d->lastItem)
    {
        d->lastItem = d->lastItem->m_prev;

        if (d->lastItem)
        {
            d->lastItem->m_next = 0;
        }
        else
        {
            d->firstItem = d->lastItem = 0;
        }
    }
    else
    {
        IconItem* i = item;

        if (i)
        {
            if (i->m_prev)
            {
                i->m_prev->m_next = i->m_next;
            }

            if (i->m_next)
            {
                i->m_next->m_prev = i->m_prev;
            }
        }
    }
}

int IconGroupItem::count() const
{
    return d->count;
}

int IconGroupItem::index(IconItem* item) const
{
    if (!item)
    {
        return -1;
    }

    if (item == d->firstItem)
    {
        return 0;
    }
    else if (item == d->lastItem)
    {
        return d->count - 1;
    }
    else
    {
        IconItem* i = d->firstItem;
        int j = 0;

        while (i && i != item)
        {
            i = i->m_next;
            ++j;
        }

        return i ? j : -1;
    }
}

void IconGroupItem::clear(bool update)
{
    d->clearing = true;

    IconItem* item = d->firstItem;

    while (item)
    {
        IconItem* tmp = item->m_next;
        delete item;
        item = tmp;
    }

    d->firstItem = 0;
    d->lastItem  = 0;
    d->count     = 0;

    if (update)
    {
        d->view->triggerRearrangement();
    }

    d->clearing = false;
}

void IconGroupItem::sort()
{
    IconGroupItemPriv::SortableItem* items = new IconGroupItemPriv::SortableItem[ count() ];

    IconItem* item = d->firstItem;
    int i = 0;

    for (; item; item = item->m_next)
    {
        items[ i++ ].item = item;
    }

    qsort(items, count(), sizeof(IconGroupItemPriv::SortableItem), cmpItems);

    IconItem* prev = 0;
    item           = 0;

    for (i = 0; i < (int)count(); ++i)
    {
        item = items[ i ].item;

        if (item)
        {
            item->m_prev = prev;

            if (item->m_prev)
            {
                item->m_prev->m_next = item;
            }

            item->m_next = 0;
        }

        if (i == 0)
        {
            d->firstItem = item;
        }

        if (i == (int)count() - 1)
        {
            d->lastItem = item;
        }

        prev = item;
    }

    delete [] items;
}

bool IconGroupItem::move(int y)
{
    if (d->y == y)
    {
        return false;
    }

    d->y = y;
    return true;
}

void IconGroupItem::paintBanner(QPainter* p)
{
    p->fillRect(rect(), d->view->palette().color(QPalette::Base));
}

int IconGroupItem::compare(IconGroupItem*)
{
    return 0;
}

int IconGroupItem::cmpItems(const void* n1, const void* n2)
{
    if (!n1 || !n2)
    {
        return 0;
    }

    IconGroupItemPriv::SortableItem* i1 = (IconGroupItemPriv::SortableItem*)n1;
    IconGroupItemPriv::SortableItem* i2 = (IconGroupItemPriv::SortableItem*)n2;

    return i1->item->compare(i2->item);
}

}  // namespace Digikam
