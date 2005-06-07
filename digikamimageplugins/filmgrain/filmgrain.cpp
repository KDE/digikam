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
         : Digikam::ThreadedFilter(orgImage, parent)
{ 
    m_sensibility      = sensibility;
    m_name             = "FilmGrain";
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
    int nStride = GetStride(Width);
    register int h, w, i = 0;       
    int nRand;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int      BitCount = LineWidth * Height;
    uchar*    pInBits = (uchar*)data;
    uchar* pGrainBits = new uchar[BitCount];    // Grain blured without curves adjustment.
    uchar*  pMaskBits = new uchar[BitCount];    // Grain mask with curves adjustment.
    uchar*   pOutBits = m_destImage.bits();     // Destination image with grain mask and original image merged.
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    // Make gray grain mask.
    
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            nRand = (rand() % Noise) - (Noise / 2);
            
            pGrainBits[i++] = LimitValues (128 + nRand);    // Red.
            pGrainBits[i++] = LimitValues (128 + nRand);    // Green.
            pGrainBits[i++] = LimitValues (128 + nRand);    // Blue.
            pGrainBits[i++] = 0;                            // Reset Alpha (not used here).
            }
        
        // Update de progress bar in dialog.
        m_eventData.starting = true;
        m_eventData.success  = false;
        m_eventData.progress = (int) (((double)h * 25.0) / Height);
        QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
        }

    // Smooth grain mask using gaussian blur.    
    
    Digikam::ImageFilters::gaussianBlurImage((uint *)pGrainBits, Width, Height, 3);
            
    // Normally, film grain tends to be most noticable in the midtones, and much less 
    // so in the shadows and highlights. Adjust histogram curve to adjust grain like this. 

    Digikam::ImageCurves *grainCurves = new Digikam::ImageCurves();
    
    // We modify only global luminosity of the grain.
    grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint::QPoint(0,   0));   
    grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8,  QPoint::QPoint(128, 128));
    grainCurves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 16, QPoint::QPoint(255, 0));
    
    // Calculate curves and lut to apply on grain.
    grainCurves->curvesCalculateCurve(Digikam::ImageHistogram::ValueChannel);
    grainCurves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
    grainCurves->curvesLutProcess((uint *)pGrainBits, (uint *)pMaskBits, Width, Height);
    delete grainCurves;
    
    // Merge src image with grain using shade coefficient.

    int Shade = 32; // This value control the shading pixel effect between original image and grain mask.
    i = 0;
        
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {        
            pOutBits[i++] = (pInBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Red.
            pOutBits[i++] = (pInBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Green.
            pOutBits[i++] = (pInBits[i] * (255 - Shade) + pMaskBits[i] * Shade) >> 8;    // Blue.
            pOutBits[i++] = pInBits[i];                                                  // Alpha.
            }
        
        // Update de progress bar in dialog.
        m_eventData.starting = true;
        m_eventData.success  = false;
        m_eventData.progress = (int) (50.0 + ((double)h * 50.0) / Height);
        QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
        }
    
    // Copy target image to destination.
    
    if (!m_cancel) 
       memcpy (data, pOutBits, BitCount);        
                
    delete [] pGrainBits;    
    delete [] pMaskBits;
}

}  // NameSpace DigikamFilmGrainImagesPlugin
