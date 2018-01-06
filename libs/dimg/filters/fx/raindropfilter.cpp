/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Raindrop threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "raindropfilter.h"

// C++ includes

#include <cstdlib>

// Qt includes

#include <QDateTime>
#include <QRect>
#include <QtMath>
#include <QtConcurrent>

// Local includes

#include "dimg.h"

namespace Digikam
{

class RainDropFilter::Private
{

public:

    Private()
      : drop(80),
        amount(150),
        coeff(30),
        selection(QRect(0, 0, 0, 0))
    {
    }

    int                   drop;
    int                   amount;
    int                   coeff;

    QRect                 selection;

    RandomNumberGenerator generator;

    QMutex                lock; // RandomNumberGenerator is not re-entrant (dixit Boost lib)
};

RainDropFilter::RainDropFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

RainDropFilter::RainDropFilter(DImg* const orgImage, QObject* const parent, int drop,
                               int amount, int coeff, const QRect& selection)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("RainDrop")),
      d(new Private)
{
    d->drop      = drop;
    d->amount    = amount;
    d->coeff     = coeff;
    d->selection = selection;

    d->generator.seedByTime();

    initFilter();
}

RainDropFilter::~RainDropFilter()
{
    cancelFilter();
    delete d;
}

void RainDropFilter::filterImage()
{
    int w = m_orgImage.width();
    int h = m_orgImage.height();

    d->generator.reseed();

    // If we have a region selection in image, use it to apply the filter modification around,
    // else, applied the filter on the full image.

    if (!d->selection.size().isNull())
    {
        DImg selectedImg = m_orgImage.copy(d->selection);

        // Cut the original image in 4 areas without clipping region.

        DImg zone1       = m_orgImage.copy(0,                                       0,                                        d->selection.x(),                        h);
        DImg zone2       = m_orgImage.copy(d->selection.x(),                        0,                                        d->selection.x() + d->selection.width(), d->selection.y());
        DImg zone3       = m_orgImage.copy(d->selection.x(),                        d->selection.y() + d->selection.height(), d->selection.x() + d->selection.width(), h);
        DImg zone4       = m_orgImage.copy(d->selection.x() + d->selection.width(), 0,                                        w,                                       h);

        DImg zone1Dest   = DImg(zone1.width(), zone1.height(), zone1.sixteenBit(), zone1.hasAlpha());
        DImg zone2Dest   = DImg(zone2.width(), zone2.height(), zone2.sixteenBit(), zone2.hasAlpha());
        DImg zone3Dest   = DImg(zone3.width(), zone3.height(), zone3.sixteenBit(), zone3.hasAlpha());
        DImg zone4Dest   = DImg(zone4.width(), zone4.height(), zone4.sixteenBit(), zone4.hasAlpha());

        // Apply effect on each area.

        rainDropsImage(&zone1, &zone1Dest, 0, d->drop, d->amount, d->coeff, true, 0,  25);
        rainDropsImage(&zone2, &zone2Dest, 0, d->drop, d->amount, d->coeff, true, 25, 50);
        rainDropsImage(&zone3, &zone3Dest, 0, d->drop, d->amount, d->coeff, true, 50, 75);
        rainDropsImage(&zone4, &zone4Dest, 0, d->drop, d->amount, d->coeff, true, 75, 100);

        // Build the target image.

        m_destImage.bitBltImage(&zone1Dest,   0,                                       0);
        m_destImage.bitBltImage(&zone2Dest,   d->selection.x(),                        0);
        m_destImage.bitBltImage(&zone3Dest,   d->selection.x(),                        d->selection.y() + d->selection.height());
        m_destImage.bitBltImage(&zone4Dest,   d->selection.x() + d->selection.width(), 0);
        m_destImage.bitBltImage(&selectedImg, d->selection.x(),                        d->selection.y());
    }
    else
    {
        rainDropsImage(&m_orgImage, &m_destImage, 0, d->drop, d->amount, d->coeff, true, 0, 100);
    }
}

