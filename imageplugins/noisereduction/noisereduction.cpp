/* ============================================================
 * File  : noisereduction.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-25
 * Description : Noise reduction threaded image filter.
 * 
 * Copyright 2005-2006 by Gilles Caulier
 *
 * Original Despeckle algorithm copyrighted 1997-1998
 * Michael Sweet (mike at easysw dot com).
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

#define VALUE_SWAP(a,b)     { register uchar t = (a); (a) = (b); (b) = t; }
#define VALUE_SWAP16(a,b)   { register unsigned short t = (a); (a) = (b); (b) = t; }
#define POINTER_SWAP(a,b)   { register uchar* t = (a); (a) = (b); (b) = t; }
#define POINTER_SWAP16(a,b) { register unsigned short* t = (a); (a) = (b); (b) = t; }

#define RGB_INTENSITY_RED    0.30
#define RGB_INTENSITY_GREEN  0.59
#define RGB_INTENSITY_BLUE   0.11
#define RGB_INTENSITY(r,g,b) ((r) * RGB_INTENSITY_RED   + \
                              (g) * RGB_INTENSITY_GREEN + \
                              (b) * RGB_INTENSITY_BLUE)

// C++ includes.
 
#include <cmath>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "noisereduction.h"

namespace DigikamNoiseReductionImagesPlugin
{

NoiseReduction::NoiseReduction(Digikam::DImg *orgImage, QObject *parent, int radius,
                               int black_level, int white_level, bool adaptativeFilter,
                               bool recursiveFilter)
              : Digikam::DImgThreadedFilter(orgImage, parent, "NoiseReduction")
{ 
    m_radius           = radius;
    m_black_level      = black_level;
    m_white_level      = white_level;
    m_adaptativeFilter = adaptativeFilter;
    m_recursiveFilter  = recursiveFilter;
    initFilter();
}

/*
 * Despeckle an image using a median filter.
 *
 * A median filter basically collects pixel values in a region around the
 * target pixel, sorts them, and uses the median value. This code uses a
 * circular row buffer to improve performance.
 *
 * The adaptive filter is based on the median filter but analizes the histogram
 * of the region around the target pixel and adjusts the despeckle diameter
 * accordingly.
 */

void NoiseReduction::filterImage(void)
{
    if (m_orgImage.sixteenBit())
        despeckle16();
    else
        despeckle();
}

void NoiseReduction::despeckle(void)
{
    int    pos1, pos2, med, jh, jv, hist0, hist255, progress;
    uchar *pixel, max, lum, red, green, blue;
    
    uchar *src   = m_orgImage.bits();
    uchar *dst   = m_destImage.bits();
    int width    = m_destImage.width();
    int height   = m_destImage.height();
    int bpp      = 4;   // This is want mean 4 uchar values by pixel

    double prog  = 0.0;
    int diameter = (2 * m_radius) + 1;
    int box      = diameter*diameter;

    uchar **buf  = new uchar*[box];
    uchar *ibuf  = new uchar[box];

    for (int x = 0 ; x < width ; x++)
    {
        for (int y = 0 ; y < height ; y++)
        {
            hist0   = 0;
            hist255 = 0;
    
            if (x >= m_radius && y >= m_radius &&
                x + m_radius < width && y + m_radius < height)
            {
                // Make sure Svm is ininialized to a sufficient large value 
                med = -1;
    
                for (jh = x-m_radius ; jh <= x+m_radius ; jh++)
                {
                    for (jv = y-m_radius, pos1 = 0 ; jv <= y+m_radius ; jv++)
                    {
                        pos2 = (jh + (jv * width)) * bpp;

                        // We compute the luminosity to check with White and Black level.
                        blue  = src[ pos2 ];
                        green = src[pos2+1];
                        red   = src[pos2+2];
                        max = (blue > green) ? blue : green;
                        lum = (red > max) ? red : max;

                        if (lum > m_black_level && lum < m_white_level)
                        {
                            med++;
                            buf[med]  = src + pos2;
                            ibuf[med] = pixel_intensity (src + pos2);
                        }
                        else
                        {
                            if (lum > m_black_level)
                                hist0++;
    
                            if (lum >= m_white_level)
                                hist255++;
                        }
                    }
                }
    
                if (med < 1)
                {
                    pos1 = (x + ( y * width)) * bpp;
                    memcpy(dst + pos1, src + pos1, bpp);
                }
                else
                {
                    pos1 = (x + (y * width)) * bpp;
                    med  = quick_median_select (buf, ibuf, med + 1);
                    pixel = buf[med];
    
                    if (m_recursiveFilter)
                        memcpy(src + pos1, pixel, bpp);
    
                    memcpy(dst + pos1, pixel, bpp);
                }
            }
            else
            {
                pos1 = (x + (y * width)) * bpp;
                memcpy(dst + pos1, src + pos1, bpp);
            }
    
            // Check the histogram and adjust the diameter accordingly...
            
            if (m_adaptativeFilter)
            {
                if (hist0 >= m_radius || hist255 >= m_radius)
                {
                    if (m_radius < diameter / 2)
                        m_radius++;
                }
                else if (m_radius > 1)
                {
                    m_radius--;
                }
            }
        }
    
        prog += height;

        progress = (int)(((double)x * 100.0) / m_destImage.width());
        if ( progress%5 == 0 )
            postProgress( progress );
    }

    delete [] buf;
    delete [] ibuf;
}

