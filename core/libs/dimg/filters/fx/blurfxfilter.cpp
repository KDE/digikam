/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Blur FX threaded image filter.
 *
 * Copyright 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Original Blur algorithms copyrighted 2004 by
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

#define ANGLE_RATIO  0.017453292519943295769236907685

#include "blurfxfilter.h"

// C++ includes

#include <cstdlib>
#include <cstring>

// Qt includes

#include <QDateTime>
#include <QtConcurrent>
#include <QtMath>

// Local includes

#include "dimg.h"
#include "blurfilter.h"
#include "randomnumbergenerator.h"

namespace Digikam
{

class BlurFXFilter::Private
{
public:

    Private()
    {
        blurFXType = ZoomBlur;
        distance   = 100;
        level      = 45;
        randomSeed = RandomNumberGenerator::timeSeed();
    }

    int     blurFXType;
    int     distance;
    int     level;
    quint32 randomSeed;
};

BlurFXFilter::BlurFXFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

BlurFXFilter::BlurFXFilter(DImg* const orgImage, QObject* const parent, int blurFXType, int distance, int level)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("BlurFX")),
      d(new Private)
{
    d->blurFXType = blurFXType;
    d->distance   = distance;
    d->level      = level;

    initFilter();
}

BlurFXFilter::~BlurFXFilter()
{
    cancelFilter();
    delete d;
}

void BlurFXFilter::filterImage()
{
    int w = m_orgImage.width();
    int h = m_orgImage.height();

    switch (d->blurFXType)
    {
        case ZoomBlur:
            zoomBlur(&m_orgImage, &m_destImage, w / 2, h / 2, d->distance);
            break;

        case RadialBlur:
            radialBlur(&m_orgImage, &m_destImage, w / 2, h / 2, d->distance);
            break;

        case FarBlur:
            farBlur(&m_orgImage, &m_destImage, d->distance);
            break;

        case MotionBlur:
            motionBlur(&m_orgImage, &m_destImage, d->distance, (double)d->level);
            break;

        case SoftenerBlur:
            softenerBlur(&m_orgImage, &m_destImage);
            break;

        case ShakeBlur:
            shakeBlur(&m_orgImage, &m_destImage, d->distance);
            break;

        case FocusBlur:
            focusBlur(&m_orgImage, &m_destImage, w / 2, h / 2, d->distance, d->level * 10);
            break;

        case SmartBlur:
            smartBlur(&m_orgImage, &m_destImage, d->distance, d->level);
            break;

        case FrostGlass:
            frostGlass(&m_orgImage, &m_destImage, d->distance);
            break;

        case Mosaic:
            mosaic(&m_orgImage, &m_destImage, d->distance, d->distance);
            break;
    }
}

void BlurFXFilter::zoomBlurMultithreaded(const Args& prm)
{
    int nh, nw;
    int sumR, sumG, sumB, nCount=0;
    double lfRadius, lfNewRadius, lfAngle;

    DColor color;
    int offset;

    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    double lfRadMax = qSqrt(Height * Height + Width * Width);

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        // ...we enter this loop to sum the bits

        // we initialize the variables
        sumR = sumG = sumB = nCount = 0;

        nw = prm.X - w;
        nh = prm.Y - prm.h;

        lfRadius    = qSqrt(nw * nw + nh * nh);
        lfAngle     = qAtan2((double)nh, (double)nw);
        lfNewRadius = (lfRadius * prm.Distance) / lfRadMax;

        for (int r = 0; runningFlag() && (r <= lfNewRadius); ++r)
        {
            // we need to calc the positions
            nw = (int)(prm.X - (lfRadius - r) * cos(lfAngle));
            nh = (int)(prm.Y - (lfRadius - r) * sin(lfAngle));

            if (IsInside(Width, Height, nw, nh))
            {
                // read color
                offset = GetOffset(Width, nw, nh, bytesDepth);
                color.setColor(data + offset, sixteenBit);

                // we sum the bits
                sumR += color.red();
                sumG += color.green();
                sumB += color.blue();
                ++nCount;
            }
        }

        if (nCount == 0)
        {
            nCount = 1;
        }

        // calculate pointer
        offset = GetOffset(Width, w, prm.h, bytesDepth);
        // read color to preserve alpha
        color.setColor(data + offset, sixteenBit);

        // now, we have to calc the arithmetic average
        color.setRed(sumR   / nCount);
        color.setGreen(sumG / nCount);
        color.setBlue(sumB  / nCount);

        // write color to destination
        color.setPixel(pResBits + offset);
    }
}

/* Function to apply the ZoomBlur effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * X, Y             => Center of zoom in the image
 * Distance         => Distance value
 * pArea            => Preview area.
 *
 * Theory           => Here we have a effect similar to RadialBlur mode Zoom from
 *                     Photoshop. The theory is very similar to RadialBlur, but has one
 *                     difference. Instead we use pixels with the same radius and
 *                     near angles, we take pixels with the same angle but near radius
 *                     This radius is always from the center to out of the image, we
 *                     calc a proportional radius from the center.
 */
