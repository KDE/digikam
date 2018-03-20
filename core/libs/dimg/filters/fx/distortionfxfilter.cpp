/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Distortion FX threaded image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by
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

#define ANGLE_RATIO 0.017453292519943295769236907685

#include "distortionfxfilter.h"

// C++ includes

#include <cstdlib>

// Qt includes

#include <QDateTime>
#include <QSize>
#include <QMutex>
#include <QtConcurrent>
#include <QtMath>

// Local includes

#include "dimg.h"
#include "pixelsaliasfilter.h"
#include "randomnumbergenerator.h"

namespace Digikam
{

class DistortionFXFilter::Private
{
public:

    Private()
    {
        antiAlias      = true;
        level          = 0;
        iteration      = 0;
        effectType     = 0;
        randomSeed     = 0;
        globalProgress = 0;
    }

    bool                   antiAlias;

    int                    level;
    int                    iteration;
    int                    effectType;
    quint32                randomSeed;

    RandomNumberGenerator generator;

    int                   globalProgress;

    QMutex                lock;
    QMutex                lock2;   // RandomNumberGenerator is not re-entrant (dixit Boost lib)
};

DistortionFXFilter::DistortionFXFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

DistortionFXFilter::DistortionFXFilter(DImg* const orgImage, QObject* const parent, int effectType,
                                       int level, int iteration, bool antialiaqSing)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("DistortionFX")),
      d(new Private)
{
    d->effectType = effectType;
    d->level      = level;
    d->iteration  = iteration;
    d->antiAlias  = antialiaqSing;
    d->randomSeed = RandomNumberGenerator::timeSeed();
    initFilter();
}

DistortionFXFilter::~DistortionFXFilter()
{
    cancelFilter();
    delete d;
}

void DistortionFXFilter::filterImage()
{
    int w = m_orgImage.width();
    int h = m_orgImage.height();
    int l = d->level;
    int f = d->iteration;

    switch (d->effectType)
    {
        case FishEye:
            fisheye(&m_orgImage, &m_destImage, (double)(l / 5.0), d->antiAlias);
            break;

        case Twirl:
            twirl(&m_orgImage, &m_destImage, l, d->antiAlias);
            break;

        case CilindricalHor:
            cilindrical(&m_orgImage, &m_destImage, (double)l, true, false, d->antiAlias);
            break;

        case CilindricalVert:
            cilindrical(&m_orgImage, &m_destImage, (double)l, false, true, d->antiAlias);
            break;

        case CilindricalHV:
            cilindrical(&m_orgImage, &m_destImage, (double)l, true, true, d->antiAlias);
            break;

        case Caricature:
            fisheye(&m_orgImage, &m_destImage, (double)(-l / 5.0), d->antiAlias);
            break;

        case MultipleCorners:
            multipleCorners(&m_orgImage, &m_destImage, l, d->antiAlias);
            break;

        case WavesHorizontal:
            waves(&m_orgImage, &m_destImage, l, f, true, true);
            break;

        case WavesVertical:
            waves(&m_orgImage, &m_destImage, l, f, true, false);
            break;

        case BlockWaves1:
            blockWaves(&m_orgImage, &m_destImage, l, f, false);
            break;

        case BlockWaves2:
            blockWaves(&m_orgImage, &m_destImage, l, f, true);
            break;

        case CircularWaves1:
            circularWaves(&m_orgImage, &m_destImage, w / 2, h / 2, (double)l, (double)f, 0.0, false, d->antiAlias);
            break;

        case CircularWaves2:
            circularWaves(&m_orgImage, &m_destImage, w / 2, h / 2, (double)l, (double)f, 25.0, true, d->antiAlias);
            break;

        case PolarCoordinates:
            polarCoordinates(&m_orgImage, &m_destImage, true, d->antiAlias);
            break;

        case UnpolarCoordinates:
            polarCoordinates(&m_orgImage, &m_destImage, false, d->antiAlias);
            break;

        case Tile:
            tile(&m_orgImage, &m_destImage, 210 - f, 210 - f, l);
            break;
    }
}

