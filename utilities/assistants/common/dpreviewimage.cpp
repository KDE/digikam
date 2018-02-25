/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a widget to preview image effect.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Kare Sars <kare dot sars at iki dot fi>
 * Copyright (C) 2012      by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "dpreviewimage.h"

// Qt includes

#include <QAction>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QScrollBar>
#include <QToolBar>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "previewloadthread.h"

namespace Digikam
{

static const qreal   selMargin = 8.0;
static const QPointF boundMargin(selMargin, selMargin);

class DSelectionItem::Private
{
public:
    
    explicit Private()
    {
        selMargin   = 0.0;
        invZoom     = 1.0;
        maxX        = 0.0;
        maxY        = 0.0;
        showAnchors = true;
        hasMaxX     = false;
        hasMaxY     = false;
        hasMax      = false;
    }
    
    QPen        penDark;
    QPen        penLight;
    QPen        penAnchors;
    QRectF      rect;
    qreal       maxX;
    qreal       maxY;
    bool        hasMaxX;
    bool        hasMaxY;
    bool        hasMax;
    qreal       invZoom;
    qreal       selMargin;
    QRectF      anchorTopLeft;
    QRectF      anchorTopRight;
    QRectF      anchorBottomLeft;
    QRectF      anchorBottomRight;
    QLineF      anchorTop;
    QLineF      anchorBottom;
    QLineF      anchorLeft;
    QLineF      anchorRight;
    bool        showAnchors;
};

DSelectionItem::DSelectionItem(const QRectF& rect)
    : QGraphicsItem(),
      d(new Private)
{
    d->selMargin = selMargin;
    setRect(rect);

    d->penDark.setColor(Qt::black);
    d->penDark.setStyle(Qt::SolidLine);
    d->penLight.setColor(Qt::white);
    d->penLight.setStyle(Qt::DashLine);
    d->penAnchors.setColor(Qt::white);
    d->penAnchors.setStyle(Qt::SolidLine);
}

DSelectionItem::~DSelectionItem()
{
    delete d;
}

void DSelectionItem::saveZoom(qreal zoom)
{
    if (zoom < 0.00001)
    {
        zoom = 0.00001;
    }

    d->invZoom   = 1 / zoom;
    d->selMargin = selMargin * d->invZoom;

    updateAnchors();
}

void DSelectionItem::setMaxRight(qreal maxX)
{
    d->maxX    = maxX;
    d->hasMaxX = true;

    if (d->hasMaxY)
    {
        d->hasMax = true;
    }
}

void DSelectionItem::setMaxBottom(qreal maxY)
{
    d->maxY    = maxY;
    d->hasMaxY = true;

    if (d->hasMaxX)
    {
        d->hasMax = true;
    }
}

DSelectionItem::Intersects DSelectionItem::intersects(QPointF& point)
{

    if ((point.x() < (d->rect.left()   - d->selMargin)) ||
        (point.x() > (d->rect.right()  + d->selMargin)) ||
        (point.y() < (d->rect.top()    - d->selMargin)) ||
        (point.y() > (d->rect.bottom() + d->selMargin)))
    {
        d->showAnchors = false;
        update();
        return None;
    }

    d->showAnchors = true;
    update();

    if (point.x() < (d->rect.left() + d->selMargin))
    {
        if (point.y() < (d->rect.top()    + d->selMargin))
            return TopLeft;

        if (point.y() > (d->rect.bottom() - d->selMargin))
            return BottomLeft;

        return Left;
    }

    if (point.x() > (d->rect.right() - d->selMargin))
    {
        if (point.y() < (d->rect.top()    + d->selMargin))
            return TopRight;

        if (point.y() > (d->rect.bottom() - d->selMargin))
            return BottomRight;

        return Right;
    }

    if (point.y() < (d->rect.top() + d->selMargin))
    {
        return Top;
    }

    if (point.y() > (d->rect.bottom()-d->selMargin))
    {
        return Bottom;
    }

    return Move;
}

void DSelectionItem::setRect(const QRectF& rect)
{
    prepareGeometryChange();
    d->rect = rect;
    d->rect = d->rect.normalized();

    if (d->hasMax)
    {
        if (d->rect.top() < 0)
        {
            d->rect.setTop(0);
        }

        if (d->rect.left() < 0)
        {
            d->rect.setLeft(0);
        }

        if (d->rect.right() > d->maxX)
        {
            d->rect.setRight(d->maxX);
        }

        if (d->rect.bottom() > d->maxY)
        {
            d->rect.setBottom(d->maxY);
        }
    }

    updateAnchors();
}

QPointF DSelectionItem::fixTranslation(QPointF dp) const
{
    if ((d->rect.left()   + dp.x()) < 0)
    {
        dp.setX(-d->rect.left());
    }

    if ((d->rect.top()    + dp.y()) < 0)
    {
        dp.setY(-d->rect.top());
    }

    if ((d->rect.right()  + dp.x()) > d->maxX)
    {
        dp.setX(d->maxX - d->rect.right());
    }

    if ((d->rect.bottom() + dp.y()) > d->maxY)
    {
        dp.setY(d->maxY - d->rect.bottom());
    }

    return dp;
}

QRectF DSelectionItem::rect() const
{
    return d->rect;
}

QRectF DSelectionItem::boundingRect() const
{
    return QRectF(d->rect.topLeft() - boundMargin, d->rect.bottomRight() + boundMargin);
}

void DSelectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(d->penDark);
    painter->drawRect(d->rect);

