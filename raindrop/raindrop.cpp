/* ============================================================
 * File  : raindrop.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Raindrop threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original RainDrop algorithm copyrighted 2004-2005 by 
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
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qdeepcopy.h>
#include <qdatetime.h> 
#include <qrect.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "raindrop.h"

namespace DigikamRainDropImagesPlugin
{

RainDrop::RainDrop(QImage *orgImage, QObject *parent, int drop, 
          int amount, int coeff, QRect *selection)
        : Digikam::ThreadedFilter(orgImage, parent, "RainDrop")
{ 
    m_drop      = drop;
    m_amount    = amount;
    m_coeff     = coeff;
    
    m_selectedX = m_selectedY = m_selectedW = m_selectedH = 0;
    
    if ( selection )
       {
       m_selectedX = selection->left();
       m_selectedY = selection->top();
       m_selectedW = selection->width();
       m_selectedH = selection->height();
       }
    
    initFilter();
}

void RainDrop::filterImage(void)
{
    int w = m_orgImage.width();
    int h = m_orgImage.height();
    
    // If we have a region selection in image, use it to apply the filter modification arround,
    // else, applied the filter on the full image.
    
    if (m_selectedW && m_selectedH)     
       {
       QImage zone1, zone2, zone3, zone4, selectedImg;
       selectedImg = m_orgImage.copy(m_selectedX, m_selectedY, m_selectedW, m_selectedH);
       
       // Cut the original image in 4 area without clipping region.       
       
       zone1 = m_orgImage.copy(0, 0, m_selectedX, w);
       zone2 = m_orgImage.copy(m_selectedX, 0, m_selectedX + m_selectedW, m_selectedY);
       zone3 = m_orgImage.copy(m_selectedX, m_selectedY + m_selectedH, m_selectedX + m_selectedW, h);
       zone4 = m_orgImage.copy(m_selectedX + m_selectedW, 0, w, h);
    
       // Apply effect on each area.
       
       rainDropsImage((uint*)zone1.bits(), zone1.width(), zone1.height(), 0, m_drop, m_amount,
                      m_coeff, true, 0, 25);
       rainDropsImage((uint*)zone2.bits(), zone2.width(), zone2.height(), 0, m_drop, m_amount, 
                      m_coeff, true, 25, 50);
       rainDropsImage((uint*)zone3.bits(), zone3.width(), zone3.height(), 0, m_drop, m_amount, 
                      m_coeff, true, 50, 75);
       rainDropsImage((uint*)zone4.bits(), zone4.width(), zone4.height(), 0, m_drop, m_amount, 
                      m_coeff, true, 75, 100);
    
       // Build the target image.
       
       bitBlt( &m_destImage, 0, 0, &zone1, 0, 0, m_selectedX, w );
       bitBlt( &m_destImage, m_selectedX, 0, &zone2, 0, 0, m_selectedX + m_selectedW, m_selectedY );
       bitBlt( &m_destImage, m_selectedX, m_selectedY + m_selectedH, &zone3, 
               0, 0, m_selectedX + m_selectedW, h );
       bitBlt( &m_destImage, m_selectedX + m_selectedW, 0, &zone4, 0, 0, w, h );
       bitBlt( &m_destImage, m_selectedX, m_selectedY, &selectedImg, 
               0, 0, selectedImg.width(), selectedImg.height());
       }
    else 
       {
       QImage orgImg = m_orgImage.copy();
       rainDropsImage((uint*)orgImg.bits(), w, h, 0, m_drop, m_amount, m_coeff, true, 0, 100);
       memcpy(m_destImage.bits(), orgImg.bits(), m_destImage.numBytes());
       }
}

/* Function to apply the RainDrops effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * MinDropSize      => It's the minimum random size for rain drop.
 * MaxDropSize      => It's the minimum random size for rain drop.
 * Amount           => It's the maximum number for rain drops inside the image.
 * Coeff            => It's the fisheye's coefficient.
 * bLimitRange      => If true, the drop will not be cut.
 * progressMin      => Min. value for progress bar (can be different if using clipping area).
 * progressMax      => Max. value for progress bar (can be different if using clipping area).
 *                                                                                   
 * Theory           => This functions does several math's functions and the engine   
 *                     is simple to undestand, but a little hard to implement. A       
 *                     control will indicate if there is or not a raindrop in that       
 *                     area, if not, a fisheye effect with a random size (max=MaxDropSize)
 *                     will be applied, after this, a shadow will be applied too.       
 *                     and after this, a blur function will finish the effect.            
 */
