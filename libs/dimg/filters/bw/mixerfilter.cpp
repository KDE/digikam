/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Chanels mixer filter
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

MixerFilter::~MixerFilter()
{
}

void MixerFilter::filterImage()
{
    m_destImage.putImageData(m_orgImage.bits());

    uchar* bits     = m_destImage.bits();
    uint width      = m_destImage.width();
    uint height     = m_destImage.height();
    bool sixteenBit = m_destImage.sixteenBit();

    uint size = width*height;
    int  progress;

    register uint i;

    double rnorm = CalculateNorm (m_settings.redRedGain, m_settings.redGreenGain, 
                                  m_settings.redBlueGain, m_settings.bPreserveLum);
    double gnorm = CalculateNorm (m_settings.greenRedGain, m_settings.greenGreenGain, 
                                  m_settings.greenBlueGain, m_settings.bPreserveLum);
    double bnorm = CalculateNorm (m_settings.blueRedGain, m_settings.blueGreenGain, 
                                  m_settings.blueBlueGain, m_settings.bPreserveLum);

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
                nGray = MixPixel (m_settings.blackRedGain, m_settings.blackGreenGain, m_settings.blackBlueGain,
                                  (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                  sixteenBit, rnorm);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = (uchar)MixPixel (m_settings.blueRedGain, m_settings.blueGreenGain, m_settings.blueBlueGain,
                                          (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, bnorm);
                ptr[1] = (uchar)MixPixel (m_settings.greenRedGain, m_settings.greenGreenGain, m_settings.greenBlueGain,
                                          (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, gnorm);
                ptr[2] = (uchar)MixPixel (m_settings.redRedGain, m_settings.redGreenGain, m_settings.redBlueGain,
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
                nGray = MixPixel (m_settings.blackRedGain, m_settings.blackGreenGain, m_settings.blackBlueGain,
                                  red, green, blue, sixteenBit, rnorm);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = MixPixel (m_settings.blueRedGain, m_settings.blueGreenGain, m_settings.blueBlueGain, 
                                   red, green, blue, sixteenBit, bnorm);
                ptr[1] = MixPixel (m_settings.greenRedGain, m_settings.greenGreenGain, m_settings.greenBlueGain, 
                                   red, green, blue, sixteenBit, gnorm);
                ptr[2] = MixPixel (m_settings.redRedGain, m_settings.redGreenGain, m_settings.redBlueGain, 
                                   red, green, blue, sixteenBit, rnorm);
            }

            ptr += 4;

            progress = (int)(((double)i * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );
        }
    }
}


double MixerFilter::CalculateNorm(double RedGain, double GreenGain, double BlueGain, bool bPreserveLum)
{
    double lfSum = RedGain + GreenGain + BlueGain;

    if ((lfSum == 0.0) || (bPreserveLum == false))
        return (1.0);

    return( fabs (1.0 / lfSum) );
}

unsigned short MixerFilter::MixPixel(double RedGain, double GreenGain, double BlueGain,
                                     unsigned short R, unsigned short G, unsigned short B, bool sixteenBit,
                                     double Norm)
{
    double lfMix = RedGain * (double)R + GreenGain * (double)G + BlueGain * (double)B;
    lfMix        *= Norm;
    int segment  = sixteenBit ? 65535 : 255;

    return( (unsigned short)CLAMP((int)lfMix, 0, segment));
}

}  // namespace Digikam
