/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : normalize image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "normalizefilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "imagehistogram.h"

namespace Digikam
{

struct NormalizeParam
{
    unsigned short* lut;
    double          min;
    double          max;
};

NormalizeFilter::NormalizeFilter(DImg* orgImage, QObject* parent)
               : DImgThreadedFilter(orgImage, parent, "NormalizeFilter")
{
    initFilter();
}

NormalizeFilter::NormalizeFilter(uchar* bits, uint width, uint height, bool sixteenBits)
               : DImgThreadedFilter()
{
    normalizeImage(bits, width, height, sixteenBits);
}

NormalizeFilter::~NormalizeFilter()
{
}

void NormalizeFilter::filterImage()
{
    normalizeImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
    m_destImage = m_orgImage;
}

/** This method scales brightness values across the active
    image so that the darkest point becomes black, and the
    brightest point becomes as bright as possible without
    altering its hue. This is often a magic fix for
    images that are dim or washed out.*/
void NormalizeFilter::normalizeImage(uchar* data, int w, int h, bool sixteenBit)
{
    NormalizeParam  param;
    int             x, i;
    unsigned short  range;

    int segments = sixteenBit ? NUM_SEGMENTS_16BIT : NUM_SEGMENTS_8BIT;

    // Memory allocation.

    param.lut = new unsigned short[segments];

    // Find min. and max. values.

    param.min = segments-1;
    param.max = 0;

    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue;
        uchar *ptr = data;

        for (i = 0 ; i < w*h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            if (red < param.min) param.min = red;
            if (red > param.max) param.max = red;

            if (green < param.min) param.min = green;
            if (green > param.max) param.max = green;

            if (blue < param.min) param.min = blue;
            if (blue > param.max) param.max = blue;

            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue;
        unsigned short *ptr = (unsigned short *)data;

        for (i = 0 ; i < w*h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            if (red < param.min) param.min = red;
            if (red > param.max) param.max = red;

            if (green < param.min) param.min = green;
            if (green > param.max) param.max = green;

            if (blue < param.min) param.min = blue;
            if (blue > param.max) param.max = blue;

            ptr += 4;
        }
    }

    // Calculate LUT.

    range = (unsigned short)(param.max - param.min);

    if (range != 0)
    {
       for (x = (int)param.min ; x <= (int)param.max ; ++x)
          param.lut[x] = (unsigned short)((segments-1) * (x - param.min) / range);
    }
    else
       param.lut[(int)param.min] = (unsigned short)param.min;

    // Apply LUT to image.

    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue;
        uchar *ptr = data;

        for (i = 0 ; i < w*h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            ptr[0] = param.lut[blue];
            ptr[1] = param.lut[green];
            ptr[2] = param.lut[red];

            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue;
        unsigned short *ptr = (unsigned short *)data;

        for (i = 0 ; i < w*h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            ptr[0] = param.lut[blue];
            ptr[1] = param.lut[green];
            ptr[2] = param.lut[red];

            ptr += 4;
        }
    }

     delete [] param.lut;
}

}  // namespace Digikam
