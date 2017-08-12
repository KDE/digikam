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

#ifndef BORDERSGROUP_H
#define BORDERSGROUP_H

#include "AbstractMovableModel.h"

#include <QPainter>
#include <QPainterPath>
#include <QDomDocument>
#include <QStyleOptionGraphicsItem>
#include "BorderDrawerInterface.h"

namespace PhotoLayoutsEditor
{
    class BordersGroupPrivate;
    class AbstractPhoto;

    class BordersGroup : public AbstractMovableModel
    {
            Q_OBJECT

            BordersGroupPrivate* d;

        public:

            BordersGroup(AbstractPhoto* graphicsItem);
            ~BordersGroup();

            QPainterPath shape();
            AbstractPhoto* graphicsItem() const;
            void paint(QPainter* painter, const QStyleOptionGraphicsItem* option);

            // Method used for model manipulation
            bool prependDrawer(BorderDrawerInterface* drawer);
            bool insertDrawer(BorderDrawerInterface* drawer, int position);
            bool appendDrawer(BorderDrawerInterface* drawer);
            BorderDrawerInterface* removeDrawer(int position);
            bool moveDrawer(int sourcePosition, int destinationPosition);

            QDomElement toSvg(QDomDocument& document);
            static BordersGroup* fromSvg(QDomElement& element, AbstractPhoto* graphicsItem);

        Q_SIGNALS:

            void drawersChanged();

        public Q_SLOTS:

            void refresh();

        protected:

            virtual QObject* item(const QModelIndex& index) const;
            virtual void setItem(QObject* graphicsItem, const QModelIndex & index);
            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
            virtual QVariant data(const QModelIndex& index, int role) const;
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
            virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
            virtual QModelIndex parent(const QModelIndex& child) const;
            virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
            virtual bool moveRowsData(int sourcePosition, int sourceCount, int destPosition);

        protected Q_SLOTS:

            void emitBordersChanged()
            {
                emit drawersChanged();
            }

        private:

            void calculateShape();
            Q_DISABLE_COPY(BordersGroup)
    };
}

#endif // BORDERSGROUP_H
