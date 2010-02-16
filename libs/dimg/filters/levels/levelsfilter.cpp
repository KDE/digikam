/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-29
 * Description : Histogram Levels color correction.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "levelsfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes

#include <QFile>

// KDE includes

#include <kdebug.h>
#include <kurl.h>

// Local includes

#include "dimg.h"
#include "imagehistogram.h"

namespace Digikam
{

class LevelsFilterPriv
{
public:

    LevelsFilterPriv()
    {
        // Obsolete in algorithm since over/under exposure indicators
        // are implemented directly with preview widget.
        Levelsind   = false;
        overExp = false;

        clipSat = true;
        mr      = 1.0;
        mg      = 1.0;
        mb      = 1.0;
        BP      = 0;
    }

    bool  clipSat;
    bool  overExp;
    bool  Levelsind;

    int   BP;
    int   WP;

    uint  rgbMax;

    float curve[65536];
    float mr;
    float mg;
    float mb;
};

LevelsFilter::LevelsFilter(DImg* orgImage, QObject* parent, const LevelsContainer& settings)
            : DImgThreadedFilter(orgImage, parent, "LevelsFilter"),
              d(new LevelsFilterPriv)
{
    m_settings = settings;
    d->WP      = m_orgImage.sixteenBit() ? 65536 : 256;
    d->rgbMax  = m_orgImage.sixteenBit() ? 65536 : 256;
    initFilter();
}

LevelsFilter::LevelsFilter(uchar* data, uint width, uint height, bool sixteenBit, const LevelsContainer& settings)
            : DImgThreadedFilter(),
              d(new LevelsFilterPriv)
{
    m_settings = settings;
    d->WP      = sixteenBit ? 65536 : 256;
    d->rgbMax  = sixteenBit ? 65536 : 256;

    // Set final lut.
    setRGBmult();
    d->mr = d->mb = 1.0;
    if (d->clipSat) d->mg = 1.0;
    setLUTv();
    setRGBmult();

    // Apply White balance adjustments.
    adjustWhiteBalance(data, width, height, sixteenBit);
}

LevelsFilter::~LevelsFilter()
{
    delete d;
}

void LevelsFilter::filterImage()
{

    m_destImage = m_orgImage;
}

void LevelsFilter::autoLevelsAdjustement(DImg* img, double& black, double& expo)
{
    autoExposureAdjustement(img->bits(), img->width(), img->height(), img->sixteenBit(), black, expo);
}

void LevelsFilter::autoLevelsAdjustement(uchar* data, int width, int height, bool sb, double& black, double& expo)
{
    // Create an histogram of original image.

    ImageHistogram* histogram = new ImageHistogram(data, width, height, sb);
    histogram->calculate();

    // Calculate optimal exposition and black level

    int    i;
    double sum, stop;
    uint   rgbMax = sb ? 65536 : 256;

    // Cutoff at 0.5% of the histogram.

    stop = width * height / 200;

    for (i = rgbMax, sum = 0; (i >= 0) && (sum < stop); --i)
        sum += histogram->getValue(LuminosityChannel, i);

    expo = -log((float)(i+1) / rgbMax) / log(2);
    kDebug() << "White level at:" << i;

    for (i = 1, sum = 0; (i < (int)rgbMax) && (sum < stop); ++i)
        sum += histogram->getValue(LuminosityChannel, i);

    black = (double)i / rgbMax;
    black /= 2;

    kDebug() << "Black:" << black << "  Exposition:" << expo;

    delete histogram;
}

void LevelsFilter::adjustWhiteBalance(uchar* data, int width, int height, bool sixteenBit)
{
    uint size = (uint)(width*height);
    uint i, j;
    int  progress;

    if (!sixteenBit)        // 8 bits image.
    {
        uchar  red, green, blue;
        uchar* ptr = data;

        for (j = 0 ; !m_cancel && (j < size) ; ++j)
        {
            int v, rv[3];

            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            rv[0] = (int)(blue  * d->mb);
            rv[1] = (int)(green * d->mg);
            rv[2] = (int)(red   * d->mr);
            v     = qMax(rv[0], rv[1]);
            v     = qMax(v, rv[2]);

            if (d->clipSat) v = qMin(v, (int)d->rgbMax-1);
            i = v;

            ptr[0] = (uchar)pixelColor(rv[0], i, v);
            ptr[1] = (uchar)pixelColor(rv[1], i, v);
            ptr[2] = (uchar)pixelColor(rv[2], i, v);
            ptr    += 4;

            progress = (int)(((double)j * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );
        }
    }
    else               // 16 bits image.
    {
        unsigned short  red, green, blue;
        unsigned short* ptr = (unsigned short *)data;

        for (j = 0 ; !m_cancel && (j < size) ; ++j)
        {
            int v, rv[3];

            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            rv[0] = (int)(blue  * d->mb);
            rv[1] = (int)(green * d->mg);
            rv[2] = (int)(red   * d->mr);
            v     = qMax(rv[0], rv[1]);
            v     = qMax(v, rv[2]);

            if (d->clipSat) v = qMin(v, (int)d->rgbMax-1);
            i = v;

            ptr[0] = pixelColor(rv[0], i, v);
            ptr[1] = pixelColor(rv[1], i, v);
            ptr[2] = pixelColor(rv[2], i, v);
            ptr    += 4;

            progress = (int)(((double)j * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );
        }
    }
}

}  // namespace Digikam