void NoiseReduction::despeckle16(void)
{
    int             pos1, pos2, med, jh, jv, hist0, hist65535, progress;
    unsigned short *pixel, max, lum, red, green, blue;
    
    unsigned short *src = (unsigned short*)m_orgImage.bits();
    unsigned short *dst = (unsigned short*)m_destImage.bits();
    int width           = m_destImage.width();
    int height          = m_destImage.height();
    int bpp             = 4;  // This is want mean 4 ushort values by pixel

    double prog  = 0.0;
    int diameter = (2 * m_radius) + 1;
    int box      = diameter*diameter;

    unsigned short **buf = new unsigned short*[box];
    unsigned short *ibuf = new unsigned short[box];
    
    for (int x = 0 ; x < width ; x++)
    {
        for (int y = 0 ; y < height ; y++)
        {
            hist0     = 0;
            hist65535 = 0;
    
            if (x >= m_radius && y >= m_radius &&
                x + m_radius < width && y + m_radius < height)
            {
                // Make sure Svm is ininialized to a sufficient large value 
                med = -1;
    
                for (jh = x-m_radius ; jh <= x+m_radius ; jh++)
                {
                    for (jv = y-m_radius, pos1 = 0 ; jv <= y+m_radius ; jv++)
                    {
                        pos2 = (jh + (jv * width)) * bpp;
                        // We compute the luminosity to check with White and Black level.
                        blue  = src[ pos2 ];
                        green = src[pos2+1];
                        red   = src[pos2+2];
                        max = (blue > green) ? blue : green;
                        lum = (red > max) ? red : max;

                        if (lum > m_black_level && lum < m_white_level)
                        {
                            med++;
                            buf[med]  = src + pos2;
                            ibuf[med] = pixel_intensity16 (src + pos2);
                        }
                        else
                        {
                            if (lum > m_black_level)
                                hist0++;
    
                            if (lum >= m_white_level)
                                hist65535++;
                        }
                    }
                }
    
                if (med < 1)
                {
                    pos1 = (x + ( y * width)) * bpp;
                    memcpy(dst + pos1, src + pos1, 8);
                }
                else
                {
                    pos1 = (x + (y * width)) * bpp;
                    med  = quick_median_select16(buf, ibuf, med + 1);
                    pixel = buf[med];
    
                    if (m_recursiveFilter)
                        memcpy(src + pos1, pixel, 8);
    
                    memcpy(dst + pos1, pixel, 8);
                }
            }
            else
            {
                pos1 = (x + (y * width)) * bpp;
                memcpy(dst + pos1, src + pos1, 8);
            }
    
            // Check the histogram and adjust the diameter accordingly...
            
            if (m_adaptativeFilter)
            {
                if (hist0 >= m_radius || hist65535 >= m_radius)
                {
                    if (m_radius < diameter / 2)
                        m_radius++;
                }
                else if (m_radius > 1)
                {
                    m_radius--;
                }
            }
        }
    
        prog += height;

        progress = (int)(((double)x * 100.0) / m_destImage.width());
        if ( progress%5 == 0 )
            postProgress( progress );
    }

    delete [] buf;
    delete [] ibuf;
}

/*
* This Quickselect routine is based on the algorithm described in
* "Numerical recipes in C", Second Edition,
* Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
* This code by Nicolas Devillard - 1998. Public domain.
*
* modified to swap pointers: swap is done by comparing intensity value
* for the pointer to RGB
*/

