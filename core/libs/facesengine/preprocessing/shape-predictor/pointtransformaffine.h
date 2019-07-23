/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : point transform class and its utilities that models
 *               affine transformations between two sets of 2d-points.
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef DIGIKAM_POINT_TRANSFORM_AFFINE_H
#define DIGIKAM_POINT_TRANSFORM_AFFINE_H

// C++ includes

#include <vector>
#include <iostream>

// Local includes

#include "matrixoperations.h"
#include "vectoroperations.h"

// using namespace std;

namespace Digikam
{

class PointTransformAffine
{
public:

    PointTransformAffine();
    PointTransformAffine(const std::vector<std::vector<float> >& m_,
                         const std::vector<float>& b_);
    explicit PointTransformAffine(const std::vector<std::vector<float> >& m_);

    const std::vector<float> operator() (const std::vector<float>& p) const;

    const std::vector<std::vector<float> >& get_m() const;
    const std::vector<float>& get_b() const;

private:

    std::vector<std::vector<float> > m;
    std::vector<float>               b;
};

// ----------------------------------------------------------------------------------------

PointTransformAffine operator* (const PointTransformAffine& lhs,
                                const PointTransformAffine& rhs);

// ----------------------------------------------------------------------------------------

PointTransformAffine inv (const PointTransformAffine& trans);

// ----------------------------------------------------------------------------------------

template <typename T>
PointTransformAffine find_affine_transform(const std::vector<std::vector<T> >& from_points,
                                           const std::vector<std::vector<T> >& to_points)
{
    std::vector<std::vector<float> > P(3, std::vector<float>(from_points.size()));
    std::vector<std::vector<float> > Q(2, std::vector<float>(from_points.size()));

    for (unsigned long i = 0 ; i < from_points.size() ; ++i)
    {
        P[0][i] = from_points[i][0];
        P[1][i] = from_points[i][1];
        P[2][i] = 1;

        Q[0][i] = to_points[i][0];
        Q[1][i] = to_points[i][1];
    }

    const std::vector<std::vector<float> > m = Q * pinv(P);

    return PointTransformAffine(m);
}

// ----------------------------------------------------------------------------------------


PointTransformAffine find_similarity_transform(const std::vector<std::vector<float> >& from_points,
                                               const std::vector<std::vector<float> >& to_points);

}; // namespace Digikam

#endif // DIGIKAM_POINT_TRANSFORM_AFFINE_H
