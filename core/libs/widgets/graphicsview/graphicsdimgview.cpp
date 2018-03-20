/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View for DImg preview
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "graphicsdimgview.h"

// Qt includes

#include <QApplication>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QToolButton>
#include <QStyle>

// Local includes

#include "digikam_debug.h"
#include "dimgpreviewitem.h"
#include "imagezoomsettings.h"
#include "paniconwidget.h"
#include "previewlayout.h"
#include "dimgchilditem.h"

namespace Digikam
{

class GraphicsDImgView::Private
{
public:

    Private()
    {
        scene            = 0;
        item             = 0;
        layout           = 0;
        cornerButton     = 0;
        panIconPopup     = 0;
        movingInProgress = false;
        showText         = true;
    }

    QGraphicsScene*           scene;
    GraphicsDImgItem*         item;
    SinglePhotoPreviewLayout* layout;

    QToolButton*              cornerButton;
    PanIconFrame*             panIconPopup;

    QPoint                    mousePressPos;
    QPoint                    panningScrollPos;
    bool                      movingInProgress;
    bool                      showText;
};

GraphicsDImgView::GraphicsDImgView(QWidget* const parent)
    : QGraphicsView(parent),
      d(new Private)
{
    d->scene  = new QGraphicsScene(this);
    d->scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setScene(d->scene);
    d->layout = new SinglePhotoPreviewLayout(this);
    d->layout->setGraphicsView(this);

    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    horizontalScrollBar()->setSingleStep(1);
    horizontalScrollBar()->setPageStep(1);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setPageStep(1);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotContentsMoved()));

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotContentsMoved()));
}

GraphicsDImgView::~GraphicsDImgView()
{
    delete d;
}

void GraphicsDImgView::setItem(GraphicsDImgItem* const item)
{
    d->item = item;
    d->scene->addItem(d->item);
    d->layout->addItem(d->item);
}

GraphicsDImgItem* GraphicsDImgView::item() const
{
    return d->item;
}

DImgPreviewItem* GraphicsDImgView::previewItem() const
{
    return dynamic_cast<DImgPreviewItem*>(item());
}

SinglePhotoPreviewLayout* GraphicsDImgView::layout() const
{
    return d->layout;
}

void GraphicsDImgView::installPanIcon()
{
    d->cornerButton = PanIconWidget::button();
    setCornerWidget(d->cornerButton);

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));
}

void GraphicsDImgView::drawForeground(QPainter* p, const QRectF& rect)
{
    QGraphicsView::drawForeground(p, rect);

    if (!d->movingInProgress)
    {
        QString text = d->item->userLoadingHint();

        if (text.isNull() || !d->showText)
        {
            return;
        }

        QRect viewportRect        = viewport()->rect();
        QRect fontRect            = p->fontMetrics().boundingRect(viewportRect, 0, text);
        QPoint drawingPoint(viewportRect.topRight().x() - fontRect.width() - 10,
                            viewportRect.topRight().y() + 5);

        QPointF sceneDrawingPoint = mapToScene(drawingPoint);
        QRectF sceneDrawingRect(sceneDrawingPoint, fontRect.size());

        if (!rect.intersects(sceneDrawingRect))
        {
            return;
        }

        drawText(p, sceneDrawingRect, text);
    }
}

void GraphicsDImgView::drawText(QPainter* p, const QRectF& rect, const QString& text)
{
    p->save();

    p->setRenderHint(QPainter::Antialiasing, true);
    p->setBackgroundMode(Qt::TransparentMode);

    // increase width by 5 and height by 2
    QRectF textRect    = rect.adjusted(0, 0, 5, 2);

    // Draw background
    p->setPen(Qt::black);
    QColor semiTransBg = palette().color(QPalette::Window);
    semiTransBg.setAlpha(190);
    p->setBrush(semiTransBg);
    //p->translate(0.5, 0.5);
    p->drawRoundRect(textRect, 10.0, 10.0);

    // Draw shadow and text
    p->setPen(palette().color(QPalette::Window).dark(115));
    p->drawText(textRect.translated(3, 1), text);
    p->setPen(palette().color(QPalette::WindowText));
    p->drawText(textRect.translated(2, 0), text);

    p->restore();
}

