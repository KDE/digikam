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
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See http://www.opensource.org/licenses/bsd-license
 *
 * ============================================================ */

#ifndef TANTRIGGSPREPROCESSOR_H
#define TANTRIGGSPREPROCESSOR_H

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

#endif // TANTRIGGSPREPROCESSOR_H
