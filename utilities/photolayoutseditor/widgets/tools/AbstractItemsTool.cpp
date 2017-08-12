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

#include "AbstractItemsTool.h"
#include "AbstractPhoto.h"
#include "ToolsDockWidget.h"

using namespace PhotoLayoutsEditor;

AbstractItemsTool::AbstractItemsTool(Scene * scene, Canvas::SelectionMode selectionMode, QWidget * parent) :
    AbstractTool(scene, selectionMode, parent),
    m_photo(0)
{
}

AbstractPhoto * AbstractItemsTool::currentItem()
{
    return m_photo;
}

void AbstractItemsTool::setCurrentItem(AbstractPhoto * photo)
{
    if (m_photo == photo)
        return;
    currentItemAboutToBeChanged();
    m_photo = photo;
    setEnabled((bool)m_photo);
    currentItemChanged();
}

QPointF AbstractItemsTool::mousePosition()
{
    return m_point;
}

void AbstractItemsTool::setMousePosition(const QPointF & position)
{
    if (m_point == position)
        return;
    positionAboutToBeChanged();
    m_point = position;
    positionChanged();
}