    painter->setPen(d->penLight);
    painter->drawRect(d->rect);

    if (d->showAnchors)
    {
        painter->setPen(d->penAnchors);
        painter->setOpacity(0.4);

        if (!d->anchorTop.isNull())
        {
            painter->drawLine(d->anchorTop);
        }

        if (!d->anchorBottom.isNull())
        {
            painter->drawLine(d->anchorBottom);
        }

        if (!d->anchorLeft.isNull())
        {
            painter->drawLine(d->anchorLeft);
        }

        if (!d->anchorRight.isNull())
        {
            painter->drawLine(d->anchorRight);
        }

        painter->setOpacity(0.4);

        if (!d->anchorTopLeft.isNull())
        {
            painter->fillRect(d->anchorTopLeft, Qt::white);
        }

        if (!d->anchorTopRight.isNull())
        {
            painter->fillRect(d->anchorTopRight, Qt::white);
        }

        if (!d->anchorBottomLeft.isNull())
        {
            painter->fillRect(d->anchorBottomLeft, Qt::white);
        }

        if (!d->anchorBottomRight.isNull())
        {
            painter->fillRect(d->anchorBottomRight, Qt::white);
        }
    }
}

void DSelectionItem::updateAnchors()
{
    QPointF moveDown(0.0, d->selMargin);
    QPointF moveRight(d->selMargin, 0.0);
    bool verticalCondition   = (d->rect.height() - 3 * d->selMargin) > 0;
    bool horizontalCondition = (d->rect.width()  - 3 * d->selMargin) > 0;

    if (verticalCondition)
    {
        if (horizontalCondition)
        {
            d->anchorTop    = QLineF(d->rect.topLeft() + moveDown,
                                     d->rect.topRight() + moveDown);
            d->anchorBottom = QLineF(d->rect.bottomLeft() - moveDown,
                                     d->rect.bottomRight() - moveDown);
            d->anchorLeft   = QLineF(d->rect.topLeft() + moveRight,
                                     d->rect.bottomLeft() + moveRight);
            d->anchorRight  = QLineF(d->rect.topRight() - moveRight,
                                     d->rect.bottomRight() - moveRight);

            d->anchorTopLeft        = QRectF(d->rect.topLeft(),
                                             d->rect.topLeft() + moveDown + moveRight);
            d->anchorTopRight       = QRectF(d->rect.topRight() - moveRight,
                                             d->rect.topRight() + moveDown);
            d->anchorBottomLeft     = QRectF(d->rect.bottomLeft() - moveDown,
                                             d->rect.bottomLeft() + moveRight);
            d->anchorBottomRight    = QRectF(d->rect.bottomRight() - moveDown - moveRight,
                                             d->rect.bottomRight());
        }
        else
        {
            // Only the top & bottom lines & middle line plus two corners are drawn
            d->anchorTop    = QLineF(d->rect.topLeft() + moveDown,
                                     d->rect.topRight() + moveDown);
            d->anchorBottom = QLineF(d->rect.bottomLeft() - moveDown,
                                     d->rect.bottomRight() - moveDown);
            d->anchorLeft   = QLineF(d->rect.topLeft() + QPointF(d->rect.width() / 2.0, 0.0),
                                     d->rect.bottomLeft() + QPointF(d->rect.width() / 2.0, 0.0));
            d->anchorRight  = QLineF();

            d->anchorTopLeft        = QRectF(d->rect.topLeft(),
                                             d->rect.topRight() + moveDown);
            d->anchorTopRight       = QRectF();
            d->anchorBottomLeft     = QRectF(d->rect.bottomLeft() - moveDown,
                                             d->rect.bottomRight());
            d->anchorBottomRight    = QRectF();
        }
    }
    else
    {
        if (horizontalCondition)
        {
            // Only the left & right lines & middle line plus two corners are drawn
            d->anchorTop    = QLineF(d->rect.topLeft() + QPointF(0.0, d->rect.height() / 2.0),
                                     d->rect.topRight() + QPointF(0.0, d->rect.height() / 2.0));
            d->anchorBottom = QLineF();
            d->anchorLeft   = QLineF(d->rect.topLeft() + moveRight,
                                     d->rect.bottomLeft() + moveRight);
            d->anchorRight  = QLineF(d->rect.topRight() - moveRight,
                                     d->rect.bottomRight() - moveRight);

            d->anchorTopLeft        = QRectF(d->rect.topLeft(),
                                             d->rect.bottomLeft() + moveRight);
            d->anchorTopRight       = QRectF(d->rect.topRight() - moveRight,
                                             d->rect.bottomRight());
            d->anchorBottomLeft     = QRectF();
            d->anchorBottomRight    = QRectF();
        }
        else
        {
            d->anchorTop    = QLineF();
            d->anchorBottom = QLineF();
            d->anchorLeft   = QLineF();
            d->anchorRight  = QLineF();

            d->anchorTopLeft        = QRectF();
            d->anchorTopRight       = QRectF();
            d->anchorBottomLeft     = QRectF();
            d->anchorBottomRight    = QRectF();
        }
    }
}

