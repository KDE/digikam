/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Infrared threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dimggaussianblur.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "dimgimagefilters.h"
#include "mixerfilter.h"
#include "globals.h"

namespace Digikam
{

InfraredFilter::InfraredFilter(DImg* orgImage, QObject* parent, int sensibility, bool grain)
              : DImgThreadedFilter(orgImage, parent, "Infrared")
{
    m_sensibility = sensibility;
    m_grain       = grain;
    initFilter();
}

InfraredFilter::InfraredFilter(uchar* bits, uint width, uint height, bool sixteenBits, int sensibility, bool grain)
              : DImgThreadedFilter()
{
    m_sensibility = sensibility;
    m_grain       = grain;
    m_orgImage    = DImg(width, height, sixteenBits, true, bits, true);
    initFilter();
    startFilterDirectly();
    memcpy(bits, m_destImage.bits(), m_destImage.numBytes());
}

// This method is based on the Simulate Infrared Film tutorial from GimpGuru.org web site
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedInfrared/

/* More info about IR film can be seen at this url :

http://www.pauck.de/marco/photo/infrared/comparison_of_films/comparison_of_films.html
*/

void InfraredFilter::filterImage()
{
    // Sensibility: 200..2600

    if (m_sensibility <= 0) return;

    int Width       = m_orgImage.width();
    int Height      = m_orgImage.height();
    int bytesDepth  = m_orgImage.bytesDepth();
    uint numBytes   = m_orgImage.numBytes();
    bool sixteenBit = m_orgImage.sixteenBit();
    uchar* data     = m_orgImage.bits();

    // Infrared film variables depending on Sensibility.
    // We can reproduce famous Ilford SFX200 infrared film
    // http://www.ilford.com/html/us_english/prod_html/sfx200/sfx200.html
    // This film have a sensibility escursion from 200 to 800 ISO.
    // Over 800 ISO, we reproduce The Kodak HIE high speed infrared film.

    // Infrared film grain.
    int Noise = (m_sensibility + 3000) / 10;
    if (sixteenBit)
        Noise = (Noise + 1) * 256 - 1;

    int    blurRadius = (int)((m_sensibility / 200.0) + 1.0);   // Gaussian blur infrared highlight effect
                                                                // [2 to 5].
    double greenBoost = 2.1 - (m_sensibility / 2000.0);         // Infrared green color boost [1.7 to 2.0].

    int nRand, offset, progress;

    uchar*      pBWBits = 0;                  // Black and White conversion.
    uchar*  pBWBlurBits = 0;                  // Black and White with blur.
    uchar*   pGrainBits = 0;                  // Grain blurred without curves adjustment.
    uchar*    pMaskBits = 0;                  // Grain mask with curves adjustment.
    uchar* pOverlayBits = 0;                  // Overlay to merge with original converted in gray scale.
    uchar*     pOutBits = m_destImage.bits(); // Destination image with merged grain mask and original.

    DColor bwData, bwBlurData, grainData, maskData, overData, outData;

    //------------------------------------------
    // 1 - Create GrayScale green boosted image.
    //------------------------------------------

    // Convert to gray scale with boosting Green channel.
    // Infrared film increase green color.

    DImg BWImage(Width, Height, sixteenBit);   // Black and White conversion.
    pBWBits = BWImage.bits();
    memcpy (pBWBits, data, numBytes);

    MixerContainer settings;
    settings.bMonochrome    = true;
    settings.blackRedGain   = 0.4;
    settings.blackGreenGain = greenBoost;
    settings.blackBlueGain  = -0.8;
    MixerFilter mixer(pBWBits, Width, Height, sixteenBit, settings);

    postProgress( 10 );
    if (m_cancel)
        return;

    // Apply a Gaussian blur to the black and white image.
    // This way simulate Infrared film dispersion for the highlights.

    DImg BWBlurImage(Width, Height, sixteenBit);
    pBWBlurBits = BWBlurImage.bits();

    DImgGaussianBlur(this, BWImage, BWBlurImage, 10, 20, blurRadius);

    if (m_cancel)
        return;

    //-----------------------------------------------------------------
    // 2 - Create Gaussian blurred overlay mask with grain if necessary.
    //-----------------------------------------------------------------

    if (m_grain)
    {
        // Create gray grain mask.

        QDateTime dt = QDateTime::currentDateTime();
        QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
        uint seed = ((uint) dt.secsTo(Y2000));
#ifdef _WIN32
        srand(seed);
#endif

        pGrainBits = new uchar[numBytes];    // Grain blurred without curves adjustment.
        uchar* ptr;
        int component;
        grainData.setSixteenBit(sixteenBit);

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
            progress = (int) (30.0 + ((double)x * 10.0) / Width);

            if (progress%5 == 0)
                postProgress( progress );
        }

        // Smooth grain mask using gaussian blur.

        DImgImageFilters().gaussianBlurImage(pGrainBits, Width, Height, sixteenBit, 1);

        postProgress( 40 );
        if (m_cancel)
        {
            delete [] pGrainBits;
            return;
        }
    }

