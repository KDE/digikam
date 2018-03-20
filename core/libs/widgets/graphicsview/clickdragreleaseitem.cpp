/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-04
 * Description : A simple item to click, drag and release
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

#include "clickdragreleaseitem.h"

// Qt includes

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QObject>
#include <QPointF>

// Local includes

#include "digikam_debug.h"
#include "itemvisibilitycontroller.h"

namespace Digikam
{

enum ClickDragState
{
    HoverState,
    PressedState,
    PressDragState,
    ClickedMoveState
};

class ClickDragReleaseItem::Private
{
public:

    Private()
        : state(HoverState)
    {
    }

    template <class Event> bool isDrag(Event* e)
    {
        return (pressPos - e->scenePos()).manhattanLength() > QApplication::startDragDistance();
    }

    template <class Event> QRectF rect(Event* e)
    {
        return QRectF(pressPos, e->scenePos()).normalized();
    }

public:

    ClickDragState state;
    QPointF        pressPos;
};

ClickDragReleaseItem::ClickDragReleaseItem(QGraphicsItem* const parent)
    : QGraphicsObject(parent),
      d(new Private)
{
    setCursor(Qt::CrossCursor);
    setFlags(ItemIsFocusable | ItemHasNoContents);
}

ClickDragReleaseItem::~ClickDragReleaseItem()
{
    delete d;
}

QRectF ClickDragReleaseItem::boundingRect() const
{
    if (parentItem())
    {
        return QRectF(QPointF(0,0), parentItem()->boundingRect().size());
    }

    return QRectF();
}

void ClickDragReleaseItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}

/**
 * 1) Press - Drag - Release:
 *    mousePress, PressedState -> mouseMoveEvent over threshold, PressDragState -> mouseReleaseEvent, finished
 * 2) Click - Move - Click:
 *    mousePressEvent, PressedState -> mouseReleaseEvent, ClickedMoveState ->
 *    hoverMoveEvent -> mouseReleaseEvent, finished
 */

void ClickDragReleaseItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        emit cancelled();
        return;
    }

    if (d->state == HoverState)
    {
        d->pressPos = e->scenePos();
        d->state = PressedState;
        emit started(e->scenePos());
    }
}

void ClickDragReleaseItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (d->state == PressedState && d->isDrag(e))
    {
        d->state = PressDragState;
        setCursor(Qt::SizeFDiagCursor);
    }

    if (d->state == PressDragState)
    {
        emit moving(d->rect(e));
    }
}

void ClickDragReleaseItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    if (d->state == ClickedMoveState)
    {
        emit moving(d->rect(e));
    }
}

void ClickDragReleaseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if (d->state == PressedState)
    {
        // click-move-click mode first click.
        // It cannot be over the drag threshold in this state, would be caught in moveEvent.
        d->state = ClickedMoveState;
        setCursor(Qt::SizeFDiagCursor);
        setAcceptHoverEvents(true);
    }
    else if (d->state == ClickedMoveState)
    {
        // click-move-click mode second click
        d->state = HoverState;
        setCursor(Qt::CrossCursor);
        setAcceptHoverEvents(false);
        emit finished(d->rect(e));
    }
    else if (d->state == PressDragState)
    {
        if (d->isDrag(e))
        {
            emit finished(d->rect(e));
        }
        else
        {
            emit cancelled();
        }

        d->state = HoverState;
        setCursor(Qt::CrossCursor);
        setAcceptHoverEvents(false);
    }
}

void ClickDragReleaseItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    e->ignore();
}

void ClickDragReleaseItem::keyPressEvent(QKeyEvent* e)
{
    qCDebug(DIGIKAM_WIDGETS_LOG) << e;

    switch (e->key())
    {
        case Qt::Key_Backspace:
        case Qt::Key_Escape:
            emit cancelled();
            break;
        default:
            e->ignore();
            break;
    }
}

} // namespace Digikam
