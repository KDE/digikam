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

#ifndef LAYERSTREEDELEGATE_H
#define LAYERSTREEDELEGATE_H

#include <QStyledItemDelegate>

namespace PhotoLayoutsEditor
{
    class LayersTreeDelegate : public QStyledItemDelegate
    {

            Q_OBJECT

            QPixmap m_eye;
            QPixmap m_eye_off;
            QPixmap m_padlock;
            QPixmap m_padlock_off;

        public:

            LayersTreeDelegate(QWidget * parent = 0);
            virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
            virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

        public Q_SLOTS:

            void itemClicked(const QModelIndex & index);

        Q_SIGNALS:

            void itemRepaintNeeded(const QModelIndex & index);
    };
}

#endif // LAYERSTREEDELEGATE_H
