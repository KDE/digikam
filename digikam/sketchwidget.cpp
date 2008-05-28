/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : a widget to draw sketch.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QPainter>
#include <QColor>
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>

// Local includes.

#include "sketchwidget.h"

namespace Digikam
{

class SketchWidgetPriv
{
public:

    SketchWidgetPriv()
    {
        isClear  = true;
        drawing  = false;
        penWidth = 10;
        penColor = Qt::black;
        pixmap   = QPixmap(256, 256);
    }

    bool    isClear;
    bool    drawing;

    int     penWidth;

    QColor  penColor;

    QPixmap pixmap;

    QPoint  lastPoint;
};

SketchWidget::SketchWidget(QWidget *parent)
            : QWidget(parent)
{
    d = new SketchWidgetPriv;

    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
    setFixedSize(256, 256);
    slotClear();
}

SketchWidget::~SketchWidget()
{
    delete d;
}

bool SketchWidget::isClear() const
{
    return d->isClear;
}

QColor SketchWidget::penColor() const
{
    return d->penColor;
}

int SketchWidget::penWidth() const
{ 
    return d->penWidth;
}

QImage SketchWidget::sketchImage() const
{ 
    return d->pixmap.toImage();
}

void SketchWidget::setPenColor(const QColor &newColor)
{
    d->penColor = newColor;
}

void SketchWidget::setPenWidth(int newWidth)
{
    d->penWidth = newWidth;
}

void SketchWidget::slotClear()
{
    d->isClear = true;
    d->pixmap.fill(qRgb(255, 255, 255));
    update();
}

void SketchWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) 
    {
        d->lastPoint = e->pos();
        d->drawing   = true;
        d->isClear   = false;
    }
}

void SketchWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (rect().contains(e->x(), e->y()))
    {
        setCursor(Qt::CrossCursor);

        if ((e->buttons() & Qt::LeftButton) && d->drawing)
            drawLineTo(e->pos());
    }
    else
    {
        unsetCursor();
    }
}

void SketchWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && d->drawing) 
    {
        drawLineTo(e->pos());
        d->drawing = false;
        emit signalSketchChanged(sketchImage());
    }
}

void SketchWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(QPoint(0, 0), d->pixmap);
}

void SketchWidget::drawLineTo(const QPoint& endPoint)
{
    QPainter painter(&d->pixmap);
    painter.setPen(QPen(d->penColor, d->penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(d->lastPoint, endPoint);

    int rad = (d->penWidth / 2) + 2;

    update(QRect(d->lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));

    d->lastPoint = endPoint;
}

}  // namespace Digikam