// -------------------------------------------------------------------------------------------------

class DPreviewImage::Private
{
public:

    enum
    {
        NONE,
        LOOKAROUND,
        DRAWSELECTION,
        EXPANDORSHRINK,
        MOVESELECTION
    }
    mouseDragAction;

public:

    explicit Private()
        : mouseDragAction(NONE),
          lastdx(0),
          lastdy(0),
          scene(0),
          pixmapItem(0),
          selection(0),
          enableSelection(false),
          mouseZone(DSelectionItem::None),
          zoomInAction(0),
          zoomOutAction(0),
          zoom2FitAction(0),
          toolBar(0),
          highLightLeft(0),
          highLightRight(0),
          highLightTop(0),
          highLightBottom(0),
          highLightArea(0)
    {
    }

    int                        lastdx;
    int                        lastdy;

    QGraphicsScene*            scene;
    QGraphicsPixmapItem*       pixmapItem;
    DSelectionItem*            selection;
    bool                       enableSelection;
    DSelectionItem::Intersects mouseZone;
    QPointF                    lastMousePoint;

    QAction*                   zoomInAction;
    QAction*                   zoomOutAction;
    QAction*                   zoom2FitAction;

    QToolBar*                  toolBar;

    QGraphicsRectItem*         highLightLeft;
    QGraphicsRectItem*         highLightRight;
    QGraphicsRectItem*         highLightTop;
    QGraphicsRectItem*         highLightBottom;
    QGraphicsRectItem*         highLightArea;
};

