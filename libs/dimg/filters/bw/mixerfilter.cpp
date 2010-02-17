/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Chanels mixer filter
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mixerfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dcolor.h"

namespace Digikam
{

MixerFilter::MixerFilter(DImg* orgImage, QObject* parent, const MixerContainer& settings)
           : DImgThreadedFilter(orgImage, parent, "MixerFilter")
{
    m_settings = settings;
    initFilter();
}

MixerFilter::MixerFilter(uchar* bits, uint width, uint height, bool sixteenBits, 
                         const MixerContainer& settings)
           : DImgThreadedFilter()
{
    m_settings = settings;
    channelMixerImage(bits, width, height, sixteenBits);
}

MixerFilter::~MixerFilter()
{
}

void MixerFilter::filterImage()
{
    channelMixerImage(m_orgImage);
    m_destImage = m_orgImage;
}

void MixerFilter::channelMixerImage(DImg& image)
{
    if (image.isNull()) return;

    channelMixerImage(image.bits(), image.width(), image.height(), image.sixteenBit());
}

    /** Mix RGB channel color from image*/
void MixerFilter::channelMixerImage(uchar* bits, uint width, uint height, bool sixteenBit)
{
    if (!bits) return;

    uint size = width*height;
    int  progress;

    register uint i;

    double rnorm = CalculateNorm (m_settings.rrGain, m_settings.rgGain, m_settings.rbGain, m_settings.bPreserveLum);
    double gnorm = CalculateNorm (m_settings.grGain, m_settings.ggGain, m_settings.gbGain, m_settings.bPreserveLum);
    double bnorm = CalculateNorm (m_settings.brGain, m_settings.bgGain, m_settings.bbGain, m_settings.bPreserveLum);

    if (!sixteenBit)        // 8 bits image.
    {
        uchar  nGray, red, green, blue;
        uchar* ptr = bits;

        for (i = 0 ; i < size ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            if (m_settings.bMonochrome)
            {
                nGray = MixPixel (m_settings.rrGain, m_settings.rgGain, m_settings.rbGain,
                                  (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                  sixteenBit, rnorm);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = (uchar)MixPixel (m_settings.brGain, m_settings.bgGain, m_settings.bbGain,
                                          (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, bnorm);
                ptr[1] = (uchar)MixPixel (m_settings.grGain, m_settings.ggGain, m_settings.gbGain,
                                          (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, gnorm);
                ptr[2] = (uchar)MixPixel (m_settings.rrGain, m_settings.rgGain, m_settings.rbGain,
                                          (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, rnorm);
            }

            ptr += 4;
            
            progress = (int)(((double)i * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );               
        }
    }
    else               // 16 bits image.
    {
        unsigned short  nGray, red, green, blue;
        unsigned short* ptr = (unsigned short *)bits;

        for (i = 0 ; i < size ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            if (m_settings.bMonochrome)
            {
                nGray = MixPixel (m_settings.rrGain, m_settings.rgGain, m_settings.rbGain, 
                                  red, green, blue, sixteenBit, rnorm);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = MixPixel (m_settings.brGain, m_settings.bgGain, m_settings.bbGain, 
                                   red, green, blue, sixteenBit, bnorm);
                ptr[1] = MixPixel (m_settings.grGain, m_settings.ggGain, m_settings.gbGain, 
                                   red, green, blue, sixteenBit, gnorm);
                ptr[2] = MixPixel (m_settings.rrGain, m_settings.rgGain, m_settings.rbGain, 
                                   red, green, blue, sixteenBit, rnorm);
            }

            ptr += 4;
            
            progress = (int)(((double)i * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );               
        }
    }
}


double MixerFilter::CalculateNorm(float RedGain, float GreenGain, float BlueGain, bool bPreserveLum)
{
    double lfSum = RedGain + GreenGain + BlueGain;

    if ((lfSum == 0.0) || (bPreserveLum == false))
        return (1.0);

    return( fabs (1.0 / lfSum) );
};

unsigned short MixerFilter::MixPixel(float RedGain, float GreenGain, float BlueGain,
                                     unsigned short R, unsigned short G, unsigned short B, bool sixteenBit,
                                     double Norm)
{
    double lfMix = RedGain * (double)R + GreenGain * (double)G + BlueGain * (double)B;
    lfMix        *= Norm;
    int segment  = sixteenBit ? 65535 : 255;

    return( (unsigned short)CLAMP((int)lfMix, 0, segment));
};
    
}  // namespace Digikam
