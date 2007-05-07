/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-18
 * Description : color modifier methods for DImg framework
 * 
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#define CLAMP_0_255(x)   QMAX(QMIN(x, 255), 0)
#define CLAMP_0_65535(x) QMAX(QMIN(x, 65535), 0)

// C++ includes.

#include <cstdio>
#include <cmath>

// Local includes.

#include "dimg.h"
#include "colormodifier.h"

namespace Digikam
{

class ColorModifierPriv
{
public:

    ColorModifierPriv()
    {
        modified      = false;
    }

    bool modified;

    int redMap[256];
    int greenMap[256];
    int blueMap[256];
    int alphaMap[256];
    
    int redMap16[65536];
    int greenMap16[65536];
    int blueMap16[65536];
    int alphaMap16[65536];
};

ColorModifier::ColorModifier()
{
    d = new ColorModifierPriv;    
    reset();
}

ColorModifier::~ColorModifier()
{
    delete d;
}

bool ColorModifier::modified() const
{
    return d->modified;
}

void ColorModifier::reset()
{
    // initialize to linear mapping

    for (int i=0; i<65536; i++)
    {
        d->redMap16[i]   = i;
        d->greenMap16[i] = i;
        d->blueMap16[i]  = i;
        d->alphaMap16[i] = i;
    }

    for (int i=0; i<256; i++)
    {
        d->redMap[i]   = i;
        d->greenMap[i] = i;
        d->blueMap[i]  = i;
        d->alphaMap[i] = i;
    }

    d->modified      = false;
}

void ColorModifier::setTables(int *redMap, int *greenMap, int *blueMap, int *alphaMap, bool sixteenBit)
{
    if (!sixteenBit)
    {
        for (int i = 0; i < 256; i++)
        {
            if (redMap)
                d->redMap[i]   = redMap[i];
            if (greenMap)
                d->greenMap[i] = greenMap[i];
            if (blueMap)
                d->blueMap[i]  = blueMap[i];
            if (alphaMap)
                d->alphaMap[i] = alphaMap[i];
        }
    }
    else
    {
        for (int i = 0; i < 65536; i++)
        {
            if (redMap)
                d->redMap16[i]   = redMap[i];
            if (greenMap)
                d->greenMap16[i] = greenMap[i];
            if (blueMap)
                d->blueMap16[i]  = blueMap[i];
            if (alphaMap)
                d->alphaMap16[i] = alphaMap[i];
        }
    }

    d->modified = true;
}

void ColorModifier::getTables(int *redMap, int *greenMap, int *blueMap, int *alphaMap, bool sixteenBit)
{
    if (!sixteenBit)
    {
        if (redMap)
            memcpy(redMap, d->redMap, (256 * sizeof(int)));
        if (greenMap)
            memcpy(greenMap, d->greenMap, (256 * sizeof(int)));
        if (blueMap)
            memcpy(blueMap, d->blueMap, (256 * sizeof(int)));
        if (alphaMap)
            memcpy(alphaMap, d->alphaMap, (256 * sizeof(int)));
    }
    else
    {
        if (redMap)
            memcpy(redMap, d->redMap16, (65536 * sizeof(int)));
        if (greenMap)
            memcpy(greenMap, d->greenMap16, (65536 * sizeof(int)));
        if (blueMap)
            memcpy(blueMap, d->blueMap16, (65536 * sizeof(int)));
        if (alphaMap)
            memcpy(alphaMap, d->alphaMap16, (65536 * sizeof(int)));
    }
}

void ColorModifier::applyColorModifier(DImg& image, double r, double g, double b, double a)
{
    if (image.isNull())
        return;

    adjustRGB(r, g, b, a, image.sixteenBit());

    if (!image.sixteenBit())                    // 8 bits image.
    {
        uchar* data = (uchar*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            data[0] = d->blueMap[data[0]];
            data[1] = d->greenMap[data[1]];
            data[2] = d->redMap[data[2]];
            data[3] = d->alphaMap[data[3]];
            
            data += 4;
        }
    }
    else                                        // 16 bits image.
    {
        ushort* data = (ushort*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            data[0] = d->blueMap16[data[0]];
            data[1] = d->greenMap16[data[1]];
            data[2] = d->redMap16[data[2]];
            data[3] = d->alphaMap16[data[3]];

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
        val2 = (int)(pow(((double)d->redMap16[i] / 65535), (1 / val)) * 65535);
        d->redMap16[i] = CLAMP_0_65535(val2);
    
        val2 = (int)(pow(((double)d->greenMap16[i] / 65535), (1 / val)) * 65535);
        d->greenMap16[i] = CLAMP_0_65535(val2);
    
        val2 = (int)(pow(((double)d->blueMap16[i] / 65535), (1 / val)) * 65535);
        d->blueMap16[i] = CLAMP_0_65535(val2);
    
        val2 = (int)(pow(((double)d->alphaMap16[i] / 65535), (1 / val)) * 65535);
        d->alphaMap16[i] = CLAMP_0_65535(val2);
    }

    for (int i=0; i<256; i++)
    {
        val2 = (int)(pow(((double)d->redMap[i] / 255), (1 / val)) * 255);
        d->redMap[i] = CLAMP_0_255(val2);
            
        val2 = (int)(pow(((double)d->greenMap[i] / 255), (1 / val)) * 255);
        d->greenMap[i] = CLAMP_0_255(val2);
            
        val2 = (int)(pow(((double)d->blueMap[i] / 255), (1 / val)) * 255);
        d->blueMap[i] = CLAMP_0_255(val2);
            
        val2 = (int)(pow(((double)d->alphaMap[i] / 255), (1 / val)) * 255);
        d->alphaMap[i] = CLAMP_0_255(val2);
    }
    
    d->modified = true;
}

void ColorModifier::adjustRGB(double r, double g, double b, double a, bool sixteenBit)
{
    int r_table[65536];
    int g_table[65536];
    int b_table[65536];
    int a_table[65536];
    int dummy_table[65536];

    if (r == 1.0 && g == 1.0 && b == 1.0 && a == 1.0)
        return ;

    if (r == g && r == b && r == a)
    {
       setGamma(r);
    }
    else
    {
       getTables(r_table, g_table, b_table, a_table, sixteenBit);

       if(r != 1.0)
       {
          setGamma(r);
          getTables(r_table, dummy_table, dummy_table, dummy_table, sixteenBit);
          reset();
       }

       if(g != 1.0)
       {
          setGamma(g);
          getTables(dummy_table, g_table, dummy_table, dummy_table, sixteenBit);
          reset();
       }

       if(b != 1.0)
       {
          setGamma(b);
          getTables(dummy_table, dummy_table, b_table, dummy_table, sixteenBit);
          reset();
       }

       if(a != 1.0)
       {
          setGamma(a);
          getTables(dummy_table, dummy_table, dummy_table, a_table, sixteenBit);
          reset();
       }

       setTables(r_table, g_table, b_table, a_table, sixteenBit);
    }
}

}  // NameSpace Digikam