void RainDropFilter::rainDropsImageMultithreaded(const Args& prm)
{
    int  nRandSize;
    int  nRandX, nRandY;
    bool bResp      = false;
    int  nWidth     = prm.orgImage->width();
    int  nHeight    = prm.orgImage->height();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int  bytesDepth = prm.orgImage->bytesDepth();
    uchar* data     = prm.orgImage->bits();
    uchar* pResBits = prm.destImage->bits();

    for (uint nCounter = prm.start ; runningFlag() && (bResp == false) && (nCounter < prm.stop) ; ++nCounter)
    {
        d->lock.lock();
        nRandX    = d->generator.number(0, nWidth - 1);
        nRandY    = d->generator.number(0, nHeight - 1);
        nRandSize = d->generator.number(prm.MinDropSize, prm.MaxDropSize);
        d->lock.unlock();
        bResp     = CreateRainDrop(data, nWidth, nHeight, sixteenBit, bytesDepth,
                                   pResBits, prm.pStatusBits,
                                   nRandX, nRandY, nRandSize, prm.Coeff, prm.bLimitRange);
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
void RainDropFilter::rainDropsImage(DImg* const orgImage, DImg* const destImage, int MinDropSize, int MaxDropSize,
                                    int Amount, int Coeff, bool bLimitRange, int progressMin, int progressMax)
{
    if (Amount <= 0)
    {
        return;
    }

    if (MinDropSize >= MaxDropSize)
    {
        MaxDropSize = MinDropSize + 1;
    }

    if (MaxDropSize <= 0)
    {
        return;
    }

    QScopedArrayPointer<uchar> pStatusBits(new uchar[orgImage->height() * orgImage->width()]);
    memset(pStatusBits.data(), 0, orgImage->height() * orgImage->width() * sizeof(uchar));

    // Initially, copy all pixels to destination

    destImage->bitBltImage(orgImage, 0, 0);

    // Randomize.

    QList<int> vals = multithreadedSteps(10000);

    Args prm;
    prm.orgImage    = orgImage;
    prm.destImage   = destImage;
    prm.MinDropSize = MinDropSize;
    prm.MaxDropSize = MaxDropSize;
    prm.Coeff       = Coeff;
    prm.bLimitRange = bLimitRange;
    prm.pStatusBits = pStatusBits.data();

    for (int i = 0; runningFlag() && (i < Amount); ++i)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start        = vals[j];
            prm.stop         = vals[j+1];

            tasks.append(QtConcurrent::run(this,
                                           &RainDropFilter::rainDropsImageMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        postProgress((int)(progressMin + ((double)(i) *
                                          (double)(progressMax - progressMin)) / (double)Amount));
    }
}

bool RainDropFilter::CreateRainDrop(uchar* const pBits, int Width, int Height, bool sixteenBit, int bytesDepth,
                                    uchar* const pResBits, uchar* const pStatusBits,
                                    int X, int Y, int DropSize, double Coeff, bool bLimitRange)
{
    if (CanBeDropped(Width, Height, pStatusBits, X, Y, DropSize, bLimitRange))
    {
       int w, h, nw1, nh1, nw2, nh2;
        int          nBright;
        double       lfRadius, lfOldRadius, lfAngle;
        DColor       imageData;
        uint         nTotalR, nTotalG, nTotalB, offset;
        int          nBlurPixels, nBlurRadius;
        Coeff         *= 0.01;
        int nHalfSize =  DropSize / 2;
        double lfDiv  =  (double)nHalfSize / log(Coeff * (double)nHalfSize + 1.0);

        for (h = -nHalfSize; runningFlag() && (h <= nHalfSize); ++h)
        {
            for (w = -nHalfSize; runningFlag() && (w <= nHalfSize); ++w)
            {
                lfRadius = qSqrt(h * h + w * w);
                lfAngle  = qAtan2((double)h, (double)w);

                if (lfRadius <= (double)nHalfSize)
                {
                    lfOldRadius = lfRadius;
                    lfRadius    = (qExp(lfRadius / lfDiv) - 1.0) / Coeff;

                    nw1         = (int)((double)X + lfRadius * qCos(lfAngle));
                    nh1         = (int)((double)Y + lfRadius * qSin(lfAngle));

                    nw2         = X + w;
                    nh2         = Y + h;

                    if (isInside(Width, Height, nw1, nh1))
                    {
                        if (isInside(Width, Height, nw2, nh2))
                        {
                            nBright = 0;

                            if (lfOldRadius >= 0.9 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.0) && (lfAngle < 2.25))
                                {
                                    nBright = -80;
                                }
                                else if ((lfAngle >= 2.25) && (lfAngle < 2.5))
                                {
                                    nBright = -40;
                                }
                                else if ((lfAngle >= -0.25) && (lfAngle < 0.0))
                                {
                                    nBright = -40;
                                }
                            }

                            else if (lfOldRadius >= 0.8 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.75) && (lfAngle < 1.50))
                                {
                                    nBright = -40;
                                }
                                else if ((lfAngle >= -0.10) && (lfAngle < 0.75))
                                {
                                    nBright = -30;
                                }
                                else if ((lfAngle >= 1.50) && (lfAngle < 2.35))
                                {
                                    nBright = -30;
                                }
                            }

                            else if (lfOldRadius >= 0.7 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.10) && (lfAngle < 2.0))
                                {
                                    nBright = -20;
                                }
                                else if ((lfAngle >= -2.50) && (lfAngle < -1.90))
                                {
                                    nBright = 60;
                                }
                            }

                            else if (lfOldRadius >= 0.6 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.50) && (lfAngle < 1.75))
                                {
                                    nBright = -20;
                                }
                                else if ((lfAngle >= 0.0) && (lfAngle < 0.25))
                                {
                                    nBright = 20;
                                }
                                else if ((lfAngle >= 2.0) && (lfAngle < 2.25))
                                {
                                    nBright = 20;
                                }
                            }

                            else if (lfOldRadius >= 0.5 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.25) && (lfAngle < 0.50))
                                {
                                    nBright = 30;
                                }
                                else if ((lfAngle >= 1.75) && (lfAngle < 2.0))
                                {
                                    nBright = 30;
                                }
                            }

                            else if (lfOldRadius >= 0.4 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.5) && (lfAngle < 1.75))
                                {
                                    nBright = 40;
                                }
                            }

                            else if (lfOldRadius >= 0.3 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.0) && (lfAngle < 2.25))
                                {
                                    nBright = 30;
                                }
                            }

                            else if (lfOldRadius >= 0.2 * (double)nHalfSize)
                            {
                                if ((lfAngle >= 0.5) && (lfAngle < 1.75))
                                {
                                    nBright = 20;
                                }
                            }

                            imageData.setColor(pBits + pixelOffset(Width, nw1, nh1, bytesDepth), sixteenBit);

                            if (sixteenBit)
                            {
                                // convert difference to 16-bit range
                                if (nBright > 0)
                                {
                                    nBright = (nBright + 1) * 256 - 1;
                                }
                                else
                                {
                                    nBright = (nBright - 1) * 256 + 1;
                                }

                                imageData.setRed(limitValues16(imageData.red()     + nBright));
                                imageData.setGreen(limitValues16(imageData.green() + nBright));
                                imageData.setBlue(limitValues16(imageData.blue()   + nBright));
                            }
                            else
                            {
                                imageData.setRed(limitValues8(imageData.red()     + nBright));
                                imageData.setGreen(limitValues8(imageData.green() + nBright));
                                imageData.setBlue(limitValues8(imageData.blue()   + nBright));
                            }

                            imageData.setPixel(pResBits + pixelOffset(Width, nw2, nh2, bytesDepth));

                        }
                    }
                }
            }
        }

        nBlurRadius = DropSize / 25 + 1;

        for (h = -nHalfSize - nBlurRadius; runningFlag() && (h <= nHalfSize + nBlurRadius); ++h)
        {
            for (w = -nHalfSize - nBlurRadius; runningFlag() && (w <= nHalfSize + nBlurRadius); ++w)
            {
                lfRadius = qSqrt(h * h + w * w);

                if (lfRadius <= (double)nHalfSize * 1.1)
                {
                    nTotalR     = nTotalG = nTotalB = 0;
                    nBlurPixels = 0;

                    for (nh1 = -nBlurRadius; runningFlag() && (nh1 <= nBlurRadius); ++nh1)
                    {
                        for (nw1 = -nBlurRadius; runningFlag() && (nw1 <= nBlurRadius); ++nw1)
                        {
                            nw2 = X + w + nw1;
                            nh2 = Y + h + nh1;

                            if (isInside(Width, Height, nw2, nh2))
                            {
                                imageData.setColor(pResBits + pixelOffset(Width, nw2, nh2, bytesDepth), sixteenBit);

                                nTotalR += imageData.red();
                                nTotalG += imageData.green();
                                nTotalB += imageData.blue();
                                ++nBlurPixels;
                            }
                        }
                    }

                    nw1 = X + w;
                    nh1 = Y + h;

                    if (isInside(Width, Height, nw1, nh1))
                    {
                        offset = pixelOffset(Width, nw1, nh1, bytesDepth);

                        // to preserve alpha channel
                        imageData.setColor(pResBits + offset, sixteenBit);

                        imageData.setRed(nTotalR   / nBlurPixels);
                        imageData.setGreen(nTotalG / nBlurPixels);
                        imageData.setBlue(nTotalB  / nBlurPixels);

                        imageData.setPixel(pResBits + offset);
                    }
                }
            }
        }

        SetDropStatusBits(Width, Height, pStatusBits, X, Y, DropSize);
    }
    else
    {
        return false;
    }

    return true;
}

