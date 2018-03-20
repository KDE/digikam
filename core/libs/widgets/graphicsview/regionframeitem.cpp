/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2010-09-09
* Description : tag region frame
*
* Copyright (C) 2007      by Aurelien Gateau <agateau at kde dot org>
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

#include "regionframeitem.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QFlags>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QRect>
#include <QTimer>
#include <QToolButton>

// Local includes

#include "graphicsdimgitem.h"
#include "itemvisibilitycontroller.h"

namespace
{
static const int HANDLE_SIZE = 15;
}

namespace Digikam
{

enum CropHandleFlag
{
    CH_None,
    CH_Top         = 1,
    CH_Left        = 2,
    CH_Right       = 4,
    CH_Bottom      = 8,
    CH_TopLeft     = CH_Top    | CH_Left,
    CH_BottomLeft  = CH_Bottom | CH_Left,
    CH_TopRight    = CH_Top    | CH_Right,
    CH_BottomRight = CH_Bottom | CH_Right,
    CH_Content     = 16
};

enum HudSide
{
    HS_None         = 0, // Special value used to avoid initial animation
    HS_Top          = 1,
    HS_Bottom       = 2,
    HS_Inside       = 4,
    HS_TopInside    = HS_Top    | HS_Inside,
    HS_BottomInside = HS_Bottom | HS_Inside
};

typedef QPair<QPointF, HudSide> OptimalPosition;

Q_DECLARE_FLAGS(CropHandle, CropHandleFlag)

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::CropHandle)

// --------------------------------------------------------------------------------

namespace Digikam
{

class RegionFrameItem::Private
{
public:

    explicit Private(RegionFrameItem* const q);

    QRectF handleRect(CropHandle handle) const;
    CropHandle handleAt(const QPointF& pos) const;
    void updateCursor(CropHandle handle, bool buttonDown);
    QRectF keepRectInsideImage(const QRectF& rect, bool moving=true) const;
    OptimalPosition computeOptimalHudWidgetPosition() const;
    void updateHudWidgetPosition();

public:

    RegionFrameItem* const q;

    HudSide                hudSide;
    QRectF                 viewportRect;
    QList<CropHandle>      cropHandleList;
    CropHandle             movingHandle;
    QPointF                lastMouseMovePos;
    double                 fixedRatio;
    QGraphicsWidget*       hudWidget;

    RegionFrameItem::Flags flags;

    AnimatedVisibility*    resizeHandleVisibility;
    qreal                  hoverAnimationOpacity;
    QTimer*                hudTimer;
    QPointF                hudEndPos;