int NoiseReduction::quick_median_select (uchar **p, uchar *i, int n)
{
    int low, high ;
    int median;
    int middle, ll, hh;
    
    low = 0 ;
    high = n-1 ;
    median = (low + high) / 2;
    
    for (;;)
    {
        if (high <= low) // One element only
            return median;
    
        if (high == low + 1)
        {
            // Two elements only 
            if (i[low] > i[high])
            {
                VALUE_SWAP (i[low], i[high]) ;
                POINTER_SWAP (p[low], p[high]) ;
            }
    
            return median;
        }
    
        // Find median of low, middle and high items; swap into position low 
        middle = (low + high) / 2;
    
        if (i[middle] > i[high])
        {
            VALUE_SWAP (i[middle], i[high]) ;
            POINTER_SWAP (p[middle], p[high]) ;
        }
    
        if (i[low] > i[high])
        {
            VALUE_SWAP (i[low], i[high]) ;
            POINTER_SWAP (p[low], p[high]) ;
        }
    
        if (i[middle] > i[low])
        {
            VALUE_SWAP (i[middle], i[low]) ;
            POINTER_SWAP (p[middle], p[low]) ;
        }
    
        // Swap low item (now in position middle) into position (low+1) 
        VALUE_SWAP (i[middle], i[low+1]) ;
        POINTER_SWAP (p[middle], p[low+1])
    
        // Nibble from each end towards middle, swapping items when stuck 
        ll = low + 1;
        hh = high;
    
        for (;;)
        {
            do ll++;
            while (i[low] > i[ll]);
    
            do hh--;
            while (i[hh]  > i[low]);
    
            if (hh < ll)
                break;
    
            VALUE_SWAP (i[ll], i[hh]);
            POINTER_SWAP (p[ll], p[hh]);
        }
    
        // Swap middle item (in position low) back into correct position 
        VALUE_SWAP (i[low], i[hh]);
        POINTER_SWAP (p[low], p[hh]);
    
        // Re-set active partition 
        if (hh <= median)
            low = ll;
        if (hh >= median)
            high = hh - 1;
    }
}

int NoiseReduction::quick_median_select16 (unsigned short **p, unsigned short *i, int n)
{
    int low, high ;
    int median;
    int middle, ll, hh;
    
    low = 0 ;
    high = n-1 ;
    median = (low + high) / 2;
    
    for (;;)
    {
        if (high <= low) // One element only
            return median;
    
        if (high == low + 1)
        {
            // Two elements only 
            if (i[low] > i[high])
            {
                VALUE_SWAP16 (i[low], i[high]) ;
                POINTER_SWAP16 (p[low], p[high]) ;
            }
    
            return median;
        }
    
        // Find median of low, middle and high items; swap into position low 
        middle = (low + high) / 2;
    
        if (i[middle] > i[high])
        {
            VALUE_SWAP16 (i[middle], i[high]) ;
            POINTER_SWAP16 (p[middle], p[high]) ;
        }
    
        if (i[low] > i[high])
        {
            VALUE_SWAP16 (i[low], i[high]) ;
            POINTER_SWAP16 (p[low], p[high]) ;
        }
    
        if (i[middle] > i[low])
        {
            VALUE_SWAP16 (i[middle], i[low]) ;
            POINTER_SWAP16 (p[middle], p[low]) ;
        }
    
        // Swap low item (now in position middle) into position (low+1) 
        VALUE_SWAP16 (i[middle], i[low+1]) ;
        POINTER_SWAP16 (p[middle], p[low+1])
    
        // Nibble from each end towards middle, swapping items when stuck 
        ll = low + 1;
        hh = high;
    
        for (;;)
        {
            do ll++;
            while (i[low] > i[ll]);
    
            do hh--;
            while (i[hh]  > i[low]);
    
            if (hh < ll)
                break;
    
            VALUE_SWAP16 (i[ll], i[hh]);
            POINTER_SWAP16 (p[ll], p[hh]);
        }
    
        // Swap middle item (in position low) back into correct position 
        VALUE_SWAP (i[low], i[hh]);
        POINTER_SWAP16 (p[low], p[hh]);
    
        // Re-set active partition 
        if (hh <= median)
            low = ll;
        if (hh >= median)
            high = hh - 1;
    }
}

uchar NoiseReduction::pixel_intensity(const uchar *p)
{
    return (uchar)RGB_INTENSITY (p[0], p[1], p[2]);
}

unsigned short NoiseReduction::pixel_intensity16(const unsigned short *p)
{
    return (unsigned short)RGB_INTENSITY (p[0], p[1], p[2]);
}

}  // NameSpace DigikamNoiseReductionImagesPlugin
