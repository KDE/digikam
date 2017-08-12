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

#ifndef MOUSEPRESSLISTENER_H
#define MOUSEPRESSLISTENER_H

#include <QObject>
#include <QPointF>

namespace PhotoLayoutsEditor
{
    class MousePressListener : public QObject
    {
            Q_OBJECT

            QPointF press;
            QPointF release;

        public:

            MousePressListener(QObject * parent = 0) :
                QObject(parent)
            {}

            bool wasDragged()
            {
                return press != release && !press.isNull();
            }
            QPointF mousePressPosition() const
            {
                return press;
            }
            QPointF mouseReleasePosition() const
            {
                return release;
            }

        public Q_SLOTS:

            void mousePress(const QPointF & scenePos)
            {
                emit mousePressed((press = scenePos));
            }
            void mouseRelease(const QPointF & scenePos)
            {
                emit mouseReleased((release = scenePos));
            }

        Q_SIGNALS:

            void mousePressed(const QPointF & scenePos);
            void mouseReleased(const QPointF & scenePos);
    };
}

#endif // MOUSEPRESSLISTENER_H