    postProgress( 50 );
    if (m_cancel)
    {
        delete [] pGrainBits;
        return;
    }

    // Normally, film grain tends to be most noticeable in the midtones, and much less
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this.

    if (m_grain)
    {
        ImageCurves* grainCurves = new ImageCurves(sixteenBit);
        pMaskBits                = new uchar[numBytes];    // Grain mask with curves adjustment.

        // We modify only global luminosity of the grain.
        if (sixteenBit)
        {
            grainCurves->setCurvePoint(LuminosityChannel, 0,  QPoint(0,   0));
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
        delete grainCurves;

        // delete it here, not used any more
        delete [] pGrainBits;
        pGrainBits = 0;
    }

    postProgress( 60 );
    if (m_cancel)
    {
        delete [] pGrainBits;
        delete [] pMaskBits;
        return;
    }

    // Merge gray scale image with grain using shade coefficient.

    if (m_grain)
    {
        pOverlayBits = new uchar[numBytes];    // Overlay to merge with original converted in gray scale.

        // get composer for default blending
        DColorComposer* composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
        int alpha;

        int Shade = 52; // This value control the shading pixel effect between original image and grain mask.
        if (sixteenBit)
            Shade = (Shade + 1) * 256 - 1;

        for (int x = 0; !m_cancel && x < Width; ++x)
        {
            for (int y = 0; !m_cancel && y < Height; ++y)
            {
                int offset = x*bytesDepth + (y*Width*bytesDepth);

                // read color from orig image
                bwBlurData.setColor(pBWBlurBits + offset, sixteenBit);
                // read color from mask
                maskData.setColor(pMaskBits + offset, sixteenBit);
                // set shade as alpha value - it will be used as source alpha when blending
                maskData.setAlpha(Shade);

                // compose, write result to blendData.
                // Preserve alpha, do not blend it (taken from old algorithm - correct?)
                alpha = bwBlurData.alpha();
                composer->compose(bwBlurData, maskData);
                bwBlurData.setAlpha(alpha);

                // write to destination
                bwBlurData.setPixel(pOverlayBits + offset);
            }

            // Update progress bar in dialog.
            progress = (int) (70.0 + ((double)x * 10.0) / Width);

            if (progress%5 == 0)
                postProgress( progress );
        }

        delete composer;

        // delete it here, not used any more
        BWBlurImage.reset();
        delete [] pMaskBits;
        pMaskBits   = 0;
    }
    else
    {
        // save a memcpy
        pOverlayBits = pBWBlurBits;
        pBWBlurBits  = 0;
    }

    //------------------------------------------
    // 3 - Merge Grayscale image & overlay mask.
    //------------------------------------------

    // Merge overlay and gray scale image using 'Overlay' Gimp method for increase the highlight.
    // The result is usually a brighter picture.
    // Overlay mode composite value computation is D =  A * (B + (2 * B) * (255 - A)).

    outData.setSixteenBit(sixteenBit);
    for (int x = 0; !m_cancel && x < Width; ++x)
    {
        for (int y = 0; !m_cancel && y < Height; ++y)
        {
            offset = x*bytesDepth + (y*Width*bytesDepth);

            bwData.setColor (pBWBits + offset, sixteenBit);
            overData.setColor(pOverlayBits + offset, sixteenBit);

            if (sixteenBit)
            {
                outData.setRed  ( intMult16 (bwData.red(),   bwData.red()   + intMult16(2 * overData.red(),   65535 - bwData.red())   ) );
                outData.setGreen( intMult16 (bwData.green(), bwData.green() + intMult16(2 * overData.green(), 65535 - bwData.green()) ) );
                outData.setBlue ( intMult16 (bwData.blue(),  bwData.blue()  + intMult16(2 * overData.blue(),  65535 - bwData.blue())  ) );
            }
            else
            {
                outData.setRed  ( intMult8  (bwData.red(),   bwData.red()   + intMult8(2 * overData.red(),    255 - bwData.red())   ) );
                outData.setGreen( intMult8  (bwData.green(), bwData.green() + intMult8(2 * overData.green(),  255 - bwData.green()) ) );
                outData.setBlue ( intMult8  (bwData.blue(),  bwData.blue()  + intMult8(2 * overData.blue(),   255 - bwData.blue())  ) );
            }
            outData.setAlpha( bwData.alpha() );
            outData.setPixel( pOutBits + offset );
        }

        // Update progress bar in dialog.
        progress = (int) (80.0 + ((double)x * 20.0) / Width);

        if (progress%5 == 0)
            postProgress(progress);
    }

    delete [] pGrainBits;
    delete [] pMaskBits;

    if (m_grain)
        delete [] pOverlayBits;
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

}  // namespace Digikam
