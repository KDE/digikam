/* ============================================================
 * File  : hslmodifier.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-06
 * Description : Hue/Saturation/Lightness modifier methods
 *               for DImg framework
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

#include "imageiface.h"
#include "dcolor.h"
#include "dimg.h"
#include "hslmodifier.h"

namespace Digikam
{

HSLModifier::HSLModifier()
{
    reset();
}

HSLModifier::~HSLModifier()
{
}

bool HSLModifier::modified() const
{
    return m_modified;    
}

void HSLModifier::reset()
{
    // initialize to linear mapping

/*    for (int i=0; i<65536; i++)
    {
        htransfer16[i] = i;
        ltransfer16[i] = i;
        stransfer16[i] = i;
    }

    for (int i=0; i<256; i++)
    {
        htransfer[i] = i;
        ltransfer[i] = i;
        stransfer[i] = i;
    }
  */  
    m_modified = false;
}

void HSLModifier::applyHSL(DImg& image)
{
    int red, green, blue;
    
    if (!m_modified || image.isNull())
        return;

    if (image.sixteenBit())                   // 16 bits image.
    {
        ushort* data = (ushort*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
/*            DColor color(data[2], data[1], data[0], 0, image.sixteenBit());
            color.getHSL(&hue, &sat, &lig);
            color.setRGB(htransfer16[hue], stransfer16[sat], ltransfer16[lig], image.sixteenBit());
        
            data[2] = color.red();
            data[1] = color.green();
            data[0] = color.blue();*/

            red   = data[2]/256;
            green = data[1]/256;
            blue  = data[0]/256;

            rgb_to_hsl(red, green, blue);
        
            red   = htransfer[red];
            green = stransfer[green];
            blue  = ltransfer[blue];
        
            hsl_to_rgb(red, green, blue);

            data[2] = red*256;
            data[1] = green*256;
            data[0] = blue*256;    
            
            data += 4;
        }
    }
    else                                      // 8 bits image.
    {
        uchar* data = (uchar*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            /*
            DColor color(data[2], data[1], data[0], 0, image.sixteenBit());
            color.getHSL(&hue, &sat, &lig);
            color.setRGB(htransfer[hue], stransfer[sat], ltransfer[lig], image.sixteenBit());
        
            data[2] = color.red();
            data[1] = color.green();
            data[0] = color.blue();
            */

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

void HSLModifier::setHue(double val)
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

void HSLModifier::setSaturation(double val)
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

void HSLModifier::setLightness(double val)
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

int HSLModifier::hsl_value (double n1, double n2, double hue)
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

void HSLModifier::rgb_to_hsl (int& r, int& g, int& b)
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

void HSLModifier::rgb_to_hsl16 (int& r, int& g, int& b)
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

void HSLModifier::hsl_to_rgb (int& hue, int& saturation, int& lightness)
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

void HSLModifier::hsl_to_rgb16 (int& hue, int& saturation, int& lightness)
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

}  // NameSpace Digikam
