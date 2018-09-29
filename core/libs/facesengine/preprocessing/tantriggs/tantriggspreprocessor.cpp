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

#include "tantriggspreprocessor.h"

namespace Digikam
{

TanTriggsPreprocessor::TanTriggsPreprocessor()
    : alpha(0.1f),
      tau(10.0f),
      gamma(0.2f),
      sigma0(1),
      sigma1(2)
{
}

cv::Mat TanTriggsPreprocessor::preprocess(const cv::Mat& inputImage)
{
    return normalize(preprocessRaw(inputImage));
}

cv::Mat TanTriggsPreprocessor::preprocessRaw(const cv::Mat& inputImage)
{
    cv::Mat X = inputImage;

    // ensure it's grayscale
    if (X.channels() > 1)
    {
        cvtColor(X, X, CV_RGB2GRAY);
    }

    // Convert to floating point:
    X.convertTo(X, CV_32FC1);

    // Start preprocessing:
    cv::Mat I;

    // Gamma correction
    cv::pow(X, gamma, I);

    // Calculate the DOG (Difference of Gaussian) Image:
    {
        cv::Mat gaussian0, gaussian1;

        // Kernel Size:
        int kernel_sz0 = (int)(3*sigma0);
        int kernel_sz1 = (int)(3*sigma1);

        // Make them odd for OpenCV:
        kernel_sz0    += ((kernel_sz0 % 2) == 0) ? 1 : 0;
        kernel_sz1    += ((kernel_sz1 % 2) == 0) ? 1 : 0;
        cv::GaussianBlur(I, gaussian0, cv::Size(kernel_sz0,kernel_sz0), sigma0, sigma0, cv::BORDER_CONSTANT);
        cv::GaussianBlur(I, gaussian1, cv::Size(kernel_sz1,kernel_sz1), sigma1, sigma1, cv::BORDER_CONSTANT);
        cv::subtract(gaussian0, gaussian1, I);
    }

    {
        double meanI = 0.0;

        {
            cv::Mat tmp;
            cv::pow(cv::abs(I), alpha, tmp);
            meanI = cv::mean(tmp).val[0];

        }

        I = I / cv::pow(meanI, 1.0/alpha);
    }

    {
        double meanI = 0.0;

        {
            cv::Mat tmp;
            cv::pow(cv::min(cv::abs(I), tau), alpha, tmp);
            meanI = cv::mean(tmp).val[0];
        }

        I = I / cv::pow(meanI, 1.0/alpha);
    }

    // Squash into the tanh:
    {
        for(int r = 0; r < I.rows; r++)
        {
            for(int c = 0; c < I.cols; c++)
            {
                I.at<float>(r,c) = tanh(I.at<float>(r,c) / tau);
            }
        }

        I = tau * I;
    }

    return I;
}

/** Normalizes a given image into a value range between 0 and 255.
 */
cv::Mat TanTriggsPreprocessor::normalize(const cv::Mat& src)
{
    // Create and return normalized image:
    cv::Mat dst;

    switch(src.channels())
    {
        case 1:
            cv::normalize(src, dst, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            break;
        case 3:
            cv::normalize(src, dst, 0, 255, cv::NORM_MINMAX, CV_8UC3);
            break;
        default:
            src.copyTo(dst);
            break;
    }

    return dst;
}

} // namespace Digikam
