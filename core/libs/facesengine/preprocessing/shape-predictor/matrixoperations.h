/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 17-8-2016
 * Description : Some matrix utility functions including singular value
 *               decomposition, inverse, and pseudo-inverse.
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_MATRIX_OPERATIONS_H
#define DIGIKAM_MATRIX_OPERATIONS_H

// C++ includes

#include <vector>
#include <iostream>

// Local includes

#include "digikam_opencv.h"

namespace Digikam
{

std::vector<std::vector<float> > inv2(const std::vector<std::vector<float> >& mat);

std::vector<std::vector<float> > pinv(const std::vector<std::vector<float> >& mat);

void stdmattocvmat(const std::vector<std::vector<float> >& src, cv::Mat& dst);

void cvmattostdmat(const cv::Mat& dst, std::vector<std::vector<float> >& src);

template <typename T>
inline T signdlib(const T& a, const T& b)
{
    if (b < 0)
    {
        return -std::abs(a);
    }
    else
    {
        return std::abs(a);
    }
}

template <typename T>
inline T pythag(const T& a, const T& b)
{
    T absa = std::abs(a);
    T absb = std::abs(b);

    if (absa > absb)
    {
        T val = absb/absa;
        val  *= val;

        return (absa * std::sqrt(1.0 + val));
    }
    else
    {
        if (absb == 0.0)
        {
            return 0.0;
        }
        else
        {
            T val = absa/absb;
            val  *= val;
            return (absb * std::sqrt(1.0 + val));
        }
    }
}

void transpose(std::vector<std::vector<float> >& src,
               std::vector<std::vector<float> >& dst);

float trace(const std::vector<std::vector<float> >& src);

bool svd3(std::vector<std::vector<float> >& a,
          std::vector<float >& w,
          std::vector<std::vector<float> >& v,
          std::vector<float >& rv1);

void svd(const std::vector<std::vector<float> >& m,
         std::vector<std::vector<float> >& u,
         std::vector<std::vector<float> >& w,
         std::vector<std::vector<float> >& v);

float determinant(const std::vector<std::vector<float> >& u);

} // namespace Digikam

#endif // DIGIKAM_MATRIX_OPERATIONS_H
