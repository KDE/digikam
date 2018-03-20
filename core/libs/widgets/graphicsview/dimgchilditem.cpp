/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-15
 * Description : Graphics View item for a child item on a DImg item
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include "imagezoomsettings.h"

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

    SimpleRectChildItem(QGraphicsItem* const parent)
        : DImgChildItem(parent)
    {
        setRelativePos(0.5, 0.5);
        setRelativeSize(0.01, 0.01);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        painter->setPen(Qt::red);
        painter->drawRect(boundingRect());
    }
};
*/

class DImgChildItem::Private
{
public:

    explicit Private(DImgChildItem* const q)
        : q(q)
    {
    }

    void connectParent(bool active = true);

public:

    QPointF              relativePos;
    QSizeF               relativeSize;

    DImgChildItem* const q;
};

void DImgChildItem::Private::connectParent(bool active)
{
    GraphicsDImgItem* const parent = q->parentDImgItem();

    if (parent)
    {
        if (active)
        {
            q->connect(parent, SIGNAL(imageSizeChanged(QSizeF)),
                       q, SLOT(imageSizeChanged(QSizeF)));
        }
        else
        {
            q->disconnect(parent, SIGNAL(imageSizeChanged(QSizeF)),
                          q, SLOT(imageSizeChanged(QSizeF)));
        }
    }
}

DImgChildItem::DImgChildItem(QGraphicsItem* const parent)
    : QGraphicsObject(parent), d(new Private(this))
{
    d->connectParent();
}

DImgChildItem::~DImgChildItem()
{
    delete d;
}

void DImgChildItem::setRelativePos(const QPointF& relativePos)
{
    if (d->relativePos == relativePos)
    {
        return;
    }

    d->relativePos = relativePos;
    updatePos();
    emit positionOnImageChanged();
    emit geometryOnImageChanged();
}

void DImgChildItem::setRelativeSize(const QSizeF& relativeSize)
{
    if (d->relativeSize == relativeSize)
    {
        return;
    }

    d->relativeSize = relativeSize;
    updateSize();
    emit sizeOnImageChanged();
    emit geometryOnImageChanged();
}

void DImgChildItem::setRelativeRect(const QRectF& rect)
{
    setRelativePos(rect.topLeft());
    setRelativeSize(rect.size());
}

QRectF DImgChildItem::relativeRect() const
{
    return QRectF(d->relativePos, d->relativeSize);
}

QPointF DImgChildItem::relativePos() const
{
    return d->relativePos;
}

QSizeF DImgChildItem::relativeSize() const
{
    return d->relativeSize;
}

void DImgChildItem::setOriginalPos(const QPointF& posInOriginal)
{
    if (!parentItem())
    {
        return;
    }

    QSizeF originalSize = parentDImgItem()->zoomSettings()->originalImageSize();
    setRelativePos( QPointF(posInOriginal.x() / originalSize.width(),
                            posInOriginal.y() / originalSize.height()) );
}

void DImgChildItem::setOriginalSize(const QSizeF& sizeInOriginal)
{
    if (!parentItem())
    {
        return;
    }

    QSizeF originalSize = parentDImgItem()->zoomSettings()->originalImageSize();
    setRelativeSize( QSizeF(sizeInOriginal.width()  / originalSize.width(),
                            sizeInOriginal.height() / originalSize.height()) );
}

void DImgChildItem::setOriginalRect(const QRectF& rect)
{
    setOriginalPos(rect.topLeft());
    setOriginalSize(rect.size());
}

QRect DImgChildItem::originalRect() const
{
    return QRect(originalPos(), originalSize());
}

QSize DImgChildItem::originalSize() const
{
    QSizeF originalSize = parentDImgItem()->zoomSettings()->originalImageSize();
    return QSizeF(d->relativeSize.width()  * originalSize.width(),
                  d->relativeSize.height() * originalSize.height()).toSize();
}

QPoint DImgChildItem::originalPos() const
{
    QSizeF originalSize = parentDImgItem()->zoomSettings()->originalImageSize();
    return QPointF(d->relativePos.x() * originalSize.width(),
                   d->relativePos.y() * originalSize.height()).toPoint();
}

void DImgChildItem::setPos(const QPointF& pos)
{
    if (!parentItem())
    {
        return;
    }

    const QSizeF imageSize = parentItem()->boundingRect().size();

    setRelativePos( QPointF(pos.x() / imageSize.width(),
                            pos.y() / imageSize.height()) );
}

void DImgChildItem::setSize(const QSizeF& size)
{
    if (!parentItem())
    {
        return;
    }

    const QSizeF imageSize = parentItem()->boundingRect().size();

    setRelativeSize( QSizeF(size.width()  / imageSize.width(),
                            size.height() / imageSize.height()) );
}

void DImgChildItem::setRect(const QRectF& rect)
{
    setPos(rect.topLeft());
    setSize(rect.size());
}

void DImgChildItem::setRectInSceneCoordinates(const QRectF& rect)
{
    if (!parentItem())
    {
        return;
    }

    QRectF itemRect(parentItem()->mapFromScene(rect.topLeft()),
                    parentItem()->mapFromScene(rect.bottomRight()));
    setRect(itemRect);
}

QRectF DImgChildItem::rect() const
{
    return QRectF(pos(), size());
}

QSizeF DImgChildItem::size() const
{
    if (!parentItem())
    {
        return QSizeF();
    }

    const QSizeF imageSize = parentItem()->boundingRect().size();

    return QSizeF(d->relativeSize.width()  * imageSize.width(),
                  d->relativeSize.height() * imageSize.height());
}

GraphicsDImgItem* DImgChildItem::parentDImgItem() const
{
    return dynamic_cast<GraphicsDImgItem*>(parentItem());
}

void DImgChildItem::updatePos()
{
    if (!parentItem())
    {
        return;
    }

    QSizeF imageSize = parentItem()->boundingRect().size();
    QGraphicsObject::setPos(imageSize.width() * d->relativePos.x(), imageSize.height() * d->relativePos.y());
    emit positionChanged();
    emit geometryChanged();
}

void DImgChildItem::updateSize()
{
    prepareGeometryChange();
    emit sizeChanged();
    emit geometryChanged();
}

void DImgChildItem::imageSizeChanged(const QSizeF&)
{
    updateSize();
    updatePos();
}

QVariant DImgChildItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemParentChange)
    {
        d->connectParent(false);    // disconnect old parent
    }
    else if (change == QGraphicsItem::ItemParentHasChanged)
    {
        d->connectParent(true);    // connect new parent
    }

    return QGraphicsObject::itemChange(change, value);
}

QRectF DImgChildItem::boundingRect() const
{
    if (!parentItem())
    {
        return QRectF();
    }

    return QRectF(QPointF(0, 0), size());
}

} // namespace Digikam
