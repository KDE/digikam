/* ============================================================
 * File  : dimgsharpen.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-17-07
 * Description : A DImgSharpen threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Sharpen algorithm copyrighted 2004 by
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
#include "dimgsharpen.h"

namespace Digikam
{

DImgSharpen::DImgSharpen(DImg *orgImage, QObject *parent, int radius)
           : Digikam::DImgThreadedFilter(orgImage, parent, "Sharpen")
{ 
    m_radius = radius;
    initFilter();
}

void DImgSharpen::filterImage(void)
{
    sharpenImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                 m_orgImage.sixteenBit(), m_radius);
}

/** Function to apply the sharpen filter on an image*/

void DImgSharpen::sharpenImage(uchar *data, int width, int height, bool sixteenBit, int radius)
{
    double Offset = 0.0;

    if (!data || !width || !height)
    {
       kdWarning() << ("DImgSharpen::sharpenImage: no image data available!")
                   << endl;
       return;
    }

    if (radius <= 0)
    {
       m_destImage = m_orgImage;
       return;
    }

    const int Kernel[] = {-1,     -1,    -1,
                          -1,  /*radius+*/8, -1,
                          -1,     -1,    -1};
    
    double DivCoeff = (double)radius;

    int nSumR=0, nSumG=0, nSumB=0, progress;
    int nKernelSize = 9;
    int depth = /*sixteenBit ? 65536 : */256;

    // We need to alloc a 2d array to help us to store the values
    
    int** arrMult = alloc2DArray (nKernelSize, depth);
    
    for (int i = 0; !m_cancel && (i < nKernelSize); i++)
        for (int j = 0; !m_cancel && (j < depth); j++)
            arrMult[i][j] = j * Kernel[i];

    // We need to initialize all the loop and iterator variables
    
    int i = 0, j = 0;
    uchar*          pOutBits   = m_destImage.bits();
    unsigned short* data16     = (unsigned short*)data;
    unsigned short* pOutBits16 = (unsigned short*)pOutBits;
    uchar *org, *dst;
    
    // now, we enter in the main loop

    for (int h = 0; !m_cancel && (h < height); h++)
    {
        for (int w = 0; !m_cancel && (w < width); w++, i+=4)
        {
            if (!sixteenBit)        // 8 bits image.
            {
               // first of all, we need to sharp the horizontal lines
                
                for (int a = -1, k = 0 ; a <= 1 ; a++)
                {
           /*         for (int b = -1 ; b <= 1 ; b++, k++)
                    {

                        // if is inside...
                        if (IsInside (width, height, w + b, h + a))
                        
                        {
                            // we points to the pixel

                            j = SetPosition(width, w + b, h + a);

                            // finally, we sum the pixels using a method
                            // similar to assigntables
    
                            org = &data[j];
                            nSumR += arrMult[k][org[2]];
                            nSumG += arrMult[k][org[1]];
                            nSumB += arrMult[k][org[0]];
                        }
                        else
                        {
                            org = &data[i];
                            nSumR += arrMult[k][org[2]];
                            nSumG += arrMult[k][org[1]];
                            nSumB += arrMult[k][org[0]];
                        }
                    }*/
                }
                
                // now, we return to sharp bits the horizontal sharpen values

                /*dst    = &pOutBits[i];
                dst[2] = (uchar)CLAMP (nSumR / DivCoeff + Offset, 0, 255);
                dst[1] = (uchar)CLAMP (nSumG / DivCoeff + Offset, 0, 255);
                dst[0] = (uchar)CLAMP (nSumB / DivCoeff + Offset, 0, 255);*/

                // ok, now we reinitialize the variables
                nSumR = nSumG = nSumB = 0;
            }
            
        progress = (int) (((double)h * 100.0) / height);
        if ( progress%5 == 0 )
           postProgress( progress );           
        }
    }

    // now, we must free memory
    
    free2DArray (arrMult, nKernelSize);
}

}  // NameSpace Digikam
