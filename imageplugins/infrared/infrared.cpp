/* ============================================================
 * File  : infrared.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Infrared threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8)) 
  
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qdatetime.h> 

// Local includes.

#include "infrared.h"

namespace DigikamInfraredImagesPlugin
{

Infrared::Infrared(QImage *orgImage, QObject *parent, int sensibility, bool grain)
        : Digikam::ThreadedFilter(orgImage, parent, "Infrared")
{ 
    m_sensibility = sensibility;
    m_grain       = grain;
    m_name        = "Infrared";
    initFilter();    
}

void Infrared::filterImage(void)
{
    infraredImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                  m_sensibility, m_grain);
}

// This method is based on the Simulate Infrared Film tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedInfrared/

void Infrared::infraredImage(uint* data, int Width, int Height, int Sensibility, bool Grain)
{
    if (Sensibility <= 0) return;
    
    // Infrared film variables depending of Sensibility.
    // This way try to reproduce famous Ilford SFX200 infrared film
    // http://www.ilford.com/html/us_english/prod_html/sfx200/sfx200.html
    // Note : this film have a sensibility escursion from 200 to 800 ISO.
    
    int        Noise = (int)((Sensibility + 3000.0) / 10.0); // Infrared film grain.
    int   blurRadius = (int)((Sensibility / 200.0) + 1.0);   // Gaussian blur infrared hightlight effect 
                                                             // [2 to 5].
    float greenBoost = 2.1 - (Sensibility / 2000.0);         // Infrared green color boost [1.7 to 2.0].
    
    register int i;       
    int nRand, progress;

    uint*      pBWBits = new uint[Width*Height];    // Black and White conversion.
    uint*  pBWBlurBits = new uint[Width*Height];    // Black and White with blur.
    uint*   pGrainBits = new uint[Width*Height];    // Grain blured without curves adjustment.
    uint*    pMaskBits = new uint[Width*Height];    // Grain mask with curves adjustment.
    uint* pOverlayBits = new uint[Width*Height];    // Overlay to merge with original converted in gray scale.
    uint*     pOutBits = (uint*)m_destImage.bits(); // Destination image with merged grain mask and original.
    
    Digikam::ImageFilters::imageData bwData;    
    Digikam::ImageFilters::imageData bwblurData;    
    Digikam::ImageFilters::imageData grainData;    
    Digikam::ImageFilters::imageData maskData;    
    Digikam::ImageFilters::imageData overData;    
    Digikam::ImageFilters::imageData outData;    
    
    //------------------------------------------
    // 1 - Create GrayScale green boosted image.
    //------------------------------------------
    
    // Convert to gray scale with boosting Green channel. 
    // Infrared film increase green color.

    memcpy (pBWBits, data, Width*Height*sizeof(uint));  
    
    Digikam::ImageFilters::channelMixerImage((uchar*)pBWBits, Width, Height, false, // Image data.        FIXME
                                             true,                   // Preserve luminosity.    
                                             true,                   // Monochrome.
                                             0.4, greenBoost, -0.8,  // Red channel gains.
                                             0.0, 1.0,         0.0,  // Green channel gains (not used).
                                             0.0, 0.0,         1.0); // Blue channel gains (not used).
    postProgress( 10 );   
    if (m_cancel) return;

    // Apply a Gaussian blur to the black and white image.
    // This way simulate Infrared film dispersion for the highlights.

    memcpy (pBWBlurBits, pBWBits, Width*Height*sizeof(uint));  
        
    Digikam::ImageFilters::gaussianBlurImage(pBWBlurBits, Width, Height, blurRadius);
    postProgress( 20 );   
    if (m_cancel) return;

    //-----------------------------------------------------------------
    // 2 - Create Gaussian blured averlay mask with grain if necessary.
    //-----------------------------------------------------------------
    
    // Create gray grain mask.
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    for (i = 0; !m_cancel && (i < Width*Height); i++)
        {
        if (Grain)
            {
            nRand = (rand() % Noise) - (Noise / 2);
            grainData.channel.red   = CLAMP(128 + nRand, 0, 255); // Red.
            grainData.channel.green = CLAMP(128 + nRand, 0, 255); // Green.
            grainData.channel.blue  = CLAMP(128 + nRand, 0, 255); // Blue.
            grainData.channel.alpha = 0;                          // Reset Alpha (not used here).
            pGrainBits[i] = grainData.raw;
            }

        // Update de progress bar in dialog.
        progress = (int) (30.0 + ((double)i * 10.0) / (Width*Height));
        
        if (progress%5 == 0)
           postProgress( progress );   
        }

    // Smooth grain mask using gaussian blur.    
   
    if (Grain)
       Digikam::ImageFilters::gaussianBlurImage(pGrainBits, Width, Height, 1);
    
    Digikam::ImageFilters::gaussianBlurImage(pBWBlurBits, Width, Height, blurRadius);
    postProgress( 50 );   
    if (m_cancel) return;
        
    // Normally, film grain tends to be most noticable in the midtones, and much less 
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this. 
    
    if (Grain)
       {
       // FIXME : support 16 bits image properly.
       Digikam::ImageCurves *grainCurves = new Digikam::ImageCurves(false);
    
       // We modify only global luminosity of the grain.
       grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint::QPoint(0,   0));   
       grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8,  QPoint::QPoint(128, 128));
       grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 16, QPoint::QPoint(255, 0));
    
       // Calculate curves and lut to apply on grain.
       grainCurves->curvesCalculateCurve(Digikam::ImageHistogram::ValueChannel);
       grainCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
       // FIXME : support 16 bits image properly.
       grainCurves->curvesLutProcess((uchar*)pGrainBits, (uchar*)pMaskBits, Width, Height);
       delete grainCurves;
       }
    
    Digikam::ImageFilters::gaussianBlurImage(pBWBlurBits, Width, Height, blurRadius);
    postProgress( 60 );   
    if (m_cancel) return;
       
    // Merge gray scale image with grain using shade coefficient.

    int Shade = 52; // This value control the shading pixel effect between original image and grain mask.
    
    for (i = 0; !m_cancel && (i < Width*Height); i++)
        {        
        bwblurData.raw = pBWBlurBits[i];
        maskData.raw   = pMaskBits[i];
    
        if (Grain)  // Merging grain.
            {
            overData.channel.red   = (bwblurData.channel.red*(255-Shade)   + maskData.channel.red*Shade) >> 8;
            overData.channel.green = (bwblurData.channel.green*(255-Shade) + maskData.channel.green*Shade) >> 8;
            overData.channel.blue  = (bwblurData.channel.blue*(255-Shade)  + maskData.channel.blue*Shade) >> 8;
            overData.channel.alpha = bwblurData.channel.alpha;
            pOverlayBits[i] = overData.raw;
            }               
        else        // Use gray scale image without grain.
            {
            pOverlayBits[i] = bwblurData.raw;
            }
        
        // Update de progress bar in dialog.
        progress = (int) (70.0 + ((double)i * 10.0) / (Width*Height));
        
        if (progress%5 == 0)
           postProgress( progress );   
        }
    
    //------------------------------------------
    // 3 - Merge Grayscale image & overlay mask.
    //------------------------------------------
    
    // Merge overlay and gray scale image using 'Overlay' Gimp method for increase the highlight.
    // The result is usually a brighter picture. 
    // Overlay mode composite value computation is D =  A * (B + (2 * B) * (255 - A)).
    
    uint tmp, tmpM;
        
    for (i = 0; !m_cancel && (i < Width*Height); i++)
        {     
        bwData.raw            = pBWBits[i];
        overData.raw          = pOverlayBits[i];
        outData.channel.red   = INT_MULT(bwData.channel.red, bwData.channel.red + 
                                            INT_MULT(2 * overData.channel.red,
                                                    255 - bwData.channel.red, tmpM), tmp);
        outData.channel.green = INT_MULT(bwData.channel.green, bwData.channel.green + 
                                            INT_MULT(2 * overData.channel.green,
                                                    255 - bwData.channel.green, tmpM), tmp);
        outData.channel.blue  = INT_MULT(bwData.channel.blue, bwData.channel.blue + 
                                            INT_MULT(2 * overData.channel.blue,
                                                    255 - bwData.channel.blue, tmpM), tmp);
        outData.channel.alpha = bwData.channel.alpha;
        pOutBits[i]           = outData.raw;

        // Update de progress bar in dialog.
        progress = (int) (80.0 + ((double)i * 20.0) / (Width*Height));
        
        if (progress%5 == 0)
           postProgress( progress );   
        }

    delete [] pBWBits;
    delete [] pBWBlurBits;
    delete [] pGrainBits;    
    delete [] pMaskBits;
    delete [] pOverlayBits;
}

}  // NameSpace DigikamInfraredImagesPlugin
