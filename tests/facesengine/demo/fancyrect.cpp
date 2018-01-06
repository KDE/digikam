/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-23
 * Description : QGraphicsRectItem wrapper for FacesEngine
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2008 by Adrien Bustany <madcat at mymadcat dot com>
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

#include "fancyrect.h"

// Qt includes

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QRectF>
#include <QPainter>

namespace Digikam
{

FancyRect::FancyRect(QGraphicsItem* const parent)
    : QGraphicsRectItem(parent)
{
}

FancyRect::FancyRect(const QRectF& rect, QGraphicsItem* const parent)
    : QGraphicsRectItem(rect, parent)
{
}

FancyRect::FancyRect(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* const parent)
    : QGraphicsRectItem(x, y, w, h, parent)
{
}

FancyRect::FancyRect(QGraphicsRectItem* const other, QGraphicsItem* const parent)
    : QGraphicsRectItem(parent)
{
    setPos(other->pos());
    setRect(other->rect());
    setPen(other->pen());
}

void FancyRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (option->state.testFlag(QStyle::State_Selected))
    {
        QPen selectedPen = pen();
        selectedPen.setColor(Qt::red);
        painter->setPen(selectedPen);
        painter->drawRect(rect());
    }
    else
    {
        QGraphicsRectItem::paint(painter, option, widget);
    }
}

}   // Namespace FacesEngine
