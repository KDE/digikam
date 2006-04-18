/* ============================================================
 * File  : filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-05-25
 * Description : FilmGrain threaded image filter.
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

// Qt includes.

#include <qdatetime.h> 

// Digikam include

#include "dcolorcomposer.h"

// Local includes.

#include "filmgrain.h"

namespace DigikamFilmGrainImagesPlugin
{

FilmGrain::FilmGrain(Digikam::DImg *orgImage, QObject *parent, int sensibility)
         : Digikam::DImgThreadedFilter(orgImage, parent, "FilmGrain")
{
    m_sensibility = sensibility;
    initFilter();
}

void FilmGrain::filterImage(void)
{
    filmgrainImage(&m_orgImage, m_sensibility);
}

// This method is based on the Simulate Film grain tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/FilmGrain

void FilmGrain::filmgrainImage(Digikam::DImg *orgImage, int Sensibility)
{
    // Sensibility: 800..6400

    if (Sensibility <= 0) return;

    int Width = orgImage->width();
    int Height = orgImage->height();
    int bytesDepth = orgImage->bytesDepth();
    bool sixteenBit = orgImage->sixteenBit();
    uchar* data = orgImage->bits();

    Digikam::DImg grain(Width, Height, sixteenBit);      // Grain blured without curves adjustment.
    Digikam::DImg mask(Width, Height, sixteenBit);       // Grain mask with curves adjustment.
    uchar* pGrainBits = grain.bits();
    uchar* pMaskBits  = mask.bits();
    uchar* pOutBits = m_destImage.bits(); // Destination image with merged grain mask and original.

    int Noise, Shade, nRand, component, progress;
    uchar *ptr;
    Digikam::DColor blendData, grainData, maskData, outData;

    if (sixteenBit)
        Noise = (Sensibility / 10 + 1) * 256 - 1;
    else
        Noise = Sensibility / 10;

    // This value controls the shading pixel effect between original image and grain mask.
    if (sixteenBit)
        Shade = (52 + 1) * 256 - 1;
    else
        Shade = 52;

    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    uint seed = (uint) dt.secsTo(Y2000);

    // Make gray grain mask.

    grainData.setSixteenBit(sixteenBit);

    for (int x = 0; !m_cancel && x < Width; x++)
    {
        for (int y = 0; !m_cancel && y < Height; y++)
        {
            ptr = pGrainBits + x*bytesDepth + (y*Width*bytesDepth);

            nRand = (rand_r(&seed) % Noise) - (Noise / 2);
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
    Digikam::DImgGaussianBlur(this, grain, grain, 25, 30, 1);

    // Normally, film grain tends to be most noticable in the midtones, and much less 
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this.

    Digikam::ImageCurves *grainCurves = new Digikam::ImageCurves(sixteenBit);

    // We modify only global luminosity of the grain.
    if (sixteenBit)
    {
        grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint(0,     0));
        grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8,  QPoint(32768, 32768));
        grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 16, QPoint(65535, 0));
    }
    else
    {
        grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint(0,   0));
        grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8,  QPoint(128, 128));
        grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 16, QPoint(255, 0));
    }

    // Calculate curves and lut to apply on grain.
    grainCurves->curvesCalculateCurve(Digikam::ImageHistogram::ValueChannel);
    grainCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
    grainCurves->curvesLutProcess(pGrainBits, pMaskBits, Width, Height);

    grain.reset();
    delete grainCurves;

    // Update progress bar in dialog.
    postProgress( 40 );

    // Merge src image with grain using shade coefficient.

    int alpha;
    // get composer for default blending
    Digikam::DColorComposer *composer = Digikam::DColorComposer::getComposer(Digikam::DColorComposer::PorterDuffNone);

    for (int x = 0; !m_cancel && x < Width; x++)
    {
        for (int y = 0; !m_cancel && y < Height; y++)
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

}  // NameSpace DigikamFilmGrainImagesPlugin
