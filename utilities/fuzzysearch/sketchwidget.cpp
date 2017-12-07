/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : a widget to draw sketch.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "sketchwidget.h"

// Qt includes

#include <QCursor>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>
#include <QTime>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class DrawEvent
{
public:

    DrawEvent() :
        penWidth(10),
        penColor(Qt::black)
    {
    };

    DrawEvent(int width, const QColor& color) :
        penWidth(width),
        penColor(color)
    {
    };

    void lineTo(const QPoint& pos)
    {
        path.lineTo(pos);
    }

public:

    int          penWidth;
    QColor       penColor;
    QPainterPath path;
};

// ------------------------------------------------------------------------------

class SketchWidget::Private
{
public:

    Private() :
        isClear(true),
        drawing(false),
        penWidth(10),
        eventIndex(-1),
        penColor(Qt::black)
    {
        pixmap = QPixmap(256, 256);
    }

    void startDrawEvent(const QPoint& pos)
    {
        // Remove all draw events from history map which are upper than current index.
        // If user redo actions and make new draw events, theses one will be queued at
        // end of history and will replace removed items.
        for (int i = drawEventList.count() - 1 ; i > eventIndex ;  --i)
        {
            drawEventList.removeAt(i);
        }

        drawEventCreationTime = QTime::currentTime();
        DrawEvent event(penWidth, penColor);
        event.path.moveTo(pos);
        drawEventList << event;

        eventIndex = drawEventList.count() - 1;
    }

    DrawEvent& currentDrawEvent()
    {
        QTime currentTime = QTime::currentTime();

        if (drawEventCreationTime.isNull() || drawEventCreationTime.msecsTo(currentTime) > 1000)
        {
            drawEventCreationTime = currentTime;
            DrawEvent event(penWidth, penColor);
            event.path.moveTo(drawEventList.last().path.currentPosition());
            drawEventList << event;
            ++eventIndex;
        }

        return drawEventList.last();
    }

    void ensureNewDrawEvent()
    {
        drawEventCreationTime = QTime();
    }

public:

    bool             isClear;
    bool             drawing;

    int              penWidth;
    int              eventIndex;

    QColor           penColor;

    QPixmap          pixmap;

    QPoint           lastPoint;
    QTime            drawEventCreationTime;

    QCursor          drawCursor;

    QList<DrawEvent> drawEventList;
};

SketchWidget::SketchWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setWhatsThis(i18n("You simply draw here a rough sketch of what you want to find "
                      "and digiKam will displays the best matches in thumbnail view."));

    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
    setFixedSize(256, 256);
    setFocusPolicy(Qt::StrongFocus);
    slotClear();
}

SketchWidget::~SketchWidget()
{
    delete d;
}

void SketchWidget::slotClear()
{
    d->isClear    = true;
    d->eventIndex = -1;
    d->pixmap.fill(qRgb(255, 255, 255));
    d->drawEventList.clear();
    update();
    emit signalUndoRedoStateChanged(false, false);
}

bool SketchWidget::isClear() const
{
    return d->isClear;
}

void SketchWidget::setPenColor(const QColor& newColor)
{
    d->penColor = newColor;
    d->ensureNewDrawEvent();
}

QColor SketchWidget::penColor() const
{
    return d->penColor;
}

void SketchWidget::setPenWidth(int newWidth)
{
    d->penWidth = newWidth;
    updateDrawCursor();
    d->ensureNewDrawEvent();
}

int SketchWidget::penWidth() const
{
    return d->penWidth;
}

void SketchWidget::slotUndo()
{
    if (d->eventIndex == -1)
    {
        return;
    }

    d->eventIndex--;

    if (d->eventIndex == -1)
    {
        d->isClear    = true;
        d->pixmap.fill(qRgb(255, 255, 255));
        update();
        emit signalUndoRedoStateChanged(false, true);
    }
    else
    {
        replayEvents(d->eventIndex);
        emit signalSketchChanged(sketchImage());
        emit signalUndoRedoStateChanged(d->eventIndex != -1,
                                        d->eventIndex != d->drawEventList.count() - 1);
    }
}

void SketchWidget::slotRedo()
{
    if (d->eventIndex == d->drawEventList.count() - 1)
    {
        return;
    }

    d->eventIndex++;
    d->isClear = false;
    replayEvents(d->eventIndex);
    emit signalSketchChanged(sketchImage());
    emit signalUndoRedoStateChanged(d->eventIndex != -1,
                                    d->eventIndex != d->drawEventList.count() - 1);
}

