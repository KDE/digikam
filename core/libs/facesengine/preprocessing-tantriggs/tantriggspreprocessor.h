/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2012-01-03
 * Description : Calculates the TanTriggs Preprocessing as described in:
 *               Tan, X., and Triggs, B. "Enhanced local texture feature sets for face
 *               recognition under difficult lighting conditions.". IEEE Transactions
 *               on Image Processing 19 (2010), 1635–650.
 *               Default parameters are taken from the paper.
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2012 Philipp Wagner <bytefish at gmx dot de>
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

#ifndef TAN_TRIGGS_PREPROCESSOR_H
#define TAN_TRIGGS_PREPROCESSOR_H

// OpenCV includes

#include "libopencv.h"

namespace Digikam
{

class TanTriggsPreprocessor
{
public:

    TanTriggsPreprocessor();

    /**
     * Performs the Tan Triggs preprocessing to reduce the influence of lightning conditions.
     * Returns a grey-scale 8-bit image.
     */
    cv::Mat preprocess(const cv::Mat& inputImage);

    /**
     * Performs the Tan Triggs preprocessing to reduce the influence of lightning conditions.
     * Expects a one-channel image.
     * Returns a one-channel floating point (CV_32F1) image.
     */
    cv::Mat preprocessRaw(const cv::Mat& inputImage);

    /// Converts CV_32F1 -> CV_8UC1
    cv::Mat normalize(const cv::Mat& preprocessedImage);

public:

    /// Parameters, initialized with the default values from the paper.
    float alpha;
    float tau;
    float gamma;
    float sigma0;
    float sigma1;
};

} // namespace Digikam

#endif // TAN_TRIGGS_PREPROCESSOR_H
