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

#include "ScalingWidgetItem.h"
#include "AbstractPhoto.h"
#include "photolayoutswindow.h"
#include "global.h"
#include <limits>

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QUndoCommand>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMap>

#include <klocalizedstring.h>

#include "digikam_debug.h"

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::MoveItemCommand : public QUndoCommand
{
    AbstractPhoto * m_item;
    QPointF m_translation;
    bool done;
public:
    MoveItemCommand(AbstractPhoto * item, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Move item"), parent),
        m_item(item),
        done(false)
    {}
    virtual void redo()
    {
        if (done)
            return;
        qCDebug(DIGIKAM_GENERAL_LOG) << done << "redo MoveItemCommand";
        m_item->moveBy(m_translation.x(), m_translation.y());
        done = true;
    }
    virtual void undo()
    {
        if (!done)
            return;
        qCDebug(DIGIKAM_GENERAL_LOG) << done << "undo MoveItemCommand";
        m_item->moveBy(-m_translation.x(), -m_translation.y());
        done = false;
    }
    void addTranslation(const QPointF & point)
    {
        m_translation += point;
    }
    QPointF translation()
    {
        return m_translation;
    }
    void setDone(bool done)
    {
        this->done = done;
    }
};

class PhotoLayoutsEditor::ScaleItemCommand : public MoveItemCommand
{
    AbstractPhoto * m_item;
    QTransform scale;
    bool done;
public:
    ScaleItemCommand(AbstractPhoto * item, QUndoCommand * parent = 0) :
        MoveItemCommand(item, parent),
        m_item(item),
        done(false)
    {
        this->setText(i18n("Scale item"));
    }
    virtual void redo()
    {
        if (done)
            return;
        m_item->setTransform( m_item->transform() * scale );
        MoveItemCommand::redo();
        done = true;
    }
    virtual void undo()
    {
        if (!done)
            return;
        QTransform inverted = scale.inverted();
        m_item->setTransform( m_item->transform() * inverted );
        MoveItemCommand::undo();
        done = false;
    }
    void addScale(const QTransform & scale)
    {
        this->scale *= scale;
    }
    void setDone(bool done)
    {
        this->done = done;
        MoveItemCommand::setDone(done);
    }
};

class PhotoLayoutsEditor::ScalingWidgetItemPrivate
{
    enum
    {
        Top,
        VCenter,
        Bottom
    };

    enum
    {
        Left,
        HCenter,
        Right
    };

    ScalingWidgetItemPrivate() :
        currentViewTransform(1, 0, 0,    0, 1, 0,   0, 0, 1),
//        recalculate(true),
        pressedVHandler(-1),
        pressedHHandler(-1)
    {
        m_handlers[Top][Left] = QRectF(0, 0, 20, 20);
        m_handlers[Top][HCenter] = QRectF(0, 0, 20, 20);
        m_handlers[Top][Right] = QRectF(0, 0, 20, 20);
        m_handlers[VCenter][Left] = QRectF(0, 0, 20, 20);
        m_handlers[VCenter][Right] = QRectF(0, 0, 20, 20);
        m_handlers[Bottom][Left] = QRectF(0, 0, 20, 20);
        m_handlers[Bottom][HCenter] = QRectF(0, 0, 20, 20);
        m_handlers[Bottom][Right] = QRectF(0, 0, 20, 20);
        m_elipse.addEllipse(QPointF(0,0), 10, 10);
    }

    QTransform currentViewTransform;
    void transformDrawings(const QTransform & viewTransform);
    void calculateHandlers();
    void correctRect(QRectF & r);

    QList<AbstractPhoto*> m_items;
    QPainterPath m_crop_shape;
    QPainterPath m_shape;
    QPainterPath m_handlers_path;
    QRectF m_rect;
    QRectF m_begin_rect;
    QRectF m_handlers[Bottom+1][Right+1];
    QPainterPath m_elipse;
//    bool recalculate;
    int pressedVHandler;
    int pressedHHandler;
    QPointF handlerOffset;

    // Undo commands
    QMap<AbstractPhoto*,ScaleItemCommand*> scale_commands;
    QMap<AbstractPhoto*,MoveItemCommand*> move_commands;

    friend class ScalingWidgetItem;
};

void ScalingWidgetItemPrivate::transformDrawings(const QTransform & viewTransform)
{
    if (currentViewTransform == viewTransform)
        return;

    currentViewTransform = viewTransform;

    this->calculateHandlers();
}

void ScalingWidgetItemPrivate::calculateHandlers()
{
    qreal tempx = -10 / currentViewTransform.m11();
    qreal tempy = -10 / currentViewTransform.m22();

    // Scale width of handlers
    qreal w = qAbs(m_rect.width()) + 12 * tempx;
    w = (w < 0 ? w / 3.0 : 0);
    w = (w < tempx ? tempx : w);
    qreal tw = w - 4 * tempx;
    m_handlers[Top][Left].setWidth(tw);
    m_handlers[Top][HCenter].setWidth(tw);
    m_handlers[Top][Right].setWidth(tw);
    m_handlers[VCenter][Left].setWidth(tw);
    m_handlers[VCenter][Right].setWidth(tw);
    m_handlers[Bottom][Left].setWidth(tw);
    m_handlers[Bottom][HCenter].setWidth(tw);
    m_handlers[Bottom][Right].setWidth(tw);

    // Scale height of handlers
    qreal h = qAbs(m_rect.height()) + 12 * tempy;
    h = (h < 0 ? h / 3.0 : 0);
    h = (h < tempy ? tempy : h);
    qreal th = h - 4 * tempy;
    m_handlers[Top][Left].setHeight(th);
    m_handlers[Top][HCenter].setHeight(th);
    m_handlers[Top][Right].setHeight(th);
    m_handlers[VCenter][Left].setHeight(th);
    m_handlers[VCenter][Right].setHeight(th);
    m_handlers[Bottom][Left].setHeight(th);
    m_handlers[Bottom][HCenter].setHeight(th);
    m_handlers[Bottom][Right].setHeight(th);

    m_elipse = QPainterPath();
    m_elipse.addEllipse(m_rect.center(), tw / 2, th / 2);

    w = qAbs(m_rect.width()) + 7 * tempx;
    w = (w < 0 ? w / 2.0 : 0);
    h = qAbs(m_rect.height()) + 7 * tempy;
    h = (h < 0 ? h / 2.0 : 0);
    m_handlers[Top][Left].moveCenter(m_rect.topLeft() + QPointF(w,h));
    m_handlers[Top][HCenter].moveCenter( QPointF( m_rect.center().x(), m_rect.top() + h ) );
    m_handlers[Top][Right].moveCenter(m_rect.topRight() + QPointF(-w,h));
    m_handlers[VCenter][Left].moveCenter( QPointF( m_rect.left() + w, m_rect.center().y() ) );
    m_handlers[VCenter][Right].moveCenter( QPointF( m_rect.right() - w, m_rect.center().y() ) );
    m_handlers[Bottom][Left].moveCenter(m_rect.bottomLeft() + QPointF(w,-h));
    m_handlers[Bottom][HCenter].moveCenter( QPointF( m_rect.center().x(), m_rect.bottom() - h ) );
    m_handlers[Bottom][Right].moveCenter(m_rect.bottomRight() + QPointF(-w,-h));

    m_shape = QPainterPath();
    m_shape.addRect(m_rect);

    m_handlers_path = QPainterPath();
    for (int i = Top; i <= Bottom; ++i)
        for (int j = Left; j <= Right; ++j)
            m_handlers_path.addRect(m_handlers[i][j]);
    m_handlers_path += m_elipse;
}

void ScalingWidgetItemPrivate::correctRect(QRectF & r)
{
    if (r.bottom() < r.top())
    {
        if (this->pressedVHandler == Top)
            r.setTop( r.bottom() - 1 );
        else
            r.setBottom( r.top() + 1);
    }

    if (r.left() > r.right())
    {
        if (this->pressedHHandler == Left)
            r.setLeft( r.right() - 1 );
        else
            r.setRight( r.left() + 1);
    }
}

ScalingWidgetItem::ScalingWidgetItem(const QList<AbstractPhoto*> & items, QGraphicsItem * parent, QGraphicsScene * scene) :
    AbstractItemInterface(parent, scene),
    d(new ScalingWidgetItemPrivate)
{
    this->setAcceptHoverEvents(true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, false);
    this->setZValue(std::numeric_limits<double>::infinity());
    this->setScaleItems(items);
}

ScalingWidgetItem::~ScalingWidgetItem()
{
    delete d;
}

QRectF ScalingWidgetItem::boundingRect() const
{
    return (d->m_shape + d->m_handlers_path).boundingRect();
}

QPainterPath ScalingWidgetItem::opaqueArea() const
{
    return (d->m_shape + d->m_handlers_path);
}

QPainterPath ScalingWidgetItem::shape() const
{
    return (d->m_shape + d->m_handlers_path);
}

void ScalingWidgetItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * widget)
{
    // Get the view
    QGraphicsView * view = qobject_cast<QGraphicsView*>(widget->parentWidget());
    if (!view)
        return;
    QTransform viewTransform = view->transform();
    d->transformDrawings(viewTransform);

    painter->save();

    // Draw bounding rect
    painter->setCompositionMode(QPainter::RasterOp_NotSourceAndNotDestination);
    painter->setPen(Qt::red);
    painter->setPen(Qt::SolidLine);
    painter->drawPath(d->m_shape);
    painter->drawPath(d->m_handlers_path);
    painter->drawPath(d->m_elipse);

    painter->restore();
}

void ScalingWidgetItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    d->pressedVHandler = -1;
    d->pressedHHandler = -1;
    d->handlerOffset = QPointF(0,0);
    this->setFocus( Qt::MouseFocusReason );
    d->m_begin_rect = d->m_rect;
    if (event->button() == Qt::LeftButton)
    {
        QPointF handledPoint = this->mapFromScene(event->buttonDownScenePos(Qt::LeftButton));
        for (int i = ScalingWidgetItemPrivate::Top; i <= ScalingWidgetItemPrivate::Bottom; ++i)
        {
            for (int j = ScalingWidgetItemPrivate::Left; j <= ScalingWidgetItemPrivate::Right; ++j)
            {
                if (d->m_handlers[i][j].contains(handledPoint))
                {
                    d->pressedVHandler = i;
                    d->pressedHHandler = j;
                    goto found;
                }
            }
        }
        if (d->m_elipse.contains(handledPoint))
        {
            d->pressedVHandler = ScalingWidgetItemPrivate::VCenter;
            d->pressedHHandler = ScalingWidgetItemPrivate::HCenter;
            event->setAccepted(true);
        }
        return;
        found:
            d->handlerOffset = d->m_handlers[d->pressedVHandler][d->pressedHHandler].center() - handledPoint;
            event->setAccepted(true);
    }
}

void ScalingWidgetItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (d->pressedHHandler == -1 || d->pressedVHandler == -1)
        return;

    // New handler position calc
    QPointF point = event->pos() + d->handlerOffset;

    QRectF temp = d->m_rect;

    // Position change
    if (d->pressedVHandler == ScalingWidgetItemPrivate::VCenter &&
        d->pressedHHandler == ScalingWidgetItemPrivate::HCenter)
    {
        QPointF dif = event->scenePos() - event->lastScenePos();
        temp.translate(dif);
        d->m_handlers_path.translate(dif);
        dif = (event->scenePos()) - (event->lastScenePos());
        foreach(AbstractPhoto* item, d->m_items)
        {
            MoveItemCommand * moveCom = d->move_commands[item];
            if (!moveCom)
                moveCom = d->move_commands[item] = new MoveItemCommand(item);
            moveCom->addTranslation(dif);

            item->QGraphicsItem::moveBy(dif.x(), dif.y());
        }
    }
    // Size change
    else
    {
        // Vertical size change
        if (d->pressedVHandler == ScalingWidgetItemPrivate::Top)
            temp.setTop( point.y() );
        else if (d->pressedVHandler == ScalingWidgetItemPrivate::Bottom)
            temp.setBottom( point.y() );

        // Horizontal size change
        if (d->pressedHHandler == ScalingWidgetItemPrivate::Right)
            temp.setRight( point.x() );
        else if (d->pressedHHandler == ScalingWidgetItemPrivate::Left)
            temp.setLeft( point.x() );

        d->correctRect( temp );

        // Keeps aspect ratio
        if (event->modifiers() & Qt::ShiftModifier)
        {
            qreal xFactor = temp.width()  / d->m_begin_rect.width();
            qreal yFactor = temp.height() / d->m_begin_rect.height();
            if (d->pressedHHandler == ScalingWidgetItemPrivate::HCenter)
            {
                qreal dif = d->m_begin_rect.width() - d->m_begin_rect.width() * yFactor;
                temp.setRight( d->m_begin_rect.right() - dif / 2.0 );
                temp.setLeft( d->m_begin_rect.left() + dif / 2.0 );
            }
            else if (d->pressedVHandler == ScalingWidgetItemPrivate::VCenter)
            {
                qreal dif = d->m_begin_rect.height() - d->m_begin_rect.height() * xFactor;
                temp.setTop( d->m_begin_rect.top() + dif / 2.0 );
                temp.setBottom( d->m_begin_rect.bottom() - dif / 2.0 );
            }
            else if (xFactor > yFactor)
            {
                qreal dif = d->m_begin_rect.width() - d->m_begin_rect.width() * yFactor;
                if (d->pressedHHandler == ScalingWidgetItemPrivate::Right)
                    temp.setRight( d->m_begin_rect.right() - dif );
                else if (d->pressedHHandler == ScalingWidgetItemPrivate::Left)
                    temp.setLeft( d->m_begin_rect.left() + dif );
            }
            else if (xFactor < yFactor)
            {
                qreal dif = d->m_begin_rect.height() - d->m_begin_rect.height() * xFactor;
                if (d->pressedVHandler == ScalingWidgetItemPrivate::Top)
                    temp.setTop( d->m_begin_rect.top() + dif );
                else if (d->pressedVHandler == ScalingWidgetItemPrivate::Bottom)
                    temp.setBottom( d->m_begin_rect.bottom() - dif );
            }
        }
    }

    QTransform sc;
    sc.scale(temp.width() / d->m_rect.width(), temp.height() / d->m_rect.height());

    if (!sc.isIdentity())
    {
        foreach(AbstractPhoto* item, d->m_items)
        {
            QRectF beforeScene = item->mapRectToScene(item->boundingRect());
            item->setTransform( item->transform() * sc );
            QRectF afterScene = item->mapRectToScene(item->boundingRect());
            QPointF p(0,0);
            if (d->pressedVHandler == ScalingWidgetItemPrivate::Top)
                p.setY(beforeScene.bottom() - afterScene.bottom());
            else if (d->pressedVHandler == ScalingWidgetItemPrivate::Bottom)
                p.setY(beforeScene.top() - afterScene.top());
            else
                p.setY((beforeScene.center() - afterScene.center()).y());

            if (d->pressedHHandler == ScalingWidgetItemPrivate::Left)
                p.setX(beforeScene.right() - afterScene.right());
            else if (d->pressedHHandler == ScalingWidgetItemPrivate::Right)
                p.setX(beforeScene.left() - afterScene.left());
            else
                p.setX((beforeScene.center() - afterScene.center()).x());

            ScaleItemCommand * scaleCom = d->scale_commands[item];
            if (!scaleCom)
                scaleCom = d->scale_commands[item] = new ScaleItemCommand(item);
            scaleCom->addScale(sc);
            scaleCom->addTranslation(p);

            item->moveBy( p.x(), p.y() );
        }
    }

    d->m_rect = temp;
    event->setAccepted(true);
}

void ScalingWidgetItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    if (d->scale_commands.count() > 1)
        PhotoLayoutsWindow::instance()->beginUndoCommandGroup( i18np("Scale item", "Scale items", d->scale_commands.count()) );
    for (QMap<AbstractPhoto*,ScaleItemCommand*>::iterator it = d->scale_commands.begin(); it != d->scale_commands.end(); ++it)
    {
        if (it.value())
        {
            it.value()->setDone(true);
            PLE_PostUndoCommand(it.value());
            it.value() = 0;
        }
    }
    if (d->scale_commands.count() > 1)
        PhotoLayoutsWindow::instance()->endUndoCommandGroup();
    d->scale_commands.clear();

    if (d->move_commands.count() > 1)
        PhotoLayoutsWindow::instance()->beginUndoCommandGroup( i18np("Move item", "Move items", d->move_commands.count()) );
    for (QMap<AbstractPhoto*,MoveItemCommand*>::iterator it = d->move_commands.begin(); it != d->move_commands.end(); ++it)
    {
        if (it.value())
        {
            it.value()->setDone(true);
            PLE_PostUndoCommand(it.value());
            it.value() = 0;
        }
    }
    if (d->move_commands.count() > 1)
        PhotoLayoutsWindow::instance()->endUndoCommandGroup();
    d->move_commands.clear();
}

void ScalingWidgetItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{}

void ScalingWidgetItem::setScaleItems(const QList<AbstractPhoto*> & items)
{
    d->m_items = items;

    foreach(AbstractPhoto* item, items)
        connect(item, SIGNAL(changed()), this, SLOT(updateShapes()));

    this->updateShapes();
}

void ScalingWidgetItem::updateShapes()
{
    d->m_crop_shape = QPainterPath();
    foreach(AbstractPhoto* item, d->m_items)
        d->m_crop_shape += this->mapFromItem(item, item->opaqueArea());

    d->m_rect = d->m_crop_shape.boundingRect();

    QPainterPath updatePath;
    updatePath.setFillRule(Qt::WindingFill);
    updatePath.addRect(d->m_rect);
    updatePath = updatePath.united(d->m_handlers_path);
    d->calculateHandlers();
    updatePath = updatePath.united(d->m_handlers_path);

    if (this->scene())
        this->scene()->update( this->mapRectToScene(updatePath.boundingRect()) );
}
