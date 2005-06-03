/* ============================================================
 * File  : oilpaint.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Oilpaint threaded image filter.
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
        : Digikam::ThreadedFilter(orgImage, parent)
{ 
    m_brushSize  = brushSize;
    m_smoothness = smoothness;
    m_name       = "OilPaint";
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
    int LineWidth = w * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
      
    uchar* newBits = (uchar*)m_destImage.bits();    
    int  i = 0;
    uint color;
    
    for (int h2 = 0; !m_cancel && (h2 < h); h2++)
       {
       for (int w2 = 0; !m_cancel && (w2 < w); w2++)
          {
          i = h2 * LineWidth + 4*w2;
          color = MostFrequentColor ((uchar*)data, w, h, w2, h2, BrushSize, Smoothness);
          
          newBits[i+3] = (uchar)(color >> 24);
          newBits[i+2] = (uchar)(color >> 16);
          newBits[i+1] = (uchar)(color >> 8);
          newBits[ i ] = (uchar)(color);
          }
       
       // Update de progress bar in dialog.
       m_eventData.starting = true;
       m_eventData.success  = false;
       m_eventData.progress = (int) (((double)h2 * 100.0) / h);
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
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

uint OilPaint::MostFrequentColor (uchar* Bits, int Width, int Height, int X, 
                                  int Y, int Radius, int Intensity)
{
    int i, w, h, I;
    uint color;
    
    double Scale = Intensity / 255.0;
    int LineWidth = 4 * Width;
    
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);   // Don't take off this step
        
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
                i = h * LineWidth + 4 * w;
                I = (uint)(GetIntensity (Bits[i+2], Bits[i+1], Bits[i]) * Scale);
                IntensityCount[I]++;

                if (IntensityCount[I] == 1)
                    {
                    AverageColorR[I] = Bits[i+2];
                    AverageColorG[I] = Bits[i+1];
                    AverageColorB[I] = Bits[ i ];
                    }
                else
                    {
                    AverageColorR[I] += Bits[i+2];
                    AverageColorG[I] += Bits[i+1];
                    AverageColorB[I] += Bits[ i ];
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

    int R, G, B;
    R = AverageColorR[I] / MaxInstance;
    G = AverageColorG[I] / MaxInstance;
    B = AverageColorB[I] / MaxInstance;
    color = qRgb (R, G, B);

    delete [] IntensityCount;        // free all the arrays
    delete [] AverageColorR;
    delete [] AverageColorG;
    delete [] AverageColorB;

    return (color);                    // return the most frequenty color
}

}  // NameSpace DigikamOilPaintImagesPlugin
