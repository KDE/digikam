/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 *
 * Copyright (C) 2005-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "blurfilter.h"

/** Don't use CImg interface (keyboard/mouse interaction) */
#define cimg_display 0
/** Only print debug information on the console */
#define cimg_debug 1

// CImg includes

#include "CImg.h"

// Qt includes

#include <qmath.h>

// KDE includes

#include <kdebug.h>

using namespace cimg_library;

namespace Digikam
{

BlurFilter::BlurFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    m_radius = 3;
    initFilter();
}

BlurFilter::BlurFilter(DImg* const orgImage, QObject* const parent, int radius)
    : DImgThreadedFilter(orgImage, parent, "GaussianBlur")
{
    m_radius = radius;
    initFilter();
}

BlurFilter::BlurFilter(DImgThreadedFilter* const parentFilter,
                       const DImg& orgImage, const DImg& destImage,
                       int progressBegin, int progressEnd, int radius)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + ": GaussianBlur")
{
    m_radius = radius;
    filterImage();
}

BlurFilter::~BlurFilter()
{
    cancelFilter();
}

void BlurFilter::filterImage()
{
#if defined(__MACOSX__) || defined(__APPLE__)
    gaussianBlurImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                      m_orgImage.sixteenBit(), m_radius);
#else
    cimgBlurImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                  m_orgImage.sixteenBit(), m_radius / 10.0);
#endif
}

void BlurFilter::cimgBlurImage(uchar* data, int width, int height, bool sixteenBit, double radius)
{
    if (!data || !width || !height)
    {
        kWarning() << ("no image data available!");
        return;
    }

    if (radius > 100.0)
    {
        radius = 100.0;
    }

    if (radius <= 0.0)
    {
        m_destImage = m_orgImage;
        postProgress(100);
        return;
    }

    kDebug() << "Radius: " << radius;

    kDebug() << "BlurFilter::Process Computation...";

    if (!sixteenBit)           // 8 bits image.
    {
        // convert DImg (interleaved RGBA) to CImg (planar RGBA)
        CImg<uchar> img = CImg<uchar>(data, 4, width, height, 1, true).
                          get_permute_axes("yzvx");
        postProgress(25);

        // blur the image
        img.blur(radius);
        postProgress(50);

        // Copy CImg onto destination.
        kDebug() << "BlurFilter::Finalization...";

        uchar* ptr = m_destImage.bits();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                // Overwrite RGB values to destination.
                ptr[0] = img(x, y, 0);        // Blue
                ptr[1] = img(x, y, 1);        // Green
                ptr[2] = img(x, y, 2);        // Red
                ptr[3] = img(x, y, 3);        // Alpha
                ptr    += 4;
            }
        }

        postProgress(75);
    }
    else                                // 16 bits image.
    {
        // convert DImg (interleaved RGBA) to CImg (planar RGBA)
        CImg<unsigned short> img = CImg<unsigned short>((unsigned short*)data, 4, width, height, 1, true).
                                   get_permute_axes("yzvx");
        postProgress(25);

        // blur the image
        img.blur(radius);
        postProgress(50);

        // Copy CImg onto destination.
        kDebug() << "BlurFilter::Finalization...";

        unsigned short* ptr = (unsigned short*)m_destImage.bits();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                // Overwrite RGB values to destination.
                ptr[0] = img(x, y, 0);        // Blue
                ptr[1] = img(x, y, 1);        // Green
                ptr[2] = img(x, y, 2);        // Red
                ptr[3] = img(x, y, 3);        // Alpha
                ptr    += 4;
            }
        }

        postProgress(75);
    }

    postProgress(100);
}

