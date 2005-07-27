/* ============================================================
 * File  : weights.cpp
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date  : 2005-03-27
 * Description : a class to calculate filter weights
 *
 * Copyright 2005 by Unai Garro
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
 * ============================================================
 * The algorithm for calculating the weights was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
 * ============================================================*/

// Local includes.

#include "weights.h"

namespace DigikamHotPixelsImagesPlugin
{

Weights::Weights(const Weights &w)
{
    (*this) = w;    
}
void Weights::operator=(const Weights &w)
{
    mHeight=w.height();
    mWidth=w.width();
    mPositions=(w.positions());
    mCoefficientNumber=w.coefficientNumber();
    mTwoDim=w.twoDim();
    mPolynomeOrder=w.polynomeOrder();
    
    // Allocate memory and copy weights
    // if the original one was calculated
    
    if (!w.weightMatrices()) return;
    else
    {
        double*** origMatrices=w.weightMatrices();
        mWeightMatrices = new double**[mPositions.count()]; //allocate mPositions.count() matrices
    
        for (uint i=0; i<mPositions.count(); i++)
        {
            mWeightMatrices[i]=new double*[mHeight]; //allocate mHeight rows on each position
            for (int j=0; j<mHeight; j++)
            {
                mWeightMatrices[i][j]=new double[mWidth]; //Allocate mWidth columns on each row
                for (int k=0; k<mWidth; k++) 
                {
                    mWeightMatrices[i][j][k]=origMatrices[i][j][k];
                }
            }
        }
    }
}
    
void Weights::calculateWeights()
{
    mCoefficientNumber = (mTwoDim
                              ? ((size_t)mPolynomeOrder + 1) * ((size_t)mPolynomeOrder + 1)
                              : (size_t)mPolynomeOrder + 1);
    double *matrix;  /* num_coeff * num_coeff */
    double *vector0; /* mPositions.count()   * num_coeff */
    double *vector1; /* mPositions.count()   * num_coeff */
    size_t ix, iy, j;
    int x, y;
    
    // Determine coordinates of pixels to be sampled
    
    if (mTwoDim)
    {
        size_t len_pos = 8;

        for (y = -mPolynomeOrder; y < height() + mPolynomeOrder; ++y)
            for (x = -mPolynomeOrder; x < width() + mPolynomeOrder; ++x)
                if ((x < 0 && y < 0 && -x - y < mPolynomeOrder + 2)
                    || (x < 0 && y >= height() && -x + y - height() < mPolynomeOrder + 1)
                    || (x >= width() && y < 0 && x - y - width() < mPolynomeOrder + 1)
                    || (x >= width() && y >= height() && x + y - width() - height() < mPolynomeOrder)
                    || (x < 0 && y >= 0 && y < height()) || (x >= width() && y >= 0 && y < height())
                    || (y < 0 && x >= 0 && x < width()) || (y >= height() && x >= 0 && x < width()))
                {
            QPoint position(x,y);
            mPositions.append(position);

                }
    }
    else
    {
        // In the one-dimensional case, only the y coordinate and y size is used.  */

        for (y = -mPolynomeOrder; y < 0; ++y)
        {
            QPoint position(0,y);
        mPositions.append(position);
        }

        for (y = height(); y < height() + mPolynomeOrder; ++y)
        {
            QPoint position(0,y);
        mPositions.append(position);
        }
    }

    // Allocate memory.
    
    matrix =  new double[mCoefficientNumber*mCoefficientNumber];
    vector0 = new double[mPositions.count() * mCoefficientNumber];
    vector1 = new double[mPositions.count() * mCoefficientNumber];
    
    // Calculate coefficient matrix and vectors
    
    for (int iy = 0; iy < mCoefficientNumber; ++iy)
    {
        for (int ix = 0; ix < mCoefficientNumber; ++ix)
            matrix [iy*mCoefficientNumber+ix] = 0.0;

        for (j = 0; j < mPositions.count(); ++j)
        {
            vector0 [iy * mPositions.count() + j] = polyTerm (iy, mPositions [j].x(), mPositions [j].y(), mPolynomeOrder);

            for (int ix = 0; ix < mCoefficientNumber; ++ix)
                matrix [iy * mCoefficientNumber + ix] += (vector0 [iy * mPositions.count() + j]
                                                 * polyTerm (ix, mPositions [j].x(), mPositions[j].y(), mPolynomeOrder));
        }
    }

    // Invert matrix.
    
    matrixInv (matrix, mCoefficientNumber);

    // Multiply inverse matrix with vector.
    
    for (iy = 0; iy < mCoefficientNumber; ++iy)
        for (j = 0; j < mPositions.count(); ++j)
        {
            vector1 [iy * mPositions.count() + j] = 0.0;

            for (ix = 0; ix < mCoefficientNumber; ++ix)
                vector1 [iy * mPositions.count() + j] += matrix [iy * mCoefficientNumber + ix] * vector0 [ix * mPositions.count() + j];
        }

    // Store weights
    
    mWeightMatrices = new double**[mPositions.count()]; //allocate mPositions.count() matrices
    
    for (int i=0; i<mPositions.count(); i++)
    {
        mWeightMatrices[i]=new double*[mHeight]; //allocate mHeight rows on each position
        for (int j=0; j<mHeight; j++) mWeightMatrices[i][j]=new double[mWidth]; //Allocate mWidth columns on each row
    }

    for (y = 0; y < mHeight; ++y)
        for (x = 0; x < mWidth; ++x)
        {

            for (j = 0; j < mPositions.count(); ++j)
            {
                mWeightMatrices [j][y][x] = 0.0;

                for (iy = 0; iy < mCoefficientNumber; ++iy)
                   mWeightMatrices [j][y][x] += vector1 [iy * mPositions.count() + j] * polyTerm (iy, x, y, mPolynomeOrder);

                mWeightMatrices [j][y][x] *= (double) mPositions.count();
            }
        }
    
    delete[] vector1;
    delete[] vector0;
    delete[] matrix;
}

bool Weights::operator==(const Weights& ws) const
{
    return (mHeight==ws.height() &&
        mWidth==ws.width() &&
        mPolynomeOrder==ws.polynomeOrder() &&
        mTwoDim==ws.twoDim()
        );
}