void DistortionFXFilter::fisheyeMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    double nh, nw, tw;

    DColor color;
    int offset;

    int nHalfW         = Width  / 2;
    int nHalfH         = Height / 2;
    double lfXScale    = 1.0;
    double lfYScale    = 1.0;
    double lfCoeffStep = prm.Coeff / 1000.0;
    double lfRadius, lfAngle;

    if (Width > Height)
    {
        lfYScale = (double)Width / (double)Height;
    }
    else if (Height > Width)
    {
        lfXScale = (double)Height / (double)Width;
    }

    double lfRadMax = (double)qMax(Height, Width) / 2.0;
    double lfCoeff  = lfRadMax / qLn(qFabs(lfCoeffStep) * lfRadMax + 1.0);
    double th       = lfYScale * (double)(prm.h - nHalfH);

    for (int w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        tw = lfXScale * (double)(w - nHalfW);

        // we find the distance from the center
        lfRadius = qSqrt(th * th + tw * tw);

        if (lfRadius < lfRadMax)
        {
            lfAngle = qAtan2(th, tw);

            if (prm.Coeff > 0.0)
            {
                lfRadius = (qExp(lfRadius / lfCoeff) - 1.0) / lfCoeffStep;
            }
            else
            {
                lfRadius = lfCoeff * qLn(1.0 + (-1.0 * lfCoeffStep) * lfRadius);
            }

            nw = (double)nHalfW + (lfRadius / lfXScale) * qCos(lfAngle);
            nh = (double)nHalfH + (lfRadius / lfYScale) * qSin(lfAngle);

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, prm.h, nw, nh, prm.AntiAlias);
        }
        else
        {
            // copy pixel
            offset = getOffset(Width, w, prm.h, bytesDepth);
            color.setColor(data + offset, sixteenBit);
            color.setPixel(pResBits + offset);
        }
    }
}

/* Function to apply the fisheye effect backported from ImageProcesqSing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Coeff            => Distortion effect coeff. Positive value render 'Fish Eyes' effect,
 *                     and negative values render 'Caricature' effect.
 * Antialias        => Smart blurring result.
 *
 * Theory           => This is a great effect if you take employee photos
 *                     Its pure trigonometry. I think if you study hard the code you
 *                     understand very well.
 */
void DistortionFXFilter::fisheye(DImg* orgImage, DImg* destImage, double Coeff, bool AntiAlias)
{
    if (Coeff == 0.0)
    {
        return;
    }

    int progress;

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Coeff     = Coeff;
    prm.AntiAlias = AntiAlias;

    // main loop

    for (int h = 0; runningFlag() && (h < (int)orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::fisheyeMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)(h) * 100.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void DistortionFXFilter::twirlMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    DColor color;
    int offset;

    int    nHalfW   = Width / 2;
    int    nHalfH   = Height / 2;
    double lfXScale = 1.0;
    double lfYScale = 1.0;
    double lfAngle, lfNewAngle, lfAngleSum, lfCurrentRadius;
    double tw, nh, nw;

    if (Width > Height)
    {
        lfYScale = (double)Width / (double)Height;
    }
    else if (Height > Width)
    {
        lfXScale = (double)Height / (double)Width;
    }

    // the angle step is dist divided by 10000
    double lfAngleStep = prm.dist / 10000.0;
    // now, we get the minimum radius
    double lfRadMax    = (double)qMax(Width, Height) / 2.0;

    double th          = lfYScale * (double)(prm.h - nHalfH);

    for (int w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        tw = lfXScale * (double)(w - nHalfW);

        // now, we get the distance
        lfCurrentRadius = qSqrt(th * th + tw * tw);

        // if distance is less than maximum radius...
        if (lfCurrentRadius < lfRadMax)
        {
            // we find the angle from the center
            lfAngle = qAtan2(th, tw);
            // we get the accumuled angle
            lfAngleSum = lfAngleStep * (-1.0 * (lfCurrentRadius - lfRadMax));
            // ok, we sum angle with accumuled to find a new angle
            lfNewAngle = lfAngle + lfAngleSum;

            // now we find the exact position's x and y
            nw = (double)nHalfW + qCos(lfNewAngle) * (lfCurrentRadius / lfXScale);
            nh = (double)nHalfH + qSin(lfNewAngle) * (lfCurrentRadius / lfYScale);

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, prm.h, nw, nh, prm.AntiAlias);
        }
        else
        {
            // copy pixel
            offset = getOffset(Width, w, prm.h, bytesDepth);
            color.setColor(data + offset, sixteenBit);
            color.setPixel(pResBits + offset);
        }
    }
}

