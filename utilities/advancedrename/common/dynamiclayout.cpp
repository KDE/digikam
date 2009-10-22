/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-22
 * Description : a dynamic layout manager
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "dynamiclayout.h"

// Qt includes

#include <QList>
#include <QRect>
#include <QLayoutItem>

// Local includes

#include "debug.h"

namespace Digikam
{

class DynamicLayoutPriv
{
public:

    DynamicLayoutPriv(int hSpacing, int vSpacing) :
        hSpace(hSpacing),
        vSpace(vSpacing),
        spaceX(0),
        spaceY(0),
        minItemWidth(0)
        {}

    int                  hSpace;
    int                  vSpace;
    int                  spaceX;
    int                  spaceY;
    int                  minItemWidth;

    QList<QLayoutItem *> itemList;
};

// --------------------------------------------------------

DynamicLayout::DynamicLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
             : QLayout(parent), d(new DynamicLayoutPriv(hSpacing,vSpacing))
{
    setContentsMargins(margin, margin, margin, margin);
}

DynamicLayout::DynamicLayout(int margin, int hSpacing, int vSpacing)
             : d(new DynamicLayoutPriv(hSpacing,vSpacing))
{
    setContentsMargins(margin, margin, margin, margin);
}

DynamicLayout::~DynamicLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
    {
        delete item;
    }
}

void DynamicLayout::addItem(QLayoutItem *item)
{
    d->minItemWidth = 0;
    d->itemList.append(item);

    foreach (QLayoutItem* item, d->itemList)
    {
        QWidget* wid    = item->widget();
        d->spaceX       = qMax<int>(wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton,
                                                                Qt::Horizontal),
                                    d->spaceX);
        d->spaceY       = qMax<int>(wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton,
                                                                Qt::Vertical),
                                    d->spaceY);
        d->minItemWidth = qMax<int>(wid->sizeHint().width(), d->minItemWidth);
    }

    foreach (QLayoutItem* item, d->itemList)
    {
        QWidget* wid = item->widget();
        wid->setMinimumWidth(d->minItemWidth);
    }
}

int DynamicLayout::horizontalSpacing() const
{
    return d->hSpace;
}

int DynamicLayout::verticalSpacing() const
{
    return d->vSpace;
}

int DynamicLayout::count() const
{
    return d->itemList.size();
}

QLayoutItem *DynamicLayout::itemAt(int index) const
{
    return d->itemList.value(index);
}

QLayoutItem *DynamicLayout::takeAt(int index)
{
    QLayoutItem* item = 0;

    if (index >= 0 && index < d->itemList.size())
    {
        item = d->itemList.takeAt(index);
    }
    return item;
}

Qt::Orientations DynamicLayout::expandingDirections() const
{
    return 0;
}

bool DynamicLayout::hasHeightForWidth() const
{
    return true;
}

int DynamicLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void DynamicLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize DynamicLayout::sizeHint() const
{
    return minimumSize();
}

QSize DynamicLayout::minimumSize() const
{
    QSize size;
    QLayoutItem *item;
    foreach (item, d->itemList)
    {
        size = size.expandedTo(item->minimumSize());
    }

    size += QSize(2 * margin(), 2 * margin());
    return size;
}

int DynamicLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left   = 0;
    int top    = 0;
    int right  = 0;
    int bottom = 0;
    getContentsMargins(&left, &top, &right, &bottom);

    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x               = effectiveRect.x();
    int y               = effectiveRect.y();
    int lineHeight      = 0;

    // --------------------------------------------------------

    int buttonWidth     = d->minItemWidth + d->spaceX;

    int maxButtonsInRow = (effectiveRect.width() - d->spaceX) / buttonWidth;
    if (maxButtonsInRow < 1)
        maxButtonsInRow = 1;

    int maxButtonWidth  = d->minItemWidth + (
            (effectiveRect.width() - (maxButtonsInRow * buttonWidth)) / maxButtonsInRow );

    // --------------------------------------------------------

    foreach (QLayoutItem* item, d->itemList)
    {
        int nextX = x + maxButtonWidth + d->spaceX;
        if ( (nextX - d->spaceX) > effectiveRect.right() && (lineHeight > 0) )
        {
            x          = effectiveRect.x();
            y          = y + lineHeight + d->spaceY;
            nextX      = x + maxButtonWidth + d->spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
        {
            QSize s = item->sizeHint();
            s.setWidth(maxButtonWidth);
            item->setGeometry(QRect(QPoint(x, y), s));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    return y + lineHeight - rect.y() + bottom;
}

} // namespace Digikam
