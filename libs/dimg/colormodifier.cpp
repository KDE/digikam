/* ============================================================
 * File  : colormodifier.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-03-06
 * Description : a color modifier methods for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 *
 * Includes code from gimp version 2.0  
 * LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))
#define CLAMP_0_255(x)   QMAX(QMIN(x, 255), 0)
#define CLAMP_0_65535(x) QMAX(QMIN(x, 65535), 0)
#define ROUND(x) ((int) ((x) + 0.5))

// C ansi includes.

#include <stdio.h>
#include <math.h>

// Local includes.

#include "dimg.h"
#include "colormodifier.h"

namespace Digikam
{

ColorModifier::ColorModifier()
{
    reset();
}

ColorModifier::~ColorModifier()
{
}

bool ColorModifier::modified() const
{
    return m_modified;    
}

void ColorModifier::reset()
{
    // initialize to linear mapping

    for (int i=0; i<65536; i++)
    {
        m_map16[i]     = i;
        htransfer16[i] = i;
        ltransfer16[i] = i;
        stransfer16[i] = i;
    }

    for (int i=0; i<256; i++)
    {
        m_map[i]     = i;
        htransfer[i] = i;
        ltransfer[i] = i;
        stransfer[i] = i;
    }
    
    m_modified = false;
}

//-------------------------------------------------------------------
// BCG correction methods

void ColorModifier::applyBCG(DImg& image)
{
    if (!m_modified || image.isNull())
        return;

    if (image.sixteenBit())
    {
        ushort* data = (ushort*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            data[0] = m_map16[data[0]];
            data[1] = m_map16[data[1]];
            data[2] = m_map16[data[2]];

            data += 4;
        }
    }
    else
    {
        uchar* data = (uchar*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            data[0] = m_map[data[0]];
            data[1] = m_map[data[1]];
            data[2] = m_map[data[2]];

            data += 4;
        }
    }
}

void ColorModifier::setGamma(double val)
{
    val = (val < 0.01) ? 0.01 : val;
    int val2;

    for (int i=0; i<65536; i++)
    {
        val2 = (int)(pow(((double)m_map16[i] / 65535), (1 / val)) * 65535);
        m_map16[i] = CLAMP_0_65535(val2);
    }

    for (int i=0; i<256; i++)
    {
        val2 = (int)(pow(((double)m_map[i] / 255), (1 / val)) * 255);
        m_map[i] = CLAMP_0_255(val2);
    }
    
    m_modified = true;
}

void ColorModifier::setBrightness(double v)
{
    int val = (int)(v * 65535);
    int val2;

    for (int i = 0; i < 65536; i++)
    {
        val2 = m_map16[i] + val;
        m_map16[i] = CLAMP_0_65535(val2);
    }

    val = (int)(v * 255);
    
    for (int i = 0; i < 256; i++)
    {
        val2 = m_map[i] + val;
        m_map[i] = CLAMP_0_255(val2);
    }
    
    m_modified = true;
}

void ColorModifier::setContrast(double val)
{
    int val2;

    for (int i = 0; i < 65536; i++)
    {
        val2 = (int)(((double)m_map16[i] - 32767) * val) + 32767;
        m_map16[i] = CLAMP_0_65535(val2);
    }                                 

    for (int i = 0; i < 256; i++)
    {
        val2 = (int)(((double)m_map[i] - 127) * val) + 127;
        m_map[i] = CLAMP_0_255(val2);
    }
    
    m_modified = true;
}

//-------------------------------------------------------------------
// HSL correction methods

void ColorModifier::applyHSL(DImg& image)
{
    int red, green, blue;
    
    if (!m_modified || image.isNull())
        return;

    if (image.sixteenBit())
    {
        ushort* data = (ushort*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            red   = data[2];
            green = data[1];
            blue  = data[0];
            
            rgb_to_hsl16(red, green, blue);
         
            red   = htransfer16[red];
            green = stransfer16[green];
            blue  = ltransfer16[blue];

            hsl_to_rgb16(red, green, blue);
        
            data[2] = red;
            data[1] = green;
            data[0] = blue;

            data += 4;
        }
    }
    else
    {
        uchar* data = (uchar*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            red   = data[2];
            green = data[1];
            blue  = data[0];
            
            rgb_to_hsl(red, green, blue);
         
            red   = htransfer[red];
            green = stransfer[green];
            blue  = ltransfer[blue];

            hsl_to_rgb(red, green, blue);
        
            data[2] = red;
            data[1] = green;
            data[0] = blue;

            data += 4;
        }
    }
}

void ColorModifier::setHue(double val)
{
    int value;

    for (int i = 0; i < 65536; i++)
    {
       value = (int)(val * 65535.0 / 360.0);
      
       if ((i + value) < 0)
          htransfer16[i] = 65535 + (i + value);
       else if ((i + value) > 65535)
          htransfer16[i] = i + value - 65535;
       else
          htransfer16[i] = i + value;
    }

    for (int i = 0; i < 256; i++)
    {
       value = (int)(val * 255.0 / 360.0);
      
       if ((i + value) < 0)
          htransfer[i] = 255 + (i + value);
       else if ((i + value) > 255)
          htransfer[i] = i + value - 255;
       else
          htransfer[i] = i + value;      
    }
    
    m_modified = true;
}

void ColorModifier::setSaturation(double val)
{
    int value;
    
    for (int i = 0; i < 65536; i++)
    {
       value = (int)(val * 65535.0 / 100.0);
       value = CLAMP (value, -65535, 65535);

       stransfer16[i] = CLAMP_0_65535 ((i * (65535 + value)) / 65535);
    }

    for (int i = 0; i < 256; i++)
    {
       value = (int)(val * 255.0 / 100.0);
       value = CLAMP (value, -255, 255);

       stransfer[i] = CLAMP_0_255 ((i * (255 + value)) / 255);
    }
    
    m_modified = true;
}

void ColorModifier::setLightness(double val)
{
    int value;

    for (int i = 0; i < 65536; i++)
    {
       value = (int)(val * 32767.0 / 100.0);
       value = CLAMP (value, -65535, 65535);

       if (value < 0)
          ltransfer16[i] = ((i * (65535 + value)) / 65535);
       else
          ltransfer16[i] = (i + ((65535 - i) * value) / 65535);
    }

    for (int i = 0; i < 256; i++)
    {
       value = (int)(val * 127.0 / 100.0);
       value = CLAMP (value, -255, 255);

       if (value < 0)
          ltransfer[i] = ((i * (255 + value)) / 255);
       else
          ltransfer[i] = (i + ((255 - i) * value) / 255);
    }
    
    m_modified = true;
}

int ColorModifier::hsl_value (double n1, double n2, double hue)
{
    double value;

    if (hue > 255)
	hue -= 255;
    else if (hue < 0)
	hue += 255;
    
    if (hue < 42.5)
	value = n1 + (n2 - n1) * (hue / 42.5);
    else if (hue < 127.5)
	value = n2;
    else if (hue < 170)
	value = n1 + (n2 - n1) * ((170 - hue) / 42.5);
    else
	value = n1;

    return ROUND(value * 255.0);
}

int ColorModifier::hsl_value16 (double n1, double n2, double hue)
{
    double value;

    if (hue > 65535)
	hue -= 65535;
    else if (hue < 0)
	hue += 65535;

    if (hue < 10922.5)
	value = n1 + (n2 - n1) * (hue / 42.5);
    else if (hue < 32767.5)
	value = n2;
    else if (hue < 43690)
	value = n1 + (n2 - n1) * ((43690 - hue) / 10922.5);
    else
	value = n1;

    return ROUND(value * 65535.0);
}

void ColorModifier::rgb_to_hsl (int& r, int& g, int& b)
{
    double h, s, l;
    int    min, max;
    int    delta;

    if (r > g)
    {
        max = QMAX (r, b);
        min = QMIN (g, b);
    }
    else
    {
        max = QMAX (g, b);
        min = QMIN (r, b);
    }

    l = (max + min) / 2.0;

    if (max == min)
    {
        s = 0.0;
        h = 0.0;
    }
    else
    {
        delta = (max - min);

        if (l < 128)
            s = 255 * (double) delta / (double) (max + min);
        else
            s = 255 * (double) delta / (double) (511 - max - min);

        if (r == max)
            h = (g - b) / (double) delta;
        else if (g == max)
            h = 2 + (b - r) / (double) delta;
        else
            h = 4 + (r - g) / (double) delta;

        h = h * 42.5;

        if (h < 0)
            h += 255;
        else if (h > 255)
            h -= 255;
    }

    r = ROUND (h);
    g = ROUND (s);
    b = ROUND (l);
}

void ColorModifier::rgb_to_hsl16 (int& r, int& g, int& b)
{
    double h, s, l;
    int    min, max;
    int    delta;

    if (r > g)
    {
        max = QMAX (r, b);
        min = QMIN (g, b);
    }
    else
    {
        max = QMAX (g, b);
        min = QMIN (r, b);
    }

    l = (max + min) / 2.0;

    if (max == min)
    {
        s = 0.0;
        h = 0.0;
    }
    else
    {
        delta = (max - min);

        if (l < 32768)
            s = 65535 * (double) delta / (double) (max + min);
        else
            s = 65535 * (double) delta / (double) (131071 - max - min);

        if (r == max)
            h = (g - b) / (double) delta;
        else if (g == max)
            h = 2 + (b - r) / (double) delta;
        else
            h = 4 + (r - g) / (double) delta;

        h = h * 10922.5;

        if (h < 0)
            h += 65535;
        else if (h > 65535)
            h -= 65535;
    }

    r = ROUND (h);
    g = ROUND (s);
    b = ROUND (l);
}

void ColorModifier::hsl_to_rgb (int& hue, int& saturation, int& lightness)
{
    double h, s, l;

    h = hue;
    s = saturation;
    l = lightness;

    if (s == 0)
    {
        //  achromatic case  
        hue        = (int) l;
        lightness  = (int) l;
        saturation = (int) l;
    }
    else
    {
        double m1, m2;

        if (l < 128)
            m2 = (l * (255 + s)) / 65025.0;
        else
            m2 = (l + s - (l * s) / 255.0) / 255.0;

        m1 = (l / 127.5) - m2;

        //  chromatic case  
        hue        = hsl_value (m1, m2, h + 85);
        saturation = hsl_value (m1, m2, h);
        lightness  = hsl_value (m1, m2, h - 85);
    }
}

void ColorModifier::hsl_to_rgb16 (int& hue, int& saturation, int& lightness)
{
    double h, s, l;

    h = hue;
    s = saturation;
    l = lightness;

    if (s == 0)
    {
        //  achromatic case  
        hue        = (int) l;
        lightness  = (int) l;
        saturation = (int) l;
    }
    else
    {
        double m1, m2;

        if (l < 32768)
            m2 = (l * (65535 + s)) / 65025.0;
        else
            m2 = (l + s - (l * s) / 65535.0) / 65535.0;

        m1 = (l / 127.5) - m2;

        //  chromatic case  
        hue        = hsl_value (m1, m2, h + 21760);
        saturation = hsl_value (m1, m2, h);
        lightness  = hsl_value (m1, m2, h - 21760);
    }
}

int ColorModifier::rgb_to_l (int red, int green, int blue)
{
    int min, max;

    if (red > green)
    {
        max = QMAX (red,   blue);
        min = QMIN (green, blue);
    }
    else
    {
        max = QMAX (green, blue);
        min = QMIN (red,   blue);
    }

    return ROUND ((max + min) / 2.0);
}


}  // NameSpace Digikam
