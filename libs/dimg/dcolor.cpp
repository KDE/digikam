/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-02
 * Description : 8-16 bits color container.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "ddebug.h"
#include "dcolor.h"

namespace Digikam
{

/*
DColor::DColor(const DColor& color)
{
    m_red        = color.m_red;
    m_green      = color.m_green;
    m_blue       = color.m_blue;
    m_alpha      = color.m_alpha;
    m_sixteenBit = color.m_sixteenBit;
}
*/

DColor::DColor(const QColor& color, bool sixteenBit)
{
    // initialize as eight bit
    m_red        = color.Qt::red();
    m_green      = color.Qt::green();
    m_blue       = color.Qt::blue();
    m_alpha      = 255;
    m_sixteenBit = false;

    // convert to sixteen bit if requested
    if (sixteenBit)
        convertToSixteenBit();
}

/*
DColor& DColor::operator=(const DColor& color)
{
    m_red        = color.m_red;
    m_green      = color.m_green;
    m_blue       = color.m_blue;
    m_alpha      = color.m_alpha;
    m_sixteenBit = color.m_sixteenBit;
    return *this;
}
*/

QColor DColor::getQColor() const
{
    if (m_sixteenBit)
    {
        DColor eightBit(*this);
        eightBit.convertToEightBit();
        return eightBit.getQColor();
    }

    return (QColor::QColor(m_red, m_green, m_blue));
}

void DColor::convertToSixteenBit()
{
    if (m_sixteenBit)
        return;

    m_red        = (m_red + 1) * 256 - 1;
    m_green      = (m_green + 1) * 256 - 1;
    m_blue       = (m_blue  + 1) * 256 - 1;
    m_alpha      = (m_alpha + 1) * 256 - 1;
    m_sixteenBit = true;
}

void DColor::convertToEightBit()
{
    if (!m_sixteenBit)
        return;

    m_red        = (m_red + 1) / 256 - 1;
    m_green      = (m_green + 1) / 256 - 1;
    m_blue       = (m_blue  + 1) / 256 - 1;
    m_alpha      = (m_alpha + 1) / 256 - 1;
    m_sixteenBit = false;
}


void DColor::getHSL(int* h, int* s, int* l) const
{
    double min;
    double max;
    double Qt::red;
    double Qt::green;
    double Qt::blue;
    double delta;
    double sum;
    double hue, sat, lig;

    double range = m_sixteenBit ? 65535.0 : 255.0;

    Qt::red   = m_red   / range;
    Qt::green = m_green / range;
    Qt::blue  = m_blue  / range;

    if (Qt::red > Qt::green)
    {
        if (Qt::red > Qt::blue)
            max = Qt::red;
        else
            max = Qt::blue;

        if (Qt::green < Qt::blue)
            min = Qt::green;
        else
            min = Qt::blue;
    }
    else
    {
        if (Qt::green > Qt::blue)
            max = Qt::green;
        else
            max = Qt::blue;

        if (Qt::red < Qt::blue)
            min = Qt::red;
        else
            min = Qt::blue;
    }

    sum = max + min;

    lig = sum / 2;
    sat = 0;
    hue = 0;

    if (max != min)
    {
        delta = max - min;

        if (lig <= 0.5)
            sat = delta / sum;
        else
            sat = delta / (2 - sum);

        if (Qt::red == max)
            hue = (Qt::green - Qt::blue) / delta;
        else if (Qt::green == max)
            hue = 2 + (Qt::blue - Qt::red) / delta;
        else if (Qt::blue == max)
            hue = 4 + (Qt::red - Qt::green) / delta;

        if (hue < 0)
            hue += 6;
        if (hue > 6)
            hue -= 6;

        hue *= 60;
    }

    *h = lround(hue * range / 360.0);
    *s = lround(sat * range);
    *l = lround(lig * range);
}

void DColor::setRGB(int h, int s, int l, bool sixteenBit)
{
    double hue;
    double lightness;
    double saturation;
    double m1, m2;
    double r, g, b;

    double range = m_sixteenBit ? 65535.0 : 255.0;

    if (s == 0)
    {
        m_red   = l;
        m_green = l;
        m_blue  = l;
    }
    else
    {
        hue        = (double)(h * 360.0 / range);
        lightness  = (double)(l / range);
        saturation = (double)(s / range);

        if (lightness <= 0.5)
            m2 = lightness * (1 + saturation);
        else
            m2 = lightness + saturation - lightness * saturation;

        m1 = 2 * lightness - m2;

        double mh;

        mh = hue + 120;
        while (mh > 360)
            mh -= 360;
        while (mh < 0)
            mh += 360;

        if (mh < 60)
            r = m1 + (m2 - m1) * mh / 60;
        else if (mh < 180)
            r = m2;
        else if (mh < 240)
            r = m1 + (m2 - m1) * (240 - mh) / 60;
        else
            r = m1;

        mh = hue;
        while (mh > 360)
            mh -= 360;
        while (mh < 0)
            mh += 360;

        if (mh < 60)
            g = m1 + (m2 - m1) * mh / 60;
        else if (mh < 180)
            g = m2;
        else if (mh < 240)
            g = m1 + (m2 - m1) * (240 - mh) / 60;
        else
            g = m1;

        mh = hue - 120;
        while (mh > 360)
            mh -= 360;
        while (mh < 0)
            mh += 360;

        if (mh < 60)
            b = m1 + (m2 - m1) * mh / 60;
        else if (mh < 180)
            b = m2;
        else if (mh < 240)
            b = m1 + (m2 - m1) * (240 - mh) / 60;
        else
            b = m1;

        m_red   = lround(r * range);
        m_green = lround(g * range);
        m_blue  = lround(b * range);
    }

    m_sixteenBit = sixteenBit;

    // Fully opaque color.
    if (m_sixteenBit)
        m_alpha = 65535;
    else
        m_alpha = 255;
}

}  // NameSpace Digikam
