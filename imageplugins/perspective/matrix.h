/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-02-17
 * Description : a matrix implementation for image
 *               perspective adjustment.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

#ifndef IMAGEEFFECT_PERSPECTIVE_MATRIX_H
#define IMAGEEFFECT_PERSPECTIVE_MATRIX_H

namespace DigikamPerspectiveImagesPlugin
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
     * @x: Translation in X direction.
     * @y: Translation in Y direction.
     *
     * Translates the matrix by x and y.
     */
    void   translate(double x, double y);

    /**
    * scale:
    * @x: X scale factor.
    * @y: Y scale factor.
    *
    * Scales the matrix by x and y
    */
    void   scale(double x, double y);

    /**
    * invert:
    *
    * Inverts this matrix.
    */
    void   invert();

    /**
    * multiply:
    * @matrix: The other input matrix.
    *
    * Multiplies this matrix with another matrix
    */
    void   multiply(const Matrix &matrix1);

    /**
    * transformPoint:
    * @x: The source X coordinate.
    * @y: The source Y coordinate.
    * @newx: The transformed X coordinate.
    * @newy: The transformed Y coordinate.
    *
    * Transforms a point in 2D as specified by the transformation matrix.
    */
    void   transformPoint(double x, double y, double *newx, double *newy) const;

    /**
    * determinant:
    *
    * Calculates the determinant of this matrix.
    *
    * Returns: The determinant.
    */
    double determinant() const;

    /**
    * coeff:
    *
    * The 3x3 matrix data
    */
    double coeff[3][3];
};

}  // namespace DigikamPerspectiveImagesPlugin

#endif // IMAGEEFFECT_PERSPECTIVE_MATRIX_H
