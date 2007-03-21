/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-07-18
 * Description : Free rotation threaded image filter.
 * 
 * Copyright 2005-2007 by Gilles Caulier
 *
 * Original FreeRotation algorithms copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

// Local includes.

#include "dimg.h"
#include "dimgimagefilters.h"
#include "freerotation.h"

namespace DigikamFreeRotationImagesPlugin
{

FreeRotation::FreeRotation(Digikam::DImg *orgImage, QObject *parent, double angle, bool antialiasing,
                           int autoCrop, QColor backgroundColor, int orgW, int orgH)
            : Digikam::DImgThreadedFilter(orgImage, parent, "FreeRotation")
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
    int          progress;
    register int w, h, nw, nh, j, i = 0;
    int          nNewHeight, nNewWidth;
    int          nhdx, nhdy, nhsx, nhsy;
    double       lfSin, lfCos, lfx, lfy;
    
    int nWidth  = m_orgImage.width();
    int nHeight = m_orgImage.height();
    
    uchar *pBits            = m_orgImage.bits();
    unsigned short *pBits16 = (unsigned short*)m_orgImage.bits();
    
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
    
    bool sixteenBit = m_orgImage.sixteenBit();
    
    m_destImage = Digikam::DImg(nNewWidth, nNewHeight, sixteenBit, m_orgImage.hasAlpha());
    m_destImage.fill( Digikam::DColor(m_backgroundColor.rgb(), sixteenBit) );

    uchar *pResBits            = m_destImage.bits();
    unsigned short *pResBits16 = (unsigned short *)m_destImage.bits();

    Digikam::DImgImageFilters filters;
    
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
                {
                    if (!sixteenBit)
                        filters.pixelAntiAliasing(pBits, nWidth, nHeight, lfx, lfy,
                                                  &pResBits[i+3], &pResBits[i+2],
                                                  &pResBits[i+1], &pResBits[i]);
                    else
                        filters.pixelAntiAliasing16(pBits16, nWidth, nHeight, lfx, lfy,
                                                    &pResBits16[i+3], &pResBits16[i+2],
                                                    &pResBits16[i+1], &pResBits16[i]);
                }
                else
                {
                    j = setPosition (nWidth, (int)lfx, (int)lfy);

                    for (int p = 0 ; p < 4 ; p++)
                    {
                        if (!sixteenBit)
                            pResBits[i] = pBits[j];
                        else
                            pResBits16[i] = pBits16[j];
                        
                        i++;
                        j++;
                    }
                }
            }
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / nNewHeight);
        if (progress%5 == 0)
            postProgress( progress );        
    }

    // Compute the rotated destination image size using original image dimensions.
    int W, H;
    double absAngle = fabs(m_angle);
    
    if (absAngle < 90.0)
    {      
        W = (int)(m_orgW * cos(absAngle * DEG2RAD) + m_orgH * sin(absAngle * DEG2RAD));
        H = (int)(m_orgH * cos(absAngle * DEG2RAD) + m_orgW * sin(absAngle * DEG2RAD));
    }
    else 
    {    
        H = (int)(m_orgW * cos((absAngle-90.0) * DEG2RAD) + m_orgH * sin((absAngle-90.0) * DEG2RAD));
        W = (int)(m_orgH * cos((absAngle-90.0) * DEG2RAD) + m_orgW * sin((absAngle-90.0) * DEG2RAD));
    }
    
    // Auto-cropping destination image without black holes around.
    QRect autoCrop;
       
    switch(m_autoCrop)
    {
        case WidestArea:
        {
           // 'Widest Area' method (by Renchi Raju).
           
           autoCrop.setX( (int)(nHeight * sin(absAngle * DEG2RAD)) );
           autoCrop.setY( (int)(nWidth  * sin(absAngle * DEG2RAD)) );
           autoCrop.setWidth(  (int)(nNewWidth  - 2*nHeight * sin(absAngle * DEG2RAD)) );
           autoCrop.setHeight( (int)(nNewHeight - 2*nWidth  * sin(absAngle * DEG2RAD)) );

           if (!autoCrop.isValid())
           {
                m_destImage = Digikam::DImg(m_orgImage.width(), m_orgImage.height(), 
                               m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
                m_destImage.fill( Digikam::DColor(m_backgroundColor.rgb(), sixteenBit) );
                m_newSize = QSize();
           }
           else
           {
                m_destImage = m_destImage.copy(autoCrop);
                m_newSize.setWidth(  (int)(W - 2*m_orgH * sin(absAngle * DEG2RAD)) );
                m_newSize.setHeight( (int)(H - 2*m_orgW * sin(absAngle * DEG2RAD)) );
           }
           break;
        }
       
        case LargestArea:
        {
           // 'Largest Area' method (by Gerhard Kulzer).
           
           float gamma = atan((float)nHeight / (float)nWidth);
           
           if (absAngle < 90.0)
           {
                autoCrop.setWidth( (int)((float)nHeight / cos(absAngle*DEG2RAD) /
                                   ( tan(gamma) + tan(absAngle*DEG2RAD) )) );
                autoCrop.setHeight( (int)((float)autoCrop.width() * tan(gamma)) );
           }
           else
           {
                autoCrop.setHeight( (int)((float)nHeight / cos((absAngle-90.0)*DEG2RAD) /
                                   ( tan(gamma) + tan((absAngle-90.0)*DEG2RAD) )) );
                autoCrop.setWidth( (int)((float)autoCrop.height() * tan(gamma)) );
           }
           
           autoCrop.moveCenter( QPoint(nNewWidth/2, nNewHeight/2));
           
           if (!autoCrop.isValid())
           {
                m_destImage = Digikam::DImg(m_orgImage.width(), m_orgImage.height(), 
                              m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
                m_destImage.fill( Digikam::DColor(m_backgroundColor.rgb(), sixteenBit) );
                m_newSize = QSize();
           }
           else
           {           
                m_destImage = m_destImage.copy(autoCrop);
                gamma = atan((float)m_orgH / (float)m_orgW);
               
                if (absAngle < 90.0)
                {
                    m_newSize.setWidth( (int)((float)m_orgH / cos(absAngle*DEG2RAD) / 
                                        ( tan(gamma) + tan(absAngle*DEG2RAD) )) );
                    m_newSize.setHeight( (int)((float)m_newSize.width() * tan(gamma)) );                     
                }
                else
                {
                    m_newSize.setHeight( (int)((float)m_orgH / cos((absAngle-90.0)*DEG2RAD) /
                                        ( tan(gamma) + tan((absAngle-90.0)*DEG2RAD) )) );
                    m_newSize.setWidth( (int)((float)m_newSize.height() * tan(gamma)) );
                }
           }
           break;
        }
        default:   // No auto croping.
        {
           m_newSize.setWidth( W );
           m_newSize.setHeight( H );
           break;
        }
    }
}

}  // NameSpace DigikamFreeRotationImagesPlugin