DPreviewImage::DPreviewImage(QWidget* const parent)
    : QGraphicsView(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setCacheMode(QGraphicsView::CacheBackground);

    d->scene           = new QGraphicsScene;
    d->pixmapItem      = new QGraphicsPixmapItem;

    d->selection       = new DSelectionItem(QRectF());
    d->selection->setZValue(10);
    d->selection->setVisible(false);
    d->enableSelection = false;

    d->scene->addItem(d->pixmapItem);
    setScene(d->scene);

    d->highLightTop    = new QGraphicsRectItem;
    d->highLightBottom = new QGraphicsRectItem;
    d->highLightRight  = new QGraphicsRectItem;
    d->highLightLeft   = new QGraphicsRectItem;
    d->highLightArea   = new QGraphicsRectItem;

    d->highLightTop->setOpacity(0.4);
    d->highLightBottom->setOpacity(0.4);
    d->highLightRight->setOpacity(0.4);
    d->highLightLeft->setOpacity(0.4);
    d->highLightArea->setOpacity(0.6);

    d->highLightTop->setPen(Qt::NoPen);
    d->highLightBottom->setPen(Qt::NoPen);
    d->highLightRight->setPen(Qt::NoPen);
    d->highLightLeft->setPen(Qt::NoPen);
    d->highLightArea->setPen(Qt::NoPen);

    d->highLightTop->setBrush(QBrush(Qt::black));
    d->highLightBottom->setBrush(QBrush(Qt::black));
    d->highLightRight->setBrush(QBrush(Qt::black));
    d->highLightLeft->setBrush(QBrush(Qt::black));

    d->scene->addItem(d->selection);
    d->scene->addItem(d->highLightTop);
    d->scene->addItem(d->highLightBottom);
    d->scene->addItem(d->highLightRight);
    d->scene->addItem(d->highLightLeft);
    d->scene->addItem(d->highLightArea);

    d->mouseZone = DSelectionItem::None;

    // create context menu

    d->zoomInAction = new QAction(QIcon::fromTheme(QString::fromLatin1("zoom-in")), i18n("Zoom In"), this);
    d->zoomInAction->setToolTip(i18n("Zoom In"));
    d->zoomInAction->setShortcut(Qt::Key_Plus);

    connect(d->zoomInAction, &QAction::triggered,
            this, &DPreviewImage::slotZoomIn);

    d->zoomOutAction = new QAction(QIcon::fromTheme(QString::fromLatin1("zoom-out")), i18n("Zoom Out"), this);
    d->zoomOutAction->setToolTip(i18n("Zoom Out"));
    d->zoomOutAction->setShortcut(Qt::Key_Minus);

    connect(d->zoomOutAction, &QAction::triggered,
            this, &DPreviewImage::slotZoomOut);

    d->zoom2FitAction = new QAction(QIcon::fromTheme(QString::fromLatin1("zoom-fit-best")), i18n("Zoom to Fit"), this);
    d->zoom2FitAction->setToolTip(i18n("Zoom to Fit"));
    d->zoom2FitAction->setShortcut(Qt::Key_Asterisk);

    connect(d->zoom2FitAction, &QAction::triggered,
            this, &DPreviewImage::slotZoom2Fit);

    addAction(d->zoomInAction);
    addAction(d->zoomOutAction);
    addAction(d->zoom2FitAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    // Create ToolBar

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->zoomInAction);
    d->toolBar->addAction(d->zoomOutAction);
    d->toolBar->addAction(d->zoom2FitAction);
    d->toolBar->hide();
    d->toolBar->installEventFilter(this);

    horizontalScrollBar()->installEventFilter(this);
    verticalScrollBar()->installEventFilter(this);
}

DPreviewImage::~DPreviewImage()
{
    delete d;
}

bool DPreviewImage::setImage(const QImage& img) const
{
    if (!img.isNull())
    {
        d->pixmapItem->setPixmap(QPixmap::fromImage(img));
        d->pixmapItem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
        d->scene->setSceneRect(0, 0, img.width(), img.height());
        return true;
    }

    return false;
}

void DPreviewImage::enableSelectionArea(bool b)
{
    d->enableSelection = b;
}

QRectF DPreviewImage::getSelectionArea() const
{
    return d->selection->rect();
}

void DPreviewImage::setSelectionArea(const QRectF& rectangle)
{
    d->selection->setRect(rectangle);

    if (!d->selection->isVisible())
        d->selection->setVisible(true);
}

bool DPreviewImage::load(const QUrl& file) const
{
    DImg dimg = PreviewLoadThread::loadHighQualitySynchronously(file.toLocalFile());
    bool ret  = setImage(dimg.copyQImage());

    if (ret && d->enableSelection)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << d->scene->height() << " " << d->scene->width();
        d->selection->setMaxRight(d->scene->width());
        d->selection->setMaxBottom(d->scene->height());
        d->selection->setRect(d->scene->sceneRect());
    }

    return ret;
}