void BlurFXFilter::zoomBlur(DImg* const orgImage, DImg* const destImage, int X, int Y, int Distance, const QRect& pArea)
{
    if (Distance <= 1)
    {
        return;
    }

    int progress;

    // We working on full image.
    int xMin = 0;
    int xMax = orgImage->width();
    int yMin = 0;
    int yMax = orgImage->height();

    // If we working in preview mode, else we using the preview area.
    if (pArea.isValid())
    {
        xMin = pArea.x();
        xMax = pArea.x() + pArea.width();
        yMin = pArea.y();
        yMax = pArea.y() + pArea.height();
    }

    QList<int> vals = multithreadedSteps(xMax, xMin);
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.X         = X;
    prm.Y         = Y;
    prm.Distance  = Distance;

    // we have reached the main loop
    for (int h = yMin; runningFlag() && (h < yMax); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::zoomBlurMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)(h - yMin) * 100.0) / (yMax - yMin));

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void BlurFXFilter::radialBlurMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    int sumR, sumG, sumB, nw, nh;
    double Radius, Angle, AngleRad;

    DColor color;
    int offset;

    QScopedArrayPointer<double> nMultArray(new double[prm.Distance * 2 + 1]);

    for (int i = -prm.Distance; i <= prm.Distance; ++i)
    {
        nMultArray[i + prm.Distance] = i * ANGLE_RATIO;
    }

    // number of added pixels
    int nCount = 0;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        // ...we enter this loop to sum the bits

        // we initialize the variables
        sumR = sumG = sumB = nCount = 0;

        nw = prm.X - w;
        nh = prm.Y - prm.h;

        Radius   = qSqrt(nw * nw + nh * nh);
        AngleRad = qAtan2((double)nh, (double)nw);

        for (int a = -prm.Distance; runningFlag() && (a <= prm.Distance); ++a)
        {
            Angle = AngleRad + nMultArray[a + prm.Distance];
            // we need to calc the positions
            nw = (int)(prm.X - Radius * qCos(Angle));
            nh = (int)(prm.Y - Radius * qSin(Angle));

            if (IsInside(Width, Height, nw, nh))
            {
                // read color
                offset = GetOffset(Width, nw, nh, bytesDepth);
                color.setColor(data + offset, sixteenBit);

                // we sum the bits
                sumR += color.red();
                sumG += color.green();
                sumB += color.blue();
                ++nCount;
            }
        }

        if (nCount == 0)
        {
            nCount = 1;
        }

        // calculate pointer
        offset = GetOffset(Width, w, prm.h, bytesDepth);
        // read color to preserve alpha
        color.setColor(data + offset, sixteenBit);

        // now, we have to calc the arithmetic average
        color.setRed(sumR   / nCount);
        color.setGreen(sumG / nCount);
        color.setBlue(sumB  / nCount);

        // write color to destination
        color.setPixel(pResBits + offset);
    }
}

/* Function to apply the radialBlur effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * X, Y             => Center of radial in the image
 * Distance         => Distance value
 * pArea            => Preview area.
 *
 * Theory           => Similar to RadialBlur from Photoshop, its an amazing effect
 *                     Very easy to understand but a little hard to implement.
 *                     We have all the image and find the center pixel. Now, we analyze
 *                     all the pixels and calc the radius from the center and find the
 *                     angle. After this, we sum this pixel with others with the same
 *                     radius, but different angles. Here I'm using degrees angles.
 */
void BlurFXFilter::radialBlur(DImg* const orgImage, DImg* const destImage, int X, int Y, int Distance, const QRect& pArea)
{
    if (Distance <= 1)
    {
        return;
    }

    int progress;

    // We working on full image.
    int xMin = 0;
    int xMax = orgImage->width();
    int yMin = 0;
    int yMax = orgImage->height();

    // If we working in preview mode, else we using the preview area.
    if (pArea.isValid())
    {
        xMin = pArea.x();
        xMax = pArea.x() + pArea.width();
        yMin = pArea.y();
        yMax = pArea.y() + pArea.height();
    }

    QList<int> vals = multithreadedSteps(xMax, xMin);
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.X         = X;
    prm.Y         = Y;
    prm.Distance  = Distance;

    // we have reached the main loop

    for (int h = yMin; runningFlag() && (h < yMax); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::radialBlurMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)(h - yMin) * 100.0) / (yMax - yMin));

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

/* Function to apply the farBlur effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Distance         => Distance value
 *
 * Theory           => This is an interesting effect, the blur is applied in that
 *                     way: (the value "1" means pixel to be used in a blur calc, ok?)
 *                     e.g. With distance = 2
 *                                            |1|1|1|1|1|
 *                                            |1|0|0|0|1|
 *                                            |1|0|C|0|1|
 *                                            |1|0|0|0|1|
 *                                            |1|1|1|1|1|
 *                     We sum all the pixels with value = 1 and apply at the pixel with*
 *                     the position "C".
 */
void BlurFXFilter::farBlur(DImg* const orgImage, DImg* const destImage, int Distance)
{
    if (Distance < 1)
    {
        return;
    }

    // we need to create our kernel
    // e.g. distance = 3, so kernel={3 1 1 2 1 1 3}

    QScopedArrayPointer<int> nKern(new int[Distance * 2 + 1]);

    for (int i = 0; i < Distance * 2 + 1; ++i)
    {
        // the first element is 3
        if (i == 0)
        {
            nKern[i] = 2;
        }
        // the center element is 2
        else if (i == Distance)
        {
            nKern[i] = 3;
        }
        // the last element is 3
        else if (i == Distance * 2)
        {
            nKern[i] = 3;
        }
        // all other elements will be 1
        else
        {
            nKern[i] = 1;
        }
    }

    // now, we apply a convolution with kernel
    MakeConvolution(orgImage, destImage, Distance, nKern.data());
}

void BlurFXFilter::motionBlurMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();
    int nCount      = prm.nCount;

    DColor color;
    int offset, sumR, sumG, sumB, nw, nh;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        // we initialize the variables
        sumR = sumG = sumB = 0;

        // ...we enter this loop to sum the bits
        for (int a = -prm.Distance; runningFlag() && (a <= prm.Distance); ++a)
        {
            // we need to calc the positions
            nw = w     + prm.lpXArray[a + prm.Distance];
            nh = prm.h + prm.lpYArray[a + prm.Distance];

            offset = GetOffsetAdjusted(Width, Height, nw, nh, bytesDepth);
            color.setColor(data + offset, sixteenBit);

            // we sum the bits
            sumR += color.red();
            sumG += color.green();
            sumB += color.blue();
        }

        if (nCount == 0)
        {
            nCount = 1;
        }

        // calculate pointer
        offset = GetOffset(Width, w, prm.h, bytesDepth);
        // read color to preserve alpha
        color.setColor(data + offset, sixteenBit);

        // now, we have to calc the arithmetic average
        color.setRed(sumR   / nCount);
        color.setGreen(sumG / nCount);
        color.setBlue(sumB  / nCount);

        // write color to destination
        color.setPixel(pResBits + offset);
    }
}

