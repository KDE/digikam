/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : misc image filters
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Original channel mixer algorithm copyrighted 2002 by
 * Martin Guldahl <mguldahl at xmission dot com> from Gimp 2.2
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

#include "dimgimagefilters.h"

// C++ includes

#include <cstring>
#include <cstdlib>

// KDE includes

#include <kdebug.h>

// Local includes

#include "imagehistogram.h"
#include "dcolor.h"
#include "blurfilter.h"
#include "dimgsharpen.h"
#include "dimgunsharpmask.h"
#include "dimgrefocus.h"

namespace Digikam
{

/** Performs image colors inversion. This tool is used for negate image
    resulting of a positive film scanned.*/
void DImgImageFilters::invertImage(uchar *data, int w, int h, bool sixteenBit)
{
    if (!data || !w || !h)
    {
       kWarning() << ("DImgImageFilters::invertImage: no image data available!");
       return;
    }

    if (!sixteenBit)        // 8 bits image.
    {
        uchar *ptr = data;

        for (int i = 0 ; i < w*h ; ++i)
        {
            ptr[0] = 255 - ptr[0];
            ptr[1] = 255 - ptr[1];
            ptr[2] = 255 - ptr[2];
            ptr[3] = 255 - ptr[3];
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short *ptr = (unsigned short *)data;

        for (int i = 0 ; i < w*h ; ++i)
        {
            ptr[0] = 65535 - ptr[0];
            ptr[1] = 65535 - ptr[1];
            ptr[2] = 65535 - ptr[2];
            ptr[3] = 65535 - ptr[3];
            ptr += 4;
        }
    }
}

/** Function to apply the GaussianBlur on an image. This method do not use a
    dedicated thread.*/
void DImgImageFilters::gaussianBlurImage(uchar *data, int width, int height, bool sixteenBit, int radius)
{
    if (!data || !width || !height)
    {
       kWarning() << ("DImgImageFilters::gaussianBlurImage: no image data available!");
       return;
    }

    if (radius > 100) radius = 100;
    if (radius <= 0) return;

    DImg orgImage(width, height, sixteenBit, true, data);
    BlurFilter* filter = new BlurFilter(&orgImage, 0L, radius);
    filter->startFilterDirectly();
    DImg imDest = filter->getTargetImage();
    memcpy( data, imDest.bits(), imDest.numBytes() );
    delete filter;
}

/** Function to apply the sharpen filter on an image. This method do not use a
    dedicated thread.*/
void DImgImageFilters::sharpenImage(uchar *data, int width, int height, bool sixteenBit, int radius)
{
    if (!data || !width || !height)
    {
       kWarning() << ("DImgImageFilters::sharpenImage: no image data available!");
       return;
    }

    if (radius > 100) radius = 100;
    if (radius <= 0) return;

    DImg orgImage(width, height, sixteenBit, true, data);
    DImgSharpen *filter = new DImgSharpen(&orgImage, 0L, radius);
    filter->startFilterDirectly();
    DImg imDest = filter->getTargetImage();
    memcpy(data, imDest.bits(), imDest.numBytes());
    delete filter;
}

void DImgImageFilters::unsharpMaskImage(uchar *data, int width, int height, bool sixteenBit,
        int radius, double amount, double threshold)
{
    if (!data || !width || !height)
    {
       kWarning() << ("DImgImageFilters::unsharpMaskImage: no image data available!");
       return;
    }

    DImg orgImage(width, height, sixteenBit, true, data);
    DImgUnsharpMask *filter = new DImgUnsharpMask(&orgImage, 0L, radius, amount, threshold);
    filter->startFilterDirectly();
    DImg imDest = filter->getTargetImage();
    memcpy(data, imDest.bits(), imDest.numBytes());
    delete filter;
}

void DImgImageFilters::refocusImage(uchar *data, int width, int height, bool sixteenBit,
        int matrixSize, double radius, double gauss, double correlation, double noise)
{
    if (!data || !width || !height)
    {
       kWarning() << ("DImgImageFilters::refocusImage: no image data available!");
       return;
    }

    DImg orgImage(width, height, sixteenBit, true, data);
    DImgRefocus *filter = new DImgRefocus(&orgImage, 0L, matrixSize,
            radius, gauss, correlation, noise);
    filter->startFilterDirectly();
    DImg imDest = filter->getTargetImage();
    memcpy(data, imDest.bits(), imDest.numBytes());
    delete filter;
}

/** Function to perform pixel antialiasing with 8 bits/color/pixel images. This method is used to smooth target
    image in transformation  method like free rotation or shear tool. */
void DImgImageFilters::pixelAntiAliasing(uchar *data, int Width, int Height, double X, double Y,
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

    for (int loopx = 0; loopx <= 1; ++loopx)
    {
        for (int loopy = 0; loopy <= 1; ++loopy)
        {
            lfWeight = lfWeightX[loopx] * lfWeightY[loopy];
            j = setPositionAdjusted (Width, Height, nX + loopx, nY + loopy);

            lfTotalB += ((double)data[j] * lfWeight);
            ++j;
            lfTotalG += ((double)data[j] * lfWeight);
            ++j;
            lfTotalR += ((double)data[j] * lfWeight);
            ++j;
            lfTotalA += ((double)data[j] * lfWeight);
            ++j;
        }
    }

    *B = CLAMP0255((int)lfTotalB);
    *G = CLAMP0255((int)lfTotalG);
    *R = CLAMP0255((int)lfTotalR);
    *A = CLAMP0255((int)lfTotalA);
}

/** Function to perform pixel antialiasing with 16 bits/color/pixel images. This method is used to smooth target
    image in transformation  method like free rotation or shear tool. */
void DImgImageFilters::pixelAntiAliasing16(unsigned short *data, int Width, int Height, double X, double Y,
                                           unsigned short *A, unsigned short *R, unsigned short *G,
                                           unsigned short *B)
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

    for (int loopx = 0; loopx <= 1; ++loopx)
    {
        for (int loopy = 0; loopy <= 1; ++loopy)
        {
            lfWeight = lfWeightX[loopx] * lfWeightY[loopy];
            j = setPositionAdjusted (Width, Height, nX + loopx, nY + loopy);

            lfTotalB += ((double)data[j] * lfWeight);
            ++j;
            lfTotalG += ((double)data[j] * lfWeight);
            ++j;
            lfTotalR += ((double)data[j] * lfWeight);
            ++j;
            lfTotalA += ((double)data[j] * lfWeight);
            ++j;
        }
    }

    *B = CLAMP065535((int)lfTotalB);
    *G = CLAMP065535((int)lfTotalG);
    *R = CLAMP065535((int)lfTotalR);
    *A = CLAMP065535((int)lfTotalA);
}

}  // namespace Digikam