void DPreviewImage::slotZoomIn()
{
    scale(1.5, 1.5);
    d->selection->saveZoom(transform().m11());
    d->zoom2FitAction->setDisabled(false);
}

void DPreviewImage::slotZoomOut()
{
    scale(1.0 / 1.5, 1.0 / 1.5);
    d->selection->saveZoom(transform().m11());
    d->zoom2FitAction->setDisabled(false);
}

void DPreviewImage::slotZoom2Fit()
{
    fitInView(d->pixmapItem->boundingRect(), Qt::KeepAspectRatio);
    d->selection->saveZoom(transform().m11());
    d->zoom2FitAction->setDisabled(true);
}

void DPreviewImage::slotSetTLX(float ratio)
{
    if (!d->selection->isVisible())
        return; // only correct the selection if it is visible

    QRectF rect = d->selection->rect();
    rect.setLeft(ratio * d->scene->width());
    d->selection->setRect(rect);
    updateSelVisibility();
}

void DPreviewImage::slotSetTLY(float ratio)
{
    if (!d->selection->isVisible())
        return; // only correct the selection if it is visible

    QRectF rect = d->selection->rect();
    rect.setTop(ratio * d->scene->height());
    d->selection->setRect(rect);
    updateSelVisibility();
}

void DPreviewImage::slotSetBRX(float ratio)
{
    if (!d->selection->isVisible())
        return; // only correct the selection if it is visible

    QRectF rect = d->selection->rect();
    rect.setRight(ratio * d->scene->width());
    d->selection->setRect(rect);
    updateSelVisibility();
}

void DPreviewImage::slotSetBRY(float ratio)
{
    if (!d->selection->isVisible())
        return; // only correct the selection if it is visible

    QRectF rect = d->selection->rect();
    rect.setBottom(ratio * d->scene->height());
    d->selection->setRect(rect);
    updateSelVisibility();
}

void DPreviewImage::slotSetSelection(float tl_x, float tl_y, float br_x, float br_y)
{
    QRectF rect;
    rect.setCoords(tl_x * d->scene->width(),
                   tl_y * d->scene->height(),
                   br_x * d->scene->width(),
                   br_y * d->scene->height());

    d->selection->setRect(rect);
    updateSelVisibility();
}

void DPreviewImage::slotClearActiveSelection()
{
    d->selection->setRect(QRectF(0, 0, 0, 0));
    d->selection->setVisible(false);
}

void DPreviewImage::slotSetHighlightArea(float tl_x, float tl_y, float br_x, float br_y)
{
    QRectF rect;

    // Left  reason for rect: setCoords(x1,y1,x2,y2) != setRect(x1,x2, width, height)
    rect.setCoords(0,
                   0,
                   tl_x * d->scene->width(),
                   d->scene->height());
    d->highLightLeft->setRect(rect);

    // Right
    rect.setCoords(br_x * d->scene->width(),
                   0,
                   d->scene->width(),
                   d->scene->height());
    d->highLightRight->setRect(rect);

    // Top
    rect.setCoords(tl_x * d->scene->width(),
                   0,
                   br_x * d->scene->width(),
                   tl_y * d->scene->height());
    d->highLightTop->setRect(rect);

    // Bottom
    rect.setCoords(tl_x * d->scene->width(),
                   br_y * d->scene->height(),
                   br_x * d->scene->width(),
                   d->scene->height());
    d->highLightBottom->setRect(rect);

    // Area
    rect.setCoords(tl_x * d->scene->width(),
                   tl_y* d->scene->height(),
                   br_x * d->scene->width(),
                   br_y* d->scene->height());

    d->highLightArea->setRect(rect);

    d->highLightLeft->show();
    d->highLightRight->show();
    d->highLightTop->show();
    d->highLightBottom->show();
    // the highlight area is hidden until setHighlightShown is called.
    d->highLightArea->hide();
}