/* Function to apply the motionBlur effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Distance         => Distance value
 * Angle            => Angle direction (degrees)
 *
 * Theory           => Similar to MotionBlur from Photoshop, the engine is very
 *                     simple to understand, we take a pixel (duh!), with the angle we
 *                     will taking near pixels. After this we blur (add and do a
 *                     division).
 */
void BlurFXFilter::motionBlur(DImg* const orgImage, DImg* const destImage, int Distance, double Angle)
{
    if (Distance == 0)
    {
        return;
    }

    int progress;

    // we try to avoid division by 0 (zero)
    if (Angle == 0.0)
    {
        Angle = 360.0;
    }

    // we initialize cos and sin for a best performance
    double nAngX = cos((2.0 * M_PI) / (360.0 / Angle));
    double nAngY = sin((2.0 * M_PI) / (360.0 / Angle));

    // total of bits to be taken is given by this formula
    int nCount = Distance * 2 + 1;

    // we will alloc size and calc the possible results
    QScopedArrayPointer<int> lpXArray(new int[nCount]);
    QScopedArrayPointer<int> lpYArray(new int[nCount]);

    for (int i = 0; i < nCount; ++i)
    {
        lpXArray[i] = lround((double)(i - Distance) * nAngX);
        lpYArray[i] = lround((double)(i - Distance) * nAngY);
    }

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Distance  = Distance;
    prm.nCount    = nCount;
    prm.lpXArray  = lpXArray.data();
    prm.lpYArray  = lpYArray.data();

    // we have reached the main loop

    for (uint h = 0; runningFlag() && (h < orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::motionBlurMultithreaded,
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

void BlurFXFilter::softenerBlurMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    int SomaR = 0, SomaG = 0, SomaB = 0;
    int Gray;

    DColor color, colorSoma;
    int offset, offsetSoma;

    int grayLimit = sixteenBit ? 32767 : 127;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        SomaR = SomaG = SomaB = 0;

        offset = GetOffset(Width, w, prm.h, bytesDepth);
        color.setColor(data + offset, sixteenBit);

        Gray = (color.red() + color.green() + color.blue()) / 3;

        if (Gray > grayLimit)
        {
            // 7x7
            for (int a = -3; runningFlag() && (a <= 3); ++a)
            {
                for (int b = -3; runningFlag() && (b <= 3); ++b)
                {
                    if ((((int)prm.h + a) < 0) || (((int)w + b) < 0))
                    {
                        offsetSoma = offset;
                    }
                    else
                    {
                        offsetSoma = GetOffset(Width, (w + Lim_Max(w, b, Width)),
                                               (prm.h + Lim_Max(prm.h, a, Height)), bytesDepth);
                    }

                    colorSoma.setColor(data + offsetSoma, sixteenBit);

                    SomaR += colorSoma.red();
                    SomaG += colorSoma.green();
                    SomaB += colorSoma.blue();
                }
            }

            // 7*7 = 49
            color.setRed(SomaR   / 49);
            color.setGreen(SomaG / 49);
            color.setBlue(SomaB  / 49);
            color.setPixel(pResBits + offset);
        }
        else
        {
            // 3x3
            for (int a = -1; runningFlag() && (a <= 1); ++a)
            {
                for (int b = -1; runningFlag() && (b <= 1); ++b)
                {
                    if ((((int)prm.h + a) < 0) || (((int)w + b) < 0))
                    {
                        offsetSoma = offset;
                    }
                    else
                    {
                        offsetSoma = GetOffset(Width, (w + Lim_Max(w, b, Width)),
                                               (prm.h + Lim_Max(prm.h, a, Height)), bytesDepth);
                    }

                    colorSoma.setColor(data + offsetSoma, sixteenBit);

                    SomaR += colorSoma.red();
                    SomaG += colorSoma.green();
                    SomaB += colorSoma.blue();
                }
            }

            // 3*3 = 9
            color.setRed(SomaR   / 9);
            color.setGreen(SomaG / 9);
            color.setBlue(SomaB  / 9);
            color.setPixel(pResBits + offset);
        }
    }
}

/* Function to apply the softenerBlur effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 *
 * Theory           => An interesting blur-like function. In dark tones we apply a
 *                     blur with 3x3 dimensions, in light tones, we apply a blur with
 *                     5x5 dimensions. Easy, hun?
 */
void BlurFXFilter::softenerBlur(DImg* const orgImage, DImg* const destImage)
{
    int progress;

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;

    // we have reached the main loop

    for (uint h = 0; runningFlag() && (h < orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::softenerBlurMultithreaded,
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

void BlurFXFilter::shakeBlurStage1Multithreaded(const Args& prm)
{
    uint Width      = prm.orgImage->width();
    uint Height     = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();

    DColor color;
    int offset, offsetLayer;
    int nw, nh;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        offsetLayer = GetOffset(Width, w, prm.h, bytesDepth);

        nh = ((prm.h + prm.Distance) >= Height) ? Height - 1 : prm.h + prm.Distance;
        offset = GetOffset(Width, w, nh, bytesDepth);
        color.setColor(data + offset, sixteenBit);
        color.setPixel(prm.layer1 + offsetLayer);

        nh = (((int)prm.h - prm.Distance) < 0) ? 0 : prm.h - prm.Distance;
        offset = GetOffset(Width, w, nh, bytesDepth);
        color.setColor(data + offset, sixteenBit);
        color.setPixel(prm.layer2 + offsetLayer);

        nw = ((w + prm.Distance) >= Width) ? Width - 1 : w + prm.Distance;
        offset = GetOffset(Width, nw, prm.h, bytesDepth);
        color.setColor(data + offset, sixteenBit);
        color.setPixel(prm.layer3 + offsetLayer);

        nw = (((int)w - prm.Distance) < 0) ? 0 : w - prm.Distance;
        offset = GetOffset(Width, nw, prm.h, bytesDepth);
        color.setColor(data + offset, sixteenBit);
        color.setPixel(prm.layer4 + offsetLayer);
    }
}

void BlurFXFilter::shakeBlurStage2Multithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    DColor color, color1, color2, color3, color4;
    int offset;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        offset = GetOffset(Width, w, prm.h, bytesDepth);
        // read original data to preserve alpha
        color.setColor(data + offset, sixteenBit);
        // read colors from all four layers
        color1.setColor(prm.layer1 + offset, sixteenBit);
        color2.setColor(prm.layer2 + offset, sixteenBit);
        color3.setColor(prm.layer3 + offset, sixteenBit);
        color4.setColor(prm.layer4 + offset, sixteenBit);

        // set color components of resulting color
        color.setRed((color1.red()     + color2.red()   + color3.red()   + color4.red())   / 4);
        color.setGreen((color1.green() + color2.green() + color3.green() + color4.green()) / 4);
        color.setBlue((color1.blue()   + color2.blue()  + color3.blue()  + color4.blue())  / 4);

        color.setPixel(pResBits + offset);
    }
}

/* Function to apply the shake blur effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Distance         => Distance between layers (from origin)
 *
 * Theory           => Similar to Fragment effect from Photoshop. We create 4 layers
 *                    each one has the same distance from the origin, but have
 *                    different positions (top, button, left and right), with these 4
 *                    layers, we join all the pixels.
 */
void BlurFXFilter::shakeBlur(DImg* const orgImage, DImg* const destImage, int Distance)
{
    int progress;

    int numBytes = orgImage->numBytes();
    QScopedArrayPointer<uchar> layer1(new uchar[numBytes]);
    QScopedArrayPointer<uchar> layer2(new uchar[numBytes]);
    QScopedArrayPointer<uchar> layer3(new uchar[numBytes]);
    QScopedArrayPointer<uchar> layer4(new uchar[numBytes]);

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Distance  = Distance;
    prm.layer1    = layer1.data();
    prm.layer2    = layer2.data();
    prm.layer3    = layer3.data();
    prm.layer4    = layer4.data();

    // we have reached the main loop

    for (uint h = 0; runningFlag() && (h < orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::shakeBlurStage1Multithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 50.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    tasks.clear();

    for (uint h = 0; runningFlag() && (h < orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::shakeBlurStage2Multithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(50.0 + ((double)h * 50.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void BlurFXFilter::focusBlurMultithreaded(const Args& prm)
{
    int    nBlendFactor;
    double lfRadius;
    int    offset;

    DColor colorOrgImage, colorBlurredImage;
    int    alpha;
    uchar* ptr = 0;

    // get composer for default blending
    DColorComposer* const composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);

    int Width       = prm.orgImage->width();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    int nw = 0;
    int nh = prm.Y - prm.h;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        nw = prm.X - w;

        lfRadius = qSqrt(nh * nh + nw * nw);

        if (sixteenBit)
        {
            nBlendFactor = CLAMP065535((int)(65535.0 * lfRadius / (double)prm.BlendRadius));
        }
        else
        {
            nBlendFactor = (uchar)CLAMP0255((int)(255.0 * lfRadius / (double)prm.BlendRadius));
        }

        // Read color values
        offset = GetOffset(Width, w, prm.h, bytesDepth);
        ptr    = pResBits + offset;
        colorOrgImage.setColor(data + offset, sixteenBit);
        colorBlurredImage.setColor(ptr, sixteenBit);

        // Preserve alpha
        alpha = colorOrgImage.alpha();

        // In normal mode, the image is focused in the middle
        // and less focused towards the border.
        // In inverse mode, the image is more focused towards the edge
        // and less focused in the middle.
        // This is achieved by swapping src and dest while blending.
        if (prm.bInversed)
        {
            // set blending alpha value as src alpha. Original value is stored above.
            colorOrgImage.setAlpha(nBlendFactor);
            // compose colors, writing to dest - colorBlurredImage
            composer->compose(colorBlurredImage, colorOrgImage);
            // restore alpha
            colorBlurredImage.setAlpha(alpha);
            // write color to destination
            colorBlurredImage.setPixel(ptr);
        }
        else
        {
            // set blending alpha value as src alpha. Original value is stored above.
            colorBlurredImage.setAlpha(nBlendFactor);
            // compose colors, writing to dest - colorOrgImage
            composer->compose(colorOrgImage, colorBlurredImage);
            // restore alpha
            colorOrgImage.setAlpha(alpha);
            // write color to destination
            colorOrgImage.setPixel(ptr);
        }
    }

    delete composer;
}

/* Function to apply the focusBlur effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * BlurRadius       => Radius of blurred image.
 * BlendRadius      => Radius of blending effect.
 * bInversed        => If true, invert focus effect.
 * pArea            => Preview area.
 *
 */
void BlurFXFilter::focusBlur(DImg* const orgImage, DImg* const destImage,
                             int X, int Y, int BlurRadius, int BlendRadius,
                             bool bInversed, const QRect& pArea)
{
    int progress;

    // We working on full image.
    int xMin = 0;
    int xMax = orgImage->width();
    int yMin = 0;
    int yMax = orgImage->height();

    // If we working in preview mode, else we using the preview area.
    if (pArea.isValid())
    {
        xMin = pArea.x();
        xMax = pArea.x() + pArea.width();
        yMin = pArea.y();
        yMax = pArea.y() + pArea.height();
    }

    if (pArea.isValid())
    {
        //UNTESTED (unused)

        // We do not have access to the loop of the Gaussian blur,
        // so we have to cut the image that we run the effect on.
        int xMinBlur = xMin - BlurRadius;
        int xMaxBlur = xMax + BlurRadius;
        int yMinBlur = yMin - BlurRadius;
        int yMaxBlur = yMax + BlurRadius;
        DImg areaImage = orgImage->copy(xMinBlur, yMaxBlur, xMaxBlur - xMinBlur, yMaxBlur - yMinBlur);

        BlurFilter(this, *orgImage, *destImage, 10, 75, BlurRadius);

        // I am unsure about differences of 1 pixel
        destImage->bitBltImage(&areaImage, xMinBlur, yMinBlur);
        destImage->bitBltImage(orgImage, 0, 0, orgImage->width(), yMinBlur, 0, 0);
        destImage->bitBltImage(orgImage, 0, yMinBlur, xMinBlur, yMaxBlur - yMinBlur, 0, yMinBlur);
        destImage->bitBltImage(orgImage, xMaxBlur + 1, yMinBlur, orgImage->width() - xMaxBlur - 1, yMaxBlur - yMinBlur, yMaxBlur, yMinBlur);
        destImage->bitBltImage(orgImage, 0, yMaxBlur + 1, orgImage->width(), orgImage->height() - yMaxBlur - 1, 0, yMaxBlur);

        postProgress(80);
    }
    else
    {
        // copy bits for blurring
        memcpy(destImage->bits(), orgImage->bits(), orgImage->numBytes());

        // Gaussian blur using the BlurRadius parameter.
        BlurFilter(this, *orgImage, *destImage, 10, 80, BlurRadius);
    }

    // Blending results.

    QList<int> vals = multithreadedSteps(xMax, xMin);
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage    = orgImage;
    prm.destImage   = destImage;
    prm.X           = X;
    prm.Y           = Y;
    prm.BlendRadius = BlendRadius;
    prm.bInversed   = bInversed;

    // we have reached the main loop

    for (int h = yMin; runningFlag() && (h < yMax); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::focusBlurMultithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(80.0 + ((double)(h - yMin) * 20.0) / (yMax - yMin));

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void BlurFXFilter::smartBlurStage1Multithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();

    int sumR, sumG, sumB, nCount;
    DColor color, radiusColor, radiusColorBlur;
    int offset, loopOffset;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        // we initialize the variables
        sumR = sumG = sumB = nCount = 0;

        // read color
        offset = GetOffset(Width, w, prm.h, bytesDepth);
        color.setColor(data + offset, sixteenBit);

        // ...we enter this loop to sum the bits
        for (int a = -prm.Radius; runningFlag() && (a <= prm.Radius); ++a)
        {
            // verify if is inside the rect
            if (IsInside(Width, Height, w + a, prm.h))
            {
                // read color
                loopOffset = GetOffset(Width, w + a, prm.h, bytesDepth);
                radiusColor.setColor(data + loopOffset, sixteenBit);

                // now, we have to check if is inside the sensibility filter
                if (IsColorInsideTheRange(color.red(), color.green(), color.blue(),
                                          radiusColor.red(), radiusColor.green(), radiusColor.blue(),
                                          prm.StrengthRange))
                {
                    // finally we sum the bits
                    sumR += radiusColor.red();
                    sumG += radiusColor.green();
                    sumB += radiusColor.blue();
                }
                else
                {
                    // finally we sum the bits
                    sumR += color.red();
                    sumG += color.green();
                    sumB += color.blue();
                }

                // increment counter
                ++nCount;
            }
        }

        if (nCount == 0)
        {
            nCount = 1;
        }

        // now, we have to calc the arithmetic average
        color.setRed(sumR   / nCount);
        color.setGreen(sumG / nCount);
        color.setBlue(sumB  / nCount);

        // write color to destination
        color.setPixel(prm.pBlur + offset);
    }
}

void BlurFXFilter::smartBlurStage2Multithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    int sumR, sumG, sumB, nCount;
    DColor color, radiusColor, radiusColorBlur;
    int offset, loopOffset;

    for (uint h = prm.start; runningFlag() && (h < prm.stop); ++h)
    {
        // we initialize the variables
        sumR = sumG = sumB = nCount = 0;

        // read color
        offset = GetOffset(Width, prm.w, h, bytesDepth);
        color.setColor(data + offset, sixteenBit);

        // ...we enter this loop to sum the bits
        for (int a = -prm.Radius; runningFlag() && (a <= prm.Radius); ++a)
        {
            // verify if is inside the rect
            if (IsInside(Width, Height, prm.w, h + a))
            {
                // read color
                loopOffset = GetOffset(Width, prm.w, h + a, bytesDepth);
                radiusColor.setColor(data + loopOffset, sixteenBit);

                // now, we have to check if is inside the sensibility filter
                if (IsColorInsideTheRange(color.red(), color.green(), color.blue(),
                                          radiusColor.red(), radiusColor.green(), radiusColor.blue(),
                                          prm.StrengthRange))
                {
                    radiusColorBlur.setColor(prm.pBlur + loopOffset, sixteenBit);
                    // finally we sum the bits
                    sumR += radiusColorBlur.red();
                    sumG += radiusColorBlur.green();
                    sumB += radiusColorBlur.blue();
                }
                else
                {
                    // finally we sum the bits
                    sumR += color.red();
                    sumG += color.green();
                    sumB += color.blue();
                }

                // increment counter
                ++nCount;
            }
        }

        if (nCount == 0)
        {
            nCount = 1;
        }

        // now, we have to calc the arithmetic average
        color.setRed(sumR   / nCount);
        color.setGreen(sumG / nCount);
        color.setBlue(sumB  / nCount);

        // write color to destination
        color.setPixel(pResBits + offset);
    }
}

/* Function to apply the SmartBlur effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Radius           => blur matrix radius.
 * Strength         => Color strength.
 *
 * Theory           => Similar to SmartBlur from Photoshop, this function has the
 *                     same engine as Blur function, but, in a matrix with n
 *                     dimensions, we take only colors that pass by sensibility filter
 *                     The result is a clean image, not totally blurred, but a image
 *                     with correction between pixels.
 */

void BlurFXFilter::smartBlur(DImg* const orgImage, DImg* const destImage, int Radius, int Strength)
{
    if (Radius <= 0)
    {
        return;
    }

    int progress;
    int StrengthRange = Strength;

    if (orgImage->sixteenBit())
    {
        StrengthRange = (StrengthRange + 1) * 256 - 1;
    }

    QScopedArrayPointer<uchar> pBlur(new uchar[orgImage->numBytes()]);

    // We need to copy our bits to blur bits

    memcpy(pBlur.data(), orgImage->bits(), orgImage->numBytes());

    QList<int> valsw = multithreadedSteps(orgImage->width());
    QList<int> valsh = multithreadedSteps(orgImage->height());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage      = orgImage;
    prm.destImage     = destImage;
    prm.StrengthRange = StrengthRange;
    prm.pBlur         = pBlur.data();
    prm.Radius        = Radius;

    // we have reached the main loop

    for (uint h = 0; runningFlag() && (h < orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < valsw.count()-1) ; ++j)
        {
            prm.start = valsw[j];
            prm.stop  = valsw[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::smartBlurStage1Multithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 50.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // we have reached the second part of main loop

    tasks.clear();

    for (uint w = 0; runningFlag() && (w < orgImage->width()); ++w)
    {
        for (int j = 0 ; runningFlag() && (j < valsh.count()-1) ; ++j)
        {
            prm.start = valsh[j];
            prm.stop  = valsh[j+1];
            prm.w     = w;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::smartBlurStage2Multithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(50.0 + ((double)w * 50.0) / orgImage->width());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

// NOTE: there is no gain to parallelize this method due to non re-entrancy of RandomColor()
//       (dixit RandomNumberGenerator which non re-entrant - Boost lib problem).

/* Function to apply the frostGlass effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Frost            => Frost value
 *
 * Theory           => Similar to Diffuse effect, but the random byte is defined
 *                     in a matrix. Diffuse uses a random diagonal byte.
 */
void BlurFXFilter::frostGlass(DImg* const orgImage, DImg* const destImage, int Frost)
{
    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    Frost = (Frost < 1) ? 1 : (Frost > 10) ? 10 : Frost;

    int h, w;

    DColor color;
    int offset;

    // Randomize.
    RandomNumberGenerator generator;
    generator.seed(d->randomSeed);

    int range = sixteenBit ? 65535 : 255;

    // it is a huge optimization to allocate these here once
    QScopedArrayPointer<uchar> IntensityCount(new uchar[range + 1]);
    QScopedArrayPointer<uint> AverageColorR(new uint[range + 1]);
    QScopedArrayPointer<uint> AverageColorG(new uint[range + 1]);
    QScopedArrayPointer<uint> AverageColorB(new uint[range + 1]);

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            offset = GetOffset(Width, w, h, bytesDepth);
            // read color to preserve alpha
            color.setColor(data + offset, sixteenBit);

            // get random color from surrounding of w|h
            color = RandomColor(data, Width, Height, sixteenBit, bytesDepth,
                                w, h, Frost, color.alpha(), generator, range, IntensityCount.data(),
                                AverageColorR.data(), AverageColorG.data(), AverageColorB.data());

            // write color to destination
            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

void BlurFXFilter::mosaicMultithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pResBits = prm.destImage->bits();

    DColor color;
    int offsetCenter, offset;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); w += prm.SizeW)
    {
        // we have to find the center pixel for mosaic's rectangle

        offsetCenter = GetOffsetAdjusted(Width, Height, w + (prm.SizeW / 2), prm.h + (prm.SizeH / 2), bytesDepth);
        color.setColor(data + offsetCenter, sixteenBit);

        // now, we fill the mosaic's rectangle with the center pixel color

        for (uint subw = w; runningFlag() && (subw <= w + prm.SizeW); ++subw)
        {
            for (uint subh = prm.h; runningFlag() && (subh <= prm.h + prm.SizeH); ++subh)
            {
                // if is inside...
                if (IsInside(Width, Height, subw, subh))
                {
                    // set color
                    offset = GetOffset(Width, subw, subh, bytesDepth);
                    color.setPixel(pResBits + offset);
                }
            }
        }
    }
}

/* Function to apply the mosaic effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Size             => Size of mosaic.
 *
 * Theory           => Ok, you can find some mosaic effects on PSC, but this one
 *                     has a great feature, if you see a mosaic in other code you will
 *                     see that the corner pixel doesn't change. The explanation is
 *                     simple, the color of the mosaic is the same as the first pixel
 *                     get. Here, the color of the mosaic is the same as the mosaic
 *                     center pixel.
 *                     Now the function scan the rows from the top (like photoshop).
 */
void BlurFXFilter::mosaic(DImg* const orgImage, DImg* const destImage, int SizeW, int SizeH)
{
    int progress;

    // we need to check for valid values
    if (SizeW < 1)
    {
        SizeW = 1;
    }

    if (SizeH < 1)
    {
        SizeH = 1;
    }

    if ((SizeW == 1) && (SizeH == 1))
    {
        return;
    }

    QList<int> vals = multithreadedSteps(orgImage->width());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.SizeW     = SizeW;
    prm.SizeH     = SizeH;

    // this loop will never look for transparent colors

    for (uint h = 0; runningFlag() && (h < orgImage->height()); h += SizeH)
    {
        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            prm.start = vals[j];
            prm.stop  = vals[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::mosaicMultithreaded,
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

/* Function to get a color in a matrix with a determined size
 *
 * Bits              => Bits array
 * Width             => Image width
 * Height            => Image height
 * X                 => Position horizontal
 * Y                 => Position vertical
 * Radius            => The radius of the matrix to be created
 *
 * Theory            => This function takes from a distinct matrix a random color
 */
DColor BlurFXFilter::RandomColor(uchar* const Bits, int Width, int Height, bool sixteenBit, int bytesDepth,
                                 int X, int Y, int Radius,
                                 int alpha, RandomNumberGenerator& generator, int range, uchar* const IntensityCount,
                                 uint* const AverageColorR, uint* const AverageColorG, uint* const AverageColorB)
{
    DColor color;
    int offset;

    int w, h, counter = 0;

    int I;

    // For 16 bit we have a problem here because this takes 255 times longer,
    // and the algorithm is really slow for 16 bit, but I think this cannot be avoided.
    memset(IntensityCount, 0, (range + 1) * sizeof(uchar));
    memset(AverageColorR,  0, (range + 1) * sizeof(uint));
    memset(AverageColorG,  0, (range + 1) * sizeof(uint));
    memset(AverageColorB,  0, (range + 1) * sizeof(uint));

    for (w = X - Radius; runningFlag() && (w <= X + Radius); ++w)
    {
        for (h = Y - Radius; runningFlag() && (h <= Y + Radius); ++h)
        {
            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
            {
                offset = GetOffset(Width, w, h, bytesDepth);
                color.setColor(Bits + offset, sixteenBit);
                I = GetIntensity(color.red(), color.green(), color.blue());
                IntensityCount[I]++;
                ++counter;

                if (IntensityCount[I] == 1)
                {
                    AverageColorR[I] = color.red();
                    AverageColorG[I] = color.green();
                    AverageColorB[I] = color.blue();
                }
                else
                {
                    AverageColorR[I] += color.red();
                    AverageColorG[I] += color.green();
                    AverageColorB[I] += color.blue();
                }
            }
        }
    }

    // check for runningFlag here before entering the do loop (will crash with SIGFPE otherwise)
    if (!runningFlag())
    {
        return DColor(0, 0, 0, 0, sixteenBit);
    }

    int RandNumber, count, Index, ErrorCount = 0;
    int J;

    do
    {
        RandNumber = generator.number(0, counter);

        count = 0;
        Index = 0;

        do
        {
            count += IntensityCount[Index];
            ++Index;
        }
        while (runningFlag() && (count < RandNumber));

        J = Index - 1;
        ++ErrorCount;
    }
    while (runningFlag() && (IntensityCount[J] == 0) && (ErrorCount <= counter));

    if (!runningFlag())
    {
        return DColor(0, 0, 0, 0, sixteenBit);
    }

    color.setSixteenBit(sixteenBit);
    color.setAlpha(alpha);
    int clampMax = sixteenBit ? 655535 : 255;

    if (ErrorCount >= counter)
    {
        if (counter == 0)
        {
            counter = 1;
        }

        color.setRed(CLAMP((int)(AverageColorR[J] / counter), 0, clampMax));
        color.setGreen(CLAMP((int)(AverageColorG[J] / counter), 0, clampMax));
        color.setBlue(CLAMP((int)(AverageColorB[J] / counter), 0, clampMax));
    }
    else
    {
        if (IntensityCount[J] == 0)
        {
            IntensityCount[J] = 1;
        }

        color.setRed(CLAMP((int)(AverageColorR[J] / IntensityCount[J]), 0, clampMax));
        color.setGreen(CLAMP((int)(AverageColorG[J] / IntensityCount[J]), 0, clampMax));
        color.setBlue(CLAMP((int)(AverageColorB[J] / IntensityCount[J]), 0, clampMax));
    }

    return color;
}

void BlurFXFilter::MakeConvolutionStage1Multithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();

    int n;

    int nSumR, nSumG, nSumB, nCount;
    DColor color;
    int offset;

    for (uint w = prm.start; runningFlag() && (w < prm.stop); ++w)
    {
        // initialize the variables
        nSumR = nSumG = nSumB = nCount = 0;

        // first of all, we need to blur the horizontal lines

        for (n = -prm.Radius; runningFlag() && (n <= prm.Radius); ++n)
        {
            // if is inside...
            if (IsInside(Width, Height, w + n, prm.h))
            {
                // read color from orgImage
                offset = GetOffset(Width, w + n, prm.h, bytesDepth);
                color.setColor(data + offset, sixteenBit);

                // finally, we sum the pixels using a method similar to assigntables
                nSumR += prm.arrMult[n + prm.Radius][color.red()];
                nSumG += prm.arrMult[n + prm.Radius][color.green()];
                nSumB += prm.arrMult[n + prm.Radius][color.blue()];

                // we need to add the kernel value to the counter
                nCount += prm.Kernel[n + prm.Radius];
            }
        }

        if (nCount == 0)
        {
            nCount = 1;
        }

        // calculate pointer
        offset = GetOffset(Width, w, prm.h, bytesDepth);
        // read color from orgImage to preserve alpha
        color.setColor(data + offset, sixteenBit);

        // now, we have to calc the arithmetic average
        if (sixteenBit)
        {
            color.setRed(CLAMP065535(nSumR   / nCount));
            color.setGreen(CLAMP065535(nSumG / nCount));
            color.setBlue(CLAMP065535(nSumB  / nCount));
        }
        else
        {
            color.setRed((uchar)CLAMP0255(nSumR   / nCount));
            color.setGreen((uchar)CLAMP0255(nSumG / nCount));
            color.setBlue((uchar)CLAMP0255(nSumB  / nCount));
        }

        // write color to blur bits
        color.setPixel(prm.pBlur + offset);
    }
}

void BlurFXFilter::MakeConvolutionStage2Multithreaded(const Args& prm)
{
    int Width       = prm.orgImage->width();
    int Height      = prm.orgImage->height();
    uchar* data     = prm.orgImage->bits();
    bool sixteenBit = prm.orgImage->sixteenBit();
    int bytesDepth  = prm.orgImage->bytesDepth();
    uchar* pOutBits = prm.destImage->bits();

    int n;

    int nSumR, nSumG, nSumB, nCount;
    DColor color;
    int offset;

    for (uint h = prm.start; runningFlag() && (h < prm.stop); ++h)
    {
        // initialize the variables
        nSumR = nSumG = nSumB = nCount = 0;

        // first of all, we need to blur the vertical lines
        for (n = -prm.Radius; runningFlag() && (n <= prm.Radius); ++n)
        {
            // if is inside...
            if (IsInside(Width, Height, prm.w, h + n))
            {
                // read color from blur bits
                offset = GetOffset(Width, prm.w, h + n, bytesDepth);
                color.setColor(prm.pBlur + offset, sixteenBit);

                // finally, we sum the pixels using a method similar to assigntables
                nSumR += prm.arrMult[n + prm.Radius][color.red()];
                nSumG += prm.arrMult[n + prm.Radius][color.green()];
                nSumB += prm.arrMult[n + prm.Radius][color.blue()];

                // we need to add the kernel value to the counter
                nCount += prm.Kernel[n + prm.Radius];
            }
        }

        if (nCount == 0)
        {
            nCount = 1;
        }

        // calculate pointer
        offset = GetOffset(Width, prm.w, h, bytesDepth);
        // read color from orgImage to preserve alpha
        color.setColor(data + offset, sixteenBit);

        // now, we have to calc the arithmetic average
        if (sixteenBit)
        {
            color.setRed(CLAMP065535(nSumR   / nCount));
            color.setGreen(CLAMP065535(nSumG / nCount));
            color.setBlue(CLAMP065535(nSumB  / nCount));
        }
        else
        {
            color.setRed((uchar)CLAMP0255(nSumR   / nCount));
            color.setGreen((uchar)CLAMP0255(nSumG / nCount));
            color.setBlue((uchar)CLAMP0255(nSumB  / nCount));
        }

        // write color to destination
        color.setPixel(pOutBits + offset);
    }
}

/* Function to simple convolve a unique pixel with a determined radius
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Radius           => kernel radius, e.g. rad=1, so array will be 3X3
 * Kernel           => kernel array to apply.
 *
 * Theory           => I've worked hard here, but I think this is a very smart
 *                     way to convolve an array, its very hard to explain how I reach
 *                     this, but the trick here its to store the sum used by the
 *                     previous pixel, so we sum with the other pixels that wasn't get
 */
void BlurFXFilter::MakeConvolution(DImg* const orgImage, DImg* const destImage, int Radius, int Kernel[])
{
    if (Radius <= 0)
    {
        return;
    }

    int progress;
    int nKernelWidth = Radius * 2 + 1;
    int range = orgImage->sixteenBit() ? 65536 : 256;

    QScopedArrayPointer<uchar> pBlur(new uchar[orgImage->numBytes()]);

    // We need to copy our bits to blur bits

    memcpy(pBlur.data(), orgImage->bits(), orgImage->numBytes());

    // We need to alloc a 2d array to help us to store the values

    int** const arrMult = Alloc2DArray(nKernelWidth, range);

    for (int i = 0; i < nKernelWidth; ++i)
    {
        for (int j = 0; j < range; ++j)
        {
            arrMult[i][j] = j * Kernel[i];
        }
    }

    QList<int> valsw = multithreadedSteps(orgImage->width());
    QList<int> valsh = multithreadedSteps(orgImage->height());
    QList <QFuture<void> > tasks;

    Args prm;
    prm.orgImage  = orgImage;
    prm.destImage = destImage;
    prm.Radius    = Radius;
    prm.Kernel    = Kernel;
    prm.arrMult   = arrMult;
    prm.pBlur     = pBlur.data();

    // Now, we enter in the first loop

    for (uint h = 0; runningFlag() && (h < orgImage->height()); ++h)
    {
        for (int j = 0 ; runningFlag() && (j < valsw.count()-1) ; ++j)
        {
            prm.start = valsw[j];
            prm.stop  = valsw[j+1];
            prm.h     = h;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::MakeConvolutionStage1Multithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 50.0) / orgImage->height());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // We enter in the second main loop

    tasks.clear();

    for (uint w = 0; runningFlag() && (w < orgImage->width()); ++w)
    {
        for (int j = 0 ; runningFlag() && (j < valsh.count()-1) ; ++j)
        {
            prm.start = valsh[j];
            prm.stop  = valsh[j+1];
            prm.w     = w;
            tasks.append(QtConcurrent::run(this,
                                           &BlurFXFilter::MakeConvolutionStage2Multithreaded,
                                           prm
                                          ));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        // Update the progress bar in dialog.
        progress = (int)(50.0 + ((double)w * 50.0) / orgImage->width());

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // now, we must free memory
    Free2DArray(arrMult, nKernelWidth);
}

FilterAction BlurFXFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("type"),     d->blurFXType);
    action.addParameter(QLatin1String("distance"), d->distance);
    action.addParameter(QLatin1String("level"),    d->level);

    if (d->blurFXType == FrostGlass)
    {
        action.addParameter(QLatin1String("randomSeed"), d->randomSeed);
    }

    return action;
}

void BlurFXFilter::readParameters(const FilterAction& action)
{
    d->blurFXType = action.parameter(QLatin1String("type")).toInt();
    d->distance   = action.parameter(QLatin1String("distance")).toInt();
    d->level      = action.parameter(QLatin1String("level")).toInt();

    if (d->blurFXType == FrostGlass)
    {
        d->randomSeed = action.parameter(QLatin1String("randomSeed")).toUInt();
    }
}

}  // namespace Digikam
