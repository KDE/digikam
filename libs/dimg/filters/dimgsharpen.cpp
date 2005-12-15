/* ============================================================
 * File  : dimgsharpen.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-17-07
 * Description : A DImgSharpen threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original sharpening filter from from gimp 2.2
 * copyright 1997-1998 Michael Sweet (mike@easysw.com)
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

void DImgSharpen::sharpenImage(uchar *data, int w, int h, bool sixteenBit, int r)
{
    if (!data || !w || !h)
    {
       kdWarning() << ("Sharpen::sharpenImage: no image data available!")
                   << endl;
       return;
    }

    if (r > 100) r = 100;
    if (r <= 0) 
    {
       m_destImage = m_orgImage;
       return;
    }
           
    // initialize the LUTs

    int fact = 100 - r;
    if (fact < 1)
        fact = 1;

    int negLUT[256];
    int posLUT[256];
    
    for (int i = 0; !m_cancel && (i < 256); i++)
    {
        posLUT[i] = 800 * i / fact;
        negLUT[i] = (4 + posLUT[i] - (i << 3)) >> 3;
    }

    uchar* dstData = m_destImage.bits();
    
    // work with four rows at one time

    uchar *src_rows[4];
    uchar *src_ptr;
    uchar *dst_row;
    int   *neg_rows[4];
    int   *neg_ptr;
    int    row;
    int    count;
    int    progress;

    int  width = w * m_orgImage.bytesDepth();

    for (row = 0; !m_cancel && (row < 4); row++)
    {
        src_rows[row] = new uchar[width];
        neg_rows[row] = new int[width];
    }       

    dst_row = new uchar[width];
    
    // Pre-load the first row for the filter...

    memcpy(src_rows[0], data, width); 

    src_ptr = src_rows[0];
    neg_ptr = neg_rows[0];

    for (int i = width; !m_cancel && (i > 0); i--)
    {
        *neg_ptr = negLUT[*src_ptr];
        src_ptr++;
        neg_ptr++;
    }

    row   = 1;
    count = 1;                                    

    for (int y = 0; !m_cancel && (y < h); y++)
    {
        // Load the next pixel row...

        if ((y + 1) < h)
        {
            
            // Check to see if our src_rows[] array is overflowing yet...

            if (count >= 3)
                count--;

            // Grab the next row...

            memcpy(src_rows[row], data + y*width, width);

            src_ptr = src_rows[row];
            neg_ptr = neg_rows[row];

            for (int i = width; !m_cancel && (i > 0); i--)
            {
                *neg_ptr = negLUT[*src_ptr];
                src_ptr++;
                neg_ptr++;
            }
            
            count++;
            row = (row + 1) & 3;
        }

        else
        {
            // No more pixels at the bottom...  Drop the oldest samples...

            count--;
        }

        // Now sharpen pixels and save the results...

        if (count == 3)
        {
            uchar* src  = src_rows[(row + 2) & 3];
            uchar* dst  = dst_row;
            int*   neg0 = neg_rows[(row + 1) & 3] + 4;
            int*   neg1 = neg_rows[(row + 2) & 3] + 4;
            int*   neg2 = neg_rows[(row + 3) & 3] + 4;
            
            // New pixel value 
            int pixel;         

            int wm = w;
            
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            wm -= 2;

            while (wm > 0)
            {
                pixel = (posLUT[*src++] - neg0[-4] - neg0[0] - neg0[4] -
                         neg1[-4] - neg1[4] -
                         neg2[-4] - neg2[0] - neg2[4]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP(pixel, 0, 255);

                pixel = (posLUT[*src++] - neg0[-3] - neg0[1] - neg0[5] -
                         neg1[-3] - neg1[5] -
                         neg2[-3] - neg2[1] - neg2[5]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP(pixel, 0, 255);

                pixel = (posLUT[*src++] - neg0[-2] - neg0[2] - neg0[6] -
                         neg1[-2] - neg1[6] -
                         neg2[-2] - neg2[2] - neg2[6]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP(pixel, 0, 255);

                *dst++ = *src++;

                neg0 += 4;
                neg1 += 4;
                neg2 += 4;
                wm --;
            }

            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            
            // Set the row...
            memcpy(dstData + y*width, dst_row, width);
        }
        else if (count == 2)
        {
            if (y == 0)
            {
                // first row 
                memcpy(dstData + y*width, src_rows[0], width);
            }
            else
            {
                // last row 
                memcpy(dstData + y*width, src_rows[(h-1) & 3], width);
            }
        }
        
    progress = (int) (((double)y * 100.0) / h);
    if ( progress%5 == 0 )
        postProgress( progress );   
    }

    // Free memory.
    
    for (row = 0; !m_cancel && (row < 4); row++)
    {
        delete [] src_rows[row];
        delete [] neg_rows[row];
    }       

    delete [] dst_row;
}

}  // NameSpace Digikam
