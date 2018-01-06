/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-23
 * Description : face marquer widget for FacesEngine
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Adrien Bustany <madcat at mymadcat dot com>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include "marquee.h"

// Qt includes

#include <QPointF>
#include <QPen>
#include <QString>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene>
#include <QDebug>

// Local includes

#include "fancyrect.h"

namespace Digikam
{

class Marquee::Private
{
public:

    enum ResizeType
    {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRight
    };

    Private()
      : handleSize(10)
    {
        htl        = 0;
        rect       = 0;
        htr        = 0;
        hbr        = 0;
        hbl        = 0;
        label      = 0;
        moving     = false;
        resizing   = false;
        resizeType = 0;
    }

    const int                handleSize;
    QPen                     rectPen;    // The pen used to draw the frames
    QPen                     outlinePen; // Text outline pen

    // Handles
    FancyRect*               htl;
    FancyRect*               htr;
    FancyRect*               hbr;
    FancyRect*               hbl;

    FancyRect*               rect;       // Main frame
    QGraphicsSimpleTextItem* label;

    bool                     moving;     // Tells if we are moving the marquee
    QPointF                  moveOffset; // where the mouse was when the user began to drag the marquee
    bool                     resizing;   // Tells if we are resizing the marquee
    int                      resizeType; // See ResizeType values.
};

Marquee::Marquee(FancyRect* const rect, QGraphicsItem* const parent)
    : QObject(0), QGraphicsItemGroup(parent), d(new Private)
{
    d->rect   = rect;
    d->rectPen.setColor(Qt::red);
    d->rectPen.setWidth(2);
    d->outlinePen.setColor(Qt::red);
    addToGroup(d->rect);
    d->rect->setPen(d->rectPen);
    setPos(d->rect->scenePos());
    d->rect->setPos(0, 0);
    createHandles();

    d->label   = new QGraphicsSimpleTextItem(QString::fromLatin1(""), this);
    d->label->setBrush(QBrush(d->rectPen.color()));
    d->label->setPen(d->outlinePen);
    d->label->setZValue(2);

    QFont font = d->label->font();
    font.setBold(true);
    font.setPointSize(12);
    d->label->setFont(font);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setSelected(true);

    emit selected(this);
}

Marquee::~Marquee()
{
    delete d;
}

QRectF Marquee::boundingRect() const
{
    return d->rect->rect();
}

QRectF Marquee::toRectF() const
{
    return QRectF(x(), y(), d->rect->rect().width(), d->rect->rect().height());
}

void Marquee::createHandles()
{
    d->htl = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->htl->setZValue(1);
    d->htr = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->htr->setZValue(1);
    d->hbl = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->hbl->setZValue(1);
    d->hbr = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->hbr->setZValue(1);

    d->htl->setPen(d->rectPen);
    d->htr->setPen(d->rectPen);
    d->hbl->setPen(d->rectPen);
    d->hbr->setPen(d->rectPen);

    addToGroup(d->htl);
    addToGroup(d->htr);
    addToGroup(d->hbl);
    addToGroup(d->hbr);
    placeHandles();
}

void Marquee::placeHandles()
{
    qreal rw = d->rect->rect().width();
    qreal rh = d->rect->rect().height();
    qreal ox = d->rect->rect().x();
    qreal oy = d->rect->rect().y();
    qreal hs = d->hbr->boundingRect().width();

    d->htl->setPos(ox,         oy);
    d->htr->setPos(ox+rw-hs+2, oy);
    d->hbl->setPos(ox,         oy+rh-hs+2);
    d->hbr->setPos(ox+rw-hs+2, oy+rh-hs+2);
}

void Marquee::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    emit selected(this);
    // Check for some resize handles under the mouse

    if (d->htl->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::TopLeft;
        return;
    }

    if (d->htr->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::TopRight;
        return;
    }

    if (d->hbl->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::BottomLeft;
        return;
    }

    if (d->hbr->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::BottomRight;
        return;
    }

    // If no handle is under the mouse, then we move the frame
    d->resizing   = false;
    d->moving     = true;
    d->moveOffset = e->pos();

    emit changed();
}

void Marquee::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (d->resizing)
    {
        QRectF r = d->rect->rect();

        switch(d->resizeType)
        {
            case Private::TopLeft:
                r.setTopLeft(e->pos());
                break;
            case Private::TopRight:
                r.setTopRight(e->pos());
                break;
            case Private::BottomLeft:
                r.setBottomLeft(e->pos());
                break;
            case Private::BottomRight:
                r.setBottomRight(e->pos());
                break;
            default:
                break;
        }

        if (r.width() < 2*d->handleSize || r.height() < 2*d->handleSize)
            return;

        setPos(pos() + r.topLeft());
        r.moveTopLeft(QPointF(0, 0));
        d->rect->setRect(r);
        placeHandles();
    }

    if (d->moving)
    {
        setPos(e->scenePos() - d->moveOffset);
    }

    emit changed();
}

void Marquee::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    Q_UNUSED(e);
    d->resizing = false;
    d->moving   = false;

    emit changed();
}

} // namespace Digikam
