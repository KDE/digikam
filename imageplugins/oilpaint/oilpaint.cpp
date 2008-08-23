/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Oil Painting threaded image filter.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * 
 * Original OilPaint algorithm copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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
#include <cstdlib>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dimggaussianblur.h"
#include "dimgimagefilters.h"
#include "oilpaint.h"

namespace DigikamOilPaintImagesPlugin
{

OilPaint::OilPaint(Digikam::DImg *orgImage, QObject *parent, int brushSize, int smoothness)
        : Digikam::DImgThreadedFilter(orgImage, parent, "OilPaint")
{
    m_brushSize  = brushSize;
    m_smoothness = smoothness;
    initFilter();
}

void OilPaint::filterImage(void)
{
    oilpaintImage(m_orgImage, m_destImage, m_brushSize, m_smoothness);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the OilPaint effect.                
 *                                                                                    
 * data             => The image data in RGBA mode.                            
 * w                => Width of image.                          
 * h                => Height of image.                          
 * BrushSize        => Brush size.
 * Smoothness       => Smooth value.                                                
 *                                                                                  
 * Theory           => Using MostFrequentColor function we take the main color in  
 *                     a matrix and simply write at the original position.            
 */                                                                                 
    
void OilPaint::oilpaintImage(Digikam::DImg &orgImage, Digikam::DImg &destImage, int BrushSize, int Smoothness)
{
    int    progress;
    Digikam::DColor mostFrequentColor;
    int    w,h;

    mostFrequentColor.setSixteenBit(orgImage.sixteenBit());
    w = (int)orgImage.width();
    h = (int)orgImage.height();
    uchar *dest    = destImage.bits();
    int bytesDepth = orgImage.bytesDepth();
    uchar *dptr;

    // Allocate some arrays to be used.
    // Do this here once for all to save a few million new / delete operations
    m_intensityCount = new uchar[Smoothness + 1];
    m_averageColorR  = new uint[Smoothness + 1];
    m_averageColorG  = new uint[Smoothness + 1];
    m_averageColorB  = new uint[Smoothness + 1];

    for (int h2 = 0; !m_cancel && (h2 < h); h2++)
    {
        for (int w2 = 0; !m_cancel && (w2 < w); w2++)
        {
            mostFrequentColor = MostFrequentColor(orgImage, w2, h2, BrushSize, Smoothness);
            dptr = dest + w2*bytesDepth + (w*h2*bytesDepth);
            mostFrequentColor.setPixel(dptr);
        }

        progress = (int) (((double)h2 * 100.0) / h);
        if ( progress%5 == 0 )
            postProgress( progress );
    }

    // free all the arrays
    delete [] m_intensityCount;
    delete [] m_averageColorR;
    delete [] m_averageColorG;
    delete [] m_averageColorB;
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to determine the most frequent color in a matrix                        
 *                                                                                
 * Bits             => Bits array                                                    
 * Width            => Image width                                                   
 * Height           => Image height                                                 
 * X                => Position horizontal                                           
 * Y                => Position vertical                                            
 * Radius           => Is the radius of the matrix to be analized                  
 * Intensity        => Intensity to calcule                                         
 *                                                                                  
 * Theory           => This function creates a matrix with the analized pixel in   
 *                     the center of this matrix and find the most frequenty color   
 */

Digikam::DColor OilPaint::MostFrequentColor(Digikam::DImg &src, int X, int Y, int Radius, int Intensity)
{
    int  i, w, h, I, Width, Height;
    uint red, green, blue;

    uchar *dest = src.bits();
    int bytesDepth = src.bytesDepth();
    uchar *sptr;
    bool sixteenBit = src.sixteenBit();

    Digikam::DColor mostFrequentColor;

    double Scale = Intensity / (sixteenBit ? 65535.0 : 255.0);
    Width  = (int)src.width();
    Height = (int)src.height();

    // Erase the array
    memset(m_intensityCount, 0, (Intensity + 1) * sizeof (uchar));

    for (w = X - Radius; w <= X + Radius; w++)
    {
        for (h = Y - Radius; h <= Y + Radius; h++)
        {
            // This condition helps to identify when a point doesn't exist

            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
            {
                sptr = dest + w*bytesDepth + (Width*h*bytesDepth);
                Digikam::DColor color(sptr, sixteenBit);
                red           = (uint)color.red();
                green         = (uint)color.green();
                blue          = (uint)color.blue();

                I = lround(GetIntensity (red, green, blue) * Scale);
                m_intensityCount[I]++;

                if (m_intensityCount[I] == 1)
                {
                    m_averageColorR[I] = red;
                    m_averageColorG[I] = green;
                    m_averageColorB[I] = blue;
                }
                else
                {
                    m_averageColorR[I] += red;
                    m_averageColorG[I] += green;
                    m_averageColorB[I] += blue;
                }
            }
        }
    }

    I = 0;
    int MaxInstance = 0;

    for (i = 0 ; i <= Intensity ; i++)
    {
        if (m_intensityCount[i] > MaxInstance)
        {
            I = i;
            MaxInstance = m_intensityCount[i];
        }
    }

    // get Alpha channel value from original (unchanged)
    mostFrequentColor = src.getPixelColor(X, Y);

    // Overwrite RGB values to destination.
    mostFrequentColor.setRed(m_averageColorR[I] / MaxInstance);
    mostFrequentColor.setGreen(m_averageColorG[I] / MaxInstance);
    mostFrequentColor.setBlue(m_averageColorB[I] / MaxInstance);

    return mostFrequentColor;
}

}  // NameSpace DigikamOilPaintImagesPlugin
