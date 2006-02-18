/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-12-02
 * Description : 16 bits color management class
 *
 * Copyright 2005-2006 by Gilles Caulier
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
 *
 * ============================================================ */

// C++ includes.

#include <cmath>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dcolor.h"

namespace Digikam
{

class DColorPriv
{
public:

    DColorPriv()
    {
        sixteenBit = false;
        red        = 0;
        green      = 0;
        blue       = 0;
        alpha      = 0;
    }

    bool sixteenBit;
 
    int  red;
    int  green;
    int  blue;
    int  alpha;
};

DColor::DColor()
{
    d = new DColorPriv;
}

DColor::DColor(const DColor& color)
{
    d = new DColorPriv;

    d->red        = color.d->red;
    d->green      = color.d->green;
    d->blue       = color.d->blue;
    d->alpha      = color.d->alpha;
    d->sixteenBit = color.d->sixteenBit;
}

DColor::DColor(const QColor& color, bool sixteenBit)
{
    d = new DColorPriv;

    d->red        = sixteenBit ? ((color.red()+1)*256)-1  : color.red();
    d->green      = sixteenBit ? ((color.red()+1)*256)-1  : color.green();
    d->blue       = sixteenBit ? ((color.blue()+1)*256)-1 : color.blue();
    d->alpha      = sixteenBit ? 65535 : 255;
    d->sixteenBit = sixteenBit;
}

DColor::DColor(int red, int green, int blue, int alpha, bool sixteenBit)
{
    d = new DColorPriv;

    d->sixteenBit = sixteenBit;
    d->red        = red;
    d->green      = green;
    d->blue       = blue;
    d->alpha      = alpha;
}

DColor::DColor(uchar *data, bool sixteenBit)
{
    d = new DColorPriv;

    d->sixteenBit = sixteenBit;

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
    delete d;
}

int DColor::red()
{
    return d->red;
}

int DColor::green()
{
    return d->green;
}

int DColor::blue()
{
    return d->blue;
}

int DColor::alpha()
{
    return d->alpha;
}

bool DColor::sixteenBit()
{
    return d->sixteenBit;
}

DColor& DColor::operator=(const DColor& color)
{
    d->red        = color.d->red;
    d->green      = color.d->green;
    d->blue       = color.d->blue;
    d->alpha      = color.d->alpha;
    d->sixteenBit = color.d->sixteenBit;
    return *this;
}

void DColor::setRed(int red)
{
    d->red = red;
}

void DColor::setGreen(int green)
{
    d->green = green;
}

void DColor::setBlue (int blue)
{
    d->blue = blue;
}

void DColor::setAlpha(int alpha)
{
    d->alpha = alpha;
}

void DColor::setSixteenBit(bool sixteenBit)
{
    d->sixteenBit = sixteenBit;
}

QColor DColor::getQColor()
{
    if (d->sixteenBit)
        return (QColor::QColor(((d->red+1)/256)-1, 
                ((d->green+1)/256)-1, 
                ((d->blue+1)/256)-1));
    
    return (QColor::QColor(d->red, d->green, d->blue));
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
    
    red   = d->red   / (d->sixteenBit ? 65535.0 : 255.0);
    green = d->green / (d->sixteenBit ? 65535.0 : 255.0);
    blue  = d->blue  / (d->sixteenBit ? 65535.0 : 255.0);
    
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
        d->red   = (int)(lightness * (sixteenBit ? 65535.0 : 255.0));
        d->green = (int)(lightness * (sixteenBit ? 65535.0 : 255.0));
        d->blue  = (int)(lightness * (sixteenBit ? 65535.0 : 255.0));
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
    
        d->red   = (int)(r * (sixteenBit ? 65535.0 : 255.0));
        d->green = (int)(g * (sixteenBit ? 65535.0 : 255.0));
        d->blue  = (int)(b * (sixteenBit ? 65535.0 : 255.0));
    }
 
    d->sixteenBit = sixteenBit;

    // Full transparent color.
    if (d->sixteenBit)
        d->alpha = 65535;
    else
        d->alpha = 255;
}

}  // NameSpace Digikam
