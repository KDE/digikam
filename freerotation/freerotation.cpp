/* ============================================================
 * File  : freerotation.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Free rotation threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original FreeRotation algorithms copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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
 
// Degrees to radian convertion coeff (PI/180). To optimize computation.
#define DEG2RAD 0.017453292519943

// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "freerotation.h"

namespace DigikamFreeRotationImagesPlugin
{

FreeRotation::FreeRotation(QImage *orgImage, QObject *parent, double angle, bool antialiasing, 
                           int autoCrop, QColor backgroundColor, int orgW, int orgH)
            : Digikam::ThreadedFilter(orgImage, parent, "FreeRotation")
{ 
    m_angle           = angle;
    m_orgW            = orgW;
    m_orgH            = orgH;
    m_antiAlias       = antialiasing;
    m_autoCrop        = autoCrop;
    m_backgroundColor = backgroundColor;
        
    initFilter();
}

void FreeRotation::filterImage(void)
{
    register int w, h, nw, nh, j, i = 0;
    int          nNewHeight, nNewWidth;
    int          nhdx, nhdy, nhsx, nhsy;
    double       lfSin, lfCos, lfx, lfy;
    
    int nWidth  = m_orgImage.width();
    int nHeight = m_orgImage.height();
    
    uchar *pBits = m_orgImage.bits();
    
    // first of all, we need to calcule the sin and cos of the given angle
    
    lfSin = sin (m_angle * -DEG2RAD);
    lfCos = cos (m_angle * -DEG2RAD);

    // now, we have to calc the new size for the destination image
    
    if ((lfSin * lfCos) < 0)
        {
        nNewWidth  = ROUND (fabs (nWidth * lfCos - nHeight * lfSin));
        nNewHeight = ROUND (fabs (nWidth * lfSin - nHeight * lfCos));
        }
    else
        {
        nNewWidth  = ROUND (fabs (nWidth * lfCos + nHeight * lfSin));
        nNewHeight = ROUND (fabs (nWidth * lfSin + nHeight * lfCos));
        }

    // getting the destination's center position
    
    nhdx =  nNewWidth / 2;
    nhdy = nNewHeight / 2;

    // getting the source's center position
    
    nhsx =  nWidth / 2;
    nhsy = nHeight / 2;

    // now, we have to alloc a new image
    
    m_destImage.create(nNewWidth, nNewHeight, 32);
    m_destImage.fill( m_backgroundColor.rgb() );
    uchar *pResBits = m_destImage.bits();
    
    // main loop
    
    for (h = 0; !m_cancel && (h < nNewHeight); h++)
        {
        nh = h - nhdy;

        for (w = 0; !m_cancel && (w < nNewWidth); w++)
            {
            nw = w - nhdx;

            i = setPosition (nNewWidth, w, h);
            
            lfx = (double)nw * lfCos - (double)nh * lfSin + nhsx;
            lfy = (double)nw * lfSin + (double)nh * lfCos + nhsy;

            if (isInside (nWidth, nHeight, (int)lfx, (int)lfy))
                {
                if (m_antiAlias)
                    Digikam::ImageFilters::pixelAntiAliasing(pBits, nWidth, nHeight, lfx, lfy, 
                             &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
                else
                    {
                    j = setPosition (nWidth, (int)lfx, (int)lfy);

                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    }   
                }
            }
        }

    // To compute the rotated destination image size using original image dimensions.        
    int W = (int)(m_orgW * cos(m_angle * DEG2RAD) + m_orgH * sin(fabs(m_angle) * DEG2RAD));
    int H = (int)(m_orgH * cos(m_angle * DEG2RAD) + m_orgW * sin(fabs(m_angle) * DEG2RAD));
    
    // Auto-cropping destination image without black holes around.
    QRect autoCrop;
       
    switch(m_autoCrop)
       {
       case WidestArea:
           {
           // 'Widest Area' method (by Renchi).
           autoCrop.setX( (int)(nHeight * sin(fabs(m_angle) * DEG2RAD)) );
           autoCrop.setY( (int)(nWidth  * sin(fabs(m_angle) * DEG2RAD)) );
           autoCrop.setWidth(  (int)(nNewWidth  - 2*nHeight * sin(fabs(m_angle) * DEG2RAD)) );
           autoCrop.setHeight( (int)(nNewHeight - 2*nWidth  * sin(fabs(m_angle) * DEG2RAD)) );        
           m_destImage = m_destImage.copy(autoCrop);
        
           m_newSize.setWidth(  (int)(W - 2*m_orgH * sin(fabs(m_angle) * DEG2RAD)) );
           m_newSize.setHeight( (int)(H - 2*m_orgW * sin(fabs(m_angle) * DEG2RAD)) );        
           break;
           }
       
       case LargestArea:
           {
           // 'Largest Area' method (by Gerhard).       
           float gamma = atan((float)nHeight / (float)nWidth);
           autoCrop.setWidth( (int)((float)nHeight / cos(fabs(m_angle)*DEG2RAD) / 
                              ( tan(gamma) + tan(fabs(m_angle)*DEG2RAD) )) );
           autoCrop.setHeight( (int)((float)autoCrop.width() * tan(gamma)) ); 
           autoCrop.moveCenter( QPoint::QPoint(nNewWidth/2, nNewHeight/2));
           m_destImage = m_destImage.copy(autoCrop);
           
           gamma = atan((float)m_orgH / (float)m_orgW);
           m_newSize.setWidth( (int)((float)m_orgH / cos(fabs(m_angle)*DEG2RAD) / 
                               ( tan(gamma) + tan(fabs(m_angle)*DEG2RAD) )) );
           m_newSize.setHeight( (int)((float)m_newSize.width() * tan(gamma)) );        
           break;
           }
       default:   // No auto croping.
           {
           m_newSize.setWidth(  W );
           m_newSize.setHeight( H );
           break;
           }
       }
}


}  // NameSpace DigikamFreeRotationImagesPlugin
