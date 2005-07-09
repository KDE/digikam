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

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "texture.h"

namespace DigikamTextureImagesPlugin
{

Texture::Texture(QImage *orgImage, QObject *parent, int blendGain, QString texturePath)
       : Digikam::ThreadedFilter(orgImage, parent, "Texture")
{ 
    m_blendGain   = blendGain;
    m_texturePath = texturePath;
    
    initFilter();
}

// This method is based on the Simulate Texture Film tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedTexture/

void Texture::filterImage(void)
{
    // Texture tile.

    int w = m_orgImage.width();
    int h = m_orgImage.height();
    kdDebug() << "Texture File:" << m_texturePath << endl;
    QImage texture(m_texturePath);
    if ( texture.isNull() ) return;
    
    m_textureImg.create(w, h, 32);
    
    for (int x = 0 ; x < w ; x+=texture.width())
       for (int y = 0 ; y < h ; y+=texture.height())
          bitBlt(&m_textureImg, x, y, &texture, 0, 0, texture.width(), texture.height(), 0);

    // Apply texture.
                        
    uint* data         = (uint*)m_orgImage.bits();
    uint* pTeData      = (uint*)m_textureImg.bits();
    uint* pOutBits     = (uint*)m_destImage.bits(); 
    uint* pTransparent = new uint[w*h];    
    memset(pTransparent, 128, w*h*sizeof(uint));

    Digikam::ImageFilters::imageData teData;    
    Digikam::ImageFilters::imageData transData;    
    Digikam::ImageFilters::imageData inData;  
    Digikam::ImageFilters::imageData outData;  
    
    register int i;
    int progress;

    // Make textured transparent layout.
    
    for (i = 0; !m_cancel && (i < w*h); i++)
        {
        // Get Alpha channel (unchanged).
        teData.raw           = pTeData[i];   
        
        // Overwrite RGB.
        teData.channel.red   = (teData.channel.red * (255 - m_blendGain) + 
                                transData.channel.red * m_blendGain) >> 8;
        teData.channel.green = (teData.channel.green * (255 - m_blendGain) + 
                                transData.channel.green * m_blendGain) >> 8;
        teData.channel.blue  = (teData.channel.blue * (255 - m_blendGain) + 
                                transData.channel.blue * m_blendGain) >> 8;
        pTeData[i]           = teData.raw; 

        // Update de progress bar in dialog.
        progress = (int) (((double)i * 50.0) / (w*h));
        
        if (progress%5 == 0)
           postProgress(progress);
        }
            
    uint tmp, tmpM;

    // Merge layout and image using overlay method.
    
    for (i = 0; !m_cancel && (i < w*h); i++)
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
        
        // Update progress bar in dialog.
        progress = (int) (50.0 + ((double)i * 50.0) / (w*h));
        
        if (progress%5 == 0)
           postProgress(progress);
        }
        
    delete [] pTransparent;
}

}  // NameSpace DigikamTextureImagesPlugin
