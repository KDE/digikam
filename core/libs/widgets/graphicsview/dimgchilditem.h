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

#ifndef DIMGCHILDITEM_H
#define DIMGCHILDITEM_H

// Qt includes

#include <QGraphicsObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class GraphicsDImgItem;

class DIGIKAM_EXPORT DImgChildItem : public QGraphicsObject
{
    Q_OBJECT

public:

    /**
     * This is a base class for items that are positioned on top
     * of a GraphicsDImgItem, positioned in relative coordinates,
     * i.e. [0;1], on the image.
     * From the set relative size, the boundingRect() is calculated.
     */

    explicit DImgChildItem(QGraphicsItem* const parent = 0);
    ~DImgChildItem();

    /**
     * Sets the position and size of this item, relative to the DImg displayed in the parent item.
     * The values of relativePosition must be in the interval [0;1].
     */
    void setRelativePos(const QPointF& relativePosition);
    void setRelativePos(qreal x, qreal y)
    {
        setRelativePos(QPointF(x, y));
    }

    void setRelativeSize(const QSizeF& relativeSize);
    void setRelativeSize(qreal width, qreal height)
    {
        setRelativeSize(QSizeF(width, height));
    }

    void setRelativeRect(const QRectF& rect);
    void setRelativeRect(qreal x, qreal y, qreal width, qreal height)
    {
        setRelativeRect(QRectF(x,y,width, height));
    }

    /**
     * Returns the position and size relative to the DImg displayed in the parent item.
     * All four values are in the interval [0;1].
     */
    QRectF  relativeRect() const;
    QPointF relativePos()  const;
    QSizeF  relativeSize() const;

    /**
     * Sets the position and size of this item, in coordinates of the original image.
     * Requires a valid parent item.
     */

    void setOriginalPos(const QPointF& posInOriginal);
    void setOriginalPos(qreal x, qreal y)
    {
        setOriginalPos(QPointF(x, y));
    }

    void setOriginalSize(const QSizeF& sizeInOriginal);
    void setOriginalSize(qreal width, qreal height)
    {
        setOriginalSize(QSizeF(width, height));
    }

    void setOriginalRect(const QRectF& rect);
    void setOriginalRect(qreal x, qreal y, qreal width, qreal height)
    {
        setOriginalRect(QRectF(x,y,width, height));
    }

    /**
     * Returns the position and size in coordinates of the original image.
     * Note that the return value is integer based. At high zoom rates,
     * different values of relativeRect() or zoomedRect() may result in the same originalRect(),
     * when one pixel in the original is represented by more than one pixel on screen.
     */
    QRect  originalRect() const;
    QPoint originalPos()  const;
    QSize  originalSize() const;

    /**
     * Sets the position and size of this item, in coordinates of the parent DImg item.
     * This is accepting unscaled parent coordinates, just like the "normal" setPos() does.
     * Requires a valid parent item.
     */
    void setPos(const QPointF& zoomedPos);
    void setPos(qreal x, qreal y)
    {
        setPos(QPointF(x, y));
    }

    void setSize(const QSizeF& zoomedSize);
    void setSize(qreal width, qreal height)
    {
        setSize(QSizeF(width, height));
    }

    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal width, qreal height)
    {
        setPos(QPointF(x,y));
        setSize(QSizeF(width, height));
    }

    /**
     * Equivalent to mapping the scene coordinates to the parent item, and calling setRect().
     */
    void setRectInSceneCoordinates(const QRectF& rect);

    /**
     * Returns position and size of this item, in coordinates of the parent DImg with the current zoom.
     * This is the same result as QRectF(pos(), boundingRect()), boundingRect is virtual and may be
     * overridden by base classes.
     */
    QRectF rect() const;
    QSizeF size() const;

    // Override
    void moveBy(qreal dx, qreal dy)
    {
        setPos(pos().x() + dx, pos().y() + dy);
    }

    /**
     * If the parent item is a GraphicsDImgItem, return it,
     * if the parent item is null or of a different class, returns 0.
     */
    GraphicsDImgItem* parentDImgItem() const;

    /**
     * Reimplemented. Returns a rectangle starting at (0,0) (pos() in parent coordinates)
     * and has a size determined by the relative size.
     */
    virtual QRectF boundingRect() const;

protected Q_SLOTS:

    void imageSizeChanged(const QSizeF&);

Q_SIGNALS:

    /**
     * These signals are emitted when the geometry, relative to the original image,
     * of this item has changed. This happens by calling any of the methods above.
     */
    void positionOnImageChanged();
    void sizeOnImageChanged();
    void geometryOnImageChanged();

    /**
     * These signals are emitted in any case when the geometry changed:
     * Either after changing the geometry relative to the original image,
     * or when the size of the parent GraphicsDImgItem changed (zooming).
     * positionChanged() is equivalent to listening to xChanged() and yChanged().
     */
    void positionChanged();
    void sizeChanged();
    void geometryChanged();

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:

    void updatePos();
    void updateSize();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIMGCHILDITEM_H
