/* ============================================================
 * File  : oilpaint.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : OilPainting threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original OilPaint algorithm copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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

#include "oilpaint.h"

namespace DigikamOilPaintImagesPlugin
{

OilPaint::OilPaint(QImage *orgImage, QObject *parent, int brushSize, int smoothness)
        : Digikam::ThreadedFilter(orgImage, parent, "OilPaint")
{ 
    m_brushSize  = brushSize;
    m_smoothness = smoothness;
    initFilter();
}

void OilPaint::filterImage(void)
{
    oilpaintImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                   m_brushSize, m_smoothness);
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
    
void OilPaint::oilpaintImage(uint* data, int w, int h, int BrushSize, int Smoothness)
{
    uint* newBits = (uint*)m_destImage.bits();    
    int          i = 0, progress;
    
    for (int h2 = 0; !m_cancel && (h2 < h); h2++)
       {
       for (int w2 = 0; !m_cancel && (w2 < w); w2++)
          {
          i = h2 * w + w2;
          newBits[i] = MostFrequentColor (data, w, h, w2, h2, BrushSize, Smoothness);
          }
       
       progress = (int) (((double)h2 * 100.0) / h);
       if ( progress%5 == 0 )
          postProgress( progress );   
       }
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

uint OilPaint::MostFrequentColor (uint* Bits, int Width, int Height, int X, 
                                  int Y, int Radius, int Intensity)
{
    int  i, w, h, I;
    uint red, green, blue;
    Digikam::ImageFilters::imageData imagedata;
    
    double Scale = Intensity / 255.0;
        
    // Alloc some arrays to be used
    uchar *IntensityCount = new uchar[(Intensity + 1) * sizeof (uchar)];
    uint  *AverageColorR  = new uint[(Intensity + 1)  * sizeof (uint)];
    uint  *AverageColorG  = new uint[(Intensity + 1)  * sizeof (uint)];
    uint  *AverageColorB  = new uint[(Intensity + 1)  * sizeof (uint)];

    // Erase the array
    memset(IntensityCount, 0, (Intensity + 1) * sizeof (uchar));

    for (w = X - Radius; w <= X + Radius; w++)
        {
        for (h = Y - Radius; h <= Y + Radius; h++)
            {
            // This condition helps to identify when a point doesn't exist
            
            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
                {
                // You'll see a lot of times this formula
                i = h * Width + w;
                
                imagedata.raw = Bits[i];
                red           = (uint)imagedata.channel.red;
                green         = (uint)imagedata.channel.green;
                blue          = (uint)imagedata.channel.blue;
                       
                I = (uint)(GetIntensity (red, green, blue) * Scale);
                IntensityCount[I]++;

                if (IntensityCount[I] == 1)
                    {
                    AverageColorR[I] = red;
                    AverageColorG[I] = green;
                    AverageColorB[I] = blue;
                    }
                else
                    {
                    AverageColorR[I] += red;
                    AverageColorG[I] += green;
                    AverageColorB[I] += blue;
                    }
                }
            }
        }

    I = 0;
    int MaxInstance = 0;

    for (i = 0 ; i <= Intensity ; i++)
       {
       if (IntensityCount[i] > MaxInstance)
          {
          I = i;
          MaxInstance = IntensityCount[i];
          }
       }

    // To get Alpha channel value from original (unchanged)
    imagedata.raw = Bits[Y * Width + X];
                
    // Overwrite RGB values to destination.
    imagedata.channel.red   = AverageColorR[I] / MaxInstance;
    imagedata.channel.green = AverageColorG[I] / MaxInstance;
    imagedata.channel.blue  = AverageColorB[I] / MaxInstance;
    
    // free all the arrays
    delete [] IntensityCount;        
    delete [] AverageColorR;
    delete [] AverageColorG;
    delete [] AverageColorB;

    // return the most frequenty color
    return (imagedata.raw);   
}

}  // NameSpace DigikamOilPaintImagesPlugin
