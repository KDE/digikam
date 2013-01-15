/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Distortion FX threaded image filter.
 *
 * Copyright (C) 2005-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qmath.h>

// Local includes

#include "dimg.h"
#include "pixelsaliasfilter.h"
#include "randomnumbergenerator.h"

namespace Digikam
{

DistortionFXFilter::DistortionFXFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    m_antiAlias  = true;
    m_level      = 0;
    m_iteration  = 0;
    m_effectType = 0;
    m_randomSeed = 0;

    initFilter();
}

DistortionFXFilter::DistortionFXFilter(DImg* const orgImage, QObject* const parent, int effectType,
                                       int level, int iteration, bool antialiaqSing)
    : DImgThreadedFilter(orgImage, parent, "DistortionFX")
{
    m_effectType = effectType;
    m_level      = level;
    m_iteration  = iteration;
    m_antiAlias  = antialiaqSing;
    m_randomSeed = RandomNumberGenerator::timeSeed();

    initFilter();
}

DistortionFXFilter::~DistortionFXFilter()
{
    cancelFilter();
}

void DistortionFXFilter::filterImage()
{
    int w = m_orgImage.width();
    int h = m_orgImage.height();
    int l = m_level;
    int f = m_iteration;

    switch (m_effectType)
    {
        case FishEye:
            fisheye(&m_orgImage, &m_destImage, (double)(l / 5.0), m_antiAlias);
            break;

        case Twirl:
            twirl(&m_orgImage, &m_destImage, l, m_antiAlias);
            break;

        case CilindricalHor:
            cilindrical(&m_orgImage, &m_destImage, (double)l, true, false, m_antiAlias);
            break;

        case CilindricalVert:
            cilindrical(&m_orgImage, &m_destImage, (double)l, false, true, m_antiAlias);
            break;

        case CilindricalHV:
            cilindrical(&m_orgImage, &m_destImage, (double)l, true, true, m_antiAlias);
            break;

        case Caricature:
            fisheye(&m_orgImage, &m_destImage, (double)(-l / 5.0), m_antiAlias);
            break;

        case MultipleCorners:
            multipleCorners(&m_orgImage, &m_destImage, l, m_antiAlias);
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
            circularWaves(&m_orgImage, &m_destImage, w / 2, h / 2, (double)l, (double)f, 0.0, false, m_antiAlias);
            break;

        case CircularWaves2:
            circularWaves(&m_orgImage, &m_destImage, w / 2, h / 2, (double)l, (double)f, 25.0, true, m_antiAlias);
            break;

        case PolarCoordinates:
            polarCoordinates(&m_orgImage, &m_destImage, true, m_antiAlias);
            break;

        case UnpolarCoordinates:
            polarCoordinates(&m_orgImage, &m_destImage, false, m_antiAlias);
            break;

        case Tile:
            tile(&m_orgImage, &m_destImage, 200 - f, 200 - f, l);
            break;
    }
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
    int offset, offsetOther;

    offset = getOffset(Width, w, h, bytesDepth);

    if (AntiAlias)
    {
        uchar* ptr = pResBits + offset;

        if (sixteenBit)
        {
            unsigned short* ptr16 = (unsigned short*)ptr;
            PixelsAliasFilter().pixelAntiAliasing16((unsigned short*)data, Width, Height, nw, nh,
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
        offsetOther = getOffsetAdjusted(Width, Height, (int)nw, (int)nh, bytesDepth);
        // read color
        color.setColor(data + offsetOther, sixteenBit);
        // write color to destination
        color.setPixel(pResBits + offset);
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

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw, th, tw;

    int progress;
    int nHalfW = Width / 2, nHalfH = Height / 2;

    DColor color;
    int offset;

    double lfXScale = 1.0, lfYScale = 1.0;
    double lfRadius, lfRadMax, lfAngle, lfCoeff, lfCoeffStep = Coeff / 1000.0;

    if (Width > Height)
    {
        lfYScale = (double)Width / (double)Height;
    }
    else if (Height > Width)
    {
        lfXScale = (double)Height / (double)Width;
    }

    lfRadMax = (double)qMax(Height, Width) / 2.0;
    lfCoeff  = lfRadMax / qLn(qFabs(lfCoeffStep) * lfRadMax + 1.0);

    // main loop

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        th = lfYScale * (double)(h - nHalfH);

        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            tw = lfXScale * (double)(w - nHalfW);

            // we find the distance from the center
            lfRadius = qSqrt(th * th + tw * tw);

            if (lfRadius < lfRadMax)
            {
                lfAngle = qAtan2(th, tw);

                if (Coeff > 0.0)
                {
                    lfRadius = (qExp(lfRadius / lfCoeff) - 1.0) / lfCoeffStep;
                }
                else
                {
                    lfRadius = lfCoeff * qLn(1.0 + (-1.0 * lfCoeffStep) * lfRadius);
                }

                nw = (double)nHalfW + (lfRadius / lfXScale) * qCos(lfAngle);
                nh = (double)nHalfH + (lfRadius / lfYScale) * qSin(lfAngle);

                setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
            }
            else
            {
                // copy pixel
                offset = getOffset(Width, w, h, bytesDepth);
                color.setColor(data + offset, sixteenBit);
                color.setPixel(pResBits + offset);
            }
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)(h) * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

/* Function to apply the twirl effect backported from ImageProcesqSing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Twirl            => Distance value.
 * Antialias        => Smart blurring result.
 *
 * Theory           => Take spiral studies, you will understand better, I'm studying
 *                     hard on this effect, because it is not too fast.
 */
void DistortionFXFilter::twirl(DImg* orgImage, DImg* destImage, int Twirl, bool AntiAlias)
{
    // if twirl value is zero, we do nothing

    if (Twirl == 0)
    {
        return;
    }

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double tw, th, nh, nw;

    DColor color;
    int offset;

    int progress;
    int nHalfW = Width / 2, nHalfH = Height / 2;

    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfNewAngle, lfAngleStep, lfAngleSum, lfCurrentRadius, lfRadMax;

    if (Width > Height)
    {
        lfYScale = (double)Width / (double)Height;
    }
    else if (Height > Width)
    {
        lfXScale = (double)Height / (double)Width;
    }

    // the angle step is twirl divided by 10000
    lfAngleStep = Twirl / 10000.0;
    // now, we get the minimum radius
    lfRadMax = (double)qMax(Width, Height) / 2.0;

    // main loop

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        th = lfYScale * (double)(h - nHalfH);

        for (w = 0; runningFlag() && (w < Width); ++w)
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

                setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
            }
            else
            {
                // copy pixel
                offset = getOffset(Width, w, h, bytesDepth);
                color.setColor(data + offset, sixteenBit);
                color.setPixel(pResBits + offset);
            }
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
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

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int progress;

    int h, w;
    double nh, nw;

    int nHalfW = Width / 2, nHalfH = Height / 2;
    double lfCoeffX = 1.0, lfCoeffY = 1.0, lfCoeffStep = Coeff / 1000.0;

    if (Horizontal)
    {
        lfCoeffX = (double)nHalfW / qLn(qFabs(lfCoeffStep) * nHalfW + 1.0);
    }

    if (Vertical)
    {
        lfCoeffY = (double)nHalfH / qLn(qFabs(lfCoeffStep) * nHalfH + 1.0);
    }

    // initial copy
    memcpy(pResBits, data, orgImage->numBytes());

    // main loop

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            // we find the distance from the center
            nh = qFabs((double)(h - nHalfH));
            nw = qFabs((double)(w - nHalfW));

            if (Horizontal)
            {
                if (Coeff > 0.0)
                {
                    nw = (qExp(nw / lfCoeffX) - 1.0) / lfCoeffStep;
                }
                else
                {
                    nw = lfCoeffX * qLn(1.0 + (-1.0 * lfCoeffStep) * nw);
                }
            }

            if (Vertical)
            {
                if (Coeff > 0.0)
                {
                    nh = (qExp(nh / lfCoeffY) - 1.0) / lfCoeffStep;
                }
                else
                {
                    nh = lfCoeffY * qLn(1.0 + (-1.0 * lfCoeffStep) * nh);
                }
            }

            nw = (double)nHalfW + ((w >= nHalfW) ? nw : -nw);
            nh = (double)nHalfH + ((h >= nHalfH) ? nh : -nh);

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
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

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw;
    int progress;

    int nHalfW = Width / 2, nHalfH = Height / 2;
    double lfAngle, lfNewRadius, lfCurrentRadius, lfRadMax;

    lfRadMax = qSqrt(Height * Height + Width * Width) / 2.0;

    // main loop

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            // we find the distance from the center
            nh = nHalfH - h;
            nw = nHalfW - w;

            // now, we get the distance
            lfCurrentRadius = qSqrt(nh * nh + nw * nw);
            // we find the angle from the center
            lfAngle = qAtan2(nh, nw) * (double)Factor;

            // ok, we sum angle with accumuled to find a new angle
            lfNewRadius = lfCurrentRadius * lfCurrentRadius / lfRadMax;

            // now we find the exact position's x and y
            nw = (double)nHalfW - (qCos(lfAngle) * lfNewRadius);
            nh = (double)nHalfH - (qSin(lfAngle) * lfNewRadius);

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
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
    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw, th, tw;
    int progress;

    int nHalfW = Width / 2, nHalfH = Height / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfRadius, lfRadMax;

    if (Width > Height)
    {
        lfYScale = (double)Width / (double)Height;
    }
    else if (Height > Width)
    {
        lfXScale = (double)Height / (double)Width;
    }

    lfRadMax = (double)qMax(Height, Width) / 2.0;

    // main loop

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        th = lfYScale * (double)(h - nHalfH);

        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            tw = lfXScale * (double)(w - nHalfW);

            if (Type)
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
                lfRadius = (double)(h) * lfRadMax / (double)Height;
                lfAngle  = (double)(w) * (2 * M_PI) / (double) Width;

                nw = (double)nHalfW - (lfRadius / lfXScale) * qSin(lfAngle);
                nh = (double)nHalfH - (lfRadius / lfYScale) * qCos(lfAngle);
            }

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
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

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw;
    int progress;

    double lfRadius, lfRadMax, lfNewAmp = Amplitude;
    double lfFreqAngle = Frequency * ANGLE_RATIO;

    Phase *= ANGLE_RATIO;

    lfRadMax = qSqrt(Height * Height + Width * Width);

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            nw = X - w;
            nh = Y - h;

            lfRadius = qSqrt(nw * nw + nh * nh);

            if (WavesType)
            {
                lfNewAmp = Amplitude * lfRadius / lfRadMax;
            }

            nw = (double)w + lfNewAmp * qSin(lfFreqAngle * lfRadius + Phase);
            nh = (double)h + lfNewAmp * qCos(lfFreqAngle * lfRadius + Phase);

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
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

    int Width       = orgImage->width();
    int Height      = orgImage->height();

    int progress;
    int h, w;

    if (Direction)        // Horizontal
    {
        int tx;

        for (h = 0; runningFlag() && (h < Height); ++h)
        {
            tx = lround(Amplitude * qSin((Frequency * 2) * h * (M_PI / 180)));
            destImage->bitBltImage(orgImage, 0, h,  Width, 1,  tx, h);

            if (FillSides)
            {
                destImage->bitBltImage(orgImage, Width - tx, h,  tx, 1,  0, h);
                destImage->bitBltImage(orgImage, 0, h,  Width - (Width - 2 * Amplitude + tx), 1,  Width + tx, h);
            }

            // Update the progress bar in dialog.
            progress = (int)(((double)h * 100.0) / Height);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
    }
    else
    {
        int ty;

        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            ty = lround(Amplitude * qSin((Frequency * 2) * w * (M_PI / 180)));
            destImage->bitBltImage(orgImage, w, 0, 1, Height, w, ty);

            if (FillSides)
            {
                destImage->bitBltImage(orgImage, w, Height - ty,  1, ty,  w, 0);
                destImage->bitBltImage(orgImage, w, 0,  1, Height - (Height - 2 * Amplitude + ty),  w, Height + ty);
            }

            // Update the progress bar in dialog.
            progress = (int)(((double)w * 100.0) / Width);

            if (progress % 5 == 0)
            {
                postProgress(progress);
            }
        }
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

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int nw, nh, progress;
//    double Radius;

    DColor color;
    int offset, offsetOther;

    int nHalfW = Width / 2, nHalfH = Height / 2;

    for (int w = 0; runningFlag() && (w < Width); ++w)
    {
        for (int h = 0; runningFlag() && (h < Height); ++h)
        {
            nw = nHalfW - w;
            nh = nHalfH - h;

//            Radius = qSqrt (nw * nw + nh * nh);

            if (Mode)
            {
                nw = (int)(w + Amplitude * qSin(Frequency * nw * (M_PI / 180)));
                nh = (int)(h + Amplitude * qCos(Frequency * nh * (M_PI / 180)));
            }
            else
            {
                nw = (int)(w + Amplitude * qSin(Frequency * w * (M_PI / 180)));
                nh = (int)(h + Amplitude * qCos(Frequency * h * (M_PI / 180)));
            }

            offset      = getOffset(Width, w, h, bytesDepth);
            offsetOther = getOffsetAdjusted(Width, Height, (int)nw, (int)nh, bytesDepth);

            // read color
            color.setColor(data + offsetOther, sixteenBit);
            // write color to destination
            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)w * 100.0) / Width);

        if (progress % 5 == 0)
        {
            postProgress(progress);
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

    int Width       = orgImage->width();
    int Height      = orgImage->height();

    RandomNumberGenerator generator;
    generator.seed(m_randomSeed);

    int tx, ty, h, w, progress;

    for (h = 0; runningFlag() && (h < Height); h += HSize)
    {
        for (w = 0; runningFlag() && (w < Width); w += WSize)
        {
            tx = generator.number(- Random / 2, Random / 2);
            ty = generator.number(- Random / 2, Random / 2);
            destImage->bitBltImage(orgImage, w, h,   WSize, HSize,   w + tx, h + ty);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
}

FilterAction DistortionFXFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("antiAlias", m_antiAlias);
    action.addParameter("type",      m_effectType);
    action.addParameter("iteration", m_iteration);
    action.addParameter("level",     m_level);

    if (m_effectType == Tile)
    {
        action.addParameter("randomSeed", m_randomSeed);
    }

    return action;
}

void DistortionFXFilter::readParameters(const FilterAction& action)
{
    m_antiAlias  = action.parameter("antiAlias").toBool();
    m_effectType = action.parameter("type").toInt();
    m_iteration  = action.parameter("iteration").toInt();
    m_level      = action.parameter("level").toInt();

    if (m_effectType == Tile)
    {
        m_randomSeed = action.parameter("randomSeed").toUInt();
    }
}

}  // namespace Digikam
