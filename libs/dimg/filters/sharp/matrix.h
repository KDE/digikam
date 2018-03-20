/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-29
 * Description : refocus deconvolution matrix implementation.
 *
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

#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

// C ++ includes

#ifndef _MSC_VER
#include <cstdio>
#endif

namespace Digikam
{

/**
* CMat:
*
* Centered matrix. This is a square matrix where
* the indices range from [-radius, radius].
* The matrix contains (2 * radius + 1) ** 2 elements.
*
**/
typedef struct
{
    /**
     * Radius of the matrix.
     */
    int     radius;
    /**
     * Size of one row = 2 * radius + 1
     */
    int     row_stride;
    /**
     * Contents of matrix
     */
    double* data;
    /**
     * Points to element with index (0, 0)
     */
    double* center;
}
CMat;

// ---------------------------------------------------------------------------------------------

/**
* Mat:
*
* Normal matrix type. Indices range from
* [0, rows -1 ] and [0, cols - 1].
*
**/
typedef struct
{
    /**
     * Number of rows in the matrix.
     */
    int     rows;
    /**
     * Number of columns in the matrix
     */
    int     cols;
    /**
     * Content of the matrix
     */
    double* data;
}
Mat;

// ---------------------------------------------------------------------------------------------

class RefocusMatrix
{

public:

    static void fill_matrix(CMat* const matrix, const int m, double f(const int, const int, const double), const double fun_arg);

    static void fill_matrix2(CMat* const matrix, const int m,
                             double f(const int, const int, const double, const double),
                             const double fun_arg1, const double fun_arg2);

    static void make_circle_convolution(const double radius, CMat* const convolution, const int m);

    static void make_gaussian_convolution(const double alpha, CMat* const convolution, const int m);

    static void convolve_star_mat(CMat* const result, const CMat* const mata, const CMat* const matb);

    static CMat* compute_g_matrix(const CMat* const convolution, const int m,
                                  const double gamma, const double noise_factor,
                                  const double musq, const bool symmetric);

    static void finish_matrix(Mat* const mat);
    static void finish_and_free_matrix(Mat* const mat);
    static void init_c_mat(CMat* const mat, const int radius);
    static void finish_c_mat(CMat* const mat);

    static double mat_elt(const Mat* const mat, const int r, const int c);
    static inline double c_mat_elt(const CMat* const mat, const int col, const int row);

private:

    static Mat*  allocate_matrix(int nrows, int ncols);
    static CMat* allocate_c_mat(const int radius);

    static double* mat_eltptr(Mat* const mat, const int r, const int c);

    static inline double* c_mat_eltptr(CMat* const mat, const int col, const int row);

    static void convolve_mat(CMat* const result, const CMat* const mata, const CMat* const matb);

    static int as_idx(const int k, const int l, const int m);
    static int as_cidx(const int k, const int l);

    static Mat* make_s_matrix(CMat* const mat, int m, double noise_factor);
    static Mat* make_s_cmatrix(CMat* const mat, int m, double noise_factor);

    static double correlation(const int x, const int y, const double gamma, const double musq);

    static Mat*  copy_vec(const CMat* const mat, const int m);
    static Mat*  copy_cvec(const CMat* const mat, const int m);
    static CMat* copy_cvec2mat(const Mat* const cvec, const int m);
    static CMat* copy_vec2mat(const Mat* const cvec, const int m);
    static CMat* compute_g(const CMat* const convolution, const int m, const double gamma,
                           const double noise_factor, const double musq, const bool symmetric);

    static double circle_integral(const double x, const double radius);
    static double circle_intensity(const int x, const int y, const double radius);
};

}  // namespace Digikam

#endif /* MATRIX_H_INCLUDED */
