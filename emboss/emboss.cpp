/* ============================================================
 * File  : emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Emboss threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Emboss algorithm copyrighted 2004 by 
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

#include "emboss.h"

namespace DigikamEmbossImagesPlugin
{

Emboss::Emboss(QImage *orgImage, QObject *parent, int depth)
      : Digikam::ThreadedFilter(orgImage, parent, "Emboss")
{ 
    m_depth = depth;
    initFilter();
}

void Emboss::filterImage(void)
{
    embossImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_depth);
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

void Emboss::embossImage(uint* data, int Width, int Height, int d)
{
    memcpy (m_destImage.bits(), data, m_destImage.numBytes());
    
    uint *Bits = (uint*) m_destImage.bits();
    float Depth = d / 10.0;

    int    i = 0, j = 0, progress;
    int    red = 0, green = 0, blue = 0;
    uchar  gray = 0, r1, r2, g1, g2, b1, b2;
    Digikam::ImageFilters::imageData imagedata;
    
    for (int h = 0 ; !m_cancel && (h < Height) ; h++)
       {
       for (int w = 0 ; !m_cancel && (w < Width) ; w++)
           {
           i = h*Width + w;
           j = (h + Lim_Max (h, 1, Height))*Width + w + Lim_Max (w, 1, Width);

           imagedata.raw = Bits[i];
           r1            = imagedata.channel.red;
           g1            = imagedata.channel.green;
           b1            = imagedata.channel.blue;
           imagedata.raw = Bits[j];
           r2            = imagedata.channel.red;
           g2            = imagedata.channel.green;
           b2            = imagedata.channel.blue;
                               
           red   = abs ((int)((r1 - r2) * Depth + 128));
           green = abs ((int)((g1 - g2) * Depth + 128));
           blue  = abs ((int)((b1 - b2) * Depth + 128));

           gray = CLAMP0255 ((red + green + blue) / 3);
           
           // To get Alpha channel value from original (unchanged)
           imagedata.raw = Bits[i];
                
           // Overwrite RGB values to destination.
           imagedata.channel.red   = gray;
           imagedata.channel.green = gray;
           imagedata.channel.blue  = gray;
           
           Bits[i] = imagedata.raw;
           }
       
       progress = (int) (((double)h * 100.0) / Height);
       if ( progress%5 == 0 )
          postProgress( progress );   
       }
}

}  // NameSpace DigikamEmbossImagesPlugin