bool RainDropFilter::CanBeDropped(int Width, int Height, uchar* const pStatusBits, int X, int Y,
                                  int DropSize, bool bLimitRange)
{
   int w, h, i = 0;
    int          nHalfSize = DropSize / 2;

    if (!pStatusBits)
    {
        return true;
    }

    for (h = Y - nHalfSize; h <= Y + nHalfSize; ++h)
    {
        for (w = X - nHalfSize; w <= X + nHalfSize; ++w)
        {
            if (isInside(Width, Height, w, h))
            {
                i = h * Width + w;

                if (pStatusBits[i])
                {
                    return false;
                }
            }
            else
            {
                if (bLimitRange)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool RainDropFilter::SetDropStatusBits(int Width, int Height, uchar* const pStatusBits,
                                       int X, int Y, int DropSize)
{
   int w, h, i = 0;
    int nHalfSize = DropSize / 2;

    if (!pStatusBits)
    {
        return false;
    }

    for (h = Y - nHalfSize; h <= Y + nHalfSize; ++h)
    {
        for (w = X - nHalfSize; w <= X + nHalfSize; ++w)
        {
            if (isInside(Width, Height, w, h))
            {
                i = h * Width + w;
                pStatusBits[i] = 255;
            }
        }
    }

    return true;
}

FilterAction RainDropFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("amount"),     d->amount);
    action.addParameter(QLatin1String("coeff"),      d->coeff);
    action.addParameter(QLatin1String("drop"),       d->drop);
    action.addParameter(QLatin1String("selectedH"),  d->selection.height());
    action.addParameter(QLatin1String("selectedW"),  d->selection.width());
    action.addParameter(QLatin1String("selectedX"),  d->selection.x());
    action.addParameter(QLatin1String("selectedY"),  d->selection.y());
    action.addParameter(QLatin1String("randomSeed"), d->generator.currentSeed());

    return action;
}

void RainDropFilter::readParameters(const FilterAction& action)
{
    int x=0, y=0, w=0, h=0;
    d->amount    = action.parameter(QLatin1String("amount")).toInt();
    d->coeff     = action.parameter(QLatin1String("coeff")).toInt();
    d->drop      = action.parameter(QLatin1String("drop")).toInt();
    h            = action.parameter(QLatin1String("selectedH")).toInt();
    w            = action.parameter(QLatin1String("selectedW")).toInt();
    x            = action.parameter(QLatin1String("selectedX")).toInt();
    y            = action.parameter(QLatin1String("selectedY")).toInt();
    d->selection = QRect(x, y, w, h);
    d->generator.seed(action.parameter(QLatin1String("randomSeed")).toUInt());
}

int RainDropFilter::limitValues8(int ColorValue)
{
    if (ColorValue > 255)
    {
        ColorValue = 255;
    }

    if (ColorValue < 0)
    {
        ColorValue = 0;
    }

    return ColorValue;
}

int RainDropFilter::limitValues16(int ColorValue)
{
    if (ColorValue > 65535)
    {
        ColorValue = 65535;
    }

    if (ColorValue < 0)
    {
        ColorValue = 0;
    }

    return ColorValue;
}

bool RainDropFilter::isInside (int Width, int Height, int X, int Y)
{
    bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
    bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
    return (bIsWOk && bIsHOk);
}

int RainDropFilter::pixelOffset(int Width, int X, int Y, int bytesDepth)
{
    return (Y * Width * bytesDepth + X * bytesDepth);
}

}  // namespace Digikam
