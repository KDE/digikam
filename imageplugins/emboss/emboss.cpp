/* ============================================================
 * File  : emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-05-25
 * Description : Emboss threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
 *
 * Original Emboss algorithm copyrighted 2004 by 
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

#include "emboss.h"

namespace DigikamEmbossImagesPlugin
{

Emboss::Emboss(Digikam::DImg *orgImage, QObject *parent, int depth)
      : Digikam::DImgThreadedFilter(orgImage, parent, "Emboss")
{
    m_depth = depth;
    initFilter();
}

void Emboss::filterImage(void)
{
    embossImage(&m_orgImage, &m_destImage, m_depth);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the Emboss effect                                             
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                          
 * d                => Emboss value                                                  
 *                                                                                
 * Theory           => This is an amazing effect. And the theory is very simple to 
 *                     understand. You get the diference between the colors and    
 *                     increase it. After this, get the gray tone            
 */

void Emboss::embossImage(Digikam::DImg *orgImage, Digikam::DImg *destImage, int d)
{
    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* Bits     = destImage->bits();

    // Initial copy
    memcpy (Bits, data, destImage->numBytes());

    double Depth = d / 10.0;

    int    progress;
    int    red, green, blue, gray;
    Digikam::DColor color, colorOther;
    int    offset, offsetOther;

    for (int h = 0 ; !m_cancel && (h < Height) ; h++)
    {
        for (int w = 0 ; !m_cancel && (w < Width) ; w++)
        {
            offset = getOffset(Width, w, h, bytesDepth);
            offsetOther = getOffset(Width, w + Lim_Max (w, 1, Width), h + Lim_Max (h, 1, Height), bytesDepth);

            color.setColor(Bits + offset, sixteenBit);
            colorOther.setColor(Bits + offsetOther, sixteenBit);

            if (sixteenBit)
            {
                red   = abs ((int)((color.red()   - colorOther.red())   * Depth + 32768));
                green = abs ((int)((color.green() - colorOther.green()) * Depth + 32768));
                blue  = abs ((int)((color.blue()  - colorOther.blue())  * Depth + 32768));

                gray = CLAMP065535 ((red + green + blue) / 3);
            }
            else
            {
                red   = abs ((int)((color.red()   - colorOther.red())   * Depth + 128));
                green = abs ((int)((color.green() - colorOther.green()) * Depth + 128));
                blue  = abs ((int)((color.blue()  - colorOther.blue())  * Depth + 128));

                gray = CLAMP0255 ((red + green + blue) / 3);
            }

            // Overwrite RGB values to destination. Alpha remains unchanged.
            color.setRed(gray);
            color.setGreen(gray);
            color.setBlue(gray);
            color.setPixel(Bits + offset);
        }

        progress = (int) (((double)h * 100.0) / Height);
        if ( progress%5 == 0 )
            postProgress( progress );
    }
}

}  // NameSpace DigikamEmbossImagesPlugin