void RainDrop::rainDropsImage(uint *data, int Width, int Height, int MinDropSize, int MaxDropSize, 
                              int Amount, int Coeff, bool bLimitRange, int progressMin, int progressMax)
{
    bool   bResp;
    int    nRandSize, i;
    int    nRandX, nRandY;
    int    nCounter = 0;
    int    nWidth   = Width;
    int    nHeight  = Height;

    if (Amount <= 0)
        return;

    if (MinDropSize >= MaxDropSize)
        MaxDropSize = MinDropSize + 1;

    if (MaxDropSize <= 0)
        return;

    uint* pResBits = new uint[Width*Height];
    memcpy (pResBits, data, Width*Height*sizeof(uint));     

    uchar *pStatusBits = new uchar[nHeight * nWidth];
    memset(pStatusBits, 0, sizeof(nHeight * nWidth));
    
    // Randomize.
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    for (i = 0; !m_cancel && (i < Amount); i++)
        {
        nCounter = 0;
        
        do 
            {
            nRandX = (int)(rand() * ((double)( nWidth - 1) / RAND_MAX));
            nRandY = (int)(rand() * ((double)(nHeight - 1) / RAND_MAX));

            nRandSize = (rand() % (MaxDropSize - MinDropSize)) + MinDropSize;

            bResp = CreateRainDrop (data, Width, Height, pResBits, pStatusBits, nRandX, nRandY,
                                    nRandSize, Coeff, bLimitRange);

            nCounter++;
            }
        while ((bResp == false) && (nCounter < 10000) && !m_cancel);

        // Update the progress bar in dialog.        
        if (nCounter >= 10000)
            {
            i = Amount;
            
            postProgress(progressMax);
            break;
            }
        
        postProgress( (int)(progressMin + ((double)(i) * 
                      (double)(progressMax-progressMin)) / (double)Amount) );
        }

    delete [] pStatusBits;
    
    if (!m_cancel) 
       memcpy (data, pResBits, Width*Height*sizeof(uint));        
                
    delete [] pResBits;
}

bool RainDrop::CreateRainDrop(uint *pBits, int Width, int Height, uint *pResBits, uchar* pStatusBits,
                              int X, int Y, int DropSize, double Coeff, bool bLimitRange)
{
    register int w, h, nw1, nh1, nw2, nh2;
    int          nHalfSize = DropSize / 2;
    int          pos1, pos2, nBright;
    double       lfRadius, lfOldRadius, lfAngle, lfDiv;
    
    // Big/Little Endian color manipulation compatibility.
    uchar red, green, blue;
    Digikam::ImageFilters::imageData imagedata;
    
    int nTotalR, nTotalG, nTotalB, nBlurPixels, nBlurRadius;

    if (CanBeDropped(Width, Height, pStatusBits, X, Y, DropSize, bLimitRange))
        {
        Coeff *= 0.01; 
        lfDiv = (double)nHalfSize / log (Coeff * (double)nHalfSize + 1.0);

        for (h = -nHalfSize; !m_cancel && (h <= nHalfSize); h++)
            {
            for (w = -nHalfSize; !m_cancel && (w <= nHalfSize); w++)
                {
                lfRadius = sqrt (h * h + w * w);
                lfAngle = atan2 (h, w);

                if (lfRadius <= (double)nHalfSize)
                    {
                    lfOldRadius = lfRadius;
                    lfRadius = (exp (lfRadius / lfDiv) - 1.0) / Coeff;
                    
                    nw1 = (int)((double)X + lfRadius * cos (lfAngle));
                    nh1 = (int)((double)Y + lfRadius * sin (lfAngle));

                    nw2 = X + w;
                    nh2 = Y + h;

                    if (IsInside(Width, Height, nw1, nh1))
                        {
                        if (IsInside(Width, Height, nw2, nh2))
                            {
                            pos1 = SetPosition(Width, nw1, nh1);
                            pos2 = SetPosition(Width, nw2, nh2);

                            nBright = 0;
                            
                            if (lfOldRadius >= 0.9 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.0) && (lfAngle < 2.25))
                                    nBright = -80;
                                else if ((lfAngle >= 2.25) && (lfAngle < 2.5))
                                    nBright = -40;
                                else if ((lfAngle >= -0.25) && (lfAngle < 0.0))
                                    nBright = -40;
                                }

                            else if (lfOldRadius >= 0.8 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.75) && (lfAngle < 1.50))
                                    nBright = -40;
                                else if ((lfAngle >= -0.10) && (lfAngle < 0.75))
                                    nBright = -30;
                                else if ((lfAngle >= 1.50) && (lfAngle < 2.35))
                                    nBright = -30;
                                }

                            else if (lfOldRadius >= 0.7 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.10) && (lfAngle < 2.0))
                                    nBright = -20;
                                else if ((lfAngle >= -2.50) && (lfAngle < -1.90))
                                    nBright = 60;
                                }
                            
                            else if (lfOldRadius >= 0.6 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.50) && (lfAngle < 1.75))
                                    nBright = -20;
                                else if ((lfAngle >= 0.0) && (lfAngle < 0.25))
                                    nBright = 20;
                                else if ((lfAngle >= 2.0) && (lfAngle < 2.25))
                                    nBright = 20;
                                }

                            else if (lfOldRadius >= 0.5 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.25) && (lfAngle < 0.50))
                                    nBright = 30;
                                else if ((lfAngle >= 1.75 ) && (lfAngle < 2.0))
                                    nBright = 30;
                                } 

                            else if (lfOldRadius >= 0.4 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.5) && (lfAngle < 1.75))
                                    nBright = 40;
                                }

                            else if (lfOldRadius >= 0.3 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.0) && (lfAngle < 2.25))
                                    nBright = 30;
                                }

                            else if (lfOldRadius >= 0.2 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.5) && (lfAngle < 1.75))
                                    nBright = 20;
                                }

                            imagedata.raw = pBits[pos1];                                
                            red           = imagedata.channel.red;
                            green         = imagedata.channel.green;
                            blue          = imagedata.channel.blue;
                            
                            imagedata.channel.red   = LimitValues(red + nBright);
                            imagedata.channel.green = LimitValues(green + nBright);
                            imagedata.channel.blue  = LimitValues(blue + nBright);
                            pResBits[pos2]        = imagedata.raw;
                            
