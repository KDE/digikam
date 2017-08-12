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

#ifndef ABSTRACTPHOTO_P_H
#define ABSTRACTPHOTO_P_H

#include <QIcon>
#include <QPainterPath>

namespace PhotoLayoutsEditor
{
    class BordersGroup;
    class PhotoEffectsGroup;

    class AbstractPhoto;
    class AbstractPhotoPrivate
    {
        public:

            AbstractPhotoPrivate(AbstractPhoto* item);

            // Crop shape
            void setCropShape(const QPainterPath& cropShape);
            QPainterPath& cropShape();

            void setName(const QString& name);
            QString name();

        public:

            QPainterPath       m_crop_shape;
            AbstractPhoto*     m_item;
            QString            m_name;

            // For loading purpose only
            bool               m_visible;
            QPointF            m_pos;
            QTransform         m_transform;

            mutable QString    m_id;
            PhotoEffectsGroup* m_effects_group;
            BordersGroup*      m_borders_group;

            // Icon object
            QIcon              m_icon;

            friend class AbstractPhoto;
            friend class AbstractPhotoItemLoader;
            friend class CropShapeChangeCommand;
            friend class ItemNameChangeCommand;
    };
}

#endif // ABSTRACTPHOTO_P_H
