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

#include "AbstractPhoto_p.h"
#include "AbstractPhoto.h"

using namespace PhotoLayoutsEditor;

AbstractPhotoPrivate::AbstractPhotoPrivate(AbstractPhoto* item) :
    m_item(item),
    m_visible(true),
    m_effects_group(0),
    m_borders_group(0)
{
}

void AbstractPhotoPrivate::setCropShape(const QPainterPath& cropShape)
{
    m_crop_shape = cropShape;
    m_item->refresh();
}

QPainterPath & AbstractPhotoPrivate::cropShape()
{
    return m_crop_shape;
}

void AbstractPhotoPrivate::setName(const QString& name)
{
    if (name.isEmpty())
        return;
    this->m_name = name;
}

QString AbstractPhotoPrivate::name()
{
    return this->m_name;
}
