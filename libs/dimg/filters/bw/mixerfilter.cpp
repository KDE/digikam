/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Chanels mixer filter
 *
 * Copyright (C) 2005-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

MixerFilter::MixerFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

MixerFilter::MixerFilter(DImg* orgImage, QObject* parent, const MixerContainer& settings)
    : DImgThreadedFilter(orgImage, parent, "MixerFilter")
{
    m_settings = settings;
    initFilter();
}

MixerFilter::~MixerFilter()
{
    cancelFilter();
}

void MixerFilter::filterImage()
{
    m_destImage.putImageData(m_orgImage.bits());

    uchar* bits     = m_destImage.bits();
    uint width      = m_destImage.width();
    uint height     = m_destImage.height();
    bool sixteenBit = m_destImage.sixteenBit();

    uint size = width * height;
    int  progress;

    register uint i;
    double   rnorm = 1;    // red channel normalizer use in RGB mode.
    double   mnorm = 1;    // monochrome normalizer used in Monochrome mode.

    if (m_settings.bMonochrome)
    {
        mnorm = CalculateNorm(m_settings.blackRedGain, m_settings.blackGreenGain,
                              m_settings.blackBlueGain, m_settings.bPreserveLum);
    }
    else
    {
        rnorm = CalculateNorm(m_settings.redRedGain, m_settings.redGreenGain,
                              m_settings.redBlueGain, m_settings.bPreserveLum);
    }

    double gnorm = CalculateNorm(m_settings.greenRedGain, m_settings.greenGreenGain,
                                 m_settings.greenBlueGain, m_settings.bPreserveLum);
    double bnorm = CalculateNorm(m_settings.blueRedGain, m_settings.blueGreenGain,
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
                nGray = MixPixel(m_settings.blackRedGain, m_settings.blackGreenGain, m_settings.blackBlueGain,
                                 (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                 sixteenBit, mnorm);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = (uchar)MixPixel(m_settings.blueRedGain, m_settings.blueGreenGain, m_settings.blueBlueGain,
                                         (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                         sixteenBit, bnorm);
                ptr[1] = (uchar)MixPixel(m_settings.greenRedGain, m_settings.greenGreenGain, m_settings.greenBlueGain,
                                         (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                         sixteenBit, gnorm);
                ptr[2] = (uchar)MixPixel(m_settings.redRedGain, m_settings.redGreenGain, m_settings.redBlueGain,
                                         (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                         sixteenBit, rnorm);
            }

            ptr += 4;

            progress = (int)(((double)i * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
    else               // 16 bits image.
    {
        unsigned short  nGray, red, green, blue;
        unsigned short* ptr = reinterpret_cast<unsigned short*>(bits);

        for (i = 0 ; i < size ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            if (m_settings.bMonochrome)
            {
                nGray = MixPixel(m_settings.blackRedGain, m_settings.blackGreenGain, m_settings.blackBlueGain,
                                 red, green, blue, sixteenBit, mnorm);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = MixPixel(m_settings.blueRedGain, m_settings.blueGreenGain, m_settings.blueBlueGain,
                                  red, green, blue, sixteenBit, bnorm);
                ptr[1] = MixPixel(m_settings.greenRedGain, m_settings.greenGreenGain, m_settings.greenBlueGain,
                                  red, green, blue, sixteenBit, gnorm);
                ptr[2] = MixPixel(m_settings.redRedGain, m_settings.redGreenGain, m_settings.redBlueGain,
                                  red, green, blue, sixteenBit, rnorm);
            }

            ptr += 4;

            progress = (int)(((double)i * 100.0) / size);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
}

double MixerFilter::CalculateNorm(double RedGain, double GreenGain, double BlueGain, bool bPreserveLum)
{
    double lfSum = RedGain + GreenGain + BlueGain;

    if ((lfSum == 0.0) || (!bPreserveLum))
    {
        return (1.0);
    }

    return(fabs(1.0 / lfSum));
}

unsigned short MixerFilter::MixPixel(double RedGain, double GreenGain, double BlueGain,
                                     unsigned short R, unsigned short G, unsigned short B, bool sixteenBit,
                                     double Norm)
{
    double lfMix = Norm * (RedGain * (double)R + GreenGain * (double)G + BlueGain * (double)B);
    return((unsigned short)CLAMP((int)lfMix, 0, sixteenBit ? 65535 : 255));
}

FilterAction MixerFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("blackBlueGain", m_settings.blackBlueGain);
    action.addParameter("blackGreenGain", m_settings.blackGreenGain);
    action.addParameter("blackRedGain", m_settings.blackRedGain);
    action.addParameter("blueBlueGain", m_settings.blueBlueGain);
    action.addParameter("blueGreenGain", m_settings.blueGreenGain);
    action.addParameter("blueRedGain", m_settings.blueRedGain);
    action.addParameter("bMonochrome", m_settings.bMonochrome);
    action.addParameter("bPreserveLum", m_settings.bPreserveLum);
    action.addParameter("greenBlueGain", m_settings.greenBlueGain);
    action.addParameter("greenGreenGain", m_settings.greenGreenGain);
    action.addParameter("greenRedGain", m_settings.greenRedGain);
    action.addParameter("redBlueGain", m_settings.redBlueGain);
    action.addParameter("redGreenGain", m_settings.redGreenGain);
    action.addParameter("redRedGain", m_settings.redRedGain);

    return action;
}

void MixerFilter::readParameters(const Digikam::FilterAction& action)
{
    m_settings.blackBlueGain = action.parameter("blackBlueGain").toDouble();
    m_settings.blackGreenGain = action.parameter("blackGreenGain").toDouble();
    m_settings.blackRedGain = action.parameter("blackRedGain").toDouble();
    m_settings.blueBlueGain = action.parameter("blueBlueGain").toDouble();
    m_settings.blueGreenGain = action.parameter("blueGreenGain").toDouble();
    m_settings.blueRedGain = action.parameter("blueRedGain").toDouble();
    m_settings.bMonochrome = action.parameter("bMonochrome").toBool();
    m_settings.bPreserveLum = action.parameter("bPreserveLum").toBool();
    m_settings.greenBlueGain = action.parameter("greenBlueGain").toDouble();
    m_settings.greenGreenGain = action.parameter("greenGreenGain").toDouble();
    m_settings.greenRedGain = action.parameter("greenRedGain").toDouble();
    m_settings.redBlueGain = action.parameter("redBlueGain").toDouble();
    m_settings.redGreenGain = action.parameter("redGreenGain").toDouble();
    m_settings.redRedGain = action.parameter("redRedGain").toDouble();
}

}  // namespace Digikam
