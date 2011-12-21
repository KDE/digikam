/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Blur FX threaded image filter.
 *
 * Copyright 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <cmath>
#include <cstdlib>
#include <cstring>

// Qt includes

#include <QDateTime>

// Local includes

#include "dimg.h"
#include "blurfilter.h"
#include "randomnumbergenerator.h"

namespace Digikam
{

BlurFXFilter::BlurFXFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

BlurFXFilter::BlurFXFilter(DImg* orgImage, QObject* parent, int blurFXType, int distance, int level)
    : DImgThreadedFilter(orgImage, parent, "BlurFX")
{
    m_blurFXType = blurFXType;
    m_distance   = distance;
    m_level      = level;
    m_randomSeed = RandomNumberGenerator::timeSeed();

    initFilter();
}

void BlurFXFilter::filterImage()
{
    int w = m_orgImage.width();
    int h = m_orgImage.height();

    switch (m_blurFXType)
    {
        case ZoomBlur:
            zoomBlur(&m_orgImage, &m_destImage, w/2, h/2, m_distance);
            break;

        case RadialBlur:
            radialBlur(&m_orgImage, &m_destImage, w/2, h/2, m_distance);
            break;

        case FarBlur:
            farBlur(&m_orgImage, &m_destImage, m_distance);
            break;

        case MotionBlur:
            motionBlur(&m_orgImage, &m_destImage, m_distance, (double)m_level);
            break;

        case SoftenerBlur:
            softenerBlur(&m_orgImage, &m_destImage);
            break;

        case ShakeBlur:
            shakeBlur(&m_orgImage, &m_destImage, m_distance);
            break;

        case FocusBlur:
            focusBlur(&m_orgImage, &m_destImage, w/2, h/2, m_distance, m_level*10);
            break;

        case SmartBlur:
            smartBlur(&m_orgImage, &m_destImage, m_distance, m_level);
            break;

        case FrostGlass:
            frostGlass(&m_orgImage, &m_destImage, m_distance);
            break;

        case Mosaic:
            mosaic(&m_orgImage, &m_destImage, m_distance, m_distance);
            break;
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
void BlurFXFilter::zoomBlur(DImg* orgImage, DImg* destImage, int X, int Y, int Distance, QRect pArea)
{
    if (Distance <= 1)
    {
        return;
    }

    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    // We working on full image.
    int xMin = 0;
    int xMax = Width;
    int yMin = 0;
    int yMax = Height;

    // If we working in preview mode, else we using the preview area.
    if ( pArea.isValid() )
    {
        xMin = pArea.x();
        xMax = pArea.x() + pArea.width();
        yMin = pArea.y();
        yMax = pArea.y() + pArea.height();
    }

    int h, w, nh, nw, r;
    int sumR, sumG, sumB, nCount;
    double lfRadius, lfNewRadius, lfRadMax, lfAngle;

    DColor color;
    int offset;

    lfRadMax = sqrt (Height * Height + Width * Width);

    // number of added pixels
    nCount = 0;

    // we have reached the main loop
    for (h = yMin; runningFlag() && (h < yMax); ++h)
    {
        for (w = xMin; runningFlag() && (w < xMax); ++w)
        {
            // ...we enter this loop to sum the bits

            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;

            nw = X - w;
            nh = Y - h;

            lfRadius    = sqrt (nw * nw + nh * nh);
            lfAngle     = atan2 ((double)nh, (double)nw);
            lfNewRadius = (lfRadius * Distance) / lfRadMax;

            for (r = 0; runningFlag() && (r <= lfNewRadius); ++r)
            {
                // we need to calc the positions
                nw = (int)(X - (lfRadius - r) * cos (lfAngle));
                nh = (int)(Y - (lfRadius - r) * sin (lfAngle));

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
            offset = GetOffset(Width, w, h, bytesDepth);
            // read color to preserve alpha
            color.setColor(data + offset, sixteenBit);

            // now, we have to calc the arithmetic average
            color.setRed  (sumR / nCount);
            color.setGreen(sumG / nCount);
            color.setBlue (sumB / nCount);

            // write color to destination
            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)(h - yMin) * 100.0) / (yMax - yMin));

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
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
 *                     We have all the image and find the center pixel. Now, we analize
 *                     all the pixels and calc the radius from the center and find the
 *                     angle. After this, we sum this pixel with others with the same
 *                     radius, but different angles. Here I'm using degrees angles.
 */
void BlurFXFilter::radialBlur(DImg* orgImage, DImg* destImage, int X, int Y, int Distance, QRect pArea)
{
    if (Distance <= 1)
    {
        return;
    }

    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    // We working on full image.
    int xMin = 0;
    int xMax = Width;
    int yMin = 0;
    int yMax = Height;

    // If we working in preview mode, else we using the preview area.
    if ( pArea.isValid() )
    {
        xMin = pArea.x();
        xMax = pArea.x() + pArea.width();
        yMin = pArea.y();
        yMax = pArea.y() + pArea.height();
    }

    int sumR, sumG, sumB, nw, nh;
    double Radius, Angle, AngleRad;

    DColor color;
    int offset;

    QScopedArrayPointer<double> nMultArray(new double[Distance * 2 + 1]);

    for (int i = -Distance; i <= Distance; ++i)
    {
        nMultArray[i + Distance] = i * ANGLE_RATIO;
    }

    // number of added pixels
    int nCount = 0;

    // we have reached the main loop

    for (int h = yMin; runningFlag() && (h < yMax); ++h)
    {
        for (int w = xMin; runningFlag() && (w < xMax); ++w)
        {
            // ...we enter this loop to sum the bits

            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;

            nw = X - w;
            nh = Y - h;

            Radius   = sqrt (nw * nw + nh * nh);
            AngleRad = atan2 ((double)nh, (double)nw);

            for (int a = -Distance; runningFlag() && (a <= Distance); ++a)
            {
                Angle = AngleRad + nMultArray[a + Distance];
                // we need to calc the positions
                nw = (int)(X - Radius * cos (Angle));
                nh = (int)(Y - Radius * sin (Angle));

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
            offset = GetOffset(Width, w, h, bytesDepth);
            // read color to preserve alpha
            color.setColor(data + offset, sixteenBit);

            // now, we have to calc the arithmetic average
            color.setRed  (sumR / nCount);
            color.setGreen(sumG / nCount);
            color.setBlue (sumB / nCount);

            // write color to destination
            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)(h - yMin) * 100.0) / (yMax - yMin));

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }
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
void BlurFXFilter::focusBlur(DImg* orgImage, DImg* destImage,
                             int X, int Y, int BlurRadius, int BlendRadius,
                             bool bInversed, QRect pArea)
{
    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    // We working on full image.
    int xMin = 0;
    int xMax = Width;
    int yMin = 0;
    int yMax = Height;

    // If we working in preview mode, else we using the preview area.
    if ( pArea.isValid() )
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
        destImage->bitBltImage(orgImage, 0, 0, Width, yMinBlur, 0, 0);
        destImage->bitBltImage(orgImage, 0, yMinBlur, xMinBlur, yMaxBlur - yMinBlur, 0, yMinBlur);
        destImage->bitBltImage(orgImage, xMaxBlur + 1, yMinBlur, Width - xMaxBlur - 1, yMaxBlur - yMinBlur, yMaxBlur, yMinBlur);
        destImage->bitBltImage(orgImage, 0, yMaxBlur + 1, Width, Height - yMaxBlur - 1, 0, yMaxBlur);

        postProgress(80);
    }
    else
    {
        // copy bits for blurring
        memcpy(pResBits, data, orgImage->numBytes());

        // Gaussian blur using the BlurRadius parameter.
        BlurFilter(this, *orgImage, *destImage, 10, 80, BlurRadius);
    }

    // Blending results.

    int nBlendFactor;
    double lfRadius;
    int offset;

    DColor colorOrgImage, colorBlurredImage;
    int alpha;
    uchar* ptr;

    // get composer for default blending
    DColorComposer* composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);

    int nh = 0, nw = 0;

    for (int h = yMin; runningFlag() && (h < yMax); ++h)
    {
        nh = Y - h;

        for (int w = xMin; runningFlag() && (w < xMax); ++w)
        {
            nw = X - w;

            lfRadius = sqrt (nh * nh + nw * nw);

            if (sixteenBit)
            {
                nBlendFactor = LimitValues16 ((int)(65535.0 * lfRadius / (double)BlendRadius));
            }
            else
            {
                nBlendFactor = LimitValues8  ((int)(255.0 * lfRadius / (double)BlendRadius));
            }

            // Read color values
            offset = GetOffset(Width, w, h, bytesDepth);
            ptr = pResBits + offset;
            colorOrgImage.setColor(data + offset, sixteenBit);
            colorBlurredImage.setColor(ptr, sixteenBit);

            // Preserve alpha
            alpha = colorOrgImage.alpha();

            // In normal mode, the image is focused in the middle
            // and less focused towards the border.
            // In inversed mode, the image is more focused towards the edge
            // and less focused in the middle.
            // This is achieved by swapping src and dest while blending.
            if (bInversed)
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

        // Update the progress bar in dialog.
        progress = (int) (80.0 + ((double)(h - yMin) * 20.0) / (yMax - yMin));

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }

    delete composer;
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
void BlurFXFilter::farBlur(DImg* orgImage, DImg* destImage, int Distance)
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

void BlurFXFilter::smartBlur(DImg* orgImage, DImg* destImage, int Radius, int Strength)
{
    if (Radius <= 0)
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
    int sumR, sumG, sumB, nCount, w, h, a;

    int StrengthRange = Strength;

    if (sixteenBit)
    {
        StrengthRange = (StrengthRange + 1) * 256 - 1;
    }

    DColor color, radiusColor, radiusColorBlur;
    int offset, loopOffset;

    QScopedArrayPointer<uchar> pBlur(new uchar[orgImage->numBytes()]);

    // We need to copy our bits to blur bits

    memcpy (pBlur.data(), data, orgImage->numBytes());

    // we have reached the main loop

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;

            // read color
            offset = GetOffset(Width, w, h, bytesDepth);
            color.setColor(data + offset, sixteenBit);

            // ...we enter this loop to sum the bits
            for (a = -Radius; runningFlag() && (a <= Radius); ++a)
            {
                // verify if is inside the rect
                if (IsInside( Width, Height, w + a, h))
                {
                    // read color
                    loopOffset = GetOffset(Width, w+a, h, bytesDepth);
                    radiusColor.setColor(data + loopOffset, sixteenBit);

                    // now, we have to check if is inside the sensibility filter
                    if (IsColorInsideTheRange (color.red(), color.green(), color.blue(),
                                               radiusColor.red(), radiusColor.green(), radiusColor.blue(),
                                               StrengthRange))
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

            // now, we have to calc the arithmetic average
            color.setRed  (sumR / nCount);
            color.setGreen(sumG / nCount);
            color.setBlue (sumB / nCount);

            // write color to destination
            color.setPixel(pBlur.data() + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 50.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }

    // we have reached the second part of main loop

    for (w = 0; runningFlag() && (w < Width); ++w)
    {
        for (h = 0; runningFlag() && ( h < Height); ++h)
        {
            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;

            // read color
            offset = GetOffset(Width, w, h, bytesDepth);
            color.setColor(data + offset, sixteenBit);

            // ...we enter this loop to sum the bits
            for (a = -Radius; runningFlag() && (a <= Radius); ++a)
            {
                // verify if is inside the rect
                if (IsInside( Width, Height, w, h + a))
                {
                    // read color
                    loopOffset = GetOffset(Width, w, h+a, bytesDepth);
                    radiusColor.setColor(data + loopOffset, sixteenBit);

                    // now, we have to check if is inside the sensibility filter
                    if (IsColorInsideTheRange (color.red(), color.green(), color.blue(),
                                               radiusColor.red(), radiusColor.green(), radiusColor.blue(),
                                               StrengthRange))
                    {
                        radiusColorBlur.setColor(pBlur.data() + loopOffset, sixteenBit);
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

            // now, we have to calc the arithmetic average
            color.setRed  (sumR / nCount);
            color.setGreen(sumG / nCount);
            color.setBlue (sumB / nCount);

            // write color to destination
            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (50.0 + ((double)w * 50.0) / Width);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
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
 *                     simple to undertand, we take a pixel (duh!), with the angle we
 *                     will taking near pixels. After this we blur (add and do a
 *                     division).
 */
void BlurFXFilter::motionBlur(DImg* orgImage, DImg* destImage, int Distance, double Angle)
{
    if (Distance == 0)
    {
        return;
    }

    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    DColor color;
    int offset;

    // we try to avoid division by 0 (zero)
    if (Angle == 0.0)
    {
        Angle = 360.0;
    }

    int sumR, sumG, sumB, nCount, nw, nh;
    double nAngX, nAngY;

    // we initialize cos and sin for a best performance
    nAngX = cos ((2.0 * M_PI) / (360.0 / Angle));
    nAngY = sin ((2.0 * M_PI) / (360.0 / Angle));

    // total of bits to be taken is given by this formula
    nCount = Distance * 2 + 1;

    // we will alloc size and calc the possible results
    QScopedArrayPointer<int> lpXArray(new int[nCount]);
    QScopedArrayPointer<int> lpYArray(new int[nCount]);

    for (int i = 0; i < nCount; ++i)
    {
        lpXArray[i] = lround( (double)(i - Distance) * nAngX);
        lpYArray[i] = lround( (double)(i - Distance) * nAngY);
    }

    // we have reached the main loop

    for (int h = 0; runningFlag() && (h < Height); ++h)
    {
        for (int w = 0; runningFlag() && (w < Width); ++w)
        {
            // we initialize the variables
            sumR = sumG = sumB = 0;

            // ...we enter this loop to sum the bits
            for (int a = -Distance; runningFlag() && (a <= Distance); ++a)
            {
                // we need to calc the positions
                nw = w + lpXArray[a + Distance];
                nh = h + lpYArray[a + Distance];

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
            offset = GetOffset(Width, w, h, bytesDepth);
            // read color to preserve alpha
            color.setColor(data + offset, sixteenBit);

            // now, we have to calc the arithmetic average
            color.setRed  (sumR / nCount);
            color.setGreen(sumG / nCount);
            color.setBlue (sumB / nCount);

            // write color to destination
            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
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
 *                     blur with 3x3 dimentions, in light tones, we apply a blur with
 *                     5x5 dimentions. Easy, hun?
 */
void BlurFXFilter::softenerBlur(DImg* orgImage, DImg* destImage)
{
    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int SomaR = 0, SomaG = 0, SomaB = 0;
    int Gray;

    DColor color, colorSoma;
    int offset, offsetSoma;

    int grayLimit = sixteenBit ? 32767 : 127;

    for (int h = 0; runningFlag() && (h < Height); ++h)
    {
        for (int w = 0; runningFlag() && (w < Width); ++w)
        {
            SomaR = SomaG = SomaB = 0;

            offset = GetOffset(Width, w, h, bytesDepth);
            color.setColor(data + offset, sixteenBit);

            Gray = (color.red() + color.green() + color.blue()) / 3;

            if (Gray > grayLimit)
            {
                // 7x7
                for (int a = -3; runningFlag() && (a <= 3); ++a)
                {
                    for (int b = -3; runningFlag() && (b <= 3); ++b)
                    {
                        if ((h + a < 0) || (w + b < 0))
                        {
                            offsetSoma = offset;
                        }
                        else
                            offsetSoma = GetOffset(Width, (w + Lim_Max (w, b, Width)),
                                                   (h + Lim_Max (h, a, Height)), bytesDepth);

                        colorSoma.setColor(data + offsetSoma, sixteenBit);

                        SomaR += colorSoma.red();
                        SomaG += colorSoma.green();
                        SomaB += colorSoma.blue();
                    }
                }

                // 7*7 = 49
                color.setRed  (SomaR / 49);
                color.setGreen(SomaG / 49);
                color.setBlue (SomaB / 49);
                color.setPixel(pResBits + offset);
            }
            else
            {
                // 3x3
                for (int a = -1; runningFlag() && (a <= 1); ++a)
                {
                    for (int b = -1; runningFlag() && (b <= 1); ++b)
                    {
                        if ((h + a < 0) || (w + b < 0))
                        {
                            offsetSoma = offset;
                        }
                        else
                            offsetSoma = GetOffset(Width, (w + Lim_Max (w, b, Width)),
                                                   (h + Lim_Max (h, a, Height)), bytesDepth);

                        colorSoma.setColor(data + offsetSoma, sixteenBit);

                        SomaR += colorSoma.red();
                        SomaG += colorSoma.green();
                        SomaB += colorSoma.blue();
                    }
                }

                // 3*3 = 9
                color.setRed  (SomaR / 9);
                color.setGreen(SomaG / 9);
                color.setBlue (SomaB / 9);
                color.setPixel(pResBits + offset);
            }
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
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
void BlurFXFilter::shakeBlur(DImg* orgImage, DImg* destImage, int Distance)
{
    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    DColor color, colorLayer, color1, color2, color3, color4;
    int offset, offsetLayer;

    int numBytes = orgImage->numBytes();
    uchar* Layer1 = new uchar[numBytes];
    uchar* Layer2 = new uchar[numBytes];
    uchar* Layer3 = new uchar[numBytes];
    uchar* Layer4 = new uchar[numBytes];

    int h, w, nw, nh;

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            offsetLayer = GetOffset(Width, w, h, bytesDepth);

            nh = (h + Distance >= Height) ? Height - 1 : h + Distance;
            offset = GetOffset(Width, w, nh, bytesDepth);
            color.setColor(data + offset, sixteenBit);
            color.setPixel(Layer1 + offsetLayer);

            nh = (h - Distance < 0) ? 0 : h - Distance;
            offset = GetOffset(Width, w, nh, bytesDepth);
            color.setColor(data + offset, sixteenBit);
            color.setPixel(Layer2 + offsetLayer);

            nw = (w + Distance >= Width) ? Width - 1 : w + Distance;
            offset = GetOffset(Width, nw, h, bytesDepth);
            color.setColor(data + offset, sixteenBit);
            color.setPixel(Layer3 + offsetLayer);

            nw = (w - Distance < 0) ? 0 : w - Distance;
            offset = GetOffset(Width, nw, h, bytesDepth);
            color.setColor(data + offset, sixteenBit);
            color.setPixel(Layer4 + offsetLayer);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 50.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }

    for (int h = 0; runningFlag() && (h < Height); ++h)
    {
        for (int w = 0; runningFlag() && (w < Width); ++w)
        {
            offset = GetOffset(Width, w, h, bytesDepth);
            // read original data to preserve alpha
            color.setColor(data + offset, sixteenBit);
            // read colors from all four layers
            color1.setColor(Layer1 + offset, sixteenBit);
            color2.setColor(Layer2 + offset, sixteenBit);
            color3.setColor(Layer3 + offset, sixteenBit);
            color4.setColor(Layer4 + offset, sixteenBit);

            // set color components of resulting color
            color.setRed  ( (color1.red()   + color2.red()   + color3.red()   + color4.red())   / 4 );
            color.setGreen( (color1.green() + color2.green() + color3.green() + color4.green()) / 4 );
            color.setBlue ( (color1.blue()  + color2.blue()  + color3.blue()  + color4.blue())  / 4 );

            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (50.0 + ((double)h * 50.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }

    delete [] Layer1;
    delete [] Layer2;
    delete [] Layer3;
    delete [] Layer4;
}

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
void BlurFXFilter::frostGlass(DImg* orgImage, DImg* destImage, int Frost)
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
    generator.seed(m_randomSeed);

    int range = sixteenBit ? 65535 : 255;

    // it is a huge optimization to allocate these here once
    uchar* IntensityCount = new uchar[range + 1];
    uint* AverageColorR   = new uint[range + 1];
    uint* AverageColorG   = new uint[range + 1];
    uint* AverageColorB   = new uint[range + 1];

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            offset = GetOffset(Width, w, h, bytesDepth);
            // read color to preserve alpha
            color.setColor(data + offset, sixteenBit);

            // get random color from surrounding of w|h
            color = RandomColor (data, Width, Height, sixteenBit, bytesDepth,
                                 w, h, Frost, color.alpha(), generator, range, IntensityCount,
                                 AverageColorR, AverageColorG, AverageColorB);

            // write color to destination
            color.setPixel(pResBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }

    delete [] IntensityCount;
    delete [] AverageColorR;
    delete [] AverageColorG;
    delete [] AverageColorB;
}

/* Function to apply the mosaic effect backported from ImageProcessing version 2
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Size             => Size of mosaic .
 *
 * Theory           => Ok, you can find some mosaic effects on PSC, but this one
 *                     has a great feature, if you see a mosaic in other code you will
 *                     see that the corner pixel doesn't change. The explanation is
 *                     simple, the color of the mosaic is the same as the first pixel
 *                     get. Here, the color of the mosaic is the same as the mosaic
 *                     center pixel.
 *                     Now the function scan the rows from the top (like photoshop).
 */
void BlurFXFilter::mosaic(DImg* orgImage, DImg* destImage, int SizeW, int SizeH)
{
    int progress;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

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

    DColor color;
    int offsetCenter, offset;

    // this loop will never look for transparent colors

    for (int h = 0; runningFlag() && (h < Height); h += SizeH)
    {
        for (int w = 0; runningFlag() && (w < Width); w += SizeW)
        {
            // we have to find the center pixel for mosaic's rectangle

            offsetCenter = GetOffsetAdjusted(Width, Height, w + (SizeW / 2), h + (SizeH / 2), bytesDepth);
            color.setColor(data + offsetCenter, sixteenBit);

            // now, we fill the mosaic's rectangle with the center pixel color

            for (int subw = w; runningFlag() && (subw <= w + SizeW); ++subw)
            {
                for (int subh = h; runningFlag() && (subh <= h + SizeH); ++subh)
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

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }
}

/* Function to get a color in a matriz with a determined size
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
DColor BlurFXFilter::RandomColor(uchar* Bits, int Width, int Height, bool sixteenBit, int bytesDepth,
                                 int X, int Y, int Radius,
                                 int alpha, RandomNumberGenerator& generator, int range, uchar* IntensityCount,
                                 uint* AverageColorR, uint* AverageColorG, uint* AverageColorB)
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
                I = GetIntensity (color.red(), color.green(), color.blue());
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
        while (count < RandNumber && runningFlag());

        J = Index - 1;
        ++ErrorCount;
    }
    while ((IntensityCount[J] == 0) && (ErrorCount <= counter)  && runningFlag());

    if (!runningFlag())
    {
        return DColor(0, 0, 0, 0, sixteenBit);
    }


    color.setSixteenBit(sixteenBit);
    color.setAlpha(alpha);

    if (ErrorCount >= counter)
    {
        color.setRed  (AverageColorR[J] / counter);
        color.setGreen(AverageColorG[J] / counter);
        color.setBlue (AverageColorB[J] / counter);
    }
    else
    {
        color.setRed  (AverageColorR[J] / IntensityCount[J]);
        color.setGreen(AverageColorG[J] / IntensityCount[J]);
        color.setBlue (AverageColorB[J] / IntensityCount[J]);
    }

    return color;
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
void BlurFXFilter::MakeConvolution (DImg* orgImage, DImg* destImage, int Radius, int Kernel[])
{
    if (Radius <= 0)
    {
        return;
    }

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pOutBits = destImage->bits();

    int progress;
    int n, h, w;

    int nSumR, nSumG, nSumB, nCount;
    int nKernelWidth = Radius * 2 + 1;
    int range = sixteenBit ? 65536 : 256;
    DColor color;
    int offset;

    QScopedArrayPointer<uchar> pBlur(new uchar[orgImage->numBytes()]);

    // We need to copy our bits to blur bits

    memcpy (pBlur.data(), data, orgImage->numBytes());

    // We need to alloc a 2d array to help us to store the values

    int** arrMult = Alloc2DArray (nKernelWidth, range);

    for (int i = 0; i < nKernelWidth; ++i)
        for (int j = 0; j < range; ++j)
        {
            arrMult[i][j] = j * Kernel[i];
        }

    // Now, we enter in the main loop

    for (h = 0; runningFlag() && (h < Height); ++h)
    {
        for (w = 0; runningFlag() && (w < Width); ++w)
        {
            // initialize the variables
            nSumR = nSumG = nSumB = nCount = 0;

            // first of all, we need to blur the horizontal lines

            for (n = -Radius; runningFlag() && (n <= Radius); ++n)
            {
                // if is inside...
                if (IsInside (Width, Height, w + n, h))
                {
                    // read color from orgImage
                    offset = GetOffset(Width, w+n, h, bytesDepth);
                    color.setColor(data + offset, sixteenBit);

                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][color.red()];
                    nSumG += arrMult[n + Radius][color.green()];
                    nSumB += arrMult[n + Radius][color.blue()];

                    // we need to add the kernel value to the counter
                    nCount += Kernel[n + Radius];
                }
            }

            if (nCount == 0)
            {
                nCount = 1;
            }

            // calculate pointer
            offset = GetOffset(Width, w, h, bytesDepth);
            // read color from orgImage to preserve alpha
            color.setColor(data + offset, sixteenBit);

            // now, we have to calc the arithmetic average
            if (sixteenBit)
            {
                color.setRed  (LimitValues16(nSumR / nCount));
                color.setGreen(LimitValues16(nSumG / nCount));
                color.setBlue (LimitValues16(nSumB / nCount));
            }
            else
            {
                color.setRed  (LimitValues8(nSumR / nCount));
                color.setGreen(LimitValues8(nSumG / nCount));
                color.setBlue (LimitValues8(nSumB / nCount));
            }

            // write color to blur bits
            color.setPixel(pBlur.data() + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 50.0) / Height);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }

    // We enter in the second main loop
    for (w = 0; runningFlag() && (w < Width); ++w)
    {
        for (h = 0; runningFlag() && (h < Height); ++h)
        {
            // initialize the variables
            nSumR = nSumG = nSumB = nCount = 0;

            // first of all, we need to blur the vertical lines
            for (n = -Radius; runningFlag() && (n <= Radius); ++n)
            {
                // if is inside...
                if (IsInside(Width, Height, w, h + n))
                {
                    // read color from blur bits
                    offset = GetOffset(Width, w, h+n, bytesDepth);
                    color.setColor(pBlur.data() + offset, sixteenBit);

                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][color.red()];
                    nSumG += arrMult[n + Radius][color.green()];
                    nSumB += arrMult[n + Radius][color.blue()];

                    // we need to add the kernel value to the counter
                    nCount += Kernel[n + Radius];
                }
            }

            if (nCount == 0)
            {
                nCount = 1;
            }

            // calculate pointer
            offset = GetOffset(Width, w, h, bytesDepth);
            // read color from orgImage to preserve alpha
            color.setColor(data + offset, sixteenBit);

            // now, we have to calc the arithmetic average
            if (sixteenBit)
            {
                color.setRed  (LimitValues16(nSumR / nCount));
                color.setGreen(LimitValues16(nSumG / nCount));
                color.setBlue (LimitValues16(nSumB / nCount));
            }
            else
            {
                color.setRed  (LimitValues8(nSumR / nCount));
                color.setGreen(LimitValues8(nSumG / nCount));
                color.setBlue (LimitValues8(nSumB / nCount));
            }

            // write color to destination
            color.setPixel(pOutBits + offset);
        }

        // Update the progress bar in dialog.
        progress = (int) (50.0 + ((double)w * 50.0) / Width);

        if (progress%5 == 0)
        {
            postProgress(progress);
        }
    }

    // now, we must free memory
    Free2DArray (arrMult, nKernelWidth);
}

FilterAction BlurFXFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("type", m_blurFXType);
    action.addParameter("distance", m_distance);
    action.addParameter("level", m_level);

    if (m_blurFXType == FrostGlass)
    {
        action.addParameter("randomSeed", m_randomSeed);
    }

    return action;
}

void BlurFXFilter::readParameters(const Digikam::FilterAction& action)
{
    m_blurFXType = action.parameter("type").toInt();
    m_distance = action.parameter("distance").toInt();
    m_level = action.parameter("level").toInt();

    if (m_blurFXType == FrostGlass)
    {
        m_randomSeed = action.parameter("randomSeed").toUInt();
    }
}



}  // namespace Digikam
