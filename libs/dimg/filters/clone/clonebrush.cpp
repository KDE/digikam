/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-06-08
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-06-08 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#include "clonebrush.moc"

// C++ includes

#include <iostream>

namespace Digikam
{

CloneBrush::CloneBrush()
{
    m_dia = 0;
}

CloneBrush::~CloneBrush()
{
}

int CloneBrush::getDia() const
{
    if(m_dia >= 0 && m_dia <= 200)
      return m_dia;
    else
      return 0;
}

QPixmap CloneBrush::getPixmap() const
{
    if(!m_brushMap.isNull())
        return m_brushMap;
    else
        return QPixmap();
}

/*
void CloneBrush::setDia(int dia)
{
    if(dia >= 0 && dia <= 200)
        m_dia = dia;
}
*/

void CloneBrush::setPixmap(const QPixmap& brushmap)
{
    if(!brushmap.isNull())
        m_brushMap = brushmap;
}

} // namespace Digikam
