/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "dimggaussianblur.h"

/** Don't use CImg interface (keyboard/mouse interaction) */
#define cimg_display 0
/** Only print debug information on the console */
#define cimg_debug 1

// KDE includes

#include <kdebug.h>

// CImg includes

#include "CImg.h"

// Local includes

#include "dimgimagefilters.h"

using namespace cimg_library;

namespace Digikam
{

DImgGaussianBlur::DImgGaussianBlur(DImg *orgImage, QObject *parent, double radius)
                : DImgThreadedFilter(orgImage, parent, "GaussianBlur")
{
    m_radius = radius;
    initFilter();
}

DImgGaussianBlur::DImgGaussianBlur(DImgThreadedFilter *parentFilter,
                                   const DImg &orgImage, const DImg &destImage,
                                   int progressBegin, int progressEnd, double radius)
                : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                                     parentFilter->filterName() + ": GaussianBlur")
{
    m_radius = radius;
    filterImage();
}

void DImgGaussianBlur::filterImage()
{
    gaussianBlurImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                      m_orgImage.sixteenBit(), m_radius);
}

/** Function to apply the Gaussian Blur on an image*/

void DImgGaussianBlur::gaussianBlurImage(uchar *data, int width, int height, bool sixteenBit, double radius)
{
    if (!data || !width || !height)
    {
       kWarning(50003) << ("DImgGaussianBlur::gaussianBlurImage: no image data available!") << endl;
       return;
    }

    if (radius > 100.0) radius = 100.0;
    if (radius <= 0.0)
    {
       m_destImage = m_orgImage;
       return;
    }

    // Copy the src image data into a CImg type image
    CImg<> img = CImg<>(width, height, 1, 4);

    if (!sixteenBit)           // 8 bits image.
    {
        uchar *ptr = data;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                img(x, y, 0) = ptr[0];        // Blue.
                img(x, y, 1) = ptr[1];        // Green.
                img(x, y, 2) = ptr[2];        // Red.
                img(x, y, 3) = ptr[3];        // Alpha.
                ptr += 4;
            }
        }
    }
    else                                // 16 bits image.
    {
        unsigned short *ptr = (unsigned short *)data;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                img(x, y, 0) = ptr[0];        // Blue.
                img(x, y, 1) = ptr[1];        // Green.
                img(x, y, 2) = ptr[2];        // Red.
                img(x, y, 3) = ptr[3];        // Alpha.
                ptr += 4;
            }
        }
    }

    kDebug(50003) << "DImgGaussianBlur::Process Computation..." << endl;

    // blur the image
    img.blur(radius);

    // Copy CImg onto destination.
    kDebug(50003) << "DImgGaussianBlur::Finalization..." << endl;

    if (!sixteenBit)           // 8 bits image.
    {
        uchar *ptr = m_destImage.bits();
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                // Overwrite RGB values to destination.
                ptr[0] = static_cast<uchar>(img(x, y, 0));        // Blue
                ptr[1] = static_cast<uchar>(img(x, y, 1));        // Green
                ptr[2] = static_cast<uchar>(img(x, y, 2));        // Red
                ptr[3] = static_cast<uchar>(img(x, y, 3));        // Alpha
                ptr    += 4;
            }
        }
    }
    else                                     // 16 bits image.
    {
        unsigned short *ptr = (unsigned short *)m_destImage.bits();
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                // Overwrite RGB values to destination.
                ptr[0] = static_cast<unsigned short>(img(x, y, 0));        // Blue
                ptr[1] = static_cast<unsigned short>(img(x, y, 1));        // Green
                ptr[2] = static_cast<unsigned short>(img(x, y, 2));        // Red
                ptr[3] = static_cast<unsigned short>(img(x, y, 3));        // Alpha
                ptr    += 4;
            }
        }
    }
}

}  // namespace Digikam