void BlurFilter::gaussianBlurImage(uchar* data, int width, int height, bool sixteenBit, int radius)
{
    if (!data || !width || !height)
    {
        kWarning(50003) << ("DImgGaussianBlur::gaussianBlurImage: no image data available!") << endl;
        return;
    }

    if (radius > 100)
    {
        radius = 100;
    }

    if (radius <= 0)
    {
        m_destImage = m_orgImage;
        return;
    }

    kDebug() << "Radius: " << radius;

    // Gaussian kernel computation using the Radius parameter.

    int          nKSize, nCenter;
    double       x, sd, factor, lnsd, lnfactor;
    register int i, j, n, h, w;

    nKSize   = 2 * radius + 1;
    nCenter  = nKSize / 2;
    QScopedArrayPointer<int> Kernel(new int[nKSize]);

    lnfactor = (4.2485 - 2.7081) / 10 * nKSize + 2.7081;
    lnsd     = (0.5878 + 0.5447) / 10 * nKSize - 0.5447;
    factor   = qExp(lnfactor);
    sd       = qExp(lnsd);

    for (i = 0; runningFlag() && (i < nKSize); ++i)
    {
        x = qSqrt((i - nCenter) * (i - nCenter));
        Kernel[i] = (int)(factor * qExp(-0.5 * qPow((x / sd), 2)) / (sd * qSqrt(2.0 * M_PI)));
    }

    // Now, we need to convolve the image descriptor.
    // I've worked hard here, but I think this is a very smart
    // way to convolve an array, its very hard to explain how I reach
    // this, but the trick here its to store the sum used by the
    // previous pixel, so we sum with the other pixels that wasn't get.

    int nSumA, nSumR, nSumG, nSumB, nCount, progress;
    int nKernelWidth = radius * 2 + 1;

    // We need to alloc a 2d array to help us to store the values

    int** arrMult = Alloc2DArray(nKernelWidth, sixteenBit ? 65536 : 256);

    for (i = 0; runningFlag() && (i < nKernelWidth); ++i)
        for (j = 0; runningFlag() && (j < (sixteenBit ? 65536 : 256)); ++j)
        {
            arrMult[i][j] = j * Kernel[i];
        }

    // We need to copy our bits to blur bits

    uchar* pOutBits = m_destImage.bits();
    QScopedArrayPointer<uchar> pBlur(new uchar[m_destImage.numBytes()]);

    memcpy(pBlur.data(), data, m_destImage.numBytes());

    // We need to initialize all the loop and iterator variables

    nSumA = nSumR = nSumG = nSumB = nCount = i = j = 0;
    unsigned short* data16     = (unsigned short*)data;
    unsigned short* pBlur16    = (unsigned short*)pBlur.data();
    unsigned short* pOutBits16 = (unsigned short*)pOutBits;

    // Now, we enter in the main loop

    for (h = 0; runningFlag() && (h < height); ++h)
    {
        for (w = 0; runningFlag() && (w < width); ++w, i += 4)
        {
            if (!sixteenBit)        // 8 bits image.
            {
                uchar* org = 0;
                uchar* dst = 0;

                // first of all, we need to blur the horizontal lines

                for (n = -radius; runningFlag() && (n <= radius); ++n)
                {
                    // if is inside...
                    if (IsInside(width, height, w + n, h))
                    {
                        // we points to the pixel
                        j = i + 4 * n;

                        // finally, we sum the pixels using a method similar to assigntables

                        org = &data[j];
                        nSumA += arrMult[n + radius][org[3]];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];

                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }

                if (nCount == 0)
                {
                    nCount = 1;
                }

                // now, we return to blur bits the horizontal blur values
                dst    = &pBlur[i];
                dst[3] = (uchar)CLAMP(nSumA / nCount, 0, 255);
                dst[2] = (uchar)CLAMP(nSumR / nCount, 0, 255);
                dst[1] = (uchar)CLAMP(nSumG / nCount, 0, 255);
                dst[0] = (uchar)CLAMP(nSumB / nCount, 0, 255);

                // ok, now we reinitialize the variables
                nSumA = nSumR = nSumG = nSumB = nCount = 0;
            }
            else                 // 16 bits image.
            {
                unsigned short* org = 0;
                unsigned short* dst = 0;

                // first of all, we need to blur the horizontal lines

                for (n = -radius; runningFlag() && (n <= radius); ++n)
                {
                    // if is inside...
                    if (IsInside(width, height, w + n, h))
                    {
                        // we points to the pixel
                        j = i + 4 * n;

                        // finally, we sum the pixels using a method similar to assigntables

                        org = &data16[j];
                        nSumA += arrMult[n + radius][org[3]];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];

                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }

                if (nCount == 0)
                {
                    nCount = 1;
                }

                // now, we return to blur bits the horizontal blur values
                dst    = &pBlur16[i];
                dst[3] = (unsigned short)CLAMP(nSumA / nCount, 0, 65535);
                dst[2] = (unsigned short)CLAMP(nSumR / nCount, 0, 65535);
                dst[1] = (unsigned short)CLAMP(nSumG / nCount, 0, 65535);
                dst[0] = (unsigned short)CLAMP(nSumB / nCount, 0, 65535);

                // ok, now we reinitialize the variables
                nSumA = nSumR = nSumG = nSumB = nCount = 0;
            }
        }

        progress = (int)(((double)h * 50.0) / height);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // getting the blur bits, we initialize position variables
    i = j = 0;

    // We enter in the second main loop
    for (w = 0; runningFlag() && (w < width); ++w, i = w * 4)
    {
        for (h = 0; runningFlag() && (h < height); ++h, i += width * 4)
        {
            if (!sixteenBit)        // 8 bits image.
            {
                uchar* org, *dst;

                // first of all, we need to blur the vertical lines
                for (n = -radius; runningFlag() && (n <= radius); ++n)
                {
                    // if is inside...
                    if (IsInside(width, height, w, h + n))
                    {
                        // we points to the pixel
                        j = i + n * 4 * width;

                        // finally, we sum the pixels using a method similar to assigntables
                        org   = &pBlur[j];
                        nSumA += arrMult[n + radius][org[3]];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];

                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }

                if (nCount == 0)
                {
                    nCount = 1;
                }

                // To preserve Alpha channel.
                memcpy(&pOutBits[i], &data[i], 4);

                // now, we return to bits the vertical blur values
                dst    = &pOutBits[i];
                dst[3] = (uchar)CLAMP(nSumA / nCount, 0, 255);
                dst[2] = (uchar)CLAMP(nSumR / nCount, 0, 255);
                dst[1] = (uchar)CLAMP(nSumG / nCount, 0, 255);
                dst[0] = (uchar)CLAMP(nSumB / nCount, 0, 255);

                // ok, now we reinitialize the variables
                nSumA = nSumR = nSumG = nSumB = nCount = 0;
            }
            else                 // 16 bits image.
            {
                unsigned short* org, *dst;

                // first of all, we need to blur the vertical lines
                for (n = -radius; runningFlag() && (n <= radius); ++n)
                {
                    // if is inside...
                    if (IsInside(width, height, w, h + n))
                    {
                        // we points to the pixel
                        j = i + n * 4 * width;

                        // finally, we sum the pixels using a method similar to assigntables
                        org = &pBlur16[j];
                        nSumA += arrMult[n + radius][org[3]];
                        nSumR += arrMult[n + radius][org[2]];
                        nSumG += arrMult[n + radius][org[1]];
                        nSumB += arrMult[n + radius][org[0]];

                        // we need to add to the counter, the kernel value
                        nCount += Kernel[n + radius];
                    }
                }

                if (nCount == 0)
                {
                    nCount = 1;
                }

                // To preserve Alpha channel.
                memcpy(&pOutBits16[i], &data16[i], 8);

                // now, we return to bits the vertical blur values
                dst    = &pOutBits16[i];
                dst[3] = (unsigned short)CLAMP(nSumA / nCount, 0, 65535);
                dst[2] = (unsigned short)CLAMP(nSumR / nCount, 0, 65535);
                dst[1] = (unsigned short)CLAMP(nSumG / nCount, 0, 65535);
                dst[0] = (unsigned short)CLAMP(nSumB / nCount, 0, 65535);

                // ok, now we reinitialize the variables
                nSumA = nSumR = nSumG = nSumB = nCount = 0;
            }
        }

        progress = (int)(50.0 + ((double)w * 50.0) / width);

        if (progress % 5 == 0)
        {
            postProgress(progress);
        }
    }

    // now, we must free memory
    Free2DArray(arrMult, nKernelWidth);
}

FilterAction BlurFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("radius", m_radius);

    return action;
}

void BlurFilter::readParameters(const FilterAction& action)
{
    m_radius = action.parameter("radius").toInt();
}

}  // namespace Digikam
