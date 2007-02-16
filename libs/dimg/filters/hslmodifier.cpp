/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-06
 * Description : Hue/Saturation/Lightness image filter.
 *
 * Copyright 2005-2007 by Gilles Caulier
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

// C++ includes.

#include <cstdio>
#include <cmath>

// Local includes.

#include "ddebug.h"
#include "dcolor.h"
#include "dimg.h"
#include "hslmodifier.h"

namespace Digikam
{

class HSLModifierPriv
{
public:

    HSLModifierPriv()
    {
        modified = false;
    }

    bool modified;
    
    int  htransfer[256];
    int  ltransfer[256];
    int  stransfer[256];
    
    int  htransfer16[65536];
    int  ltransfer16[65536];
    int  stransfer16[65536];
};

HSLModifier::HSLModifier()
{
    d = new HSLModifierPriv;  
    reset();
}

HSLModifier::~HSLModifier()
{
    delete d;
}

bool HSLModifier::modified() const
{
    return d->modified;    
}

void HSLModifier::reset()
{
    // initialize to linear mapping

    for (int i=0; i<65536; i++)
    {
        d->htransfer16[i] = i;
        d->ltransfer16[i] = i;
        d->stransfer16[i] = i;
    }

    for (int i=0; i<256; i++)
    {
        d->htransfer[i] = i;
        d->ltransfer[i] = i;
        d->stransfer[i] = i;
    }
    
    d->modified = false;
}

void HSLModifier::applyHSL(DImg& image)
{
    if (!d->modified || image.isNull())
        return;

    bool sixteenBit     = image.sixteenBit();
    uint numberOfPixels = image.numPixels();

    if (sixteenBit)                   // 16 bits image.
    {
        unsigned short* data = (unsigned short*) image.bits();

        for (uint i=0; i<numberOfPixels; i++)
        {
            int hue, sat, lig;

            DColor color(data[2], data[1], data[0], 0, sixteenBit);

            // convert RGB to HSL
            color.getHSL(&hue, &sat, &lig);

            // convert HSL to RGB
            color.setRGB(d->htransfer16[hue], d->stransfer16[sat], d->ltransfer16[lig], sixteenBit);

            data[2] = color.red();
            data[1] = color.green();
            data[0] = color.blue();

            data += 4;
        }
    }
    else                                      // 8 bits image.
    {
        uchar* data = image.bits();

        for (uint i=0; i<numberOfPixels; i++)
        {
            int hue, sat, lig;

            DColor color(data[2], data[1], data[0], 0, sixteenBit);

            // convert RGB to HSL
            color.getHSL(&hue, &sat, &lig);

            // convert HSL to RGB
            color.setRGB(d->htransfer[hue], d->stransfer[sat], d->ltransfer[lig], sixteenBit);

            data[2] = color.red();
            data[1] = color.green();
            data[0] = color.blue();

            data += 4;
        }
    }
}

void HSLModifier::setHue(double val)
{
    int value;

    for (int i = 0; i < 65536; i++)
    {
       value = lround(val * 65535.0 / 360.0);
      
       if ((i + value) < 0)
          d->htransfer16[i] = 65535 + (i + value);
       else if ((i + value) > 65535)
          d->htransfer16[i] = i + value - 65535;
       else
          d->htransfer16[i] = i + value;
    }

    for (int i = 0; i < 256; i++)
    {
       value = lround(val * 255.0 / 360.0);
      
       if ((i + value) < 0)
          d->htransfer[i] = 255 + (i + value);
       else if ((i + value) > 255)
          d->htransfer[i] = i + value - 255;
       else
          d->htransfer[i] = i + value;
    }
    
    d->modified = true;
}

void HSLModifier::setSaturation(double val)
{
    val = CLAMP(val, -100.0, 100.0);
    int value;

    for (int i = 0; i < 65536; i++)
    {
        value = lround( (i * (100.0 + val)) / 100.0 );
        d->stransfer16[i] = CLAMP_0_65535(value);
    }

    for (int i = 0; i < 256; i++)
    {
        value = lround( (i * (100.0 + val)) / 100.0 );
        d->stransfer[i]  = CLAMP_0_255(value);
    }

    d->modified = true;
}

void HSLModifier::setLightness(double val)
{
    // val needs to be in that range so that the result is in the range 0..65535
    val = CLAMP(val, -100.0, 100.0);

    if (val < 0)
    {
        for (int i = 0; i < 65536; i++)
        {
            d->ltransfer16[i] = lround( (i * ( val + 100.0 )) / 100.0);
        }

        for (int i = 0; i < 256; i++)
        {
            d->ltransfer[i] = lround( (i * ( val + 100.0 )) / 100.0);
        }
    }
    else
    {
        for (int i = 0; i < 65536; i++)
        {
            d->ltransfer16[i] = lround( i * ( 1.0 - val / 100.0 )  + 65535.0 / 100.0 * val );
        }

        for (int i = 0; i < 256; i++)
        {
            d->ltransfer[i] = lround( i * ( 1.0 - val / 100.0 )  + 255.0 / 100.0 * val );
        }
    }

    d->modified = true;
}

}  // NameSpace Digikam
