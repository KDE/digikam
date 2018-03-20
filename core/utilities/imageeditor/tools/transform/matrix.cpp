/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a matrix implementation for image
 *               perspective adjustment.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Matrix3 implementation inspired from gimp 2.0
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include "matrix.h"

// C++ includes

#include <cstring>

namespace Digikam
{

static double identityMatrix[3][3] = { { 1.0, 0.0, 0.0 },
                                       { 0.0, 1.0, 0.0 },
                                       { 0.0, 0.0, 1.0 }
                                     };

Matrix::Matrix()
{
    memcpy(coeff, identityMatrix, sizeof(coeff));
}

void Matrix::translate (double x, double y)
{
    double g, h, i;

    g = coeff[2][0];
    h = coeff[2][1];
    i = coeff[2][2];

    coeff[0][0] += x * g;
    coeff[0][1] += x * h;
    coeff[0][2] += x * i;
    coeff[1][0] += y * g;
    coeff[1][1] += y * h;
    coeff[1][2] += y * i;
}

void Matrix::scale(double x, double y)
{
    coeff[0][0] *= x;
    coeff[0][1] *= x;
    coeff[0][2] *= x;

    coeff[1][0] *= y;
    coeff[1][1] *= y;
    coeff[1][2] *= y;
}

void Matrix::multiply(const Matrix& matrix)
{
    Matrix tmp;
    double t1, t2, t3;

    for (int i = 0; i < 3; ++i)
    {
        t1 = matrix.coeff[i][0];
        t2 = matrix.coeff[i][1];
        t3 = matrix.coeff[i][2];

        for (int j = 0; j < 3; ++j)
        {
            tmp.coeff[i][j]  = t1 * coeff[0][j];
            tmp.coeff[i][j] += t2 * coeff[1][j];
            tmp.coeff[i][j] += t3 * coeff[2][j];
        }
    }

    *this = tmp;
}

void Matrix::transformPoint(double x, double y, double* newx, double* newy) const
{
    double  w = coeff[2][0] * x + coeff[2][1] * y + coeff[2][2];

    if (w == 0.0)
    {
        w = 1.0;
    }
    else
    {
        w = 1.0/w;
    }

    *newx = (coeff[0][0]  * x +
             coeff[0][1]  * y +
             coeff[0][2]) * w;
    *newy = (coeff[1][0]  * x +
             coeff[1][1]  * y +
             coeff[1][2]) * w;
}

void Matrix::invert()
{
    Matrix  inv;
    double  det = determinant();

    if (det == 0.0)
    {
        return;
    }

    det = 1.0 / det;

    inv.coeff[0][0] =   (coeff[1][1] * coeff[2][2] -
                         coeff[1][2] * coeff[2][1]) * det;

    inv.coeff[1][0] = - (coeff[1][0] * coeff[2][2] -
                         coeff[1][2] * coeff[2][0]) * det;

    inv.coeff[2][0] =   (coeff[1][0] * coeff[2][1] -
                         coeff[1][1] * coeff[2][0]) * det;

    inv.coeff[0][1] = - (coeff[0][1] * coeff[2][2] -
                         coeff[0][2] * coeff[2][1]) * det;

    inv.coeff[1][1] =   (coeff[0][0] * coeff[2][2] -
                         coeff[0][2] * coeff[2][0]) * det;

    inv.coeff[2][1] = - (coeff[0][0] * coeff[2][1] -
                         coeff[0][1] * coeff[2][0]) * det;

    inv.coeff[0][2] =   (coeff[0][1] * coeff[1][2] -
                         coeff[0][2] * coeff[1][1]) * det;

    inv.coeff[1][2] = - (coeff[0][0] * coeff[1][2] -
                         coeff[0][2] * coeff[1][0]) * det;

    inv.coeff[2][2] =   (coeff[0][0] * coeff[1][1] -
                         coeff[0][1] * coeff[1][0]) * det;

    *this = inv;
}

double Matrix::determinant() const
{
    double determinant;

    determinant  = (coeff[0][0]  *
                    (coeff[1][1] * coeff[2][2] -
                     coeff[1][2] * coeff[2][1]));
    determinant -= (coeff[1][0]  *
                    (coeff[0][1] * coeff[2][2] -
                     coeff[0][2] * coeff[2][1]));
    determinant += (coeff[2][0]  *
                    (coeff[0][1] * coeff[1][2] -
                     coeff[0][2] * coeff[1][1]));

    return determinant;
}

}  // namespace Digikam
