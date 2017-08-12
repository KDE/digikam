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
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "global.h"
#include "UndoCommandEvent.h"
#include "photolayoutswindow.h"

#include <QPrinter>
#include <QQueue>

#include <QApplication>

namespace PhotoLayoutsEditor
{

QString name()
{
    return QLatin1String("ple");
}

QString uri()
{
    return QLatin1String("http://www.digikam.org/ple");
}

QString templateUri()
{
    return QLatin1String("http://www.digikam.org/ple/template");
}

void PLE_PostUndoCommand(QUndoCommand * command)
{
    PhotoLayoutsWindow::instance()->addUndoCommand(command);
}

QDomDocument pathToSvg(const QPainterPath & path)
{
    QDomDocument document;

    // If path is empty
    if (path.isEmpty())
        return document;

    // Conversion loop
    QString str_path_d;
    int elementsCount = path.elementCount();

    for (int i = 0; i < elementsCount; ++i)
    {
        QPainterPath::Element e = path.elementAt(i);
        switch (e.type)
        {
            case QPainterPath::LineToElement:
                str_path_d.append(QLatin1String("L ") + QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
            case QPainterPath::MoveToElement:
                str_path_d.append(QLatin1String("M ") + QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
            case QPainterPath::CurveToElement:
                str_path_d.append(QLatin1String("C ") + QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
            case QPainterPath::CurveToDataElement:
                str_path_d.append(QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
            default:
                Q_ASSERT(e.type == QPainterPath::CurveToDataElement ||
                         e.type == QPainterPath::CurveToElement ||
                         e.type == QPainterPath::LineToElement ||
                         e.type == QPainterPath::MoveToElement);
        }
    }
    str_path_d.append(QLatin1String("z"));

    // If path length is empty
    if (str_path_d.isEmpty())
        return document;

    // Create QDomElement
    QDomElement element = document.createElement(QLatin1String("path"));
    element.setAttribute(QLatin1String("d"), str_path_d);
    document.appendChild(element);
    return document;
}

QPainterPath pathFromSvg(const QDomElement & element)
{
    QPainterPath result;
    if (element.tagName() != QLatin1String("path"))
        return result;
    QString d = element.attribute(QLatin1String("d"));
    QStringList list = d.split(QLatin1Char(' '), QString::SkipEmptyParts);
    QStringList::const_iterator it = list.constBegin();
    QQueue<qreal> coordinates;
    QQueue<char> operations;
    while (it != list.constEnd())
    {
        if (*it == QLatin1String("M"))
            operations.enqueue('M');
        else if (*it == QLatin1String("L"))
            operations.enqueue('L');
        else if (*it == QLatin1String("C"))
            operations.enqueue('C');
        else if (*it == QLatin1String("z"))
            operations.enqueue('z');
        else
        {
            QString str = *it;
            bool isOK;
            qreal value = str.toDouble(&isOK);
            if (isOK)
                coordinates.enqueue(value);
            else
                return QPainterPath();
        }
        ++it;
    }
    qreal t1, t2, t3, t4, t5, t6;
    while (operations.count())
    {
        char opc = operations.dequeue();

        switch (opc)
        {
            case 'M':
                if (coordinates.count() < 2)
                    return QPainterPath();
                t1 = coordinates.dequeue();
                t2 = coordinates.dequeue();
                result.moveTo(t1, t2);
                break;
            case 'L':
                if (coordinates.count() < 2)
                    return QPainterPath();
                t1 = coordinates.dequeue();
                t2 = coordinates.dequeue();
                result.lineTo(t1, t2);
                break;
            case 'C':
                if (coordinates.count() < 4)
                    return QPainterPath();
                t1 = coordinates.dequeue();
                t2 = coordinates.dequeue();
                t3 = coordinates.dequeue();
                t4 = coordinates.dequeue();
                t5 = coordinates.dequeue();
                t6 = coordinates.dequeue();
                result.cubicTo(t1, t2, t3, t4, t5, t6);
                break;
            case 'z':
                result.closeSubpath();
                break;
            default:
                return QPainterPath();
        }
    }

    return result;
}

} // namespace PhotoLayoutsEditor
