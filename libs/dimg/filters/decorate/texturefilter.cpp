/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : TextureFilter threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "texturefilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

TextureFilter::TextureFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    m_blendGain = 200;
    initFilter();
}

TextureFilter::TextureFilter(DImg* const orgImage, QObject* const parent, int blendGain, const QString& texturePath)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("Texture"))
{
    m_blendGain   = blendGain;
    m_texturePath = texturePath;

    initFilter();
}

TextureFilter::~TextureFilter()
{
    cancelFilter();
}

/** This method is based on the Simulate Texture Film tutorial from GimpGuru.org web site
  * available at this url : http://www.gimpguru.org/Tutorials/SimulatedTexture/
  */

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

void TextureFilter::filterImage()
{
    // Texture tile.

    int w           = m_orgImage.width();
    int h           = m_orgImage.height();
    int bytesDepth  = m_orgImage.bytesDepth();
    bool sixteenBit = m_orgImage.sixteenBit();

    qCDebug(DIGIKAM_DIMG_LOG) << "Texture File: " << m_texturePath;
    DImg texture(m_texturePath);

    if (texture.isNull())
    {
        return;
    }

    DImg textureImg(w, h, m_orgImage.sixteenBit(), m_orgImage.hasAlpha());

    texture.convertToDepthOfImage(&textureImg);

    for (int x = 0 ; x < w ; x += texture.width())
    {
        for (int y = 0 ; y < h ; y += texture.height())
        {
            textureImg.bitBltImage(&texture, x, y);
        }
    }

    // Apply texture.

    uchar* data     = m_orgImage.bits();
    uchar* pTeData  = textureImg.bits();
    uchar* pOutBits = m_destImage.bits();
    uint   offset;

    DColor teData, transData, inData, outData;
    uchar* ptr=0, *dptr=0, *tptr=0;
    int    progress;

    int blendGain;

    if (sixteenBit)
    {
        blendGain = (m_blendGain + 1) * 256 - 1;
    }
    else
    {
        blendGain = m_blendGain;
    }

    // Make textured transparent layout.

    for (int x = 0; runningFlag() && x < w; ++x)
    {
        for (int y = 0; runningFlag() && y < h; ++y)
        {
            offset = x * bytesDepth + (y * w * bytesDepth);
            ptr    = data + offset;
            tptr   = pTeData + offset;

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
        progress = (int)(((double) x * 50.0) / w);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // Merge layout and image using overlay method.

    for (int x = 0; runningFlag() && x < w; ++x)
    {
        for (int y = 0; runningFlag() && y < h; ++y)
        {
            offset = x * bytesDepth + (y * w * bytesDepth);
            ptr    = data + offset;
            dptr   = pOutBits + offset;
            tptr   = pTeData + offset;

            inData.setColor(ptr, sixteenBit);
            outData.setColor(dptr, sixteenBit);
            teData.setColor(tptr, sixteenBit);

            if (sixteenBit)
            {
                outData.setRed(intMult16(inData.red(), inData.red() + intMult16(2 * teData.red(), 65535 - inData.red())));
                outData.setGreen(intMult16(inData.green(), inData.green() + intMult16(2 * teData.green(), 65535 - inData.green())));
                outData.setBlue(intMult16(inData.blue(), inData.blue() + intMult16(2 * teData.blue(), 65535 - inData.blue())));
            }
            else
            {
                outData.setRed(intMult8(inData.red(), inData.red() + intMult8(2 * teData.red(), 255 - inData.red())));
                outData.setGreen(intMult8(inData.green(), inData.green() + intMult8(2 * teData.green(), 255 - inData.green())));
                outData.setBlue(intMult8(inData.blue(), inData.blue() + intMult8(2 * teData.blue(), 255 - inData.blue())));
            }

            outData.setAlpha(inData.alpha());
            outData.setPixel(dptr);
        }

        // Update progress bar in dialog.
        progress = (int)(50.0 + ((double) x * 50.0) / w);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

FilterAction TextureFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("blendGain"),   m_blendGain);
    action.addParameter(QLatin1String("texturePath"), m_texturePath);

    return action;
}

void TextureFilter::readParameters(const Digikam::FilterAction& action)
{
    m_blendGain   = action.parameter(QLatin1String("blendGain")).toInt();
    m_texturePath = action.parameter(QLatin1String("texturePath")).toString();
}

}  // namespace Digikam
