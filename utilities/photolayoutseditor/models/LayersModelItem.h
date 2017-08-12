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

#ifndef LAYERSMOTELITEM_H
#define LAYERSMOTELITEM_H

#include <QObject>
#include <QDebug>
#include <QGraphicsItem>

namespace PhotoLayoutsEditor
{
    class AbstractPhoto;
    class LayersModel;

    class LayersModelItem : public QObject
    {
            Q_OBJECT

            static const int COLUMN_COUNT = 4;

        public:

            enum
            {
                EyeIcon = 1, PadLockIcon, NameString = COLUMN_COUNT-1, Thumbnail
            };

            LayersModelItem(AbstractPhoto * item, LayersModelItem * parent, LayersModel * model);
            virtual ~LayersModelItem();
            void removeChild(LayersModelItem * child);
            LayersModelItem * parent() const;
            void setParent(LayersModelItem * parent);
            int row() const;
            int columnCount() const;
            int childCount() const;
            LayersModelItem * child(int row) const;
            QVariant data(int column) const;
            QList<QVariant> data() const;
            bool insertChildren(int position, LayersModelItem * item);
            bool removeChildren(int position, int count);
            bool moveChildren(int sourcePosition, int count, LayersModelItem * destParent, int destPosition);
            bool setData(const QVariant & data, int type);
            void setPhoto(AbstractPhoto * photo);
            AbstractPhoto * photo() const;

        protected:

            void setData(const QList<QVariant> & data);

        protected Q_SLOTS:

            void updateData();

        private:

            void refreshZValues();

            LayersModelItem * parentItem;
            QList<LayersModelItem*> childItems;
            AbstractPhoto * itemPhoto;
            LayersModel * itemModel;
    };
}

#endif // LAYERSMOTELITEM_H