void DPreviewImage::slotSetHighlightShown(int percentage, QColor highLightColor)
{
    if (percentage >= 100)
    {
        d->highLightArea->hide();
        return;
    }

    d->highLightArea->setBrush(highLightColor);

    qreal diff  = d->highLightBottom->rect().top() - d->highLightTop->rect().bottom();
    diff       -= (diff * percentage) / 100;

    QRectF rect = d->highLightArea->rect();
    rect.setTop(d->highLightBottom->rect().top() - diff);

    d->highLightArea->setRect(rect);

    d->highLightArea->show();
}

void DPreviewImage::slotClearHighlight()
{
    d->highLightLeft->hide();
    d->highLightRight->hide();
    d->highLightTop->hide();
    d->highLightBottom->hide();
    d->highLightArea->hide();
}

void DPreviewImage::wheelEvent(QWheelEvent* e)
{
    if (e->modifiers() == Qt::ControlModifier)
    {
        if (e->delta() > 0)
        {
            slotZoomIn();
        }
        else
        {
            slotZoomOut();
        }
    }
    else
    {
        QGraphicsView::wheelEvent(e);
    }
}

void DPreviewImage::mousePressEvent(QMouseEvent* e)
{
    if (e->button() & Qt::LeftButton)
    {
        d->lastdx          = e->x();
        d->lastdy          = e->y();
        QPointF scenePoint = mapToScene(e->pos());
        d->lastMousePoint  = scenePoint;

        if (e->modifiers() != Qt::ControlModifier && d->enableSelection)
        {
            if (!d->selection->isVisible() || !d->selection->contains(scenePoint))
            {
                // Beginning of a selection area change
                d->mouseDragAction = Private::DRAWSELECTION;
                d->selection->setVisible(true);
                d->selection->setRect(QRectF(scenePoint, QSizeF(0,0)));
                d->mouseZone = DSelectionItem::BottomRight;
            }
            else if (d->selection->isVisible() &&
                     d->mouseZone != DSelectionItem::None &&
                     d->mouseZone != DSelectionItem::Move)
            {
                // Modification of the selection area
                d->mouseDragAction = Private::EXPANDORSHRINK;
            }
            else
            {
                // Selection movement called by QGraphicsView
                d->mouseDragAction = Private::MOVESELECTION;
            }
            updateHighlight();
        }
        else
        {
            // Beginning of moving around the picture
            d->mouseDragAction = Private::LOOKAROUND;
            setCursor(Qt::ClosedHandCursor);
        }
    }

    QGraphicsView::mousePressEvent(e);
}

void DPreviewImage::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() & Qt::LeftButton)
    {
        if (d->mouseDragAction == Private::DRAWSELECTION)
        {
            // Stop and setup the selection area
            // Only one case: small rectangle that we drop
            if ((d->selection->rect().width()  < 0.001) ||
                (d->selection->rect().height() < 0.001))
            {
                slotClearActiveSelection();
            }
        }

        if (!d->selection->isVisible() || !d->selection->contains(e->pos()))
        {
            setCursor(Qt::CrossCursor);
        }
    }

    d->mouseDragAction = Private::NONE;
    updateHighlight();

    QGraphicsView::mouseReleaseEvent(e);
}

