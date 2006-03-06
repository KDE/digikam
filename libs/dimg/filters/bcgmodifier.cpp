/* ============================================================
 * File  : bcgmodifier.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2005-03-06
 * Description : a Brighness/Contrast/Gamma modifier methods
 *               for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 Gilles Caulier
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
#include "bcgmodifier.h"

namespace Digikam
{

class BCGModifierPriv
{
public:

    BCGModifierPriv()
    {
        modified      = false;
        overIndicator = false;
    }

    bool overIndicator;
    bool modified;
    
    int  map16[65536];
    int  map[256];
};

BCGModifier::BCGModifier()
{
    d = new BCGModifierPriv;    
    reset();
}

BCGModifier::~BCGModifier()
{
    delete d;
}

bool BCGModifier::modified() const
{
    return d->modified;
}

void BCGModifier::setOverIndicator(bool overIndicator)
{
    d->overIndicator = overIndicator;
}
    
void BCGModifier::reset()
{
    // initialize to linear mapping

    for (int i=0; i<65536; i++)
    {
        d->map16[i] = i;
    }

    for (int i=0; i<256; i++)
    {
        d->map[i] = i;
    }

    d->modified      = false;
    d->overIndicator = false;
}

void BCGModifier::applyBCG(DImg& image)
{
    if (!d->modified || image.isNull())
        return;

    uint size = image.width()*image.height();

    if (!image.sixteenBit())                    // 8 bits image.
    {
        uchar* data = (uchar*) image.bits();

        for (uint i=0; i<size; i++)
        {
            if (d->map[data[0]] < 0 || d->map[data[1]] < 0 || d->map[data[2]] < 0)
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
            }
            else
            {            
                data[0] = d->map[data[0]];
                data[1] = d->map[data[1]];
                data[2] = d->map[data[2]];
            }
            
            data += 4;
        }
    }
    else                                        // 16 bits image.
    {
        ushort* data = (ushort*) image.bits();

        for (uint i=0; i<size; i++)
        {
            if (d->map16[data[0]] < 0 || d->map16[data[1]] < 0 || d->map16[data[2]] < 0)
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
            }
            else
            {            
                data[0] = d->map16[data[0]];
                data[1] = d->map16[data[1]];
                data[2] = d->map16[data[2]];
            }

            data += 4;
        }
    }
}

void BCGModifier::setGamma(double val)
{
    val = (val < 0.01) ? 0.01 : val;
    int val2;

    /*
       What is the idea with setting the values when overIndicator is true?
       When the correct value is beyond the upper limit,
       we set the value to be negative.
       When the next function is called (setGamma, setBrightness, setContrast),
       it has the opportunity to correct the excess, or the value
       will again be set to its negative.
       When the correction arrays are applied, all colors with 
       negative values in the arrays will be to black.
     */

    for (int i=0; i<65536; i++)
    {
        if (d->map16[i] < 0)
            d->map16[i] =  - d->map16[i];

        val2 = lround(pow(((double)d->map16[i] / 65535.0), (1.0 / val)) * 65535.0);

        if (d->overIndicator && val2 > 65535)
            d->map16[i] =  - val2;
        else
            d->map16[i] = CLAMP_0_65535(val2);
    }

    for (int i=0; i<256; i++)
    {
        if (d->map[i] < 0)
            d->map[i] =  - d->map[i];

        val2 = lround(pow(((double)d->map[i] / 255.0), (1.0 / val)) * 255.0);

        if (d->overIndicator && val2 > 255)
            d->map[i] =  - val2;
        else
            d->map[i] = CLAMP_0_255(val2);
    }
    
    d->modified = true;
}

void BCGModifier::setBrightness(double val)
{
    int val1 = lround(val * 65535);
    int val2;

    for (int i = 0; i < 65536; i++)
    {
        if (d->map16[i] < 0)
            d->map16[i] =  - d->map16[i];

        val2 = d->map16[i] + val1;

        if (d->overIndicator && val2 > 65535)
            d->map16[i] =  - val2;
        else
            d->map16[i] = CLAMP_0_65535(val2);
    }

    val1 = lround(val * 255);
    
    for (int i = 0; i < 256; i++)
    {
        if (d->map[i] < 0)
            d->map[i] =  - d->map[i];

        val2 = d->map[i] + val1;

        if (d->overIndicator && val2 > 255)
            d->map[i] =  - val2;
        else
            d->map[i] = CLAMP_0_255(val2);
    }
    
    d->modified = true;
}

void BCGModifier::setContrast(double val)
{
    int val2;

    for (int i = 0; i < 65536; i++)
    {
        if (d->map16[i] < 0)
            d->map16[i] =  - d->map16[i];

        val2 = lround((d->map16[i] - 32767) * val) + 32767;

        if (d->overIndicator && val2 > 65535)
            d->map16[i] =  - val2;
        else
            d->map16[i] = CLAMP_0_65535(val2);
    }

    for (int i = 0; i < 256; i++)
    {
        if (d->map[i] < 0)
            d->map[i] =  - d->map[i];

        val2 = lround((d->map[i] - 127) * val) + 127;

        if (d->overIndicator && val2 > 255)
            d->map[i] =  - val2;
        else
            d->map[i] = CLAMP_0_255(val2);
    }
    
    d->modified = true;
}

}  // NameSpace Digikam