/* Function to apply the twirl effect backported from ImageProcesqSing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * dist             => Distance value.
 * Antialias        => Smart blurring result.
 *
 * Theory           => Take spiral studies, you will understand better, I'm studying
 *                     hard on this effect, because it is not too fast.
 */
void DistortionFXFilter::twirl(DImg* orgImage, DImg* destImage, int dist, bool AntiAlias)
{
    // if dist value is zero, we do nothing

    if (dist == 0)
    {
        return;
    }

    int progress;

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.dist      = dist;
    prm.AntiAlias = AntiAlias;

    // main loop

    for (int h = 0; runningFlag() && (h < (int)orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::twirlMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void DistortionFXFilter::cilindricalMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    double nh, nw;

    int    nHalfW      = Width / 2;
    int    nHalfH      = Height / 2;
    double lfCoeffX    = 1.0;
    double lfCoeffY    = 1.0;
    double lfCoeffStep = prm.Coeff / 1000.0;

    if (prm.Horizontal)
    {
        lfCoeffX = (double)nHalfW / qLn(qFabs(lfCoeffStep) * nHalfW + 1.0);
    }

    if (prm.Vertical)
    {
        lfCoeffY = (double)nHalfH / qLn(qFabs(lfCoeffStep) * nHalfH + 1.0);
    }

    for (int w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        // we find the distance from the center
        nh = qFabs((double)(prm.h - nHalfH));
        nw = qFabs((double)(w - nHalfW));

        if (prm.Horizontal)
        {
            if (prm.Coeff > 0.0)
            {
                nw = (qExp(nw / lfCoeffX) - 1.0) / lfCoeffStep;
            }
            else
            {
                nw = lfCoeffX * qLn(1.0 + (-1.0 * lfCoeffStep) * nw);
            }
        }

        if (prm.Vertical)
        {
            if (prm.Coeff > 0.0)
            {
                nh = (qExp(nh / lfCoeffY) - 1.0) / lfCoeffStep;
            }
            else
            {
                nh = lfCoeffY * qLn(1.0 + (-1.0 * lfCoeffStep) * nh);
            }
        }

        nw = (double)nHalfW + ((w >= nHalfW)     ? nw : -nw);
        nh = (double)nHalfH + ((prm.h >= nHalfH) ? nh : -nh);

        setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, prm.h, nw, nh, prm.AntiAlias);
    }
}

/* Function to apply the Cilindrical effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Coeff            => Cilindrical value.
 * Horizontal       => Apply horizontally.
 * Vertical         => Apply vertically.
 * Antialias        => Smart blurring result.
 *
 * Theory           => This is a great effect, similar to Spherize (Photoshop).
 *                     If you understand FishEye, you will understand Cilindrical
 *                     FishEye apply a logarithm function using a sphere radius,
 *                     Spherize use the same function but in a rectangular
 *                     environment.
 */
void DistortionFXFilter::cilindrical(DImg* orgImage, DImg* destImage, double Coeff,
                                     bool Horizontal, bool Vertical, bool AntiAlias)

{
    if ((Coeff == 0.0) || (!(Horizontal || Vertical)))
    {
        return;
    }

    int progress;

    // initial copy
    memcpy(destImage->bits(), orgImage->bits(), orgImage->numBytes());

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage   = orgImage;
    prm.destImage  = destImage;
    prm.Coeff      = Coeff;
    prm.Horizontal = Horizontal;
    prm.Vertical   = Vertical;
    prm.AntiAlias  = AntiAlias;

    // main loop

    for (int h = 0; runningFlag() && (h < (int)orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::cilindricalMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void DistortionFXFilter::multipleCornersMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    double nh, nw;

    int    nHalfW   = Width / 2;
    int    nHalfH   = Height / 2;
    double lfRadMax = qSqrt(Height * Height + Width * Width) / 2.0;
    double lfAngle, lfNewRadius, lfCurrentRadius;

    for (int w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        // we find the distance from the center
        nh = nHalfH - prm.h;
        nw = nHalfW - w;

        // now, we get the distance
        lfCurrentRadius = qSqrt(nh * nh + nw * nw);
        // we find the angle from the center
        lfAngle = qAtan2(nh, nw) * (double)prm.Factor;

        // ok, we sum angle with accumuled to find a new angle
        lfNewRadius = lfCurrentRadius * lfCurrentRadius / lfRadMax;

        // now we find the exact position's x and y
        nw = (double)nHalfW - (qCos(lfAngle) * lfNewRadius);
        nh = (double)nHalfH - (qSin(lfAngle) * lfNewRadius);

        setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, prm.h, nw, nh, prm.AntiAlias);
    }
}

/* Function to apply the Multiple Corners effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Factor           => nb corners.
 * Antialias        => Smart blurring result.
 *
 * Theory           => This is an amazing function, you've never seen this before.
 *                     I was testing some trigonometric functions, and I saw that if
 *                     I multiply the angle by 2, the result is an image like this
 *                     If we multiply by 3, we can create the SixCorners effect.
 */
void DistortionFXFilter::multipleCorners(DImg* orgImage, DImg* destImage, int Factor, bool AntiAlias)
{
    if (Factor == 0)
    {
        return;
    }

    int progress;

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Factor    = Factor;
    prm.AntiAlias = AntiAlias;

    // main loop

    for (int h = 0; runningFlag() && (h < (int)orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::multipleCornersMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void DistortionFXFilter::wavesHorizontalMultithreaded(const Args& prm)
{
    int oldProgress=0, progress=0, tx;

    for (int h = prm.start; runningFlag() && (h < prm.stop); ++h)
    {
        tx = lround(prm.Amplitude * qSin((prm.Frequency * 2) * h * (M_PI / 180)));
        prm.destImage->bitBltImage(prm.orgImage, 0, h,  prm.orgImage->width(), 1,  tx, h);

        if (prm.FillSides)
        {
            prm.destImage->bitBltImage(prm.orgImage, prm.orgImage->width() - tx, h,  tx, 1,  0, h);
            prm.destImage->bitBltImage(prm.orgImage, 0, h, prm.orgImage->width() - (prm.orgImage->width() - 2 * prm.Amplitude + tx), 1,  prm.orgImage->width() + tx, h);
        }

        // Update the progress bar in dialog.
        progress = (int)( ( (double)h * (100.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (prm.stop - prm.start));

        if ((progress % 5 == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress       = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }
}

void DistortionFXFilter::wavesVerticalMultithreaded(const Args& prm)
{
    int oldProgress=0, progress=0, ty;

    for (int w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        ty = lround(prm.Amplitude * qSin((prm.Frequency * 2) * w * (M_PI / 180)));
        prm.destImage->bitBltImage(prm.orgImage, w, 0, 1, prm.orgImage->height(), w, ty);

        if (prm.FillSides)
        {
            prm.destImage->bitBltImage(prm.orgImage, w, prm.orgImage->height() - ty,  1, ty,  w, 0);
            prm.destImage->bitBltImage(prm.orgImage, w, 0,  1, prm.orgImage->height() - (prm.orgImage->height() - 2 * prm.Amplitude + ty),  w, prm.orgImage->height() + ty);
        }

        // Update the progress bar in dialog.
        progress = (int)( ( (double)w * (100.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (prm.stop - prm.start));

        if ((progress % 5 == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress       = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }
}

/* Function to apply the waves effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Amplitude        => Sinoidal maximum height.
 * Frequency        => Frequency value.
 * FillSides        => Like a boolean variable.
 * Direction        => Vertical or horizontal flag.
 *
 * Theory           => This is an amazing effect, very funny, and very simple to
 *                     understand. You just need understand how qSin and qCos works.
 */
void DistortionFXFilter::waves(DImg* orgImage, DImg* destImage,
                               int Amplitude, int Frequency,
                               bool FillSides, bool Direction)
{
    if (Amplitude < 0)
    {
        Amplitude = 0;
    }

    if (Frequency < 0)
    {
        Frequency = 0;
    }

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Amplitude = Amplitude;
    prm.Frequency = Frequency;
    prm.FillSides = FillSides;

    if (Direction)        // Horizontal
    {
        QList<int> vals = multithreadedSteps(orgImage->height());
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::wavesHorizontalMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();
    }
    else
    {
        QList<int> vals = multithreadedSteps(orgImage->width());
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::wavesVerticalMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();
    }
}

void DistortionFXFilter::blockWavesMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    int nw, nh;

    DColor color;
    int offset, offsetOther;

    int nHalfW = Width  / 2;
    int nHalfH = Height / 2;

    for (int h = prm.start; runningFlag() && (h < prm.stop); ++h)
    {
        nw = nHalfW - prm.w;
        nh = nHalfH - h;

        if (prm.Mode)
        {
            nw = (int)(prm.w + prm.Amplitude * qSin(prm.Frequency * nw * (M_PI / 180)));
            nh = (int)(h     + prm.Amplitude * qCos(prm.Frequency * nh * (M_PI / 180)));
        }
        else
        {
            nw = (int)(prm.w + prm.Amplitude * qSin(prm.Frequency * prm.w * (M_PI / 180)));
            nh = (int)(h     + prm.Amplitude * qCos(prm.Frequency * h     * (M_PI / 180)));
        }

        offset      = getOffset(Width, prm.w, h, bytesDepth);
        offsetOther = getOffsetAdjusted(Width, Height, (int)nw, (int)nh, bytesDepth);

        // read color
        color.setColor(data + offsetOther, sixteenBit);
        // write color to destination
        color.setPixel(pResBits + offset);
    }
}

/* Function to apply the block waves effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Amplitude        => Sinoidal maximum height
 * Frequency        => Frequency value
 * Mode             => The mode to be applied.
 *
 * Theory           => This is an amazing effect, very funny when amplitude and
 *                     frequency are small values.
 */
void DistortionFXFilter::blockWaves(DImg* orgImage, DImg* destImage,
                                    int Amplitude, int Frequency, bool Mode)
{
    if (Amplitude < 0)
    {
        Amplitude = 0;
    }

    if (Frequency < 0)
    {
        Frequency = 0;
    }

    int progress;

    QList<int> vals = multithreadedSteps(orgImage->height());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Mode      = Mode;
    prm.Frequency = Frequency;
    prm.Amplitude = Amplitude;

    for (int w = 0; runningFlag() && (w < (int)orgImage->width()); ++w)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.w     = w;
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::blockWavesMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)w * 100.0) / orgImage->width());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void DistortionFXFilter::circularWavesMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    double nh, nw;

    double lfRadius, lfRadMax;
    double lfNewAmp     = prm.Amplitude;
    double lfFreqAngle  = prm.Frequency * ANGLE_RATIO;
    double phase        = prm.Phase     * ANGLE_RATIO;
    lfRadMax            = qSqrt(Height * Height + Width * Width);

    for (int w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        nw = prm.X - w;
        nh = prm.Y - prm.h;

        lfRadius = qSqrt(nw * nw + nh * nh);

        if (prm.WavesType)
        {
            lfNewAmp = prm.Amplitude * lfRadius / lfRadMax;
        }

        nw = (double)w     + lfNewAmp * qSin(lfFreqAngle * lfRadius + phase);
        nh = (double)prm.h + lfNewAmp * qCos(lfFreqAngle * lfRadius + phase);

        setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, prm.h, nw, nh, prm.AntiAlias);
    }
}

/* Function to apply the circular waves effect backported from ImageProcesqSing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * X, Y             => Position of circle center on the image.
 * Amplitude        => Sinoidal maximum height
 * Frequency        => Frequency value.
 * Phase            => Phase value.
 * WavesType        => If true  the amplitude is proportional to radius.
 * Antialias        => Smart bluring result.
 *
 * Theory           => Similar to Waves effect, but here I apply a senoidal function
 *                     with the angle point.
 */
void DistortionFXFilter::circularWaves(DImg* orgImage, DImg* destImage, int X, int Y, double Amplitude,
                                       double Frequency, double Phase, bool WavesType, bool AntiAlias)
{
    if (Amplitude < 0.0)
    {
        Amplitude = 0.0;
    }

    if (Frequency < 0.0)
    {
        Frequency = 0.0;
    }

    int progress;

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Phase     = Phase;
    prm.Frequency = Frequency;
    prm.Amplitude = Amplitude;
    prm.WavesType = WavesType;
    prm.X         = X;
    prm.Y         = Y;
    prm.AntiAlias = AntiAlias;

    for (int h = 0; runningFlag() && (h < (int)orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::circularWavesMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void DistortionFXFilter::polarCoordinatesMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    int nHalfW      = Width / 2;
    int nHalfH      = Height / 2;
    double lfXScale = 1.0;
    double lfYScale = 1.0;
    double lfAngle, lfRadius, lfRadMax;
    double nh, nw, tw;

    if (Width > Height)
    {
        lfYScale = (double)Width / (double)Height;
    }
    else if (Height > Width)
    {
        lfXScale = (double)Height / (double)Width;
    }

    lfRadMax = (double)qMax(Height, Width) / 2.0;

    double th = lfYScale * (double)(prm.h - nHalfH);

    for (int w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        tw = lfXScale * (double)(w - nHalfW);

        if (prm.Type)
        {
            // now, we get the distance
            lfRadius = qSqrt(th * th + tw * tw);
            // we find the angle from the center
            lfAngle = qAtan2(tw, th);

            // now we find the exact position's x and y
            nh = lfRadius * (double) Height / lfRadMax;
            nw =  lfAngle * (double)  Width / (2 * M_PI);

            nw = (double)nHalfW + nw;
        }
        else
        {
            lfRadius = (double)(prm.h) * lfRadMax   / (double)Height;
            lfAngle  = (double)(w)     * (2 * M_PI) / (double) Width;

            nw = (double)nHalfW - (lfRadius / lfXScale) * qSin(lfAngle);
            nh = (double)nHalfH - (lfRadius / lfYScale) * qCos(lfAngle);
        }

        setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, prm.h, nw, nh, prm.AntiAlias);
    }
}

/* Function to apply the Polar Coordinates effect backported from ImageProcesqSing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Type             => if true Polar Coordinate to Polar else inverse.
 * Antialias        => Smart blurring result.
 *
 * Theory           => Similar to PolarCoordinates from Photoshop. We apply the polar
 *                     transformation in a proportional (Height and Width) radius.
 */
void DistortionFXFilter::polarCoordinates(DImg* orgImage, DImg* destImage, bool Type, bool AntiAlias)
{
    int progress;

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Type      = Type;
    prm.AntiAlias = AntiAlias;

    // main loop

    for (int h = 0; runningFlag() && (h < (int)orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &DistortionFXFilter::polarCoordinatesMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void DistortionFXFilter::tileMultithreaded(const Args& prm)
{
    int tx, ty, progress=0, oldProgress=0;

    for (int h = prm.start; runningFlag() && (h < prm.stop); h += prm.HSize)
    {
        for (int w = 0; runningFlag() && (w < (int)prm.orgImage->width()); w += prm.WSize)
        {
            d->lock2.lock();
            tx = d->generator.number(-prm.Random / 2, prm.Random / 2);
            ty = d->generator.number(-prm.Random / 2, prm.Random / 2);
            d->lock2.unlock();
            prm.destImage->bitBltImage(prm.orgImage, w, h, prm.WSize, prm.HSize, w + tx, h + ty);
        }

        // Update the progress bar in dialog.
        progress = (int)( ( (double)h * (100.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (prm.stop - prm.start));

        if ((progress % 5 == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress       = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }
}

/* Function to apply the tile effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * WSize            => Tile Width
 * HSize            => Tile Height
 * Random           => Maximum random value
 *
 * Theory           => Similar to Tile effect from Photoshop and very easy to
 *                     understand. We get a rectangular area uqSing WSize and HSize and
 *                     replace in a position with a random distance from the original
 *                     position.
 */
void DistortionFXFilter::tile(DImg* orgImage, DImg* destImage,
                              int WSize, int HSize, int Random)
{
    if (WSize < 1)
    {
        WSize = 1;
    }

    if (HSize < 1)
    {
        HSize = 1;
    }

    if (Random < 1)
    {
        Random = 1;
    }

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.WSize     = WSize;
    prm.HSize     = HSize;
    prm.Random    = Random;

    d->generator.seed(d->randomSeed);

    QList<int> vals = multithreadedSteps(orgImage->height());
    QList <QFuture<void> > tasks;

    for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
    {
        prm.start = vals[j];
        prm.stop  = vals[j+1];
        tasks.append(QtConcurrent::run(this,
                                       &DistortionFXFilter::tileMultithreaded,
                                       prm
                                      ));
    }

    foreach(QFuture<void> t, tasks)
        t.waitForFinished();
}

/*
    This code is shared by six methods.
    Write value of pixel w|h in data to pixel nw|nh in pResBits.
    Antialias if requested.
*/
void DistortionFXFilter::setPixelFromOther(int Width, int Height, bool sixteenBit, int bytesDepth,
                                           uchar* data, uchar* pResBits,
                                           int w, int h, double nw, double nh, bool AntiAlias)
{
    DColor color;
    int offset = getOffset(Width, w, h, bytesDepth);

    if (AntiAlias)
    {
        uchar* const ptr = pResBits + offset;

        if (sixteenBit)
        {
            unsigned short* ptr16 = reinterpret_cast<unsigned short*>(ptr);
            PixelsAliasFilter().pixelAntiAliasing16(reinterpret_cast<unsigned short*>(data), Width, Height, nw, nh,
                                                    ptr16 + 3, ptr16 + 2, ptr16 + 1, ptr16);
        }
        else
        {
            PixelsAliasFilter().pixelAntiAliasing(data, Width, Height, nw, nh,
                                                  ptr + 3, ptr + 2, ptr + 1, ptr);
        }
    }
    else
    {
        // we get the position adjusted
        int offsetOther = getOffsetAdjusted(Width, Height, (int)nw, (int)nh, bytesDepth);
        // read color
        color.setColor(data + offsetOther, sixteenBit);
        // write color to destination
        color.setPixel(pResBits + offset);
    }
}

FilterAction DistortionFXFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("antiAlias"), d->antiAlias);
    action.addParameter(QLatin1String("type"),      d->effectType);
    action.addParameter(QLatin1String("iteration"), d->iteration);
    action.addParameter(QLatin1String("level"),     d->level);

    if (d->effectType == Tile)
    {
        action.addParameter(QLatin1String("randomSeed"), d->randomSeed);
    }

    return action;
}

void DistortionFXFilter::readParameters(const FilterAction& action)
{
    d->antiAlias  = action.parameter(QLatin1String("antiAlias")).toBool();
    d->effectType = action.parameter(QLatin1String("type")).toInt();
    d->iteration  = action.parameter(QLatin1String("iteration")).toInt();
    d->level      = action.parameter(QLatin1String("level")).toInt();

    if (d->effectType == Tile)
    {
        d->randomSeed = action.parameter(QLatin1String("randomSeed")).toUInt();
    }
}

}  // namespace Digikam
