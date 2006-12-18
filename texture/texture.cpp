/* ============================================================
 * File  : texture.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-05-25
 * Description : Texture threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

#include "texture.h"

namespace DigikamTextureImagesPlugin
{

Texture::Texture(Digikam::DImg *orgImage, QObject *parent, int blendGain, QString texturePath)
       : Digikam::DImgThreadedFilter(orgImage, parent, "Texture")
{
    m_blendGain   = blendGain;
    m_texturePath = texturePath;

    initFilter();
}

// This method is based on the Simulate Texture Film tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedTexture/

//#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ( ( (t >> 8) + t ) >> 8))

inline static int intMult8(uint a, uint b)
{
    uint t = a * b + 0x80;
    return ((t >> 8) + t) >> 8;
}

inline static int intMult16(uint a, uint b)
{
    uint t = a * b + 0x8000;
    return ((t >> 16) + t) >> 16;
}

void Texture::filterImage(void)
{
    // Texture tile.

    int w = m_orgImage.width();
    int h = m_orgImage.height();
    int bytesDepth  = m_orgImage.bytesDepth();
    bool sixteenBit = m_orgImage.sixteenBit();

    DDebug() << "Texture File: " << m_texturePath << endl;
    Digikam::DImg texture(m_texturePath);
    if ( texture.isNull() ) return;

    Digikam::DImg textureImg(w, h, m_orgImage.sixteenBit(), m_orgImage.hasAlpha());

    texture.convertToDepthOfImage(&textureImg);

    for (int x = 0 ; x < w ; x+=texture.width())
        for (int y = 0 ; y < h ; y+=texture.height())
            textureImg.bitBltImage(&texture, x, y);

    // Apply texture.

    uchar* data         = m_orgImage.bits();
    uchar* pTeData      = textureImg.bits();
    uchar* pOutBits     = m_destImage.bits();
    uint offset;

    Digikam::DColor teData, transData, inData, outData;
    uchar *ptr, *dptr, *tptr;
    int progress;

    int blendGain;
    if (sixteenBit)
        blendGain = (m_blendGain + 1) * 256 - 1;
    else
        blendGain = m_blendGain;

    // Make textured transparent layout.

    for (int x = 0; !m_cancel && x < w; x++)
    {
        for (int y = 0; !m_cancel && y < h; y++)
        {
            offset = x*bytesDepth + (y*w*bytesDepth);
            ptr = data + offset;
            tptr = pTeData + offset;

            // Read color
            teData.setColor(tptr, sixteenBit);

            // in the old algorithm, this was
            //teData.channel.red   = (teData.channel.red * (255 - m_blendGain) +
            //      transData.channel.red * m_blendGain) >> 8;
            // but transdata was uninitialized, its components were apparently 0,
            // so I removed the part after the "+".

            if (sixteenBit)
            {
                teData.blendInvAlpha16(blendGain);
            }
            else
            {
                teData.blendInvAlpha8(blendGain);
            }

            // Overwrite RGB.
            teData.setPixel(tptr);
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 50.0) / w);

        if (progress%5 == 0)
            postProgress(progress);
    }

    // Merge layout and image using overlay method.

    for (int x = 0; !m_cancel && x < w; x++)
    {
        for (int y = 0; !m_cancel && y < h; y++)
        {
            offset = x*bytesDepth + (y*w*bytesDepth);
            ptr = data + offset;
            dptr = pOutBits + offset;
            tptr = pTeData + offset;

            inData.setColor (ptr, sixteenBit);
            outData.setColor(dptr, sixteenBit);
            teData.setColor (tptr, sixteenBit);

            if (sixteenBit)
            {
                outData.setRed  ( intMult16 (inData.red(),   inData.red()   + intMult16(2 * teData.red(),   65535 - inData.red())   ) );
                outData.setGreen( intMult16 (inData.green(), inData.green() + intMult16(2 * teData.green(), 65535 - inData.green()) ) );
                outData.setBlue ( intMult16 (inData.blue(),  inData.blue()  + intMult16(2 * teData.blue(),  65535 - inData.blue())  ) );
            }
            else
            {
                outData.setRed  ( intMult8  (inData.red(),   inData.red()   + intMult8(2 * teData.red(),    255 - inData.red())   ) );
                outData.setGreen( intMult8  (inData.green(), inData.green() + intMult8(2 * teData.green(),  255 - inData.green()) ) );
                outData.setBlue ( intMult8  (inData.blue(),  inData.blue()  + intMult8(2 * teData.blue(),   255 - inData.blue())  ) );
            }
            outData.setAlpha( inData.alpha() );
            outData.setPixel( dptr );
        }

        // Update progress bar in dialog.
        progress = (int) (50.0 + ((double)x * 50.0) / w);

        if (progress%5 == 0)
            postProgress(progress);
    }

}

}  // NameSpace DigikamTextureImagesPlugin
