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

#ifndef SCALINGWIDGETITEM_H
#define SCALINGWIDGETITEM_H

#include "AbstractItemInterface.h"

namespace PhotoLayoutsEditor
{
    class AbstractPhoto;
    class ScalingWidgetItemPrivate;

    class MoveItemCommand;
    class ScaleItemCommand;

    class ScalingWidgetItem : public AbstractItemInterface
    {
            Q_OBJECT

            ScalingWidgetItemPrivate * d;

        public:

            explicit ScalingWidgetItem(const QList<AbstractPhoto*> & items, QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);
            virtual ~ScalingWidgetItem();

            virtual QRectF boundingRect() const;
            virtual QPainterPath opaqueArea() const;
            virtual QPainterPath shape() const;
            virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

            virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

        private:

            void setScaleItems(const QList<AbstractPhoto*> & items);

        private Q_SLOTS:

            void updateShapes();

        friend class ScalingWidgetItemPrivate;

        friend class MoveItemCommand;
        friend class ScaleItemCommand;
    };
}

#endif // SCALINGWIDGETITEM_H
