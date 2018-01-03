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

#ifndef IMAGEEFFECT_PERSPECTIVE_MATRIX_H
#define IMAGEEFFECT_PERSPECTIVE_MATRIX_H

namespace Digikam
{

class Matrix
{
public:

    /**
     * Matrix:
     *
     * Initializes matrix to the identity matrix.
     */
    Matrix();

    /**
     * translate:
     * @param x Translation in X direction.
     * @param y Translation in Y direction.
     *
     * Translates the matrix by x and y.
     */
    void translate(double x, double y);

    /**
     * scale:
     * @param x X scale factor.
     * @param y Y scale factor.
     *
     * Scales the matrix by x and y
     */
    void scale(double x, double y);

    /**
     * invert:
     *
     * Inverts this matrix.
     */
    void invert();

    /**
     * multiply:
     * @param matrix1 The other input matrix.
     *
     * Multiplies this matrix with another matrix
     */
    void multiply(const Matrix& matrix1);

    /**
     * transformPoint:
     * @param x The source X coordinate.
     * @param y The source Y coordinate.
     * @param newx The transformed X coordinate.
     * @param newy The transformed Y coordinate.
     *
     * Transforms a point in 2D as specified by the transformation matrix.
     */
    void transformPoint(double x, double y, double* newx, double* newy) const;

    /**
     * determinant:
     *
     * Calculates the determinant of this matrix.
     *
     * Returns: The determinant.
     */
    double determinant() const;

public:

     /**
     * coeff:
     *
     * The 3x3 matrix data
     */
    double coeff[3][3];
};

}  // namespace Digikam

#endif // IMAGEEFFECT_PERSPECTIVE_MATRIX_H