void SketchWidget::replayEvents(int index)
{
    d->pixmap.fill(qRgb(255, 255, 255));

    for (int i = 0; i <= index; ++i)
    {
        const DrawEvent& drawEvent = d->drawEventList.at(i);
        drawPath(drawEvent.penWidth, drawEvent.penColor, drawEvent.path);
    }

    update();
}

void SketchWidget::sketchImageToXML(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("SketchImage"));

    for (int i = 0 ; i <= d->eventIndex ; ++i)
    {
        const DrawEvent& event = d->drawEventList.at(i);

        // Write the pen size and color
        writer.writeStartElement(QLatin1String("Path"));
        writer.writeAttribute(QLatin1String("Size"), QString::number(event.penWidth));
        writer.writeAttribute(QLatin1String("Color"), event.penColor.name());

        // Write the lines contained in the QPainterPath

        // Initial position is 0,0
        QPointF pos(0, 0);

        for (int j = 0 ; j < event.path.elementCount() ; ++j)
        {
            const QPainterPath::Element& element = event.path.elementAt(j);

            // Store begin and end point of a line, so no need to write moveTo elements to XML
            if (element.isLineTo())
            {
                QPoint begin = pos.toPoint();
                QPoint end = ((QPointF)element).toPoint();
                writer.writeStartElement(QLatin1String("Line"));
                writer.writeAttribute(QLatin1String("x1"), QString::number(begin.x()));
                writer.writeAttribute(QLatin1String("y1"), QString::number(begin.y()));
                writer.writeAttribute(QLatin1String("x2"), QString::number(end.x()));
                writer.writeAttribute(QLatin1String("y2"), QString::number(end.y()));
                writer.writeEndElement();
            }

            // Keep track of current position after this element
            // The starting point of the next element is the end point of this element
            // This handles both lineTo and moveTo elements
            pos = element;
        }

        writer.writeEndElement();
    }

    writer.writeEndElement();
}

QString SketchWidget::sketchImageToXML()
{
    QString xml;
    QXmlStreamWriter writer(&xml);
    writer.writeStartDocument();
    sketchImageToXML(writer);
    writer.writeEndDocument();
    return xml;
}

bool SketchWidget::setSketchImageFromXML(const QString& xml)
{
    QXmlStreamReader reader(xml);
    QXmlStreamReader::TokenType element;

    while (!reader.atEnd())
    {
        element = reader.readNext();

        if (element == QXmlStreamReader::StartElement &&
            reader.name() == QLatin1String("SketchImage"))
        {
            return setSketchImageFromXML(reader);
        }
    }

    return false;
}

bool SketchWidget::setSketchImageFromXML(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType element;

    // We assume that the reader is positioned at the start element for our XML
    if (!reader.isStartElement() ||
        reader.name() != QLatin1String("SketchImage"))
    {
        return false;
    }

    d->isClear = false;
    // rebuild list of drawing chunks
    d->drawEventList.clear();

    while (!reader.atEnd())
    {
        element = reader.readNext();

        if (element == QXmlStreamReader::StartElement)
        {
            // every chunk (DrawEvent) is stored as a vector path
            if (reader.name() == QLatin1String("Path"))
            {
                addPath(reader);    // recurse
            }
        }
        else if (element == QXmlStreamReader::EndElement)
        {
            // we have finished
            if (reader.name() == QLatin1String("SketchImage"))
            {
                break;
            }
        }
    }

    // set current event to the last event
    d->eventIndex = d->drawEventList.count() - 1;

    // apply events to our pixmap
    replayEvents(d->eventIndex);
    emit signalUndoRedoStateChanged(d->eventIndex != -1, false);

    return true;
}