    const int              HUD_TIMER_MAX_PIXELS_PER_UPDATE;
    const int              HUD_TIMER_ANIMATION_INTERVAL;
};

RegionFrameItem::Private::Private(RegionFrameItem* const qq)
    : q(qq),
      HUD_TIMER_MAX_PIXELS_PER_UPDATE(20),
      HUD_TIMER_ANIMATION_INTERVAL(20)
{
    hudSide               = HS_None;
    movingHandle          = CH_None;
    fixedRatio            = 0;
    resizeHandleVisibility= 0;
    hudWidget             = 0;
    hoverAnimationOpacity = 1.0;
    hudTimer              = 0;
    flags                 = NoFlags;

    cropHandleList << CH_Left << CH_Right << CH_Top << CH_Bottom
                   << CH_TopLeft << CH_TopRight
                   << CH_BottomLeft << CH_BottomRight;
}

QRectF RegionFrameItem::Private::handleRect(CropHandle handle) const
{
    QSizeF size = q->boundingRect().size();
    double left, top;

    if (handle & CH_Top)
    {
        top = 0;
    }
    else if (handle & CH_Bottom)
    {
        top = size.height() - HANDLE_SIZE;
    }
    else
    {
        top = (size.height() - HANDLE_SIZE) / 2;
    }

    if (handle & CH_Left)
    {
        left = 0;
    }
    else if (handle & CH_Right)
    {
        left = size.width() - HANDLE_SIZE;
    }
    else
    {
        left = (size.width() - HANDLE_SIZE) / 2;
    }

    return QRectF(left, top, HANDLE_SIZE, HANDLE_SIZE);
}

CropHandle RegionFrameItem::Private::handleAt(const QPointF& pos) const
{
    if (flags & ShowResizeHandles)
    {
        foreach(const CropHandle& handle, cropHandleList)
        {
            QRectF rect = handleRect(handle);

            if (rect.contains(pos))
            {
                return handle;
            }
        }
    }

    if (flags & MoveByDrag)
    {
        if (q->boundingRect().contains(pos))
        {
            return CH_Content;
        }
    }


    return CH_None;
}

void RegionFrameItem::Private::updateCursor(CropHandle handle, bool buttonDown)
{
    Qt::CursorShape shape;

    switch (handle)
    {
        case CH_TopLeft:
        case CH_BottomRight:
            shape = Qt::SizeFDiagCursor;
            break;

        case CH_TopRight:
        case CH_BottomLeft:
            shape = Qt::SizeBDiagCursor;
            break;

        case CH_Left:
        case CH_Right:
            shape = Qt::SizeHorCursor;
            break;

        case CH_Top:
        case CH_Bottom:
            shape = Qt::SizeVerCursor;
            break;

        case CH_Content:
            shape = buttonDown ? Qt::ClosedHandCursor : Qt::OpenHandCursor;
            break;

        default:
            shape = Qt::ArrowCursor;
            break;
    }

    q->setCursor(shape);
}

QRectF RegionFrameItem::Private::keepRectInsideImage(const QRectF& rect, bool moving) const
{
    QRectF r(rect);
    const QSizeF imageSize = q->parentDImgItem()->boundingRect().size();

    if (r.width() > imageSize.width() || r.height() > imageSize.height())
    {
        // This can happen when the crop ratio changes
        QSizeF rectSize = r.size();
        rectSize.scale(imageSize, Qt::KeepAspectRatio);
        r.setSize(rectSize);
    }

    if (r.right() > imageSize.width())
    {
        moving ? r.moveRight(imageSize.width()) : r.setRight(imageSize.width());
    }
    else if (r.left() < 0)
    {
        moving ? r.moveLeft(0) : r.setLeft(0);
    }

    if (r.bottom() > imageSize.height())
    {
        moving ? r.moveBottom(imageSize.height()) : r.setBottom(imageSize.height());
    }
    else if (r.top() < 0)
    {
        moving ? r.moveTop(0) : r.setTop(0);
    }

    return r;
}

OptimalPosition RegionFrameItem::Private::computeOptimalHudWidgetPosition() const
{
    const QRectF visibleSceneRect = viewportRect.isValid() ? viewportRect : q->scene()->sceneRect();
    const QRectF rect             = q->sceneBoundingRect();

    const int margin              = HANDLE_SIZE;
    const int hudHeight           = hudWidget->boundingRect().height();
    const QRectF hudMaxRect       = visibleSceneRect.adjusted(0, 0, 0, -hudHeight);

    OptimalPosition ret;

    // Compute preferred and fallback positions. Preferred is outside rect
    // on the same side, fallback is outside on the other side.
    OptimalPosition preferred     = OptimalPosition(QPointF(rect.left(), rect.bottom() + margin),          HS_Bottom);
    OptimalPosition fallback      = OptimalPosition(QPointF(rect.left(), rect.top() - margin - hudHeight), HS_Top);

    if (hudSide & HS_Top)
    {
        std::swap(preferred, fallback);
    }

    // Check if a position outside rect fits
    if (hudMaxRect.contains(preferred.first))
    {
        ret = preferred;
    }
    else if (hudMaxRect.contains(fallback.first))
    {
        ret= fallback;
    }
    else
    {
        // Does not fit outside, use a position inside rect
        QPoint pos;

        if (hudSide & HS_Top)
        {
            pos = QPoint(rect.left() + margin, rect.top() + margin);
        }
        else
        {
            pos = QPoint(rect.left() + margin, rect.bottom() - margin - hudHeight);
        }

        ret = OptimalPosition(pos, HudSide(hudSide | HS_Inside));
    }

    // Ensure it's always fully visible
    ret.first.rx() = qMin(ret.first.rx(), hudMaxRect.width() - hudWidget->boundingRect().width());

    // map from scene to item coordinates
    ret.first      = q->mapFromScene(ret.first);

    return ret;
}

void RegionFrameItem::Private::updateHudWidgetPosition()
{
    if (!hudWidget || !q->scene())
    {
        return;
    }

    OptimalPosition result = computeOptimalHudWidgetPosition();

    if (result.first == hudWidget->pos() && result.second == hudSide)
    {
        return;
    }

    if (hudSide == HS_None)
    {
        hudSide = result.second;
    }

    if (hudSide == result.second && !hudTimer->isActive())
    {
        // Not changing side and not in an animation, move directly the hud
        // to the final position to avoid lagging effect
        hudWidget->setPos(result.first);
    }
    else
    {
        hudEndPos = result.first;
        hudSide   = result.second;

        if (!hudTimer->isActive())
        {
            hudTimer->start();
        }
    }
}

// ---------------------------------------------------------------------------------------

RegionFrameItem::RegionFrameItem(QGraphicsItem* const item)
    : DImgChildItem(item), d(new Private(this))
{
    d->resizeHandleVisibility = new AnimatedVisibility(this);
    d->resizeHandleVisibility->controller()->setShallBeShown(false);

    connect(d->resizeHandleVisibility, SIGNAL(visibleChanged()),
            this, SLOT(slotUpdate()));

    connect(d->resizeHandleVisibility, SIGNAL(opacityChanged()),
            this, SLOT(slotUpdate()));

    d->hudTimer = new QTimer(this);
    d->hudTimer->setInterval(d->HUD_TIMER_ANIMATION_INTERVAL);

    connect(d->hudTimer, SIGNAL(timeout()),
            this, SLOT(moveHudWidget()));

    connect(this, SIGNAL(positionChanged()),
            this, SLOT(slotPosChanged()));

    connect(this, SIGNAL(sizeChanged()),
            this, SLOT(slotSizeChanged()));

    setFlags(GeometryEditable);

    d->updateHudWidgetPosition();
}

RegionFrameItem::~RegionFrameItem()
{
    if (d->hudWidget)
    {
        // See bug #359196: hide or close the QGraphicsWidget befor delete it. Possible Qt bug?
        d->hudWidget->hide();
        delete d->hudWidget;
    }

    delete d;
}

void RegionFrameItem::setHudWidget(QWidget* const widget, Qt::WindowFlags wFlags)
{
    QGraphicsProxyWidget* const proxy = new QGraphicsProxyWidget(0, wFlags);
    /*
     * This is utterly undocumented magic. If you add a normal widget directly,
     * with transparent parts (round corners), you will have ugly color in the corners.
     * If you set WA_TranslucentBackground on the widget directly, a lot of the
     * painting and stylesheets is broken. Like this, with an extra container, it seems to work.
     */
    QWidget* const container  = new QWidget;
    container->setAttribute(Qt::WA_TranslucentBackground);
    QHBoxLayout* const layout = new QHBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    layout->addWidget(widget);
    container->setLayout(layout);
    proxy->setWidget(container);
    // Reset fixed sizes wrongly copied by setWidget onto the QGraphicsWidget
    proxy->setMinimumSize(QSizeF());
    proxy->setMaximumSize(QSizeF());

    setHudWidget(proxy);
}

void RegionFrameItem::setHudWidget(QGraphicsWidget* const hudWidget)
{
    if (d->hudWidget == hudWidget)
    {
        return;
    }

    if (d->hudWidget)
    {
        d->hudWidget->hide();
        delete d->hudWidget;
    }

    d->hudWidget = hudWidget;

    if (d->hudWidget)
    {
        d->hudWidget->setParentItem(this);
        d->hudWidget->installEventFilter(this);
        d->updateHudWidgetPosition();
    }
}

QGraphicsWidget* RegionFrameItem::hudWidget() const
{
    return d->hudWidget;
}

void RegionFrameItem::setFlags(Flags flags)
{
    if (d->flags == flags)
    {
        return;
    }

    d->flags = flags;
    update();
    setAcceptHoverEvents(d->flags & GeometryEditable);
    d->resizeHandleVisibility->controller()->setShallBeShown(d->flags & ShowResizeHandles);

    // ensure cursor is reset
    CropHandle handle = d->handleAt(QCursor::pos());
    d->updateCursor(handle, false/* buttonDown*/);
}

void RegionFrameItem::changeFlags(Flags flags, bool addOrRemove)
{
    if (addOrRemove)
    {
        setFlags(d->flags | flags);
    }
    else
    {
        setFlags(d->flags & ~flags);
    }
}

void RegionFrameItem::setHudWidgetVisible(bool visible)
{
    if (d->hudWidget)
    {
        d->hudWidget->setVisible(visible);
    }
}

RegionFrameItem::Flags RegionFrameItem::flags() const
{
    return d->flags;
}

void RegionFrameItem::setFixedRatio(double ratio)
{
    d->fixedRatio = ratio;
}

void RegionFrameItem::slotSizeChanged()
{
    d->updateHudWidgetPosition();
}

void RegionFrameItem::slotPosChanged()
{
    d->updateHudWidgetPosition();
}

void RegionFrameItem::hudSizeChanged()
{
    d->updateHudWidgetPosition();
}

void RegionFrameItem::setViewportRect(const QRectF& rect)
{
    d->viewportRect = rect;
    d->updateHudWidgetPosition();
}

QRectF RegionFrameItem::boundingRect() const
{
    return DImgChildItem::boundingRect();//.adjusted(-1, -1, 1, 1);
}

void RegionFrameItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
/*
    QRect rect                      = d->viewportCropRect();
    QRect imageRect                 = imageView()->rect();
    static const QColor outerColor  = QColor::fromHsvF(0, 0, 0, 0.5);
    QRegion outerRegion             = QRegion(imageRect) - QRegion(rect);

    foreach(const QRect& outerRect, outerRegion.rects())
    {
        painter->fillRect(outerRect, outerColor);
    }
*/

