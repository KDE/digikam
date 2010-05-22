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

#ifndef DIMGCHILDITEM_H
#define DIMGCHILDITEM_H

// Qt includes

#include <QGraphicsItem>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImgChildItemPriv;
class GraphicsDImgItem;

class DIGIKAM_EXPORT AbstractDImgChildItem : public QGraphicsItem
{
public:

    AbstractDImgChildItem(QGraphicsItem* parent = 0);

    /**
     * When an AbstractDImgChildItem is added as a direct child of a
     * GraphicsDImgItem, the method will be called whenever the size
     * of the GraphicsDImgItem has changed, most notably after zooming.
     * The item may choose to resize or reposition itself here.
     */
    virtual void sizeHasChanged() = 0;

    /**
     * If the parent item is a GraphicsDImgItem, return it,
     * if the parent item is null or of a different class, returns 0.
     */
    GraphicsDImgItem *parentDImgItem() const;
};

class DIGIKAM_EXPORT DImgChildItem : public AbstractDImgChildItem
{
public:

    /**
     * This is a base class for items that are positioned on top
     * of a GraphicsDImgItem, positioned in relative coordinates,
     * i.e. [0;1], on the image.
     * From the set relative size, the boundingRect() is calculated.
     */

    DImgChildItem(QGraphicsItem* parent = 0);
    ~DImgChildItem();

    /**
     * Sets the position of this item, relative to the DImg displayed in the parent item.
     * The values of relativePosition must be in the interval [0;1].
     * Calls setPos() accordingly.
     */
    void setRelativePos(const QPointF& relativePosition);
    void setRelativePos(qreal x, qreal y) { setRelativePos(QPointF(x, y)); }

    /**
     * Sets the size of this item, relative to the DImg displayed in the parent item.
     * The values of relativePosition must be in the interval [0;1].
     */
    void setRelativeSize(const QSizeF& relativeSize);
    void setRelativeSize(qreal width, qreal height) { setRelativeSize(QSizeF(width, height)); }

    void setRelativeRect(const QRectF& rect)
        { setRelativePos(rect.topLeft()); setRelativeSize(rect.size()); }
    void setRelativeRect(qreal x, qreal y, qreal width, qreal height)
        { setRelativePos(QPointF(x,y)); setRelativeSize(QSizeF(width, height)); }

    /**
     * Implemented. Returns a rectangle starting at (0,0) (pos() in parent coordinates)
     * and has a size determined by the relative size.
     */
    virtual QRectF boundingRect() const;

    void updatePos();

    virtual void sizeHasChanged();

private:

    DImgChildItemPriv *const d;
};

} // namespace Digikam

#endif // DIMGCHILDITEM_H