void GraphicsDImgView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QGraphicsView::mouseDoubleClickEvent(e);
/*
    if (!acceptsMouseClick(e))
    {
        return;
    }
*/
    if (e->button() == Qt::LeftButton)
    {
        emit leftButtonDoubleClicked();

        if (!qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
        {
            emit activated();
        }
    }
}

void GraphicsDImgView::mousePressEvent(QMouseEvent* e)
{
    QGraphicsView::mousePressEvent(e);

    d->mousePressPos    = QPoint();
    d->movingInProgress = false;

    if (!acceptsMouseClick(e))
    {
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        emit leftButtonClicked();
    }

    if (e->button() == Qt::LeftButton || e->button() == Qt::MidButton)
    {
        d->mousePressPos = e->pos();

        if (!qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick) || e->button() == Qt::MidButton)
        {
            startPanning(e->pos());
        }

        return;
    }

    if (e->button() == Qt::RightButton)
    {
        emit rightButtonClicked();
    }
}

void GraphicsDImgView::mouseMoveEvent(QMouseEvent* e)
{
    QGraphicsView::mouseMoveEvent(e);

    if ((e->buttons() & Qt::LeftButton || e->buttons() & Qt::MidButton) && !d->mousePressPos.isNull())
    {
        if (!d->movingInProgress && e->buttons() & Qt::LeftButton)
        {
            if ((d->mousePressPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
            {
                startPanning(d->mousePressPos);
            }
        }

        if (d->movingInProgress)
        {
            continuePanning(e->pos());
        }
    }
}

void GraphicsDImgView::mouseReleaseEvent(QMouseEvent* e)
{
    QGraphicsView::mouseReleaseEvent(e);

    // Do not call acceptsMouseClick() here, only on press. Seems that release event are accepted per default.

    if ((e->button() == Qt::LeftButton || e->button() == Qt::MidButton) && !d->mousePressPos.isNull())
    {
        if (!d->movingInProgress && e->button() == Qt::LeftButton)
        {
            if (qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
            {
                emit activated();
            }
        }
        else
        {
            finishPanning();
        }
    }

    d->movingInProgress = false;
    d->mousePressPos    = QPoint();
}

bool GraphicsDImgView::acceptsMouseClick(QMouseEvent* e)
{
    // the basic condition is that now item ate the event
    if (e->isAccepted())
    {
        return false;
    }

    return true;
}

void GraphicsDImgView::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
    d->layout->updateZoomAndSize();
    emit resized();
    emit viewportRectChanged(mapToScene(viewport()->rect()).boundingRect());
}

void GraphicsDImgView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    emit viewportRectChanged(mapToScene(viewport()->rect()).boundingRect());
}

void GraphicsDImgView::startPanning(const QPoint& pos)
{
    if (horizontalScrollBar()->maximum() || verticalScrollBar()->maximum())
    {
        d->movingInProgress = true;
        d->mousePressPos    = pos;
        d->panningScrollPos = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
        viewport()->setCursor(Qt::SizeAllCursor);
    }
}

void GraphicsDImgView::continuePanning(const QPoint& pos)
{
    QPoint delta = pos - d->mousePressPos;
    horizontalScrollBar()->setValue(d->panningScrollPos.x() + (isRightToLeft() ? delta.x() : -delta.x()));
    verticalScrollBar()->setValue(d->panningScrollPos.y() - delta.y());
    emit contentsMoved(false);
    viewport()->update();
}

void GraphicsDImgView::finishPanning()
{
    emit contentsMoved(true);
    viewport()->unsetCursor();
}

void GraphicsDImgView::scrollPointOnPoint(const QPointF& scenePos, const QPoint& viewportPos)
{
    // This is inspired from QGraphicsView's centerOn()
    QPointF viewPoint = matrix().map(scenePos);

    if (horizontalScrollBar()->maximum())
    {
        if (isRightToLeft())
        {
            qint64 horizontal = 0;
            horizontal += horizontalScrollBar()->minimum();
            horizontal += horizontalScrollBar()->maximum();
            horizontal -= int(viewPoint.x() - viewportPos.x());
            horizontalScrollBar()->setValue(horizontal);
        }
        else
        {
            horizontalScrollBar()->setValue(int(viewPoint.x() - viewportPos.x()));
        }
    }

    if (verticalScrollBar()->maximum())
    {
        verticalScrollBar()->setValue(int(viewPoint.y() - viewportPos.y()));
    }

    viewport()->update();
}

void GraphicsDImgView::wheelEvent(QWheelEvent* e)
{
    if (e->modifiers() & Qt::ShiftModifier)
    {
        e->accept();

        if (e->delta() < 0)
        {
            emit toNextImage();
        }
        else if (e->delta() > 0)
        {
            emit toPreviousImage();
        }

        return;
    }
    else if (e->modifiers() & Qt::ControlModifier)
    {
        // When zooming with the mouse-wheel, the image center is kept fixed.
        if (e->delta() < 0)
        {
            d->layout->decreaseZoom(e->pos());
        }
        else if (e->delta() > 0)
        {
            d->layout->increaseZoom(e->pos());
        }

        return;
    }

    QGraphicsView::wheelEvent(e);
}

void GraphicsDImgView::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
    }

    d->panIconPopup          = new PanIconFrame(this);
    PanIconWidget* const pan = new PanIconWidget(d->panIconPopup);

    //connect(pan, SIGNAL(signalSelectionTakeFocus()),
    //      this, SIGNAL(signalContentTakeFocus()));

    connect(pan, SIGNAL(signalSelectionMoved(QRect,bool)),
            this, SLOT(slotPanIconSelectionMoved(QRect,bool)));

    connect(pan, SIGNAL(signalHidden()),
            this, SLOT(slotPanIconHidden()));

    pan->setImage(180, 120, item()->image());
    QRectF sceneRect(mapToScene(viewport()->rect().topLeft()), mapToScene(viewport()->rect().bottomRight()));
    pan->setRegionSelection(item()->zoomSettings()->sourceRect(sceneRect).toRect());
    pan->setMouseFocus();
    d->panIconPopup->setMainWidget(pan);
    //slotContentTakeFocus();

    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x()+ viewport()->size().width());
    g.setY(g.y()+ viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(),
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void GraphicsDImgView::slotPanIconHidden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void GraphicsDImgView::slotPanIconSelectionMoved(const QRect& imageRect, bool b)
{
    QRectF zoomRect = item()->zoomSettings()->mapImageToZoom(imageRect);
    qCDebug(DIGIKAM_WIDGETS_LOG) << imageRect << zoomRect;
    centerOn(item()->mapToScene(zoomRect.center()));

    if (b)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
        slotPanIconHidden();
        //slotContentLeaveFocus();
    }
}

void GraphicsDImgView::slotContentsMoved()
{
    emit contentsMoving(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

int GraphicsDImgView::contentsX() const
{
    return horizontalScrollBar()->value();
}

int GraphicsDImgView::contentsY() const
{
    return verticalScrollBar()->value();
}

void GraphicsDImgView::setContentsPos(int x, int y)
{
    horizontalScrollBar()->setValue(x);
    verticalScrollBar()->setValue(y);
}

void GraphicsDImgView::setShowText(bool val)
{
    d->showText = val;
}

QRect GraphicsDImgView::visibleArea() const
{
    return (mapToScene(viewport()->geometry()).boundingRect().toRect());
}

void GraphicsDImgView::fitToWindow()
{
    layout()->fitToWindow();
    update();
}

void GraphicsDImgView::toggleFullScreen(bool set)
{
    if (set)
    {
        d->scene->setBackgroundBrush(Qt::black);
        setFrameShape(QFrame::NoFrame);
    }
    else
    {
        d->scene->setBackgroundBrush(Qt::NoBrush);
        setFrameShape(QFrame::StyledPanel);
    }
}

} // namespace Digikam
