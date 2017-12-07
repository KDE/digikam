/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Infrared threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "infraredfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QDateTime>

// Local includes

#include "dimg.h"
#include "blurfilter.h"
#include "mixerfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

InfraredFilter::InfraredFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

InfraredFilter::InfraredFilter(DImg* orgImage, QObject* parent, const InfraredContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("Infrared"))
{
    m_settings = settings;
    initFilter();
}

InfraredFilter::~InfraredFilter()
{
    cancelFilter();
}

/** This method is based on the Simulate Infrared Film tutorial from GimpGuru.org web site
    available at this url : http://www.gimpguru.org/Tutorials/SimulatedInfrared/

    More info about IR film can be seen at this url :

    http://www.pauck.de/marco/photo/infrared/comparison_of_films/comparison_of_films.html
*/

void InfraredFilter::filterImage()
{
    m_destImage.putImageData(m_orgImage.bits());

    int Width       = m_destImage.width();
    int Height      = m_destImage.height();
    int bytesDepth  = m_destImage.bytesDepth();
    bool sixteenBit = m_destImage.sixteenBit();
    uchar* data     = m_destImage.bits();

    postProgress(10);

    if (!runningFlag())
    {
        return;
    }

    // Infrared film variables depending on Sensibility.
    // We can reproduce famous Ilford SFX200 infrared film
    // http://www.ilford.com/html/us_english/prod_html/sfx200/sfx200.html
    // This film have a sensibility escursion from 200 to 800 ISO.
    // Over 800 ISO, we reproduce The Kodak HIE high speed infrared film.

    int    blurRadius = (int)((m_settings.sensibility / 200.0) + 1.0);   // Gaussian blur infrared highlight effect [2 to 5].
    int    offset, progress;

    uchar* pOverlayBits = 0;                  // Overlay to merge with original converted in gray scale.
    uchar*     pOutBits = m_destImage.bits(); // Destination image with merged grain mask and original.

    DColor bwData, bwBlurData, maskData, overData, outData;

    postProgress(20);

    if (!runningFlag())
    {
        return;
    }

    //------------------------------------------
    // 1 - Create GrayScale green boosted image.
    //------------------------------------------

    // Convert to gray scale with boosting Green channel.
    // Infrared film increase green color.

    DImg BWImage(Width, Height, sixteenBit, true, data);   // Black and White conversion.

    MixerContainer settings;
    settings.bMonochrome    = true;
    settings.blackRedGain   = m_settings.redGain;
    settings.blackGreenGain = m_settings.greenGain - (m_settings.sensibility / 2000.0);   // Infrared green color boost [1.7 to 2.0].
    settings.blackBlueGain  = m_settings.blueGain;
    MixerFilter mixer(&BWImage, 0L, settings);
    mixer.startFilterDirectly();
    BWImage.putImageData(mixer.getTargetImage().bits());

    postProgress(30);

    if (!runningFlag())
    {
        return;
    }

    // Apply a Gaussian blur to the black and white image.
    // This way simulate Infrared film dispersion for the highlights.

    DImg BWBlurImage(Width, Height, sixteenBit);
    BlurFilter(this, BWImage, BWBlurImage, 10, 20, blurRadius);

    // save a memcpy
    pOverlayBits = BWBlurImage.bits();

    postProgress(40);

    if (!runningFlag())
    {
        return;
    }

    //------------------------------------------
    // 2 - Merge Grayscale image & overlay mask.
    //------------------------------------------

    // Merge overlay and gray scale image using 'Overlay' Gimp method for increase the highlight.
    // The result is usually a brighter picture.
    // Overlay mode composite value computation is D =  A * (B + (2 * B) * (255 - A)).

    outData.setSixteenBit(sixteenBit);

    for (int x = 0; runningFlag() && x < Width; ++x)
    {
        for (int y = 0; runningFlag() && y < Height; ++y)
        {
            offset = x * bytesDepth + (y * Width * bytesDepth);

            bwData.setColor(BWImage.bits() + offset, sixteenBit);
            overData.setColor(pOverlayBits + offset, sixteenBit);

            if (sixteenBit)
            {
                outData.setRed(intMult16(bwData.red(),   bwData.red()   + intMult16(2 * overData.red(),   65535 - bwData.red())));
                outData.setGreen(intMult16(bwData.green(), bwData.green() + intMult16(2 * overData.green(), 65535 - bwData.green())));
                outData.setBlue(intMult16(bwData.blue(),  bwData.blue()  + intMult16(2 * overData.blue(),  65535 - bwData.blue())));
            }
            else
            {
                outData.setRed(intMult8(bwData.red(),   bwData.red()   + intMult8(2 * overData.red(),    255 - bwData.red())));
                outData.setGreen(intMult8(bwData.green(), bwData.green() + intMult8(2 * overData.green(),  255 - bwData.green())));
                outData.setBlue(intMult8(bwData.blue(),  bwData.blue()  + intMult8(2 * overData.blue(),   255 - bwData.blue())));
            }

            outData.setAlpha(bwData.alpha());
            outData.setPixel(pOutBits + offset);
        }

        // Update progress bar in dialog.
        progress = (int)(50.0 + ((double)x * 50.0) / Width);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

int InfraredFilter::intMult8(uint a, uint b)
{
    uint t = a * b + 0x80;
    return ((t >> 8) + t) >> 8;
}

int InfraredFilter::intMult16(uint a, uint b)
{
    uint t = a * b + 0x8000;
    return ((t >> 16) + t) >> 16;
}

FilterAction InfraredFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("blueGain"),    m_settings.blueGain);
    action.addParameter(QLatin1String("greenGain"),   m_settings.greenGain);
    action.addParameter(QLatin1String("redGain"),     m_settings.redGain);
    action.addParameter(QLatin1String("sensibility"), m_settings.sensibility);

    return action;
}

void InfraredFilter::readParameters(const FilterAction& action)
{
    m_settings.blueGain = action.parameter(QLatin1String("blueGain")).toDouble();
    m_settings.greenGain = action.parameter(QLatin1String("greenGain")).toDouble();
    m_settings.redGain = action.parameter(QLatin1String("redGain")).toDouble();
    m_settings.sensibility = action.parameter(QLatin1String("sensibility")).toInt();
}


}  // namespace Digikam
