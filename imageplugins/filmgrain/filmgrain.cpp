/* ============================================================
 * File  : filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : FilmGrain threaded image filter.
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
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qdatetime.h> 

// Local includes.

#include "filmgrain.h"

namespace DigikamFilmGrainImagesPlugin
{

FilmGrain::FilmGrain(QImage *orgImage, QObject *parent, int sensibility)
         : Digikam::ThreadedFilter(orgImage, parent, "FilmGrain")
{ 
    m_sensibility = sensibility;
    initFilter();
}

void FilmGrain::filterImage(void)
{
    filmgrainImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_sensibility);
}

// This method is based on the Simulate Film grain tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/FilmGrain

void FilmGrain::filmgrainImage(uint* data, int Width, int Height, int Sensibility)
{
    if (Sensibility <= 0) return;
    
    int Noise = (int)(Sensibility / 10.0);
    register int i;       
    int nRand, progress;
    
    uint* pGrainBits = new uint[Width*Height];    // Grain blured without curves adjustment.
    uint*  pMaskBits = new uint[Width*Height];    // Grain mask with curves adjustment.
    uint*   pOutBits = (uint*)m_destImage.bits(); // Destination image with merged grain mask and original.
    
    Digikam::ImageFilters::imageData inData;    
    Digikam::ImageFilters::imageData grainData;    
    Digikam::ImageFilters::imageData maskData;    
    Digikam::ImageFilters::imageData outData;    
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    // Make gray grain mask.
    
    for (i = 0; !m_cancel && (i < Width*Height); i++)
        {
        nRand = (rand() % Noise) - (Noise / 2);
        grainData.channel.red   = CLAMP(128 + nRand, 0, 255); // Red.
        grainData.channel.green = CLAMP(128 + nRand, 0, 255); // Green.
        grainData.channel.blue  = CLAMP(128 + nRand, 0, 255); // Blue.
        grainData.channel.alpha = 0;                          // Reset Alpha (not used here).
        pGrainBits[i] = grainData.raw;
        
        // Update de progress bar in dialog.
        progress = (int) (((double)i * 25.0) / (Width*Height));
        
        if (progress%5 == 0)
           postProgress( progress );   
        }

    // Smooth grain mask using gaussian blur.    
    
    Digikam::ImageFilters::gaussianBlurImage(pGrainBits, Width, Height, 1);
    
    // Update de progress bar in dialog.
    postProgress( 30 );   
            
    // Normally, film grain tends to be most noticable in the midtones, and much less 
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this. 

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
    
    // Update de progress bar in dialog.
    postProgress( 40 );   
    
    // Merge src image with grain using shade coefficient.

    int Shade = 52; // This value control the shading pixel effect between original image and grain mask.
        
    for (i = 0; !m_cancel && (i < Width*Height); i++)
        {        
        inData.raw            = data[i];
        maskData.raw          = pMaskBits[i];
        outData.channel.red   = (inData.channel.red*(255-Shade)   + maskData.channel.red*Shade) >> 8;
        outData.channel.green = (inData.channel.green*(255-Shade) + maskData.channel.green*Shade) >> 8;
        outData.channel.blue  = (inData.channel.blue*(255-Shade)  + maskData.channel.blue*Shade) >> 8;
        outData.channel.alpha = inData.channel.alpha;
        pOutBits[i]           = outData.raw;
    
        // Update de progress bar in dialog.
        progress = (int) (50.0 + ((double)i * 50.0) / (Width*Height));
        
        if (progress%5 == 0)
           postProgress( progress );   
        }
    
    delete [] pGrainBits;    
    delete [] pMaskBits;
}

}  // NameSpace DigikamFilmGrainImagesPlugin
