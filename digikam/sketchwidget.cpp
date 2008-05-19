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
        drawing  = false;
        penWidth = 10;
        penColor = Qt::black;
        pixmap   = QPixmap(256, 256);
    }

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
    setFixedSize(256, 256);
    slotClear();
}

SketchWidget::~SketchWidget()
{
    delete d;
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
    d->pixmap.fill(qRgb(255, 255, 255));
    update();
}

void SketchWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) 
    {
        d->lastPoint = event->pos();
        d->drawing   = true;
    }
}

void SketchWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && d->drawing)
        drawLineTo(event->pos());
}

void SketchWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && d->drawing) 
    {
        drawLineTo(event->pos());
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
