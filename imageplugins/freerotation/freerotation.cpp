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
 
// Degrees to radian convertion coeff. to optimize computation.
#define DEG2RAD -0.017453292519943

// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qwmatrix.h> 

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "freerotation.h"

namespace DigikamFreeRotationImagesPlugin
{

FreeRotation::FreeRotation(QImage *orgImage, QObject *parent, double angle, bool antialiasing, int orgW, int orgH)
            : Digikam::ThreadedFilter(orgImage, parent, "FreeRotation")
{ 
    m_angle     = angle;
    m_orgW      = orgW;
    m_orgH      = orgH;
    m_antiAlias = antialiasing;
        
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
    
    lfSin = sin (m_angle * DEG2RAD);
    lfCos = cos (m_angle * DEG2RAD);

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
    m_newSize = m_destImage.size();
    uchar *pResBits = m_destImage.bits();
    
    // main loop
    
    for (h = 0; h < nNewHeight; h++)
        {
        nh = h - nhdy;

        for (w = 0; w < nNewWidth; w++)
            {
            nw = w - nhdx;

            i = setPosition (nNewWidth, w, h);
            
            lfx = (double)nw * lfCos - (double)nh * lfSin + nhsx;
            lfy = (double)nw * lfSin + (double)nh * lfCos + nhsy;

            if (isInside (nWidth, nHeight, (int)lfx, (int)lfy))
                {
                if (m_antiAlias)
                    antiAliasing (pBits, nWidth, nHeight, lfx, lfy, &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
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

    // To compute the destination image size using original image dimensions.        
    QWMatrix matrix;
    matrix.rotate(m_angle);
    m_newSize = matrix.mapRect(QRect::QRect(0, 0, m_orgW, m_orgH)).size();
}

inline void FreeRotation::antiAliasing (uchar *data, int Width, int Height, double X, double Y, 
                                        uchar *A, uchar *R, uchar *G, uchar *B)
{
    int nX, nY, j;
    double lfWeightX[2], lfWeightY[2], lfWeight;
    double lfTotalR = 0.0, lfTotalG = 0.0, lfTotalB = 0.0, lfTotalA = 0.0;

    nX = (int)X;
    nY = (int)Y;

    if (Y >= 0.0)
        lfWeightY[0] = 1.0 - (lfWeightY[1] = Y - (double)nY);
    else
        lfWeightY[1] = 1.0 - (lfWeightY[0] = -(Y - (double)nY));

    if (X >= 0.0)
        lfWeightX[0] = 1.0 - (lfWeightX[1] = X - (double)nX);
    else
        lfWeightX[1] = 1.0 - (lfWeightX[0] = -(X - (double)nX));

    for (int loopx = 0; loopx <= 1; loopx++)
        {
        for (int loopy = 0; loopy <= 1; loopy++)
            {
            lfWeight = lfWeightX[loopx] * lfWeightY[loopy];
            j = setPositionAdjusted (Width, Height, nX + loopx, nY + loopy);

            lfTotalB += ((double)data[j++] * lfWeight);
            lfTotalG += ((double)data[j++] * lfWeight);
            lfTotalR += ((double)data[j++] * lfWeight);
            lfTotalA += ((double)data[j++] * lfWeight);
            }
        }
         
    *B = CLAMP0255 ((int)lfTotalB);
    *G = CLAMP0255 ((int)lfTotalG);
    *R = CLAMP0255 ((int)lfTotalR);
    *A = CLAMP0255 ((int)lfTotalA);
}

}  // NameSpace DigikamFreeRotationImagesPlugin
