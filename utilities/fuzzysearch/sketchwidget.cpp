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

#include <QMap>
#include <QPainter>
#include <QColor>
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "sketchwidget.h"

namespace Digikam
{

class DrawEvent
{
public:

    DrawEvent(int width, const QColor& color, const QPoint& begin, const QPoint& end)
    {
        penWidth   = width;
        penColor   = color;
        beginPoint = begin;
        endPoint   = end;
    }

    int    penWidth;

    QColor penColor;

    QPoint beginPoint;
    QPoint endPoint;
};

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

    bool                 isClear;
    bool                 drawing;

    int                  penWidth;

    QColor               penColor;

    QPixmap              pixmap;

    QPoint               lastPoint;

    QMap<int, DrawEvent> drawEventList;
};

SketchWidget::SketchWidget(QWidget *parent)
            : QWidget(parent)
{
    d = new SketchWidgetPriv;

    setWhatsThis(i18n("You simply draw here a rough sketch of what you want to find "
                      "and digiKam will displays the best matches in thumbnail view."));

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

QString SketchWidget::sketchImageAsXML()
{
    QDomDocument sketchDoc;
    QDomElement  imageElem = sketchDoc.createElement(QString::fromLatin1("SketchImage"));
    sketchDoc.appendChild(imageElem);

    QMap<int, DrawEvent>::const_iterator it;

    for (it = d->drawEventList.begin(); it != d->drawEventList.end(); ++it)
    {
        QDomElement line = sketchDoc.createElement("Line");
        imageElem.appendChild(line);
        addXmlTextElement(sketchDoc, line, "Id",     QString::number(it.key()));
        addXmlTextElement(sketchDoc, line, "Size",   QString::number(it.value().penWidth));
        addXmlTextElement(sketchDoc, line, "Color",  it.value().penColor.name());
        addXmlTextElement(sketchDoc, line, "BeginX", QString::number(it.value().beginPoint.x()));
        addXmlTextElement(sketchDoc, line, "BeginY", QString::number(it.value().beginPoint.y()));
        addXmlTextElement(sketchDoc, line, "EndX",   QString::number(it.value().endPoint.x()));
        addXmlTextElement(sketchDoc, line, "EndY",   QString::number(it.value().endPoint.y()));
    }

    QString xml = sketchDoc.toString();
    return xml;
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
    d->drawEventList.clear();
    update();
}

void SketchWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) 
    {
        if (d->isClear)
        {
            d->pixmap.fill(qRgb(255, 255, 255));
            d->isClear = false;
            update();
        }

        d->lastPoint = e->pos();
        d->drawing   = true;
    }
}

void SketchWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (rect().contains(e->x(), e->y()))
    {
        setCursor(Qt::CrossCursor);

        if ((e->buttons() & Qt::LeftButton) && d->drawing)
        {
            QPoint currentPos = e->pos();
            drawLineTo(currentPos);
            d->drawEventList.insert(d->drawEventList.size()+1, 
                                    DrawEvent(d->penWidth, d->penColor, d->lastPoint, currentPos));
        }
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
        QPoint currentPos = e->pos();
        drawLineTo(currentPos);
        d->drawEventList.insert(d->drawEventList.size()+1, 
                                DrawEvent(d->penWidth, d->penColor, d->lastPoint, currentPos));
        d->drawing = false;
        emit signalSketchChanged(sketchImage());
    }
}

void SketchWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if (d->isClear)
    {
        p.drawText(0, 0, width(), height(), Qt::AlignCenter,
                   i18n("Draw a sketch here\nto perform a\nFuzzy search"));
    }
    else
    {
        p.drawPixmap(QPoint(0, 0), d->pixmap);
    }
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

QDomElement SketchWidget::addXmlTextElement(QDomDocument &document, QDomElement &target,
                                              const QString& tag, const QString& text)
{
    QDomElement element  = document.createElement(tag);
    target.appendChild(element);
    QDomText textElement = document.createTextNode(text);
    element.appendChild(textElement);
    return element;
}

}  // namespace Digikam
