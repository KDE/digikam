/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "RotationWidgetItem.h"

#include <limits>

#include <QUndoCommand>

#include <klocalizedstring.h>

#include "photolayoutswindow.h"
#include "global.h"

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::RotateItemCommand : public QUndoCommand
{
    AbstractPhoto* item;
    QPointF        rotationPoint;
    qreal          angle;
    bool           done;

public:

    RotateItemCommand(AbstractPhoto * item, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Rotate item"), parent),
        item(item),
        angle(0),
        done(false)
    {
    }

    virtual void redo()
    {
        if (done)
            return;
        QTransform tr;
        tr.translate(rotationPoint.x(), rotationPoint.y());
        tr.rotate(angle);
        tr.translate(-rotationPoint.x(), -rotationPoint.y());
        QRectF updateRect = item->mapRectToScene(item->boundingRect());
        QTransform rotated = item->transform() * tr;
        item->setTransform(rotated);
        updateRect = updateRect.united( item->mapRectToScene(item->boundingRect()) );
        if (item->scene())
            item->scene()->invalidate(updateRect);
        done = true;
    }

    virtual void undo()
    {
        if (!done)
            return;
        QTransform tr;
        tr.translate(rotationPoint.x(), rotationPoint.y());
        tr.rotate(angle);
        tr.translate(-rotationPoint.x(), -rotationPoint.y());
        QRectF updateRect = item->mapRectToScene(item->boundingRect());
        QTransform rotated = item->transform() * tr.inverted();
        item->setTransform(rotated);
        updateRect = updateRect.united( item->mapRectToScene(item->boundingRect()) );
        if (item->scene())
            item->scene()->invalidate(updateRect);
        done = false;
    }

    void setRotationPoint(const QPointF & point)
    {
        rotationPoint = point;
    }

    void setAngle(qreal angle)
    {
        this->angle = angle;
    }

    void setDone(bool done)
    {
        this->done = done;
    }
};

class PhotoLayoutsEditor::RotationWidgetItemPrivate
{
    RotationWidgetItemPrivate(RotationWidgetItem * item) :
        item(item),
        rotation_angle(0.0),
        elipse_pressed(false)
    {
        m_elipse.addEllipse(-10,-10,20,20);
    }

    QPointF viewportToItemPosition(const QPoint & pos, QWidget * widget)
    {
        QGraphicsView *view = 0;

        if (widget)
            view = qobject_cast<QGraphicsView *>(widget->parentWidget());

        if (view)
        {
            QTransform deviceTransform = item->deviceTransform(view->viewportTransform()).inverted();
            return deviceTransform.map(QPointF(view->mapFromGlobal(pos)));
        }

        return pos;
    }

    QRectF itemToViewportRect(const QRectF & rect, QWidget * widget)
    {
        QGraphicsView *view = 0;

        if (widget)
            view = qobject_cast<QGraphicsView *>(widget->parentWidget());

        if (view)
            return item->mapRectToScene(view->transform().inverted().mapRect(rect));

        return rect;
    }

    void transformDrawings(const QTransform & viewTransform)
    {
        if (currentViewTransform == viewTransform)
            return;

        currentViewTransform = viewTransform;

        this->calculateDrawings();
    }

    void calculateDrawings()
    {
        m_elipse = QPainterPath();
        m_elipse.addEllipse(handler_pos,
                            -20 / currentViewTransform.m11(),
                            -20 / currentViewTransform.m22());
    }

    RotationWidgetItem*                     item;
    QList<AbstractPhoto*>                   m_items;
    QPainterPath                            rotated_shape;
    QPointF                                 rotation_point;
    QPointF                                 rotation_point_offset;
    QPainterPath                            m_elipse;
    QPointF                                 initial_position;
    QPointF                                 handler_pos;
    qreal                                   rotation_angle;
    QTransform                              currentViewTransform;
    bool                                    elipse_pressed;
    QMap<AbstractPhoto*,RotateItemCommand*> rotate_commands;

    friend class RotationWidgetItem;
};

RotationWidgetItem::RotationWidgetItem(const QList<AbstractPhoto*> & items, QGraphicsItem * parent):
    AbstractItemInterface(parent),
    d(new RotationWidgetItemPrivate(this))
{
    this->setAcceptHoverEvents(true);
    this->setFlag(QGraphicsItem::ItemIsMovable);
    this->setFlag(QGraphicsItem::ItemIsSelectable, false);
    this->setZValue(std::numeric_limits<double>::infinity());
    reset();
    this->setItems(items);
}

void RotationWidgetItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * widget)
{
    // Get the view
    QGraphicsView * view = qobject_cast<QGraphicsView*>(widget->parentWidget());
    if (!view)
        return;
    QTransform viewTransform = view->transform();
    d->transformDrawings(viewTransform);

    painter->save();
    painter->setPen(QPen(Qt::red, 2));
    QPen p = painter->pen();
    p.setCosmetic(true);
    painter->setPen(p);
    painter->setCompositionMode(QPainter::RasterOp_NotSourceAndNotDestination);
    painter->drawPath(d->m_elipse);
    painter->setBrush(Qt::white);
    painter->drawEllipse(-3,-3,6,6);
    painter->restore();
}

QPainterPath RotationWidgetItem::shape() const
{
    return d->m_elipse + d->rotated_shape;
}

QPainterPath RotationWidgetItem::opaqueArea() const
{
    return shape();
}

QRectF RotationWidgetItem::boundingRect() const
{
    return shape().boundingRect();
}

