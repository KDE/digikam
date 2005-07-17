/* ============================================================
 * File  : gaussianblur.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Gaussian Blur algorithm copyrighted 2004 by 
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

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "imagefilters.h"
#include "gaussianblur.h"

namespace Digikam
{

GaussianBlur::GaussianBlur(QImage *orgImage, QObject *parent, int radius)
            : Digikam::ThreadedFilter(orgImage, parent, "GaussianBlur")
{ 
    m_radius = radius;
    initFilter();
}

void GaussianBlur::filterImage(void)
{
    gaussianBlurImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_radius);
}

/* Function to apply the GaussianBlur on an image
 *
 * data             => The image data in RGBA mode.  
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Radius           => blur matrix radius                                         
 *                                                                                 
 * Theory           => this is the famous gaussian blur like in photoshop or gimp.  
 */
void GaussianBlur::gaussianBlurImage(uint *data, int Width, int Height, int Radius)
{
    if (!data || !Width || !Height)
       {
       kdWarning() << ("GaussianBlur::gaussianBlurImage: no image data available!")
                   << endl;
       return;
       }

    if (Radius > 100) Radius = 100;
    if (Radius <= 0) 
       {
       m_destImage = m_orgImage;
       return;
       }
       
    // Gaussian kernel computation using the Radius parameter.
      
    int          nKSize, nCenter;
    double       x, sd, factor, lnsd, lnfactor;
    register int i, j, n, h, w;

    nKSize = 2 * Radius + 1;
    nCenter = nKSize / 2;
    int *Kernel = new int[nKSize];

    lnfactor = (4.2485 - 2.7081) / 10 * nKSize + 2.7081;
    lnsd = (0.5878 + 0.5447) / 10 * nKSize - 0.5447;
    factor = exp (lnfactor);
    sd = exp (lnsd);

    for (i = 0; !m_cancel && (i < nKSize); i++)
        {
        x = sqrt ((i - nCenter) * (i - nCenter));
        Kernel[i] = (int)(factor * exp (-0.5 * pow ((x / sd), 2)) / (sd * sqrt (2.0 * M_PI)));
        }
    
    // Now, we need to convolve the image descriptor.
    // I've worked hard here, but I think this is a very smart       
    // way to convolve an array, its very hard to explain how I reach    
    // this, but the trick here its to store the sum used by the       
    // previous pixel, so we sum with the other pixels that wasn't get.
    
    int nSumR, nSumG, nSumB, nCount, progress;
    int nKernelWidth = Radius * 2 + 1;
    Digikam::ImageFilters::imageData imagedata;
    
    uint* pOutBits = (uint*)m_destImage.bits(); 
    uint* pBlur    = new uint[Width*Height];
    
    // We need to copy our bits to blur bits
    
    memcpy (pBlur, data, Width*Height*4);       

    // We need to alloc a 2d array to help us to store the values
    
    int** arrMult = Alloc2DArray (nKernelWidth, 256);
    
    for (i = 0; !m_cancel && (i < nKernelWidth); i++)
        for (j = 0; !m_cancel && (j < 256); j++)
            arrMult[i][j] = j * Kernel[i];

    // We need to initialize all the loop and iterator variables
    
    nSumR = nSumG = nSumB = nCount = i = j = 0;

    // Now, we enter in the main loop
    
    for (h = 0; !m_cancel && (h < Height); h++)
        {
        for (w = 0; !m_cancel && (w < Width); w++, i++)
            {
            // first of all, we need to blur the horizontal lines
                
            for (n = -Radius; !m_cancel && (n <= Radius); n++)
               {
               // if is inside...
               if (IsInside (Width, Height, w + n, h))
                    {
                    // we points to the pixel
                    j = i + n;
                    
                    // finally, we sum the pixels using a method similar to assigntables
                    imagedata.raw = data[j];
                    nSumR += arrMult[n + Radius][imagedata.channel.red];
                    nSumG += arrMult[n + Radius][imagedata.channel.green];
                    nSumB += arrMult[n + Radius][imagedata.channel.blue];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to blur bits the horizontal blur values
            imagedata.channel.red   = (uchar)CLAMP (nSumR / nCount, 0, 255);
            imagedata.channel.green = (uchar)CLAMP (nSumG / nCount, 0, 255);
            imagedata.channel.blue  = (uchar)CLAMP (nSumB / nCount, 0, 255);
            pBlur[i]                = imagedata.raw;
            
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        
        progress = (int) (((double)h * 50.0) / Height);
        if ( progress%5 == 0 )
           postProgress( progress );   
        }

    // getting the blur bits, we initialize position variables
    i = j = 0;

    // We enter in the second main loop
    for (w = 0; !m_cancel && (w < Width); w++, i = w)
        {
        for (h = 0; !m_cancel && (h < Height); h++, i += Width)
            {
            // first of all, we need to blur the vertical lines
            for (n = -Radius; !m_cancel && (n <= Radius); n++)
                {
                // if is inside...
                if (IsInside(Width, Height, w, h + n))
                    {
                    // we points to the pixel
                    j = i + n * Width;
                      
                    // finally, we sum the pixels using a method similar to assigntables
                    imagedata.raw = pBlur[j];
                    nSumR += arrMult[n + Radius][imagedata.channel.red];
                    nSumG += arrMult[n + Radius][imagedata.channel.green];
                    nSumB += arrMult[n + Radius][imagedata.channel.blue];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // To preserve Alpha channel.
            imagedata.raw = data[i];
                
            // now, we return to bits the vertical blur values
            imagedata.channel.red   = (uchar)CLAMP (nSumR / nCount, 0, 255);
            imagedata.channel.green = (uchar)CLAMP (nSumG / nCount, 0, 255);
            imagedata.channel.blue  = (uchar)CLAMP (nSumB / nCount, 0, 255);
            pOutBits[i]             = imagedata.raw;
                
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        
        progress = (int) (50.0 + ((double)w * 50.0) / Width);
        if ( progress%5 == 0 )
           postProgress( progress );   
        }

    // now, we must free memory
    Free2DArray (arrMult, nKernelWidth);
    delete [] pBlur;
    delete [] Kernel;
}

}  // NameSpace Digikam