    const QColor borderColor = QColor::fromHsvF(0, 0, 1.0, 0.66 + 0.34 * d->hoverAnimationOpacity);
    const QColor fillColor   = QColor::fromHsvF(0, 0, 0.75, 0.66);

    // will paint to the left and bottom of logical coordinates
    QRectF drawRect          = boundingRect();

    painter->setPen(borderColor);
    painter->drawRect(drawRect);

    if (d->resizeHandleVisibility->isVisible())
    {
        // Only draw handles when user is not resizing
        if (d->movingHandle == CH_None)
        {
            painter->setOpacity(d->resizeHandleVisibility->opacity());
            painter->setBrush(fillColor);

            foreach(const CropHandle& handle, d->cropHandleList)
            {
                QRectF rect = d->handleRect(handle);
                painter->drawRect(rect);
            }
        }
    }
}

void RegionFrameItem::slotUpdate()
{
    update();
}

void RegionFrameItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    if (boundingRect().contains(e->pos()))
    {
        d->resizeHandleVisibility->controller()->show();
    }
}

void RegionFrameItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    if (!boundingRect().contains(e->pos()))
    {
        d->resizeHandleVisibility->controller()->hide();
    }
}

void RegionFrameItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    if (boundingRect().contains(e->pos()))
    {
        if (d->flags & GeometryEditable)
        {
            CropHandle handle = d->handleAt(e->pos());
            d->updateCursor(handle, false/* buttonDown*/);
        }

        d->resizeHandleVisibility->controller()->show();
    }
}

void RegionFrameItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // FIXME: Fade out?
    //d->hudWidget->hide();
    if (!(d->flags & GeometryEditable))
    {
        return DImgChildItem::mousePressEvent(event);
    }

    d->movingHandle = d->handleAt(event->pos());
    d->updateCursor(d->movingHandle, event->buttons() != Qt::NoButton);

    if (d->movingHandle == CH_Content)
    {
        d->lastMouseMovePos = mapToParent(event->pos());
    }

    // Update to hide handles
    update();
}

void RegionFrameItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QSizeF maxSize = parentDImgItem()->boundingRect().size();
    const QPointF point  = mapToParent(event->pos());
    qreal posX           = qBound<qreal>(0., point.x(), maxSize.width());
    qreal posY           = qBound<qreal>(0., point.y(), maxSize.height());
    QRectF r             = rect();

    // Adjust edge
    if (d->movingHandle & CH_Top)
    {
        r.setTop(posY);
    }
    else if (d->movingHandle & CH_Bottom)
    {
        r.setBottom(posY);
    }

    if (d->movingHandle & CH_Left)
    {
        r.setLeft(posX);
    }
    else if (d->movingHandle & CH_Right)
    {
        r.setRight(posX);
    }

    // Normalize rect and handles (this is useful when user drag the right side
    // of the crop rect to the left of the left side)
    if (r.height() < 0)
    {
        d->movingHandle = d->movingHandle ^ (CH_Top | CH_Bottom);
    }

    if (r.width() < 0)
    {
        d->movingHandle = d->movingHandle ^ (CH_Left | CH_Right);
    }

    r = r.normalized();

    // Enforce ratio
    if (d->fixedRatio > 0.)
    {
        if (d->movingHandle == CH_Top || d->movingHandle == CH_Bottom)
        {
            // Top or bottom
            int width = int(r.height() / d->fixedRatio);
            r.setWidth(width);
        }
        else if (d->movingHandle == CH_Left || d->movingHandle == CH_Right)
        {
            // Left or right
            int height = int(r.width() * d->fixedRatio);
            r.setHeight(height);
        }
        else if (d->movingHandle & CH_Top)
        {
            // Top left or top right
            int height = int(r.width() * d->fixedRatio);
            r.setTop(r.bottom() - height);
        }
        else if (d->movingHandle & CH_Bottom)
        {
            // Bottom left or bottom right
            int height = int(r.width() * d->fixedRatio);
            r.setHeight(height);
        }
    }

    if (d->movingHandle == CH_Content)
    {
        QPointF delta       = point - d->lastMouseMovePos;
        r.adjust(delta.x(), delta.y(), delta.x(), delta.y());
        d->lastMouseMovePos = mapToParent(event->pos());
    }

    setRect(d->keepRectInsideImage(r));

    d->updateHudWidgetPosition();
}

void RegionFrameItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    // FIXME: Fade in?
    //d->hudWidget->show();
    d->movingHandle = CH_None;
    d->updateCursor(d->handleAt(event->pos()), false);

    // Update to show handles
    update();
}

bool RegionFrameItem::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d->hudWidget && event->type() == QEvent::GraphicsSceneResize)
    {
        d->updateHudWidgetPosition();
    }

    return DImgChildItem::eventFilter(watched, event);
}

void RegionFrameItem::moveHudWidget()
{
    const QPointF delta   = d->hudEndPos - d->hudWidget->pos();
    const double distance = sqrt(pow(delta.x(), 2) + pow(delta.y(), 2));
    QPointF pos;

    if (distance > double(d->HUD_TIMER_MAX_PIXELS_PER_UPDATE))
    {
        pos = d->hudWidget->pos() + delta * double(d->HUD_TIMER_MAX_PIXELS_PER_UPDATE) / distance;
    }
    else
    {
        pos = d->hudEndPos;
        d->hudTimer->stop();
    }

    d->hudWidget->setPos(pos);
}

void RegionFrameItem::setRectInSceneCoordinatesAdjusted(const QRectF& rect)
{
    setRectInSceneCoordinates(d->keepRectInsideImage(rect, false));
}

} // namespace Digikam
