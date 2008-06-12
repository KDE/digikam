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

    DrawEvent()
    {
        penWidth = 10;
        penColor = Qt::black;
    };

    DrawEvent(int width, const QColor& color, const QPoint& begin, const QPoint& end)
    {
        penWidth   = width;
        penColor   = color;
        beginPoint = begin;
        endPoint   = end;
    };

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
        isClear    = true;
        drawing    = false;
        penWidth   = 10;
        penColor   = Qt::black;
        pixmap     = QPixmap(256, 256);
        eventIndex = 0;
    }

    bool                 isClear;
    bool                 drawing;

    int                  penWidth;
    int                  eventIndex;

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

void SketchWidget::slotClear()
{
    d->isClear    = true;
    d->eventIndex = 0;
    d->pixmap.fill(qRgb(255, 255, 255));
    d->drawEventList.clear();
    update();
}

bool SketchWidget::isClear() const
{
    return d->isClear;
}

void SketchWidget::setPenColor(const QColor &newColor)
{
    d->penColor = newColor;
}

QColor SketchWidget::penColor() const
{
    return d->penColor;
}

void SketchWidget::setPenWidth(int newWidth)
{
    d->penWidth = newWidth;
}

int SketchWidget::penWidth() const
{
    return d->penWidth;
}

void SketchWidget::slotUndo()
{
    if (d->eventIndex == 0) return;
    d->eventIndex--;
    replayEvents(d->eventIndex);
    emit signalSketchChanged(sketchImage());
}

void SketchWidget::slotRedo()
{
    if (d->eventIndex == d->drawEventList.count()) return;
    d->eventIndex++;
    replayEvents(d->eventIndex);
    emit signalSketchChanged(sketchImage());
}

void SketchWidget::replayEvents(int index)
{
    d->pixmap.fill(qRgb(255, 255, 255));

    DrawEvent drawEvent;

    for (int i = 0; i < index; i++)
    {
        drawEvent = d->drawEventList[i];
        drawLineTo(drawEvent.penWidth, drawEvent.penColor, drawEvent.beginPoint, drawEvent.endPoint);
    }

    update();
}

QString SketchWidget::sketchImageToXML()
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

bool SketchWidget::setSketchImageFromXML(const QString& xml)
{
    QDomDocument sketchDoc;
    QString      error;
    int          row, col;

    if (!sketchDoc.setContent(xml, &error, &row, &col))
    {
        DDebug() << "Error to import Sketch XML data:" << endl;
        DDebug() << error << " :: row=" << row << " , col=" << col << endl;
        return false;
    }

    QDomElement imageElem = sketchDoc.documentElement();
    if (imageElem.tagName() != QString::fromLatin1("SketchImage"))
        return false;

    d->isClear = false;
    d->drawEventList.clear();

    for (QDomNode nLine = imageElem.firstChild();
         !nLine.isNull(); nLine = nLine.nextSibling())
    {
        QDomElement lineElem = nLine.toElement();
        if (lineElem.isNull()) continue;
        if (lineElem.tagName() != "Line") continue;

        int       indexEvent = -1;
        DrawEvent drawEvent;
        QString   temp;

        for (QDomNode nLineMeta = lineElem.firstChild();
             !nLineMeta.isNull(); nLineMeta = nLineMeta.nextSibling())
        {
            QDomElement lineMetaElem = nLineMeta.toElement();

            if (lineMetaElem.isNull()) 
            {
                continue;
            }
            else if (lineMetaElem.tagName() == QString("Id"))
            {
                temp       = lineMetaElem.text();
                indexEvent = temp.toInt();
            }
            else if (lineMetaElem.tagName() == QString("Size"))
            {
                temp               = lineMetaElem.text();
                drawEvent.penWidth = temp.toInt();
            }
            else if (lineMetaElem.tagName() == QString("Color"))
            {
                temp = lineMetaElem.text();
                drawEvent.penColor.setNamedColor(temp);
            }
            else if (lineMetaElem.tagName() == QString("BeginX"))
            {
                temp = lineMetaElem.text();
                drawEvent.beginPoint.setX(temp.toInt());
            }
            else if (lineMetaElem.tagName() == QString("BeginY"))
            {
                temp = lineMetaElem.text();
                drawEvent.beginPoint.setY(temp.toInt());
            }
            else if (lineMetaElem.tagName() == QString("EndX"))
            {
                temp = lineMetaElem.text();
                drawEvent.endPoint.setX(temp.toInt());
            }
            else if (lineMetaElem.tagName() == QString("EndY"))
            {
                temp = lineMetaElem.text();
                drawEvent.endPoint.setY(temp.toInt());

                d->drawEventList.insert(indexEvent, drawEvent);
            }
        }
    }

    d->eventIndex = d->drawEventList.count();
    replayEvents(d->eventIndex);

    return true;
}

QImage SketchWidget::sketchImage() const
{
    return d->pixmap.toImage();
}

void SketchWidget::setSketchImage(const QImage& image)
{
    d->isClear    = false;
    d->pixmap     = QPixmap::fromImage(image);
    d->eventIndex = 0;
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

	// Remove all draw events from history map which are upper than current index.
	// If user redo actions and make new draw events, theses one will be queued at 
	// end of history and will remplace removed items.
        for (int i = d->eventIndex + 1; i <= d->drawEventList.count(); i++)
        {
            d->drawEventList.remove(i);
        }
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
            d->drawEventList.insert(d->eventIndex++,
                                    DrawEvent(d->penWidth, d->penColor, d->lastPoint, currentPos));
            drawLineTo(currentPos);
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
        d->drawEventList.insert(d->eventIndex++,
                                DrawEvent(d->penWidth, d->penColor, d->lastPoint, currentPos));
        drawLineTo(currentPos);
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
    drawLineTo(d->penWidth, d->penColor, d->lastPoint, endPoint);
}

void SketchWidget::drawLineTo(int width, const QColor& color, const QPoint& start, const QPoint& end)
{
    QPainter painter(&d->pixmap);
    painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(start, end);

    int rad = (width / 2) + 2;

    update(QRect(start, end).normalized().adjusted(-rad, -rad, +rad, +rad));
    d->lastPoint = end;
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
