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

#include "UndoBorderChangeCommand.h"
#include "AbstractPhoto.h"

#include <QDebug>

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

UndoBorderChangeCommand::UndoBorderChangeCommand(AbstractPhoto * photo, qreal newWidth, Qt::PenJoinStyle newCornerStyle, const QColor & newColor, QUndoCommand * parent) :
    QUndoCommand(i18n("Border changed"), parent),
    m_photo(photo),
    m_width(newWidth),
    m_corner_style(newCornerStyle),
    m_color(newColor)
{
}

void UndoBorderChangeCommand::redo()
{
    qreal tempWidth = m_photo->borderWidth();
    QColor tempColor = m_photo->borderColor();
    Qt::PenJoinStyle tempCornerStyle = m_photo->borderCornersStyle();
    m_photo->setBorderWidth(m_width);
    m_photo->setBorderColor(m_color);
    m_photo->setBorderCornersStyle(m_corner_style);
    m_width = tempWidth;
    m_color = tempColor;
    m_corner_style = tempCornerStyle;
}

void UndoBorderChangeCommand::undo()
{
    qreal tempWidth = m_photo->borderWidth();
    QColor tempColor = m_photo->borderColor();
    Qt::PenJoinStyle tempCornerStyle = m_photo->borderCornersStyle();
    m_photo->setBorderWidth(m_width);
    m_photo->setBorderColor(m_color);
    m_photo->setBorderCornersStyle(m_corner_style);
    m_width = tempWidth;
    m_color = tempColor;
    m_corner_style = tempCornerStyle;
}