void SketchWidget::addPath(QXmlStreamReader& reader)
{
    QXmlStreamReader::TokenType element;

    DrawEvent event;

    // Retrieve pen color and size
    QStringRef size  = reader.attributes().value(QLatin1String("Size"));
    QStringRef color = reader.attributes().value(QLatin1String("Color"));

    if (!size.isEmpty())
    {
        event.penWidth = size.toString().toInt();
    }

    if (!color.isEmpty())
    {
        event.penColor.setNamedColor(color.toString());
    }

    QPointF begin(0, 0), end(0, 0);

    while (!reader.atEnd())
    {
        element = reader.readNext();

        if (element == QXmlStreamReader::StartElement)
        {
            // The line element has four attributes, x1,y1,x2,y2
            if (reader.name() == QLatin1String("Line"))
            {
                QStringRef x1 = reader.attributes().value(QLatin1String("x1"));
                QStringRef y1 = reader.attributes().value(QLatin1String("y1"));
                QStringRef x2 = reader.attributes().value(QLatin1String("x2"));
                QStringRef y2 = reader.attributes().value(QLatin1String("y2"));

                if (!x1.isEmpty() && !y1.isEmpty())
                {
                    begin.setX(x1.toString().toInt());
                    begin.setY(y1.toString().toInt());
                }
                else
                {
                    begin = end;
                }

                if (!x2.isEmpty() && !y2.isEmpty())
                {
                    end.setX(x2.toString().toInt());
                    end.setY(y2.toString().toInt());
                }

                // move to starting point
                event.path.moveTo(begin);
                // draw line
                event.path.lineTo(end);
            }
        }
        else if (element == QXmlStreamReader::EndElement)
        {
            // we have finished
            if (reader.name() == QLatin1String("Path"))
            {
                break;
            }
        }
    }

    d->drawEventList << event;
}

QImage SketchWidget::sketchImage() const
{
    return d->pixmap.toImage();
}

void SketchWidget::setSketchImage(const QImage& image)
{
    d->isClear    = false;
    d->pixmap     = QPixmap::fromImage(image);
    d->eventIndex = -1;
    d->drawEventList.clear();
    emit signalUndoRedoStateChanged(false, false);
    update();
}

void SketchWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        if (d->isClear)
        {
            d->pixmap.fill(qRgb(255, 255, 255));
            d->isClear = false;
            update();
        }

        // sample color
        if (e->modifiers() & Qt::CTRL)
        {
            QImage img = d->pixmap.toImage();
            emit signalPenColorChanged((img.pixel(e->pos())));
            return;
        }

        d->lastPoint = e->pos();
        d->drawing   = true;
        setCursor(d->drawCursor);

        d->startDrawEvent(e->pos());
    }
}

void SketchWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (rect().contains(e->x(), e->y()))
    {
        setFocus();

        if (d->drawing || !(e->modifiers() & Qt::CTRL))
        {
            setCursor(d->drawCursor);
        }
        else
        {
            setCursor(Qt::CrossCursor);
        }

        if ((e->buttons() & Qt::LeftButton))
        {
            QPoint currentPos = e->pos();
            d->currentDrawEvent().lineTo(currentPos);
            drawLineTo(currentPos);
        }
    }
    else
    {
        unsetCursor();
        clearFocus();
    }
}

void SketchWidget::wheelEvent(QWheelEvent* e)
{
    if (rect().contains(e->x(), e->y()))
    {
        int size = d->penWidth;
        int decr = (e->modifiers() & Qt::SHIFT) ? 1 : 10;

        if (e->delta() > 0)
        {
            size += decr;
        }
        else
        {
            size -= decr;
        }

        emit signalPenSizeChanged(size);
        setCursor(d->drawCursor);
    }
}

void SketchWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && d->drawing)
    {
        QPoint currentPos = e->pos();
        d->currentDrawEvent().lineTo(currentPos);
        d->drawing = false;
        emit signalSketchChanged(sketchImage());
        emit signalUndoRedoStateChanged(true, false);
    }
}

void SketchWidget::keyPressEvent(QKeyEvent* e)
{
    QWidget::keyPressEvent(e);

    if (e->modifiers() == Qt::CTRL)
    {
        setCursor(Qt::CrossCursor);
    }
}

void SketchWidget::keyReleaseEvent(QKeyEvent* e)
{
    QWidget::keyReleaseEvent(e);

    if (e->key() == Qt::Key_Control)
    {
        setCursor(d->drawCursor);
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

void SketchWidget::drawPath(int width, const QColor& color, const QPainterPath& path)
{
    QPainter painter(&d->pixmap);
    painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(path);

    update(path.boundingRect().toRect());
    d->lastPoint = path.currentPosition().toPoint();
}

void SketchWidget::updateDrawCursor()
{
    int size = d->penWidth;

    if (size > 64)
    {
        size = 64;
    }

    if (size < 3)
    {
        size = 3;
    }

    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawEllipse(1, 1, size - 2, size - 2);

    d->drawCursor = QCursor(pix);
}

}  // namespace Digikam
