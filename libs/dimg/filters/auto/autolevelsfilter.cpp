/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : auto levels image filter.
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

#include "autolevelsfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "imagehistogram.h"
#include "imagelevels.h"

namespace Digikam
{

AutoLevelsFilter::AutoLevelsFilter(DImg* orgImage, QObject* parent)
                : DImgThreadedFilter(orgImage, parent, "AutoLevelsFilter")
{
    initFilter();
}

AutoLevelsFilter::AutoLevelsFilter(uchar* data, uint width, uint height, bool sixteenBit)
                : DImgThreadedFilter()
{
    autoLevelsCorrectionImage(data, width, height, sixteenBit);
}

AutoLevelsFilter::~AutoLevelsFilter()
{
}

void AutoLevelsFilter::filterImage()
{
    autoLevelsCorrectionImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
    m_destImage = m_orgImage;
}

/** Performs histogram auto correction of levels.
    This method maximizes the tonal range in the Red,
    Green, and Blue channels. It search the image shadow and highlight
    limit values and adjust the Red, Green, and Blue channels
    to a full histogram range.*/
void AutoLevelsFilter::autoLevelsCorrectionImage(uchar* data, int w, int h, bool sixteenBit)
{
    if (!data || !w || !h)
    {
       kWarning() << ("no image data available!");
       return;
    }

    postProgress(10);
    uchar* desData            = 0;
    ImageHistogram* histogram = 0;
    ImageLevels* levels       = 0;

    // Create the new empty destination image data space.
    if (!m_cancel)
    {
        if (sixteenBit)
            desData = new uchar[w*h*8];
        else
            desData = new uchar[w*h*4];

        postProgress(20);
    }

    // Create an histogram of the current image.
    if (!m_cancel)
    {
        histogram = new ImageHistogram(data, w, h, sixteenBit);
        histogram->calculate();
        postProgress(30);
    }

    // Create an empty instance of levels to use.
    if (!m_cancel)
    {
        levels = new ImageLevels(sixteenBit);
        postProgress(40);
    }

    // Initialize an auto levels correction of the histogram.
    if (!m_cancel)
    {
        levels->levelsAuto(histogram);
        postProgress(50);
    }

    // Calculate the LUT to apply on the image.
    if (!m_cancel)
    {
        levels->levelsLutSetup(AlphaChannel);
        postProgress(60);
    }

    // Apply the lut to the image.
    if (!m_cancel)
    {
        levels->levelsLutProcess(data, desData, w, h);
        postProgress(70);
    }

    if (!m_cancel)
    {
        if (sixteenBit)
            memcpy (data, desData, w*h*8);
        else
            memcpy (data, desData, w*h*4);

        postProgress(80);
    }

    delete [] desData;
    delete histogram;
    delete levels;

    if (!m_cancel)
        postProgress(90);
}

}  // namespace Digikam