void DPreviewImage::mouseMoveEvent(QMouseEvent* e)
{
    QPointF scenePoint = mapToScene(e->pos());

    if (e->buttons() & Qt::LeftButton)
    {
        if (d->mouseDragAction == Private::LOOKAROUND)
        {
            int dx    = e->x() - d->lastdx;
            int dy    = e->y() - d->lastdy;
            verticalScrollBar()->setValue(verticalScrollBar()->value() - dy);
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - dx);
            d->lastdx = e->x();
            d->lastdy = e->y();
        }
        else if (d->mouseDragAction == Private::DRAWSELECTION  ||
                 d->mouseDragAction == Private::EXPANDORSHRINK ||
                 d->mouseDragAction == Private::MOVESELECTION)
        {
            ensureVisible(QRectF(scenePoint, QSizeF(0,0)), 1, 1);
            QRectF rect = d->selection->rect();

            switch (d->mouseZone)
            {
                case DSelectionItem::None:
                    // should not be here :)
                    break;

                case DSelectionItem::Top:
                    if (scenePoint.y() < rect.bottom())
                    {
                        rect.setTop(scenePoint.y());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::Bottom;
                        rect.setTop(rect.bottom());
                    }
                    break;

                case DSelectionItem::TopRight:
                    if (scenePoint.x() > rect.left())
                    {
                        rect.setRight(scenePoint.x());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::TopLeft;
                        setCursor(Qt::SizeFDiagCursor);
                        rect.setRight(rect.left());
                    }

                    if (scenePoint.y() < rect.bottom())
                    {
                        rect.setTop(scenePoint.y());
                    }
                    else
                    {
                        if (d->mouseZone != DSelectionItem::TopLeft)
                        {
                            d->mouseZone = DSelectionItem::BottomRight;
                            setCursor(Qt::SizeFDiagCursor);
                        }
                        else
                        {
                            d->mouseZone = DSelectionItem::BottomLeft;
                            setCursor(Qt::SizeBDiagCursor);
                        }

                        rect.setTop(rect.bottom());
                    }
                    break;

                case DSelectionItem::Right:
                    if (scenePoint.x() > rect.left())
                    {
                        rect.setRight(scenePoint.x());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::Left;
                        rect.setRight(rect.left());
                    }
                    break;

                case DSelectionItem::BottomRight:
                    if (scenePoint.x() > rect.left())
                    {
                        rect.setRight(scenePoint.x());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::BottomLeft;
                        setCursor(Qt::SizeBDiagCursor);
                        rect.setRight(rect.left());
                    }

                    if (scenePoint.y() > rect.top())
                    {
                        rect.setBottom(scenePoint.y());
                    }
                    else
                    {
                        if (d->mouseZone != DSelectionItem::BottomLeft)
                        {
                            d->mouseZone = DSelectionItem::TopRight;
                            setCursor(Qt::SizeBDiagCursor);
                        }
                        else
                        {
                            d->mouseZone = DSelectionItem::TopLeft;
                            setCursor(Qt::SizeFDiagCursor);
                        }

                        rect.setBottom(rect.top());
                    }
                    break;

                case DSelectionItem::Bottom:
                    if (scenePoint.y() > rect.top())
                    {
                        rect.setBottom(scenePoint.y());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::Top;
                        rect.setBottom(rect.top());
                    }
                    break;

                case DSelectionItem::BottomLeft:
                    if (scenePoint.x() < rect.right())
                    {
                        rect.setLeft(scenePoint.x());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::BottomRight;
                        setCursor(Qt::SizeFDiagCursor);
                        rect.setLeft(rect.right());
                    }

                    if (scenePoint.y() > rect.top())
                    {
                        rect.setBottom(scenePoint.y());
                    }
                    else
                    {
                        if (d->mouseZone != DSelectionItem::BottomRight)
                        {
                            d->mouseZone = DSelectionItem::TopLeft;
                            setCursor(Qt::SizeFDiagCursor);
                        }
                        else
                        {
                            d->mouseZone = DSelectionItem::TopRight;
                            setCursor(Qt::SizeBDiagCursor);
                        }

                        rect.setBottom(rect.top());
                    }
                    break;

                case DSelectionItem::Left:
                    if (scenePoint.x() < rect.right())
                    {
                        rect.setLeft(scenePoint.x());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::Right;
                        rect.setLeft(rect.right());
                    }
                    break;

                case DSelectionItem::TopLeft:
                    if (scenePoint.x() < rect.right())
                    {
                        rect.setLeft(scenePoint.x());
                    }
                    else
                    {
                        d->mouseZone = DSelectionItem::TopRight;
                        setCursor(Qt::SizeBDiagCursor);
                        rect.setLeft(rect.right());
                    }

                    if (scenePoint.y() < rect.bottom())
                    {
                        rect.setTop(scenePoint.y());
                    }
                    else
                    {
                        if (d->mouseZone != DSelectionItem::TopRight)
                        {
                            d->mouseZone = DSelectionItem::BottomLeft;
                            setCursor(Qt::SizeBDiagCursor);
                        }
                        else
                        {
                            d->mouseZone = DSelectionItem::BottomRight;
                            setCursor(Qt::SizeFDiagCursor);
                        }

                        rect.setTop(rect.bottom());
                    }
                    break;

                case DSelectionItem::Move:
                    rect.translate(d->selection->fixTranslation(scenePoint - d->lastMousePoint));
                    break;
            }

            d->selection->setRect(rect);
        }
    }
    else if (d->selection->isVisible())
    {
        d->mouseZone = d->selection->intersects(scenePoint);

        switch (d->mouseZone)
        {
            case DSelectionItem::None:
                setCursor(Qt::CrossCursor);
                break;
            case DSelectionItem::Top:
                setCursor(Qt::SizeVerCursor);
                break;
            case DSelectionItem::TopRight:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case DSelectionItem::Right:
                setCursor(Qt::SizeHorCursor);
                break;
            case DSelectionItem::BottomRight:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case DSelectionItem::Bottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case DSelectionItem::BottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case DSelectionItem::Left:
                setCursor(Qt::SizeHorCursor);
                break;
            case DSelectionItem::TopLeft:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case DSelectionItem::Move:
                setCursor(Qt::SizeAllCursor);
                break;
        }
    }
    else
    {
        setCursor(Qt::CrossCursor);
    }

    d->lastMousePoint = scenePoint;
    updateHighlight();
    QGraphicsView::mouseMoveEvent(e);
}

