/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-02
 * Description : 8-16 bits color container.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dcolor.h"
#include "digikam_globals.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DColor::DColor(const QColor& color, bool sixteenBit)
{
    // initialize as eight bit
    if (color.isValid())
    {
        m_red   = color.red();
        m_green = color.green();
        m_blue  = color.blue();
    }
    else
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "QColor is invalid. reset color component to zero";
        m_red   = 0;
        m_green = 0;
        m_blue  = 0;
    }

    m_alpha      = 255;
    m_sixteenBit = false;

    // convert to sixteen bit if requested
    if (sixteenBit)
    {
        convertToSixteenBit();
    }
}

QColor DColor::getQColor() const
{
    if (m_sixteenBit)
    {
        DColor eightBit(*this);
        eightBit.convertToEightBit();
        return eightBit.getQColor();
    }

    return QColor(m_red, m_green, m_blue);
}

void DColor::convertToSixteenBit()
{
    if (m_sixteenBit)
    {
        return;
    }

    m_red        = (m_red   + 1) * 256 - 1;
    m_green      = (m_green + 1) * 256 - 1;
    m_blue       = (m_blue  + 1) * 256 - 1;
    m_alpha      = (m_alpha + 1) * 256 - 1;
    m_sixteenBit = true;
}

void DColor::convertToEightBit()
{
    if (!m_sixteenBit)
    {
        return;
    }

    m_red        = (m_red   + 1) / 256 - 1;
    m_green      = (m_green + 1) / 256 - 1;
    m_blue       = (m_blue  + 1) / 256 - 1;
    m_alpha      = (m_alpha + 1) / 256 - 1;
    m_sixteenBit = false;
}

void DColor::getHSL(int* const h, int* const s, int* const l) const
{
    double min;
    double max;
    double red;
    double green;
    double blue;
    double sum;
    double hue, sat, lig;

    double range = m_sixteenBit ? 65535.0 : 255.0;
    red          = m_red   / range;
    green        = m_green / range;
    blue         = m_blue  / range;

    if (red > green)
    {
        if (red > blue)
        {
            max = red;
        }
        else
        {
            max = blue;
        }

        if (green < blue)
        {
            min = green;
        }
        else
        {
            min = blue;
        }
    }
    else
    {
        if (green > blue)
        {
            max = green;
        }
        else
        {
            max = blue;
        }

        if (red < blue)
        {
            min = red;
        }
        else
        {
            min = blue;
        }
    }

    sum = max + min;
    lig = sum / 2;
    sat = 0;
    hue = 0;

    if (max != min)
    {
        double delta = max - min;

        if (lig <= 0.5)
        {
            sat = delta / sum;
        }
        else
        {
            sat = delta / (2 - sum);
        }

        if (red == max)
        {
            hue = (green - blue) / delta;
        }
        else if (green == max)
        {
            hue = 2 + (blue - red) / delta;
        }
        else if (blue == max)
        {
            hue = 4 + (red - green) / delta;
        }

        if (hue < 0)
        {
            hue += 6;
        }

        if (hue > 6)
        {
            hue -= 6;
        }

        hue *= 60;
    }

    *h = lround(hue * range / 360.0);
    *s = lround(sat * range);
    *l = lround(lig * range);
}

void DColor::setHSL(int h, int s, int l, bool sixteenBit)
{
    double range = m_sixteenBit ? 65535.0 : 255.0;

    if (s == 0)
    {
        m_red   = l;
        m_green = l;
        m_blue  = l;
    }
    else
    {
        double r, g, b, m2;
        double hue        = (double)(h * 360.0 / range);
        double lightness  = (double)(l         / range);
        double saturation = (double)(s         / range);

        if (lightness <= 0.5)
        {
            m2 = lightness * (1 + saturation);
        }
        else
        {
            m2 = lightness + saturation - lightness * saturation;
        }

        double m1 = 2 * lightness - m2;

        double mh;

        mh = hue + 120;

        while (mh > 360)
        {
            mh -= 360;
        }

        while (mh < 0)
        {
            mh += 360;
        }

        if (mh < 60)
        {
            r = m1 + (m2 - m1) * mh / 60;
        }
        else if (mh < 180)
        {
            r = m2;
        }
        else if (mh < 240)
        {
            r = m1 + (m2 - m1) * (240 - mh) / 60;
        }
        else
        {
            r = m1;
        }

        mh = hue;

        while (mh > 360)
        {
            mh -= 360;
        }

        while (mh < 0)
        {
            mh += 360;
        }

        if (mh < 60)
        {
            g = m1 + (m2 - m1) * mh / 60;
        }
        else if (mh < 180)
        {
            g = m2;
        }
        else if (mh < 240)
        {
            g = m1 + (m2 - m1) * (240 - mh) / 60;
        }
        else
        {
            g = m1;
        }

        mh = hue - 120;

        while (mh > 360)
        {
            mh -= 360;
        }

        while (mh < 0)
        {
            mh += 360;
        }

        if (mh < 60)
        {
            b = m1 + (m2 - m1) * mh / 60;
        }
        else if (mh < 180)
        {
            b = m2;
        }
        else if (mh < 240)
        {
            b = m1 + (m2 - m1) * (240 - mh) / 60;
        }
        else
        {
            b = m1;
        }

        m_red   = lround(r * range);
        m_green = lround(g * range);
        m_blue  = lround(b * range);
    }

    m_sixteenBit = sixteenBit;

    // Fully opaque color.
    if (m_sixteenBit)
    {
        m_alpha = 65535;
    }
    else
    {
        m_alpha = 255;
    }
}

void DColor::getYCbCr(double* const y, double* const cb, double* const cr) const
{
    double r = m_red   / (m_sixteenBit ? 65535.0 : 255.0);
    double g = m_green / (m_sixteenBit ? 65535.0 : 255.0);
    double b = m_blue  / (m_sixteenBit ? 65535.0 : 255.0);
    *y       =  0.2990 * r + 0.5870 * g + 0.1140 * b;
    *cb      = -0.1687 * r - 0.3313 * g + 0.5000 * b + 0.5;
    *cr      =  0.5000 * r - 0.4187 * g - 0.0813 * b + 0.5;
}

void DColor::setYCbCr(double y, double cb, double cr, bool sixteenBit)
{
    double r = y + 1.40200 * (cr - 0.5);
    double g = y - 0.34414 * (cb - 0.5) - 0.71414 * (cr - 0.5);
    double b = y + 1.77200 * (cb - 0.5);
    double q = (sixteenBit ? 65535.0 : 255.0);

    m_red    = CLAMP((int)lround(r * q), 0, (int)q);
    m_green  = CLAMP((int)lround(g * q), 0, (int)q);
    m_blue   = CLAMP((int)lround(b * q), 0, (int)q);

    m_sixteenBit = sixteenBit;

    // Fully opaque color.
    if (m_sixteenBit)
    {
        m_alpha = 65535;
    }
    else
    {
        m_alpha = 255;
    }
}

/*
DColor::DColor(const DColor& color)
{
    m_red        = color.m_red;
    m_green      = color.m_green;
    m_blue       = color.m_blue;
    m_alpha      = color.m_alpha;
    m_sixteenBit = color.m_sixteenBit;
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
*/

}  // namespace Digikam
