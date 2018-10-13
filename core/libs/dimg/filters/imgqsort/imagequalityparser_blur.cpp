/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser - blur detection
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 *
 * References  : https://stackoverflow.com/questions/7765810/is-there-a-way-to-detect-if-an-image-is-blurry
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

#include "imagequalityparser_p.h"

namespace Digikam
{

void ImageQualityParser::cannyThreshold(int, void*) const
{
    // Reduce noise with a kernel 3x3.
    blur(d->src_gray, d->detected_edges, Size(3, 3));

    // Canny detector.
    Canny(d->detected_edges,
          d->detected_edges,
          d->lowThreshold,
          d->lowThreshold * d->ratio,
          d->kernel_size);
}

double ImageQualityParser::blurDetector() const
{
    d->lowThreshold   = 0.4;
    d->ratio          = 3;
    double maxval     = 0.0;
    cannyThreshold(0, 0);

    double average    = mean(d->detected_edges)[0];
    int* const maxIdx = new int[sizeof(d->detected_edges)];
    minMaxIdx(d->detected_edges, 0, &maxval, 0, maxIdx);

    double blurresult = average / maxval;

    qCDebug(DIGIKAM_DIMG_LOG) << "The average of the edge intensity is " << average;
    qCDebug(DIGIKAM_DIMG_LOG) << "The maximum of the edge intensity is " << maxval;
    qCDebug(DIGIKAM_DIMG_LOG) << "The result of the edge intensity is  " << blurresult;

    delete [] maxIdx;

    return blurresult;
}

short ImageQualityParser::blurDetector2() const
{
    // Algorithm using Laplacian of Gaussian Filter to detect blur.
    Mat out;
    Mat noise_free;
    qCDebug(DIGIKAM_DIMG_LOG) << "Algorithm using LoG Filter started";

    // To remove noise from the image.
    GaussianBlur(d->src_gray, noise_free, Size(3, 3), 0, 0, BORDER_DEFAULT);

    // Aperture size of 1 corresponds to the correct matrix.
    int kernel_size = 3;
    int scale       = 1;
    int delta       = 0;
    int ddepth      = CV_16S;

    Laplacian(noise_free, out, ddepth, kernel_size, scale, delta, BORDER_DEFAULT);

    // noise_free:  The input image without noise.
    // out:         Destination (output) image.
    // ddepth:      Depth of the destination image. Since our input is CV_8U we define ddepth = CV_16S to avoid overflow.
    // kernel_size: The kernel size of the Sobel operator to be applied internally. We use 3 here.

    short maxLap = -32767;

    for (int i = 0 ; i < out.rows ; ++i)
    {
        for (int j = 0 ; j < out.cols ; ++j)
        {
            short value = out.at<short>(i, j);

            if (value > maxLap)
            {
                maxLap = value;
            }
        }
    }

    return maxLap;
}

} // namespace Digikam
