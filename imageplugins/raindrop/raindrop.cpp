/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-05-25
 * Description : Raindrop threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
 *
 * Original RainDrop algorithm copyrighted 2004-2005 by 
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
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qdeepcopy.h>
#include <qdatetime.h> 
#include <qrect.h>

// Local includes.

#include "dimg.h"
#include "dimgimagefilters.h"
#include "raindrop.h"

namespace DigikamRainDropImagesPlugin
{

RainDrop::RainDrop(Digikam::DImg *orgImage, QObject *parent, int drop, 
          int amount, int coeff, QRect *selection)
        : Digikam::DImgThreadedFilter(orgImage, parent, "RainDrop")
{
    m_drop   = drop;
    m_amount = amount;
    m_coeff  = coeff;

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

    // If we have a region selection in image, use it to apply the filter modification around,
    // else, applied the filter on the full image.

    if (m_selectedW && m_selectedH)
    {
        Digikam::DImg zone1, zone2, zone3, zone4,
                      zone1Dest, zone2Dest, zone3Dest, zone4Dest,
                      selectedImg;
        selectedImg = m_orgImage.copy(m_selectedX, m_selectedY, m_selectedW, m_selectedH);

        // Cut the original image in 4 areas without clipping region.

        zone1 = m_orgImage.copy(0, 0, m_selectedX, w);
        zone2 = m_orgImage.copy(m_selectedX, 0, m_selectedX + m_selectedW, m_selectedY);
        zone3 = m_orgImage.copy(m_selectedX, m_selectedY + m_selectedH, m_selectedX + m_selectedW, h);
        zone4 = m_orgImage.copy(m_selectedX + m_selectedW, 0, w, h);

        zone1Dest = Digikam::DImg(zone1.width(), zone1.height(), zone1.sixteenBit(), zone1.hasAlpha());
        zone2Dest = Digikam::DImg(zone2.width(), zone2.height(), zone2.sixteenBit(), zone2.hasAlpha());
        zone3Dest = Digikam::DImg(zone3.width(), zone3.height(), zone3.sixteenBit(), zone3.hasAlpha());
        zone4Dest = Digikam::DImg(zone4.width(), zone4.height(), zone4.sixteenBit(), zone4.hasAlpha());

        // Apply effect on each area.

        rainDropsImage(&zone1, &zone1Dest, 0, m_drop, m_amount, m_coeff, true, 0, 25);
        rainDropsImage(&zone2, &zone2Dest, 0, m_drop, m_amount, m_coeff, true, 25, 50);
        rainDropsImage(&zone3, &zone3Dest, 0, m_drop, m_amount, m_coeff, true, 50, 75);
        rainDropsImage(&zone4, &zone4Dest, 0, m_drop, m_amount, m_coeff, true, 75, 100);

        // Build the target image.

        m_destImage.bitBltImage(&zone1Dest, 0, 0);
        m_destImage.bitBltImage(&zone2Dest, m_selectedX, 0);
        m_destImage.bitBltImage(&zone3Dest, m_selectedX, m_selectedY + m_selectedH);
        m_destImage.bitBltImage(&zone4Dest, m_selectedX + m_selectedW, 0);
        m_destImage.bitBltImage(&selectedImg, m_selectedX, m_selectedY);
    }
    else
    {
        rainDropsImage(&m_orgImage, &m_destImage, 0, m_drop, m_amount, m_coeff, true, 0, 100);
    }
}

/* Function to apply the RainDrops effect backported from ImageProcessing version 2
 *
 * orgImage         => The image
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
void RainDrop::rainDropsImage(Digikam::DImg *orgImage, Digikam::DImg *destImage, int MinDropSize, int MaxDropSize, 
                              int Amount, int Coeff, bool bLimitRange, int progressMin, int progressMax)
{
    bool   bResp;
    int    nRandSize, i;
    int    nRandX, nRandY;
    int    nCounter = 0;
    int    nWidth   = orgImage->width();
    int    nHeight  = orgImage->height();
    bool   sixteenBit = orgImage->sixteenBit();
    int    bytesDepth = orgImage->bytesDepth();
    uchar *data = orgImage->bits();
    uchar *pResBits = destImage->bits();

    if (Amount <= 0)
        return;

    if (MinDropSize >= MaxDropSize)
        MaxDropSize = MinDropSize + 1;

    if (MaxDropSize <= 0)
        return;

    uchar *pStatusBits = new uchar[nHeight * nWidth];
    memset(pStatusBits, 0, sizeof(nHeight * nWidth));

    // Initially, copy all pixels to destination

    destImage->bitBltImage(orgImage, 0, 0);

    // Randomize.

    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    uint seed = dt.secsTo(Y2000);

    for (i = 0; !m_cancel && (i < Amount); i++)
    {
        nCounter = 0;

        do
        {
            nRandX = (int)(rand_r(&seed) * ((double)( nWidth - 1) / RAND_MAX));
            nRandY = (int)(rand_r(&seed) * ((double)(nHeight - 1) / RAND_MAX));

            nRandSize = (rand() % (MaxDropSize - MinDropSize)) + MinDropSize;

            bResp = CreateRainDrop (data, nWidth, nHeight, sixteenBit, bytesDepth,
                                    pResBits, pStatusBits,
                                    nRandX, nRandY, nRandSize, Coeff, bLimitRange);

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
}

bool RainDrop::CreateRainDrop(uchar *pBits, int Width, int Height, bool sixteenBit, int bytesDepth,
                              uchar *pResBits, uchar* pStatusBits,
                              int X, int Y, int DropSize, double Coeff, bool bLimitRange)
{
    register int w, h, nw1, nh1, nw2, nh2;
    int          nHalfSize = DropSize / 2;
    int          nBright;
    double       lfRadius, lfOldRadius, lfAngle, lfDiv;

    Digikam::DColor imageData;

    uint nTotalR, nTotalG, nTotalB, offset;
    int nBlurPixels, nBlurRadius;

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

                            imageData.setColor(pBits + Offset(Width, nw1, nh1, bytesDepth), sixteenBit);

                            if (sixteenBit)
                            {
                                // convert difference to 16-bit range
                                if (nBright > 0)
                                    nBright = (nBright + 1) * 256 - 1;
                                else
                                    nBright = (nBright - 1) * 256 + 1;

                                imageData.setRed  (LimitValues16(imageData.red()   + nBright));
                                imageData.setGreen(LimitValues16(imageData.green() + nBright));
                                imageData.setBlue (LimitValues16(imageData.blue()  + nBright));
                            }
                            else
                            {
                                imageData.setRed  (LimitValues8(imageData.red()   + nBright));
                                imageData.setGreen(LimitValues8(imageData.green() + nBright));
                                imageData.setBlue (LimitValues8(imageData.blue()  + nBright));
                            }

                            imageData.setPixel(pResBits + Offset(Width, nw2, nh2, bytesDepth));

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
                                imageData.setColor(pResBits + Offset(Width, nw2, nh2, bytesDepth), sixteenBit);

                                nTotalR += imageData.red();
                                nTotalG += imageData.green();
                                nTotalB += imageData.blue();
                                nBlurPixels++;
                            }
                        }
                    }

                    nw1 = X + w;
                    nh1 = Y + h;

                    if (IsInside (Width, Height, nw1, nh1))
                    {
                        offset = Offset(Width, nw1, nh1, bytesDepth);

                        // to preserve alpha channel
                        imageData.setColor(pResBits + offset, sixteenBit);

                        imageData.setRed  (nTotalR / nBlurPixels);
                        imageData.setGreen(nTotalG / nBlurPixels);
                        imageData.setBlue (nTotalB / nBlurPixels);

                        imageData.setPixel(pResBits + offset);
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
