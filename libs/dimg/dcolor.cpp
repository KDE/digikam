/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-12-02
 * Description : 16 bits color management class
 *
 * Copyright 2005 by Gilles Caulier
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
 * ============================================================ */

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dcolor.h"

namespace Digikam
{

DColor::DColor()
{
}

DColor::DColor(int red, int green, int blue, int alpha)
{
    m_red   = red;
    m_green = green;
    m_blue  = blue;
    m_alpha = alpha;
}

DColor::DColor(uchar *data, bool sixteenBits)
{
    if (!sixteenBits)          // 8 bits image
    {
        setBlue (data[0]);
        setGreen(data[1]);
        setRed  (data[2]);
        setAlpha(data[3]);
    }
    else                      // 16 bits image
    {
        unsigned short* data16 = (unsigned short*)data;
        setBlue (data16[0]);
        setGreen(data16[1]);
        setRed  (data16[2]);
        setAlpha(data16[3]);
    }
}

DColor::~DColor()
{
}

int DColor::red()
{
    return m_red;
}

int DColor::green()
{
    return m_green;
}

int DColor::blue()
{
    return m_blue;
}

int DColor::alpha()
{
    return m_alpha;
}

DColor& DColor::operator=(const DColor& color)
{
    m_red   = color.m_red;
    m_green = color.m_green;
    m_blue  = color.m_blue;
    m_alpha = color.m_alpha;
    return *this;
}

void DColor::setRed(int red)
{
    m_red = red;
}

void DColor::setGreen(int green)
{
    m_green = green;
}

void DColor::setBlue (int blue)
{
    m_blue = blue;
}

void DColor::setAlpha(int alpha)
{
    m_alpha = alpha;
}

}  // NameSpace Digikam