void DPreviewImage::enterEvent(QEvent*)
{
    d->toolBar->show();
}

void DPreviewImage::leaveEvent(QEvent*)
{
    d->toolBar->hide();
}

bool DPreviewImage::eventFilter(QObject* obj, QEvent* ev)
{
    if ( obj == d->toolBar )
    {
        if ( ev->type() == QEvent::Enter)
            setCursor(Qt::ArrowCursor);
        else if ( ev->type() == QEvent::Leave)
            unsetCursor();

        return false;
    }
    else if ( obj == verticalScrollBar() && verticalScrollBar()->isVisible())
    {
        if ( ev->type() == QEvent::Enter)
            setCursor(Qt::ArrowCursor);
        else if ( ev->type() == QEvent::Leave)
            unsetCursor();

        return false;
    }
    else if ( obj == horizontalScrollBar() && horizontalScrollBar()->isVisible())
    {
        if ( ev->type() == QEvent::Enter)
            setCursor(Qt::ArrowCursor);
        else if ( ev->type() == QEvent::Leave)
            unsetCursor();

        return false;
    }

    return QGraphicsView::eventFilter(obj, ev);
}

void DPreviewImage::resizeEvent(QResizeEvent* e)
{
    if (!d->zoom2FitAction->isEnabled())
    {
        // Fit the image to the new size...
        fitInView(d->pixmapItem->boundingRect(), Qt::KeepAspectRatio);
        d->selection->saveZoom(transform().m11());
    }

    QGraphicsView::resizeEvent(e);
}

void DPreviewImage::updateSelVisibility()
{
    if ((d->selection->rect().width()  > 0.001) &&
        (d->selection->rect().height() > 0.001) &&
        ((d->scene->width() - d->selection->rect().width()  > 0.1) ||
        (d->scene->height() - d->selection->rect().height() > 0.1)))
    {
        d->selection->setVisible(true);
    }
    else
    {
        d->selection->setVisible(false);
    }

    updateHighlight();
}

void DPreviewImage::updateHighlight()
{
    if (d->selection->isVisible())
    {
        QRectF rect;
        // Left
        rect.setCoords(0,
                       0,
                       d->selection->rect().left(),
                       d->scene->height());
        d->highLightLeft->setRect(rect);

        // Right
        rect.setCoords(d->selection->rect().right(),
                       0,
                       d->scene->width(),
                       d->scene->height());
        d->highLightRight->setRect(rect);

        // Top
        rect.setCoords(d->selection->rect().left(),
                       0,
                       d->selection->rect().right(),
                       d->selection->rect().top());
        d->highLightTop->setRect(rect);

        // Bottom
        rect.setCoords(d->selection->rect().left(),
                       d->selection->rect().bottom(),
                       d->selection->rect().right(),
                       d->scene->height());
        d->highLightBottom->setRect(rect);

        d->highLightLeft->show();
        d->highLightRight->show();
        d->highLightTop->show();
        d->highLightBottom->show();
        d->highLightArea->hide();
    }
    else
    {
        d->highLightLeft->hide();
        d->highLightRight->hide();
        d->highLightTop->hide();
        d->highLightBottom->hide();
        d->highLightArea->hide();
    }
}

} // namespace Digikam
