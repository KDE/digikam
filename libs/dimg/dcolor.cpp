/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-12-02
 * Description : 16 bits color management class
 *
 * Copyright 2005 by Gilles Caulier
 *
 * RGB<->HLS transformation algorithms are inspired from methods
 * describe at this url :
 * http://www.paris-pc-gis.com/MI_Enviro/Colors/color_models.htm
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

// C++ includes.

#include <cmath>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dcolor.h"

namespace Digikam
{

DColor::DColor()
{
    m_sixteenBit = false;
    m_red        = 0;
    m_green      = 0;
    m_blue       = 0;
    m_alpha      = 0;
}

DColor::DColor(const DColor& color)
{
    m_red        = color.m_red;
    m_green      = color.m_green;
    m_blue       = color.m_blue;
    m_alpha      = color.m_alpha;
    m_sixteenBit = color.m_sixteenBit;
}

DColor::DColor(const QColor& color)
{
    m_red        = color.red();
    m_green      = color.green();
    m_blue       = color.blue();
    m_alpha      = 255;
    m_sixteenBit = false;
}

DColor::DColor(int red, int green, int blue, int alpha, bool sixteenBit)
{
    m_sixteenBit = sixteenBit;
    m_red        = red;
    m_green      = green;
    m_blue       = blue;
    m_alpha      = alpha;
}

DColor::DColor(uchar *data, bool sixteenBit)
{
    m_sixteenBit = sixteenBit;

    if (!sixteenBit)          // 8 bits image
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

bool DColor::sixteenBit()
{
    return m_sixteenBit;
}

DColor& DColor::operator=(const DColor& color)
{
    m_red        = color.m_red;
    m_green      = color.m_green;
    m_blue       = color.m_blue;
    m_alpha      = color.m_alpha;
    m_sixteenBit = color.m_sixteenBit;
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

void DColor::setSixteenBit(bool sixteenBit)
{
    m_sixteenBit = sixteenBit;
}

QColor DColor::getQColor()
{
    if (m_sixteenBit)
        return (QColor::QColor(m_red/65535, m_green/65535, m_blue/65535));
    
    return (QColor::QColor(m_red, m_green, m_blue));
}

void DColor::getHSL(int* h, int* s, int* l)
{
    double min;
    double max;
    double red;
    double green;
    double blue;
    double delta;
    double hue, sat, lig;
    
    red   = m_red   / (m_sixteenBit ? 65535.0 : 255.0);
    green = m_green / (m_sixteenBit ? 65535.0 : 255.0);
    blue  = m_blue  / (m_sixteenBit ? 65535.0 : 255.0);
    
    if (red > green)
    {
        if (red > blue)
            max = red;
        else
            max = blue;
    
        if (green < blue)
            min = green;
        else
            min = blue;
    }
    else
    {
        if (green > blue)
            max = green;
        else
            max = blue;
    
        if (red < blue)
            min = red;
        else
            min = blue;
    }
    
    lig = (max + min) / 2;
    sat = 0;
    hue = 0;
    
    if (max != min)
    {
        if (lig <= 0.5)
            sat = (max - min) / (max + min);
        else
            sat = (max - min) / (2 - max - min);
    
        delta = max -min;
        if (red == max)
            hue = (green - blue) / delta;
        else if (green == max)
            hue = 2 + (blue - red) / delta;
        else if (blue == max)
            hue = 4 + (red - green) / delta;
    
        hue *= 60;
        if (hue < 0.0)
            hue += 360;
    }

    *h = (int)(hue / 360.0 * 255.0);
    *s = (int)(sat * 255.0);
    *l = (int)(lig * 255.0);
}

void DColor::setRGB(int h, int s, int l, bool sixteenBit)
{
    double hue;
    double lightness;
    double saturation;  
    double m1, m2;
    double r, g, b;
    
    hue        = (double)(h * 360.0 / 255.0);
    lightness  = (double)(l / 255.0);
    saturation = (double)(s / 255.0);
    
    if (lightness <= 0.5)
        m2 = lightness * (1 + saturation);
    else
        m2 = lightness + saturation - lightness * saturation;

    m1 = 2 * lightness - m2;
    
    if (saturation == 0)
    {
        m_red   = (int)(lightness * (sixteenBit ? 65535.0 : 255.0));
        m_green = (int)(lightness * (sixteenBit ? 65535.0 : 255.0));
        m_blue  = (int)(lightness * (sixteenBit ? 65535.0 : 255.0));
    }
    else
    {
        hue = h + 120;
        while (hue > 360)
            hue -= 360;
        while (hue < 0)
            hue += 360;
    
        if (hue < 60)
            r = m1 + (m2 - m1) * hue / 60;
        else if (hue < 180)
            r = m2;
        else if (hue < 240)
            r = m1 + (m2 - m1) * (240 - hue) / 60;
        else
            r = m1;
    
        hue = h;
        while (hue > 360)
            hue -= 360;
        while (hue < 0)
            hue += 360;
    
        if (hue < 60)
            g = m1 + (m2 - m1) * hue / 60;
        else if (hue < 180)
            g = m2;
        else if (hue < 240)
            g = m1 + (m2 - m1) * (240 - hue) / 60;
        else
            g = m1;
    
        hue = h - 120;
        while (hue > 360)
            hue -= 360;
        while (hue < 0)
            hue += 360;
    
        if (hue < 60)
            b = m1 + (m2 - m1) * hue / 60;
        else if (hue < 180)
            b = m2;
        else if (hue < 240)
            b = m1 + (m2 - m1) * (240 - hue) / 60;
        else
            b = m1;
    
        m_red   = (int)(r * (sixteenBit ? 65535.0 : 255.0));
        m_green = (int)(g * (sixteenBit ? 65535.0 : 255.0));
        m_blue  = (int)(b * (sixteenBit ? 65535.0 : 255.0));
    }
 
    m_sixteenBit = sixteenBit;

    // Full transparent color.
    if (m_sixteenBit)
        m_alpha = 65535;
    else
        m_alpha = 255;
}

}  // NameSpace Digikam
