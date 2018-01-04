/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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

#ifndef POINT_TRANSFORM_AFFINE_H
#define POINT_TRANSFORM_AFFINE_H

// C++ includes

#include <vector>
#include <iostream>

// Local includes

#include "matrixoperations.h"
#include "vectoroperations.h"

using namespace std;

namespace Digikam
{

class PointTransformAffine
{
public:

    PointTransformAffine()
    {
        m       = std::vector<std::vector<float> >(2, std::vector<float>(2, 0));
        m[0][0] = 1.0;
        m[1][1] = 1.0;
        b       = std::vector<float>(2, 0);
    }

    PointTransformAffine(const std::vector<std::vector<float> >& m_,
                         const std::vector<float>& b_)
      : m(m_),
        b(b_)
    {
    }

    PointTransformAffine(const std::vector<std::vector<float> >& m_)
    {
        m = std::vector<std::vector<float> >(2, std::vector<float>(2, 0));
        b = std::vector<float >(2, 0);

        for (unsigned int i = 0 ; i < m_.size() ; i++)
        {
            for (unsigned int j = 0 ; j < m_[0].size() ; j++)
            {
                if (j == 2)
                {
                    b[i]    = m_[i][2];
                }
                else
                {
                    m[i][j] = m_[i][j];
                }
            }
        }
    }

    const std::vector<float> operator() (const std::vector<float>& p) const
    {
        return m*p + b;
    }


    const std::vector<std::vector<float> >& get_m() const
    {
        return m;
    }

    const std::vector<float>& get_b() const
    {
        return b;
    }

private:

    std::vector<std::vector<float> > m;
    std::vector<float>               b;
};

// ----------------------------------------------------------------------------------------

inline PointTransformAffine operator* (const PointTransformAffine& lhs,
                                       const PointTransformAffine& rhs)
{
    return PointTransformAffine(lhs.get_m() * rhs.get_m(), lhs.get_m() * rhs.get_b() + lhs.get_b());
}

// ----------------------------------------------------------------------------------------

inline PointTransformAffine inv (const PointTransformAffine& trans)
{
    std::vector<std::vector<float> > im = inv2(trans.get_m());

    return PointTransformAffine(im, -(im * trans.get_b()));
}

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
                                               const std::vector<std::vector<float> >& to_points)
{
    // We use the formulas from the paper: Least-squares estimation of transformation
    // parameters between two point patterns by Umeyama.  They are equations 34 through
    // 43.

    std::vector<float> mean_from(2, 0), mean_to(2, 0);
    float sigma_from = 0;
    float sigma_to   = 0;
    std::vector<std::vector<float> > cov(2, std::vector<float>(2, 0));

    for (unsigned long i = 0 ; i < from_points.size() ; ++i)
    {
        mean_from = mean_from + from_points[i];
        mean_to   = mean_to   + to_points[i];
    }

    mean_from = mean_from / from_points.size();
    mean_to   = mean_to   / from_points.size();

    for (unsigned long i = 0 ; i < from_points.size() ; ++i)
    {
        sigma_from = sigma_from + length_squared(from_points[i] - mean_from);
        sigma_to   = sigma_to   + length_squared(to_points[i]   - mean_to);
        cov        = cov + (to_points[i] - mean_to)*(from_points[i] - mean_from);
    }

    sigma_from = sigma_from / from_points.size();
    sigma_to   = sigma_to   / from_points.size();
    cov        = cov        / from_points.size();

    std::vector<std::vector<float> >  u(2,std::vector<float>(2));
    std::vector<std::vector<float> >  v(2,std::vector<float>(2));
    std::vector<std::vector<float> > vt(2,std::vector<float>(2));
    std::vector<std::vector<float> >  d(2,std::vector<float>(2));
    std::vector<std::vector<float> >  s(2,std::vector<float>(2,0));

    svd(cov, u,d,vt);
    s[0][0] = 1;
    s[1][1] = 1;

    if (determinant(cov) < 0 ||
        (determinant(cov) == 0 && determinant(u) * determinant(v) < 0))
    {
        if (d[1][1] < d[0][0])
            s[1][1] = -1;
        else
            s[0][0] = -1;
    }

    transpose(vt,v);
    std::vector<std::vector<float> >  r = u * s * v;
    float c = 1;

    if (sigma_from != 0)
    {
        c = 1.0 / sigma_from * trace(d * s);
    }

    std::vector<float> t = mean_to - r * mean_from * c;

    return PointTransformAffine(r * c, t);
}

}  // namespace Digikam

#endif // POINT_TRANSFORM_AFFINE_H
