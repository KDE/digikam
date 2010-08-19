/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-15
 * Description : Graphics View item for a child item on a DImg item
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimgchilditem.h"

// Local includes

#include "graphicsdimgitem.h"

namespace Digikam
{

/**
class SimpleRectChildItem : public DImgChildItem
{
public:

    / ** This is a simple example. Just create
     *  new SimpleRectChildItem(item);
     *  where item is a GrahpicsDImgItem,
     *  and at the center of the image,
     *  a rectangle of 1% the size of the image will be drawn.
     * /

    SimpleRectChildItem(QGraphicsItem *parent)
        : DImgChildItem(parent)
    {
        setRelativePos(0.5, 0.5);
        setRelativeSize(0.01, 0.01);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        painter->setPen(Qt::red);
        painter->drawRect(boundingRect());
    }
};
*/

class DImgChildItem::DImgChildItemPriv
{
public:

    DImgChildItemPriv()
    {
    }

    QPointF relativePos;
    QSizeF  relativeSize;
};

DImgChildItem::DImgChildItem(QGraphicsItem* parent)
             : QGraphicsItem(parent), d(new DImgChildItemPriv)
{
}

DImgChildItem::~DImgChildItem()
{
    delete d;
}

void DImgChildItem::setRelativePos(const QPointF& relativePos)
{
    d->relativePos = relativePos;
    updatePos();
}

void DImgChildItem::setRelativeSize(const QSizeF& relativeSize)
{
    d->relativeSize = relativeSize;
    prepareGeometryChange();
}

GraphicsDImgItem* DImgChildItem::parentDImgItem() const
{
    return dynamic_cast<GraphicsDImgItem*>(parentItem());
}

void DImgChildItem::updatePos()
{
    if (!parentItem())
        return;

    QSizeF imageSize = parentItem()->boundingRect().size();
    setPos(imageSize.width() * d->relativePos.x(), imageSize.height() * d->relativePos.y());
}

void DImgChildItem::sizeHasChanged()
{
    prepareGeometryChange();
    updatePos();
}

QRectF DImgChildItem::boundingRect() const
{
    if (!parentItem())
        return QRectF();

    QSizeF imageSize = parentItem()->boundingRect().size();
    return QRectF(0, 0, imageSize.width() * d->relativeSize.width(), imageSize.height() * d->relativeSize.height());
}

/*
QVariant DImgChildItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemParentHasChanged)
    {
        QGraphicsItem *item = value.value<QGraphicsItem*>();
        d->dimgItem = dynamic_cast<GraphicsDImgItem*>(item);
    }
    return QGraphicsItem::itemChange(change, value);
}
*/

} // namespace Digikam
