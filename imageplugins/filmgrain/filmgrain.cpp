/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : FilmGrain threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "filmgrain.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QDateTime>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dimggaussianblur.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "globals.h"

namespace DigikamFilmGrainImagesPlugin
{

FilmGrain::FilmGrain(DImg* orgImage, QObject* parent, int sensibility)
         : DImgThreadedFilter(orgImage, parent, "FilmGrain")
{
    m_sensibility = sensibility;
    initFilter();
}

// This method is based on the Simulate Film grain tutorial from GimpGuru.org web site
// available at this url : http://www.gimpguru.org/Tutorials/FilmGrain

void FilmGrain::filterImage()
{
    // Sensibility: 800..6400

    if (m_sensibility <= 0) return;

    int Width       = m_orgImage.width();
    int Height      = m_orgImage.height();
    int bytesDepth  = m_orgImage.bytesDepth();
    bool sixteenBit = m_orgImage.sixteenBit();
    uchar* data     = m_orgImage.bits();

    DImg grain(Width, Height, sixteenBit);      // Grain blurred without curves adjustment.
    DImg mask(Width, Height, sixteenBit);       // Grain mask with curves adjustment.
    uchar* pGrainBits = grain.bits();
    uchar* pMaskBits  = mask.bits();
    uchar* pOutBits   = m_destImage.bits(); // Destination image with merged grain mask and original.

    int    Noise, Shade, nRand, component, progress;
    uchar* ptr = 0;
    DColor blendData, grainData, maskData, outData;

    if (sixteenBit)
        Noise = (m_sensibility / 10 + 1) * 256 - 1;
    else
        Noise = m_sensibility / 10;

    // This value controls the shading pixel effect between original image and grain mask.
    if (sixteenBit)
        Shade = (52 + 1) * 256 - 1;
    else
        Shade = 52;

    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    uint seed    = (uint) dt.secsTo(Y2000);

    // Make gray grain mask.

    grainData.setSixteenBit(sixteenBit);
#ifdef _WIN32
    srand(seed);
#endif

    for (int x = 0; !m_cancel && x < Width; ++x)
    {
        for (int y = 0; !m_cancel && y < Height; ++y)
        {
            ptr = pGrainBits + x*bytesDepth + (y*Width*bytesDepth);

#ifndef _WIN32
            nRand = (rand_r(&seed) % Noise) - (Noise / 2);
#else
            nRand = (rand() % Noise) - (Noise / 2);
#endif
            if (sixteenBit)
                component = CLAMP(32768 + nRand, 0, 65535);
            else
                component = CLAMP(128 + nRand, 0, 255);

            grainData.setRed  (component);
            grainData.setGreen(component);
            grainData.setBlue (component);
            grainData.setAlpha(0);

            grainData.setPixel(ptr);
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 25.0) / Width);

        if (progress%5 == 0)
            postProgress( progress );
    }

    // Smooth grain mask using gaussian blur with radius 1.
    DImgGaussianBlur(this, grain, grain, 25, 30, 1);

    // Normally, film grain tends to be most noticeable in the midtones, and much less
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this.

    ImageCurves* grainCurves = new ImageCurves(sixteenBit);

    // We modify only global luminosity of the grain.
    if (sixteenBit)
    {
        grainCurves->setCurvePoint(LuminosityChannel, 0,  QPoint(0,     0));
        grainCurves->setCurvePoint(LuminosityChannel, 8,  QPoint(32768, 32768));
        grainCurves->setCurvePoint(LuminosityChannel, 16, QPoint(65535, 0));
    }
    else
    {
        grainCurves->setCurvePoint(LuminosityChannel, 0,  QPoint(0,   0));
        grainCurves->setCurvePoint(LuminosityChannel, 8,  QPoint(128, 128));
        grainCurves->setCurvePoint(LuminosityChannel, 16, QPoint(255, 0));
    }

    // Calculate curves and lut to apply on grain.
    grainCurves->curvesCalculateCurve(LuminosityChannel);
    grainCurves->curvesLutSetup(AlphaChannel);
    grainCurves->curvesLutProcess(pGrainBits, pMaskBits, Width, Height);

    grain.reset();
    delete grainCurves;

    // Update progress bar in dialog.
    postProgress( 40 );

    // Merge src image with grain using shade coefficient.

    int alpha;
    // get composer for default blending
    DColorComposer* composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);

    for (int x = 0; !m_cancel && x < Width; ++x)
    {
        for (int y = 0; !m_cancel && y < Height; ++y)
        {
            int offset = x*bytesDepth + (y*Width*bytesDepth);

            // read color from orig image
            blendData.setColor(data + offset, sixteenBit);
            // read color from mask
            maskData.setColor(pMaskBits + offset, sixteenBit);
            // set shade as alpha value - it will be used as source alpha when blending
            maskData.setAlpha(Shade);

            // compose, write result to blendData.
            // Preserve alpha, do not blend it (taken from old algorithm - correct?)
            alpha = blendData.alpha();
            composer->compose(blendData, maskData);
            blendData.setAlpha(alpha);

            // write to destination
            blendData.setPixel(pOutBits + offset);
        }

        // Update progress bar in dialog.
        progress = (int) (50.0 + ((double)x * 50.0) / Width);

        if (progress%5 == 0)
           postProgress( progress );
    }

    delete composer;
}

}  // namespace DigikamFilmGrainImagesPlugin
