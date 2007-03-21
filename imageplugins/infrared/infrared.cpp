/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-05-25
 * Description : Infrared threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier 
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dimggaussianblur.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "dimgimagefilters.h"
#include "infrared.h"

namespace DigikamInfraredImagesPlugin
{

Infrared::Infrared(Digikam::DImg *orgImage, QObject *parent, int sensibility, bool grain)
        : Digikam::DImgThreadedFilter(orgImage, parent, "Infrared")
{ 
    m_sensibility = sensibility;
    m_grain       = grain;
    initFilter();
}

void Infrared::filterImage(void)
{
    infraredImage(&m_orgImage, m_sensibility, m_grain);
}

// This method is based on the Simulate Infrared Film tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedInfrared/

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

/* More info about IR film can be seen at this url : 

http://www.pauck.de/marco/photo/infrared/comparison_of_films/comparison_of_films.html
*/

void Infrared::infraredImage(Digikam::DImg *orgImage, int Sensibility, bool Grain)
{
    // Sensibility: 200..2600

    if (Sensibility <= 0) return;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    int bytesDepth  = orgImage->bytesDepth();
    uint numBytes   = orgImage->numBytes();
    bool sixteenBit = orgImage->sixteenBit();
    uchar* data     = orgImage->bits();

    // Infrared film variables depending on Sensibility.
    // We can reproduce famous Ilford SFX200 infrared film
    // http://www.ilford.com/html/us_english/prod_html/sfx200/sfx200.html
    // This film have a sensibility escursion from 200 to 800 ISO.
    // Over 800 ISO, we reproduce The Kodak HIE hight speed infrared film.

    // Infrared film grain.
    int Noise = (Sensibility + 3000) / 10;
    if (sixteenBit)
        Noise = (Noise + 1) * 256 - 1;

    int   blurRadius = (int)((Sensibility / 200.0) + 1.0);   // Gaussian blur infrared hightlight effect 
                                                             // [2 to 5].
    float greenBoost = 2.1 - (Sensibility / 2000.0);         // Infrared green color boost [1.7 to 2.0].

    int nRand, offset, progress;

    uchar*      pBWBits = 0;    // Black and White conversion.
    uchar*  pBWBlurBits = 0;    // Black and White with blur.
    uchar*   pGrainBits = 0;    // Grain blured without curves adjustment.
    uchar*    pMaskBits = 0;    // Grain mask with curves adjustment.
    uchar* pOverlayBits = 0;    // Overlay to merge with original converted in gray scale.
    uchar*     pOutBits = m_destImage.bits(); // Destination image with merged grain mask and original.

    Digikam::DColor bwData, bwBlurData, grainData, maskData, overData, outData;

    //------------------------------------------
    // 1 - Create GrayScale green boosted image.
    //------------------------------------------

    // Convert to gray scale with boosting Green channel. 
    // Infrared film increase green color.

    Digikam::DImg BWImage(Width, Height, sixteenBit);   // Black and White conversion.
    pBWBits = BWImage.bits();
    memcpy (pBWBits, data, numBytes);

    Digikam::DImgImageFilters().channelMixerImage(pBWBits, Width, Height, sixteenBit, // Image data.
                                                  true,                   // Preserve luminosity.
                                                  true,                   // Monochrome.
                                                  0.4, greenBoost, -0.8,  // Red channel gains.
                                                  0.0, 1.0,         0.0,  // Green channel gains (not used).
                                                  0.0, 0.0,         1.0); // Blue channel gains (not used).
    postProgress( 10 );
    if (m_cancel)
    {
        return;
    }

    // Apply a Gaussian blur to the black and white image.
    // This way simulate Infrared film dispersion for the highlights.

    Digikam::DImg BWBlurImage(Width, Height, sixteenBit);
    pBWBlurBits = BWBlurImage.bits();

    Digikam::DImgGaussianBlur(this, BWImage, BWBlurImage, 10, 20, blurRadius);

    if (m_cancel)
    {
        return;
    }

    //-----------------------------------------------------------------
    // 2 - Create Gaussian blured averlay mask with grain if necessary.
    //-----------------------------------------------------------------


    if (Grain)
    {

        // Create gray grain mask.

        QDateTime dt = QDateTime::currentDateTime();
        QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
        uint seed = ((uint) dt.secsTo(Y2000));

        pGrainBits = new uchar[numBytes];    // Grain blured without curves adjustment.
        uchar *ptr;
        int component;
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
            progress = (int) (30.0 + ((double)x * 10.0) / Width);

            if (progress%5 == 0)
                postProgress( progress );
        }

        // Smooth grain mask using gaussian blur.    

        Digikam::DImgImageFilters().gaussianBlurImage(pGrainBits, Width, Height, sixteenBit, 1);

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

    if (Grain)
    {
        Digikam::ImageCurves *grainCurves = new Digikam::ImageCurves(sixteenBit);
        pMaskBits = new uchar[numBytes];    // Grain mask with curves adjustment.

        // We modify only global luminosity of the grain.
        if (sixteenBit)
        {
            grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint(0,   0));
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

    if (Grain)
    {
        pOverlayBits = new uchar[numBytes];    // Overlay to merge with original converted in gray scale.

        // get composer for default blending
        Digikam::DColorComposer *composer = Digikam::DColorComposer::getComposer(Digikam::DColorComposer::PorterDuffNone);
        int alpha;

        int Shade = 52; // This value control the shading pixel effect between original image and grain mask.
        if (sixteenBit)
            Shade = (Shade + 1) * 256 - 1;

        for (int x = 0; !m_cancel && x < Width; x++)
        {
            for (int y = 0; !m_cancel && y < Height; y++)
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
    for (int x = 0; !m_cancel && x < Width; x++)
    {
        for (int y = 0; !m_cancel && y < Height; y++)
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
    
    if (Grain)
        delete [] pOverlayBits;
}

}  // NameSpace DigikamInfraredImagesPlugin