 //Invert a quadratic matrix. 
void Weights::matrixInv (double *const a, const size_t size)
{
    double *const b = new double[size * size];
    size_t ix, iy, j;

    // Copy matrix to new location.  
    
    memcpy (b, a, sizeof (double) * size * size);

    // Set destination matrix to unit matrix. 
    
    for (iy = 0; iy < size; ++iy)
        for (ix = 0; ix < size; ++ix)
            a [iy * size + ix] = ix == iy ? 1.0 : 0.0;

    // Convert matrix to upper triangle form.  
    
    for (iy = 0; iy < size - 1; ++iy)
        for (j = iy + 1; j < size; ++j)
        {
            const double factor = b [j * size + iy] / b [iy * size + iy];

            for (ix = 0; ix < size; ++ix)
            {
                b [j * size + ix] -= factor * b [iy * size + ix];
                a [j * size + ix] -= factor * a [iy * size + ix];
            }
        }

    // Convert matrix to diagonal form.  
    
    for (iy = size - 1; iy > 0; --iy)
        for (j = 0; j < iy; ++j)
        {
            const double factor =  b [j * size + iy] / b [iy * size + iy];

            for (ix = 0; ix < size; ++ix)
                a [j * size + ix] -= factor * a [iy * size + ix];
        }

    // Convert matrix to unit matrix.
    
    for (iy = 0; iy < size; ++iy)
        for (ix = 0; ix < size; ++ix)
            a [iy * size + ix] /= b [iy * size + iy];

    delete[] b;
}

// Calculates one term of the polynomial
double Weights::polyTerm (const size_t i_coeff, const int x, const int y, const int poly_order)
{
    const size_t x_power = i_coeff / ((size_t)poly_order + 1);
    const size_t y_power = i_coeff % ((size_t)poly_order + 1);
    int result;
    size_t i;

    result = 1;

    for (i = 0; i < x_power; ++i)
        result *= x;
    for (i = 0; i < y_power; ++i)
        result *= y;

    return (double)result;
}

}  // NameSpace DigikamHotPixelsImagesPlugin

