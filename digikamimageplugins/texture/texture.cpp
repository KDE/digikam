/* ============================================================
 * File  : texture.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Texture threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8)) 
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Local includes.

#include "texture.h"

namespace DigikamTextureImagesPlugin
{

Texture::Texture(QImage *orgImage, QObject *parent, int blendGain, QImage *textureImg)
       : Digikam::ThreadedFilter(orgImage, parent)
{ 
    m_textureImg = textureImg->copy();
    m_blendGain  = blendGain;
    m_name       = "Texture";
}

void Texture::filterImage(void)
{
    textureImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_blendGain);
}

// This method is based on the Simulate Texture Film tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedTexture/

void Texture::textureImage(uint* data, int Width, int Height, int blendGain)
{
    uint* pTeData      = (uint*)m_textureImg.bits();
    uint* pOutBits     = (uint*)m_destImage.bits(); 
    uint* pTransparent = new uint[Width*Height];    
    memset(pTransparent, 128, Width*Height*sizeof(uint));

    Digikam::ImageFilters::imageData teData;    
    Digikam::ImageFilters::imageData transData;    
    Digikam::ImageFilters::imageData inData;  
    Digikam::ImageFilters::imageData outData;  
    
    register int i = 0, h, w;

    // Make textured transparent layout.
    
    for (h = 0; !m_cancel && (h < Height); h++)
        {
        for (w = 0; !m_cancel && (w < Width); w++, i++)
            {     
            // Get Alpha channel (unchaged).
            teData.raw            = pTeData[i];   
            
            // Overwrite RGB.
            teData.channel.red   = (teData.channel.red * (255 - blendGain) + 
                                   transData.channel.red * blendGain) >> 8;
            teData.channel.green = (teData.channel.green * (255 - blendGain) + 
                                   transData.channel.green * blendGain) >> 8;
            teData.channel.blue  = (teData.channel.blue * (255 - blendGain) + 
                                   transData.channel.blue * blendGain) >> 8;
            pTeData[i]           = teData.raw; 
            }

        // Update de progress bar in dialog.
        m_eventData.starting = true;
        m_eventData.success  = false;
        m_eventData.progress = (int) (((double)h * 50.0) / Height);
        QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
        }
            
    uint tmp, tmpM;
    i = 0;

    // Merge layout and image using overlay method.
    
    for (h = 0; !m_cancel && (h < Height); h++)
        {
        for (w = 0; !m_cancel && (w < Width); w++, i++)
            {     
            inData.raw            = data[i];
            outData.raw           = pOutBits[i];
            teData.raw            = pTeData[i];
            outData.channel.red   = INT_MULT(inData.channel.red, inData.channel.red + 
                                             INT_MULT(2 * teData.channel.red, 
                                                      255 - inData.channel.red, tmpM), tmp);
            outData.channel.green = INT_MULT(inData.channel.green, inData.channel.green + 
                                             INT_MULT(2 * teData.channel.green, 
                                                      255 - inData.channel.green, tmpM), tmp);
            outData.channel.blue  = INT_MULT(inData.channel.blue, inData.channel.blue + 
                                             INT_MULT(2 * teData.channel.blue, 
                                                      255 - inData.channel.blue, tmpM), tmp);
            outData.channel.alpha = inData.channel.alpha;
            pOutBits[i]           = outData.raw;
            }
        
        // Update progress bar in dialog.
        m_eventData.starting = true;
        m_eventData.success  = false;
        m_eventData.progress = (int) (50.0 + ((double)h * 50.0) / Height);
        QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
        }
        
    delete [] pTransparent;
}

}  // NameSpace DigikamTextureImagesPlugin
