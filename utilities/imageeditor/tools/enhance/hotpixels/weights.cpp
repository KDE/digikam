/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a class to calculate filter weights
 *
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "weights.h"

// C++ includes

#include <cstring>

// Qt includes

#include <QScopedArrayPointer>

namespace Digikam
{

Weights::Weights() :
    mHeight(0),
    mWidth(0),
    mCoefficientNumber(0),
    mTwoDim(false),
    mPolynomeOrder(0),
    mWeightMatrices(0),
    mPositions(QList<QPoint>())
{
}

Weights::Weights(const Weights& w) :
    mHeight(0),
    mWidth(0),
    mCoefficientNumber(0),
    mTwoDim(false),
    mPolynomeOrder(0),
    mWeightMatrices(0),
    mPositions(QList<QPoint>())
{
    (*this) = w;
}

Weights& Weights::operator=(const Weights& w)
{
    if (this == &w) {
        // we have to be sure that we are not self-assignment
        return *this;
    }

    mHeight            = w.height();
    mWidth             = w.width();
    mPositions         = w.positions();
    mCoefficientNumber = w.coefficientNumber();
    mTwoDim            = w.twoDim();
    mPolynomeOrder     = w.polynomeOrder();

    // Allocate memory and copy weights
    // if the original one was calculated

    if (!w.weightMatrices())
    {
        return *this;
    }
    else
    {
        double** * const origMatrices = w.weightMatrices();
        // Allocate mPositions.count() matrices
        mWeightMatrices         = new double** [mPositions.count()];

        for (int i=0 ; i < mPositions.count() ; ++i)
        {
            // Allocate mHeight rows on each position
            mWeightMatrices[i] = new double*[mHeight];

            for (uint j=0 ; j < mHeight ; ++j)
            {
                // Allocate mWidth columns on each row
                mWeightMatrices[i][j] = new double[mWidth];

                for (uint k=0 ; k < mWidth ; ++k)
                {
                    mWeightMatrices[i][j][k] = origMatrices[i][j][k];
                }
            }
        }
    }

    return *this;
}

void Weights::calculateWeights()
{
    mCoefficientNumber = (mTwoDim ? ((size_t)mPolynomeOrder + 1) * ((size_t)mPolynomeOrder + 1)
                                  :  (size_t)mPolynomeOrder + 1);
    size_t  ix, iy, i, j;
    int     x, y;

    // Determine coordinates of pixels to be sampled

    if (mTwoDim)
    {

        int iPolynomeOrder = (int) mPolynomeOrder; //lets avoid signed/unsigned comparison warnings
        int iHeight        = (int) height();
        int iWidth         = (int) width();

        for (y = -iPolynomeOrder; y < iHeight + iPolynomeOrder; ++y)
        {
            for (x = -iPolynomeOrder; x < iWidth + iPolynomeOrder; ++x)
            {
                if ((x < 0 && y < 0 && -x - y < iPolynomeOrder + 2)                             ||
                    (x < 0 && y >= iHeight && -x + y - iHeight < iPolynomeOrder + 1)            ||
                    (x >= iWidth && y < 0 && x - y - iWidth < iPolynomeOrder + 1)               ||
                    (x >= iWidth && y >= iHeight && x + y - iWidth - iHeight < iPolynomeOrder)  ||
                    (x < 0 && y >= 0 && y < iHeight) || (x >= iWidth  && y >= 0 && y < iHeight) ||
                    (y < 0 && x >= 0 && x < iWidth ) || (y >= iHeight && x >= 0 && x < iWidth))
                {
                    QPoint position(x,y);
                    mPositions.append(position);
                }
            }
        }
    }
    else
    {
        // In the one-dimensional case, only the y coordinate and y size is used.  */

        for (y = (-1)*mPolynomeOrder; y < 0; ++y)
        {
            QPoint position(0,y);
            mPositions.append(position);
        }

        for (y = (int) height(); y < (int) height() + (int) mPolynomeOrder; ++y)
        {
            QPoint position(0,y);
            mPositions.append(position);
        }
    }

    // Allocate memory.

    QScopedArrayPointer<double> matrix (new double[mCoefficientNumber * mCoefficientNumber]);
    QScopedArrayPointer<double> vector0(new double[mPositions.count() * mCoefficientNumber]);
    QScopedArrayPointer<double> vector1(new double[mPositions.count() * mCoefficientNumber]);

    // Calculate coefficient matrix and vectors

    for (iy = 0; iy < mCoefficientNumber; ++iy)
    {
        for (ix = 0; ix < mCoefficientNumber; ++ix)
        {
            matrix [iy* mCoefficientNumber+ix] = 0.0;
        }

        for (j = 0; j < (size_t)mPositions.count(); ++j)
        {
            vector0 [iy * mPositions.count() + j] = polyTerm (iy, mPositions.at(j).x(),
                                                    mPositions.at(j).y(), mPolynomeOrder);

            for (ix = 0; ix < mCoefficientNumber; ++ix)
            {
                matrix [iy* mCoefficientNumber + ix] += (vector0 [iy * mPositions.count() + j]
                                                        * polyTerm (ix, mPositions.at(j).x(), mPositions.at(j).y(), mPolynomeOrder));
            }
        }
    }

    // Invert matrix.

    matrixInv (matrix.data(), mCoefficientNumber);

    // Multiply inverse matrix with vector.

    for (iy = 0; iy < mCoefficientNumber; ++iy)
    {
        for (j = 0; j < (size_t)mPositions.count(); ++j)
        {
            vector1 [iy * mPositions.count() + j] = 0.0;

            for (ix = 0; ix < mCoefficientNumber; ++ix)
            {
                vector1 [iy * mPositions.count() + j] += matrix [iy * mCoefficientNumber + ix]
                        * vector0 [ix * mPositions.count() + j];
            }
        }
    }

    // Store weights

    // Allocate mPositions.count() matrices.
    mWeightMatrices = new double** [mPositions.count()];

    for (i=0 ; i < (size_t)mPositions.count() ; ++i)
    {
        // Allocate mHeight rows on each position
        mWeightMatrices[i] = new double*[mHeight];

        for (j=0 ; j < mHeight ; ++j)
        {
            // Allocate mWidth columns on each row
            mWeightMatrices[i][j] = new double[mWidth];
        }
    }

    for (y = 0; y < (int) mHeight; ++y)
    {
        for (x = 0; x < (int) mWidth; ++x)
        {
            for (j = 0; j < (size_t)mPositions.count(); ++j)
            {
                mWeightMatrices [j][y][x] = 0.0;

                for (iy = 0; iy < mCoefficientNumber; ++iy)
                {
                    mWeightMatrices [j][y][x] += vector1 [iy * mPositions.count() + j]
                                                 * polyTerm (iy, x, y, mPolynomeOrder);
                }

                mWeightMatrices [j][y][x] *= (double) mPositions.count();
            }
        }
    }
}

bool Weights::operator==(const Weights& ws) const
{
    return (mHeight        == ws.height()        &&
            mWidth         == ws.width()         &&
            mPolynomeOrder == ws.polynomeOrder() &&
            mTwoDim        == ws.twoDim());
}

//Invert a quadratic matrix.
void Weights::matrixInv (double* const a, const size_t size)
{
    QScopedArrayPointer<double> b(new double[size * size]);
    size_t ix, iy, j;

    // Copy matrix to new location.

    memcpy (b.data(), a, sizeof (double) * size * size);

    // Set destination matrix to unit matrix.

    for (iy = 0; iy < size; ++iy)
    {
        for (ix = 0; ix < size; ++ix)
        {
            a [iy* size + ix] = ix == iy ? 1.0 : 0.0;
        }
    }

    // Convert matrix to upper triangle form.

    for (iy = 0; iy < size - 1; ++iy)
    {
        for (j = iy + 1; j < size; ++j)
        {
            const double factor = b [j * size + iy] / b [iy * size + iy];

            for (ix = 0; ix < size; ++ix)
            {
                b [j* size + ix] -= factor * b [iy * size + ix];
                a [j* size + ix] -= factor * a [iy * size + ix];
            }
        }
    }

    // Convert matrix to diagonal form.

    for (iy = size - 1; iy > 0; --iy)
    {
        for (j = 0; j < iy; ++j)
        {
            const double factor =  b [j * size + iy] / b [iy * size + iy];

            for (ix = 0; ix < size; ++ix)
            {
                a [j* size + ix] -= factor * a [iy * size + ix];
            }
        }
    }

    // Convert matrix to unit matrix.

    for (iy = 0; iy < size; ++iy)
    {
        for (ix = 0; ix < size; ++ix)
        {
            a [iy* size + ix] /= b [iy * size + iy];
        }
    }
}

// Calculates one term of the polynomial
double Weights::polyTerm (const size_t i_coeff, const int x, const int y, const int poly_order) const
{
    const size_t x_power = i_coeff / ((size_t)poly_order + 1);
    const size_t y_power = i_coeff % ((size_t)poly_order + 1);
    int result;
    size_t i;

    result = 1;

    for (i = 0; i < x_power; ++i)
    {
        result *= x;
    }

    for (i = 0; i < y_power; ++i)
    {
        result *= y;
    }

    return (double)result;
}

}  // namespace Digikam