/*                            pResBits[pos2++] = LimitValues (pBits[pos1++] + nBright);
                            pResBits[pos2++] = LimitValues (pBits[pos1++] + nBright);
                            pResBits[pos2++] = LimitValues (pBits[pos1++] + nBright);*/
                            }
                        }
                    }
                }
            }

        nBlurRadius = DropSize / 25 + 1;

        for (h = -nHalfSize - nBlurRadius; !m_cancel && (h <= nHalfSize + nBlurRadius); h++)
            {
            for (w = -nHalfSize - nBlurRadius; !m_cancel && (w <= nHalfSize + nBlurRadius); w++)
                {
                lfRadius = sqrt (h * h + w * w);

                if (lfRadius <= (double)nHalfSize * 1.1)
                    {
                    nTotalR = nTotalG = nTotalB = 0;
                    nBlurPixels = 0;

                    for (nh1 = -nBlurRadius; !m_cancel && (nh1 <= nBlurRadius); nh1++)
                        {
                        for (nw1 = -nBlurRadius; !m_cancel && (nw1 <= nBlurRadius); nw1++)
                            {
                            nw2 = X + w + nw1;
                            nh2 = Y + h + nh1;

                            if (IsInside (Width, Height, nw2, nh2))
                                {
                                pos1 = SetPosition (Width, nw2, nh2);
                                
                                imagedata.raw = pResBits[pos1];         
                                nTotalR += imagedata.channel.red;
                                nTotalG += imagedata.channel.green;
                                nTotalB += imagedata.channel.blue;
                                nBlurPixels++;
                                }
                            }
                        }

                    nw1 = X + w;
                    nh1 = Y + h;

                    if (IsInside (Width, Height, nw1, nh1))
                        {
                        pos1 = SetPosition (Width, nw1, nh1);

                        imagedata.channel.red   = nTotalR / nBlurPixels;
                        imagedata.channel.green = nTotalG / nBlurPixels;
                        imagedata.channel.blue  = nTotalB / nBlurPixels;
                        pResBits[pos1]        = imagedata.raw;
                        
                        /*pResBits[pos1++] = nTotalB / nBlurPixels;
                        pResBits[pos1++] = nTotalG / nBlurPixels;
                        pResBits[pos1++] = nTotalR / nBlurPixels;*/
                        }
                    }
                }
            }

        SetDropStatusBits (Width, Height, pStatusBits, X, Y, DropSize);
        }
    else
        return (false);

    return (true);
}


bool RainDrop::CanBeDropped(int Width, int Height, uchar *pStatusBits, int X, int Y, 
                            int DropSize, bool bLimitRange)
{
    register int w, h, i = 0;
    int nHalfSize = DropSize / 2;

    if (pStatusBits == NULL)
        return (true);
    
    for (h = Y - nHalfSize; h <= Y + nHalfSize; h++)
        {
        for (w = X - nHalfSize; w <= X + nHalfSize; w++)
            {
            if (IsInside (Width, Height, w, h))
                {
                i = h * Width + w;
                if (pStatusBits[i])
                    return (false);
                }
            else
                {
                if (bLimitRange)
                    return (false);
                }
            }
        }

    return (true);
}

bool RainDrop::SetDropStatusBits (int Width, int Height, uchar *pStatusBits, 
                                  int X, int Y, int DropSize)
{
    register int w, h, i = 0;
    int nHalfSize = DropSize / 2;

    if (pStatusBits == NULL)
        return (false);

    for (h = Y - nHalfSize; h <= Y + nHalfSize; h++)
        {
        for (w = X - nHalfSize; w <= X + nHalfSize; w++)
            {
            if (IsInside (Width, Height, w, h))
                {
                i = h * Width + w;
                pStatusBits[i] = 255;
                }
            }
        }

    return (true);
}

}  // NameSpace DigikamRainDropImagesPlugin