void RotationWidgetItem::initRotation(const QPainterPath & path, const QPointF & rotationPoint)
{
    d->rotated_shape = path;
    QRectF boundingRect = path.boundingRect();
    this->setPos(boundingRect.center());
    d->rotated_shape.translate(-boundingRect.center());
    d->rotation_point = rotationPoint;
    d->rotation_angle = 0;
    d->rotation_point_offset = QPointF();
}

void RotationWidgetItem::reset()
{
    d->rotated_shape = QPainterPath();
    d->rotation_point = QPointF();
    d->rotation_point_offset = QPointF();
    d->rotation_angle = 0;
}

qreal RotationWidgetItem::angle() const
{
    return d->rotation_angle;
}

QPointF RotationWidgetItem::rotationPoint() const
{
    return d->rotation_point + d->rotation_point_offset;
}

bool RotationWidgetItem::isRotated() const
{
    return d->rotation_angle != 0;
}

void RotationWidgetItem::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    this->setCursor(QCursor(Qt::OpenHandCursor));
}

void RotationWidgetItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    this->unsetCursor();
}

void RotationWidgetItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QPointF buttonDownPos = d->viewportToItemPosition(event->buttonDownScreenPos(Qt::LeftButton), event->widget());
    QRectF buttonDownPosRect(buttonDownPos-QPointF(1,1), QSizeF(2,2));
    if (d->m_elipse.intersects(buttonDownPosRect))
    {
        d->elipse_pressed = true;
        d->initial_position = pos();
    }
    else
        d->elipse_pressed = false;
    this->setCursor(QCursor(Qt::ClosedHandCursor));
    event->setAccepted(true);
}

void RotationWidgetItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    this->setCursor(QCursor(Qt::OpenHandCursor));

    if (d->rotate_commands.count() > 1)
        PhotoLayoutsWindow::instance()->beginUndoCommandGroup( i18np("Rotate item", "Rotate items", d->rotate_commands.count()) );
    for (QMap<AbstractPhoto*,RotateItemCommand*>::iterator it = d->rotate_commands.begin(); it != d->rotate_commands.end(); ++it)
    {
        if (it.value())
        {
            it.value()->setDone(true);
            PLE_PostUndoCommand(it.value());
            it.value() = 0;
        }
    }
    if (d->rotate_commands.count() > 1)
        PhotoLayoutsWindow::instance()->endUndoCommandGroup();
    d->rotate_commands.clear();

    d->rotation_angle = 0;
}

void RotationWidgetItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (d->elipse_pressed)
    {
        QPointF dif = event->scenePos() - event->lastScenePos();
        moveBy(dif.x(), dif.y());
        d->rotation_point_offset += dif;
        d->rotated_shape.translate(-dif);
    }
    else
    {
        // Calculate movement parameters
        QRectF refreshRect = this->boundingRect();
        QPointF itemPos = d->viewportToItemPosition(event->screenPos(), event->widget());
        QPointF currentPos = d->viewportToItemPosition(event->buttonDownScreenPos(Qt::LeftButton), event->widget());
        qreal currentLength = qSqrt(currentPos.rx()*currentPos.rx()+currentPos.ry()*currentPos.ry());
        qreal newLength = qSqrt(itemPos.rx()*itemPos.rx()+itemPos.ry()*itemPos.ry());
        qreal scalar = currentPos.rx()*itemPos.rx()+currentPos.ry()*itemPos.ry();

        // Calculate angle
        qreal cos = scalar/(currentLength*newLength);
        qreal prev_rotation_angle = d->rotation_angle;
        if (currentPos.rx()*itemPos.ry()-currentPos.ry()*itemPos.rx()>0)
            d->rotation_angle = 180*qAcos(cos)/M_PI;
        else
            d->rotation_angle = -180*qAcos(cos)/M_PI;

        // Rotation with 15 deegres step - Shift modifier
        if (event->modifiers() & Qt::ShiftModifier)
            d->rotation_angle = qRound(d->rotation_angle / 15) * 15.0;
        if (d->rotation_angle == prev_rotation_angle)
            return;

        QTransform transform;
        transform.rotate(d->rotation_angle-prev_rotation_angle);
        d->rotated_shape = transform.map(d->rotated_shape);

        // Updates widgets view
        refreshRect = refreshRect.united(this->boundingRect());
        this->scene()->invalidate(d->itemToViewportRect(refreshRect, event->widget()));

        // Rotate items
        foreach(AbstractPhoto* item, d->m_items)
        {
            RotateItemCommand * rotCom = d->rotate_commands[item];
            if (!rotCom)
                rotCom = d->rotate_commands[item] = new RotateItemCommand(item);
            rotCom->setRotationPoint(d->rotation_point+d->rotation_point_offset);
            rotCom->setAngle(d->rotation_angle);

            QPointF point = d->rotation_point+d->rotation_point_offset;
            QTransform transform;
            transform.translate(point.rx(), point.ry());
            transform.rotate( d->rotation_angle-prev_rotation_angle );
            transform.translate(-point.rx(), -point.ry());
            QRectF updateRect = item->mapRectToScene(item->boundingRect());
            QTransform rotated = item->transform() * transform;
            item->setTransform(rotated);
            updateRect = updateRect.united( item->mapRectToScene( item->boundingRect() ) );
            if (item->scene())
                item->scene()->invalidate(updateRect);
        }
    }
    event->accept();
}

void RotationWidgetItem::setItems(const QList<AbstractPhoto*> & items)
{
    d->m_items = items;

    if (items.count() == 0)
        return;

    QPainterPath itemsPath;
    foreach(AbstractPhoto* item, items)
        itemsPath += this->mapFromItem(item, item->shape());
    initRotation(itemsPath, items.at(0)->boundingRect().center() * items.at(0)->transform());
    setPos(itemsPath.boundingRect().center());
}
