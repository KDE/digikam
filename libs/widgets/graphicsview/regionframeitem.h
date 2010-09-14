/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2010-09-09
* Description : tag region frame
*
* Copyright (C) 2007 by Aurelien Gateau <agateau@kde.org>
* Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef REGION_FRAME_ITEM_H
#define REGION_FRAME_ITEM_H

// Qt includes

#include <QFlags>

// KDE includes

// Local includes

#include "dimgchilditem.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RegionFrameItem : public DImgChildItem
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverAnimationOpacity READ hoverAnimationOpacity WRITE setHoverAnimationOpacity)

public:

    enum Flag
    {
        NoFlags           = 0,
        ShowResizeHandles = 1 << 0,
        MoveByDrag        = 1 << 1,

        GeometryEditable  = ShowResizeHandles | MoveByDrag
    };
    Q_DECLARE_FLAGS(Flags, Flag)

public:

    RegionFrameItem(QGraphicsItem* parent);
    ~RegionFrameItem();

    void setFlags(Flags flags);
    Flags flags() const;

    void setFixedRatio(double ratio);

    virtual QRectF boundingRect() const;

    qreal hoverAnimationOpacity() const;
    void setHoverAnimationOpacity(qreal alpha);

public Q_SLOTS:

    /**
     * The associated HUD item is dynamically moved to be visible.
     * This can only be done for _one_ region at a time.
     * Set the current primary view region of the scene by this method
     * to dynamically reposition the HUD inside this region.
     * The rect given is in scene coordinates.
     */
    void setViewportRect(const QRectF& rect);

protected:

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

private Q_SLOTS:

    void slotSizeChanged();
    void slotPosChanged();
    void hudSizeChanged();
    void moveHudWidget();

private:

    class RegionFrameItemPriv;
    RegionFrameItemPriv* const d;
};

} // namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::RegionFrameItem::Flags)

#endif /* REGION_FRAME_ITEM_H */
