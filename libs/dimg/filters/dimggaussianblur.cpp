/* ============================================================
 * File  : dimggaussianblur.cpp
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

#include "dimgimagefilters.h"
#include "dimggaussianblur.h"

namespace Digikam
{

DImgGaussianBlur::DImgGaussianBlur(DImg *orgImage, QObject *parent, int radius)
                : Digikam::DImgThreadedFilter(orgImage, parent, "GaussianBlur")
{ 
    m_radius = radius;
    initFilter();
}

void DImgGaussianBlur::filterImage(void)
{
    gaussianBlurImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                      m_orgImage.sixteenBit(), m_radius);
}

/** Function to apply the Gaussian Blur on an image*/

void DImgGaussianBlur::gaussianBlurImage(uchar *data, int width, int height, bool sixteenBit, int radius)
{
    if (!data || !width || !height)
    {
       kdWarning() << ("DImgGaussianBlur::gaussianBlurImage: no image data available!")
                   << endl;
       return;
    }

    if (radius > 100) radius = 100;
    if (radius <= 0)
    {
       m_destImage = m_orgImage;
       return;
    }
       
    // Gaussian kernel computation using the Radius parameter.
      
    int          nKSize, nCenter;
    double       x, sd, factor, lnsd, lnfactor;
    register int i, j, n, h, w;

    nKSize = 2 * radius + 1;
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
    int nKernelWidth = radius * 2 + 1;
    
    // We need to alloc a 2d array to help us to store the values
    
    int** arrMult = Alloc2DArray (nKernelWidth, sixteenBit ? 65536 : 256);
    
    for (i = 0; !m_cancel && (i < nKernelWidth); i++)
        for (j = 0; !m_cancel && (j < (sixteenBit ? 65536 : 256)); j++)
            arrMult[i][j] = j * Kernel[i];

    // We need to copy our bits to blur bits

    uchar* pOutBits = m_destImage.bits();
    uchar* pBlur    = new uchar[m_destImage.numBytes()];

    memcpy (pBlur, data, m_destImage.numBytes());

    // We need to initialize all the loop and iterator variables
    
    nSumR = nSumG = nSumB = nCount = i = j = 0;
    unsigned short* data16     = (unsigned short*)data;
    unsigned short* pBlur16    = (unsigned short*)pBlur;
    unsigned short* pOutBits16 = (unsigned short*)pOutBits;
    
    // Now, we enter in the main loop
    
    for (h = 0; !m_cancel && (h < height); h++)
    {
        for (w = 0; !m_cancel && (w < width); w++, i+=4)
        {
            if (!sixteenBit)        // 8 bits image.
            {
                uchar *org, *dst;
    
                // first of all, we need to blur the horizontal lines
                    
                for (n = -radius; !m_cancel && (n <= radius); n++)
                {
                    // if is inside...
                    if (IsInside (width, height, w + n, h))
                    {
                        // we points to the pixel
                        j = i + 4*n;

                        // finally, we sum the pixels using a method similar to assigntables

                        org = &data[j];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];

                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }
                    
                if (nCount == 0) nCount = 1;                    
                    
                // now, we return to blur bits the horizontal blur values
                dst    = &pBlur[i];
                dst[2] = (uchar)CLAMP (nSumR / nCount, 0, 255);
                dst[1] = (uchar)CLAMP (nSumG / nCount, 0, 255);
                dst[0] = (uchar)CLAMP (nSumB / nCount, 0, 255);
                
                // ok, now we reinitialize the variables
                nSumR = nSumG = nSumB = nCount = 0;
            }
            else                 // 16 bits image.
            {
                unsigned short *org, *dst;
    
                // first of all, we need to blur the horizontal lines
                    
                for (n = -radius; !m_cancel && (n <= radius); n++)
                {
                    // if is inside...
                    if (IsInside (width, height, w + n, h))
                    {
                        // we points to the pixel
                        j = i + 4*n;

                        // finally, we sum the pixels using a method similar to assigntables

                        org = &data16[j];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];

                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }
                    
                if (nCount == 0) nCount = 1;                    
                    
                // now, we return to blur bits the horizontal blur values
                dst    = &pBlur16[i];
                dst[2] = (unsigned short)CLAMP (nSumR / nCount, 0, 65535);
                dst[1] = (unsigned short)CLAMP (nSumG / nCount, 0, 65535);
                dst[0] = (unsigned short)CLAMP (nSumB / nCount, 0, 65535);
                
                // ok, now we reinitialize the variables
                nSumR = nSumG = nSumB = nCount = 0;
            }
        }
        
        progress = (int) (((double)h * 50.0) / height);
        if ( progress%5 == 0 )
           postProgress( progress );   
    }

    // getting the blur bits, we initialize position variables
    i = j = 0;

    // We enter in the second main loop
    for (w = 0; !m_cancel && (w < width); w++, i = w*4)
    {
        for (h = 0; !m_cancel && (h < height); h++, i += width*4)
        {
            if (!sixteenBit)        // 8 bits image.
            {
                uchar *org, *dst;

                // first of all, we need to blur the vertical lines
                for (n = -radius; !m_cancel && (n <= radius); n++)
                {
                    // if is inside...
                    if (IsInside(width, height, w, h + n))
                    {
                        // we points to the pixel
                        j = i + n * 4 * width;
                        
                        // finally, we sum the pixels using a method similar to assigntables
                        org = &pBlur[j];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];
                        
                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }
                    
                if (nCount == 0) nCount = 1;                    
                    
                // To preserve Alpha channel.
                memcpy (&pOutBits[i], &data[i], 4);
                    
                // now, we return to bits the vertical blur values
                dst    = &pOutBits[i];
                dst[2] = (uchar)CLAMP (nSumR / nCount, 0, 255);
                dst[1] = (uchar)CLAMP (nSumG / nCount, 0, 255);
                dst[0] = (uchar)CLAMP (nSumB / nCount, 0, 255);
                    
                // ok, now we reinitialize the variables
                nSumR = nSumG = nSumB = nCount = 0;
            }
            else                 // 16 bits image.
            {
                unsigned short *org, *dst;

                // first of all, we need to blur the vertical lines
                for (n = -radius; !m_cancel && (n <= radius); n++)
                {
                    // if is inside...
                    if (IsInside(width, height, w, h + n))
                    {
                        // we points to the pixel
                        j = i + n * 4 * width;
                        
                        // finally, we sum the pixels using a method similar to assigntables
                        org = &pBlur16[j];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];
                        
                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }
                    
                if (nCount == 0) nCount = 1;                    
                    
                // To preserve Alpha channel.
                memcpy (&pOutBits16[i], &data16[i], 8);
                    
                // now, we return to bits the vertical blur values
                dst    = &pOutBits16[i];
                dst[2] = (unsigned short)CLAMP (nSumR / nCount, 0, 65535);
                dst[1] = (unsigned short)CLAMP (nSumG / nCount, 0, 65535);
                dst[0] = (unsigned short)CLAMP (nSumB / nCount, 0, 65535);
                    
                // ok, now we reinitialize the variables
                nSumR = nSumG = nSumB = nCount = 0;
            }
        }
        
        progress = (int) (50.0 + ((double)w * 50.0) / width);
        if ( progress%5 == 0 )
           postProgress( progress );   
    }

    // now, we must free memory
    Free2DArray (arrMult, nKernelWidth);
    delete [] pBlur;
    delete [] Kernel;
}

}  // NameSpace Digikam
