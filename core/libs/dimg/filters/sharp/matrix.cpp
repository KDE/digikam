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
 * Original implementation from Refocus Gimp plug-in
 * Copyright (C) 1999-2003 Ernst Lippe
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

// Uncomment this line to debug matrix computation.
//#define RF_DEBUG 1

// Square
#define SQR(x) ((x) * (x))

#include "matrix.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QString>

// Eigen includes

// Pragma directives to reduce warnings from Eigen header files.
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <Eigen/Eigen>
#include <Eigen/LU>

// Restore warnings
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

#ifdef RF_DEBUG

// Debug methods.
void print_refocus_c_mat(const CMat* const mat)
{
   int x, y;

    for (y = -mat->radius; y <= mat->radius; ++y)
    {
        QString output, num;

        for (x = -mat->radius; x <= mat->radius; ++x)
        {
            output.append(num.setNum(RefocusMatrix::c_mat_elt(mat, x, y)));
        }

        qCDebug(DIGIKAM_DIMG_LOG) << output;
    }
}

void print_refocus_matrix(Mat* const matrix)
{
    int col_idx, row_idx;

    for (row_idx = 0; row_idx < matrix->rows; ++row_idx)
    {
        QString output, num;

        for (col_idx = 0; col_idx < matrix->cols; ++col_idx)
        {
            output.append(num.setNum(RefocusMatrix::mat_elt(matrix, row_idx, col_idx)));
        }

        qCDebug(DIGIKAM_DIMG_LOG) << output;
    }
}
#endif // RF_DEBUG

// ---------------------------------------------------------------------------

Mat* RefocusMatrix::allocate_matrix(int nrows, int ncols)
{
    Mat* const result = new Mat;
    memset(result, 0, sizeof(Mat));

    result->cols = ncols;
    result->rows = nrows;
    result->data = new double [nrows * ncols];
    memset(result->data, 0, nrows * ncols * sizeof(double));

    return (result);
}

void RefocusMatrix::finish_matrix(Mat* const mat)
{
    delete [] mat->data;
}

void RefocusMatrix::finish_and_free_matrix(Mat* const mat)
{
    delete [] mat->data;
    delete mat;
}

double* RefocusMatrix::mat_eltptr(Mat* const mat, const int r, const int c)
{
    Q_ASSERT((r >= 0) && (r < mat->rows));
    Q_ASSERT((c >= 0) && (c < mat->rows));

    return (&(mat->data[mat->rows * c + r]));
}

double RefocusMatrix::mat_elt(const Mat* const mat, const int r, const int c)
{
    Q_ASSERT((r >= 0) && (r < mat->rows));
    Q_ASSERT((c >= 0) && (c < mat->rows));

    return (mat->data[mat->rows * c + r]);
}

void RefocusMatrix::init_c_mat(CMat* const mat, const int radius)
{
    mat->radius     = radius;
    mat->row_stride = 2 * radius + 1;
    mat->data       = new double [SQR(mat->row_stride)];
    memset(mat->data, 0, SQR(mat->row_stride) * sizeof(double));
    mat->center     = mat->data + mat->row_stride * mat->radius + mat->radius;
}

CMat* RefocusMatrix::allocate_c_mat(const int radius)
{
    CMat* const result = new CMat;
    memset(result, 0, sizeof(CMat));
    init_c_mat(result, radius);
    return (result);
}

void RefocusMatrix::finish_c_mat(CMat* const mat)
{
    delete [] mat->data;
    mat->data = NULL;
}

inline double* RefocusMatrix::c_mat_eltptr(CMat* const mat, const int col, const int row)
{
    Q_ASSERT((qAbs(row) <= mat->radius) && (qAbs(col) <= mat->radius));

    return (mat->center + mat->row_stride * row + col);
}

inline double RefocusMatrix::c_mat_elt(const CMat* const mat, const int col, const int row)
{
    Q_ASSERT((qAbs(row) <= mat->radius) && (qAbs(col) <= mat->radius));

    return (mat->center[mat->row_stride * row + col]);
}

void RefocusMatrix::convolve_mat(CMat* const result, const CMat* const mata, const CMat* const matb)
{
   int xr, yr, xa, ya;

    for (yr = -result->radius; yr <= result->radius; ++yr)
    {
        for (xr = -result->radius; xr <= result->radius; ++xr)
        {
            const int ya_low  = qMax(-mata->radius, yr - matb->radius);
            const int ya_high = qMin(mata->radius, yr + matb->radius);
            const int xa_low  = qMax(-mata->radius, xr - matb->radius);
            const int xa_high = qMin(mata->radius, xr + matb->radius);
           double val = 0.0;

            for (ya = ya_low; ya <= ya_high; ++ya)
            {
                for (xa = xa_low; xa <= xa_high; ++xa)
                {
                    val += c_mat_elt(mata, xa, ya) *
                           c_mat_elt(matb, xr - xa, yr - ya);
                }
            }

            *c_mat_eltptr(result, xr, yr) = val;
        }
    }
}

void RefocusMatrix::convolve_star_mat(CMat* const result, const CMat* const mata, const CMat* const matb)
{
   int xr, yr, xa, ya;

    for (yr = -result->radius; yr <= result->radius; ++yr)
    {
        for (xr = -result->radius; xr <= result->radius; ++xr)
        {
            const int ya_low    = qMax(-mata->radius, -matb->radius - yr);
            const int ya_high   = qMin(mata->radius, matb->radius - yr);
            const int xa_low    = qMax(-mata->radius, -matb->radius - xr);
            const int xa_high   = qMin(mata->radius, matb->radius - xr);
           double val = 0.0;

            for (ya = ya_low; ya <= ya_high; ++ya)
            {
                for (xa = xa_low; xa <= xa_high; ++xa)
                {
                    val += c_mat_elt(mata, xa, ya) * c_mat_elt(matb, xr + xa, yr + ya);
                }
            }

            *c_mat_eltptr(result, xr, yr) = val;
        }
    }
}

int RefocusMatrix::as_idx(const int k, const int l, const int m)
{
    return ((k + m) * (2 * m + 1) + (l + m));
}

int RefocusMatrix::as_cidx(const int k, const int l)
{
    const int a = qMax(qAbs(k), qAbs(l));
    const int b = qMin(qAbs(k), qAbs(l));

    return ((a * (a + 1)) / 2 + b);
}

Mat* RefocusMatrix::make_s_matrix(CMat* const mat, int m, double noise_factor)
{
    const int mat_size = SQR(2 * m + 1);
    Mat* const result  = allocate_matrix(mat_size, mat_size);
   int yr, yc, xr, xc;

    for (yr = -m; yr <= m; ++yr)
    {
        for (xr = -m; xr <= m; ++xr)
        {
            for (yc = -m; yc <= m; ++yc)
            {
                for (xc = -m; xc <= m; ++xc)
                {
                    *mat_eltptr(result, as_idx(xr, yr, m), as_idx(xc, yc, m)) = c_mat_elt(mat, xr - xc, yr - yc);

                    if ((xr == xc) && (yr == yc))
                    {
                        *mat_eltptr(result, as_idx(xr, yr, m), as_idx(xc, yc, m)) += noise_factor;
                    }
                }
            }
        }
    }

    return (result);
}

Mat* RefocusMatrix::make_s_cmatrix(CMat* const mat, int m, double noise_factor)
{
    const int mat_size = as_cidx(m + 1, 0);
    Mat* const result  = allocate_matrix(mat_size, mat_size);
   int yr, yc, xr, xc;

    for (yr = 0; yr <= m; ++yr)
    {
        for (xr = 0; xr <= yr; ++xr)
        {
            for (yc = -m; yc <= m; ++yc)
            {
                for (xc = -m; xc <= m; ++xc)
                {
                    *mat_eltptr(result, as_cidx(xr, yr), as_cidx(xc, yc)) += c_mat_elt(mat, xr - xc, yr - yc);

                    if ((xr == xc) && (yr == yc))
                    {
                        *mat_eltptr(result, as_cidx(xr, yr), as_cidx(xc, yc)) += noise_factor;
                    }
                }
            }
        }
    }

    return (result);
}

double RefocusMatrix::correlation(const int x, const int y, const double gamma, const double musq)
{
    return (musq + pow((double)gamma, (double)sqrt(SQR(x) + SQR(y))));
}

Mat* RefocusMatrix::copy_vec(const CMat* const mat, const int m)
{
    Mat* const result        = allocate_matrix(SQR(2 * m + 1), 1);
   int x, y, index = 0;

    for (y = -m; y <= m; ++y)
    {
        for (x = -m; x <= m; ++x)
        {
            *mat_eltptr(result, index, 0) = c_mat_elt(mat, x, y);
            ++index;
        }
    }

    Q_ASSERT(index == SQR(2 * m + 1));

    return (result);
}

Mat* RefocusMatrix::copy_cvec(const CMat* const mat, const int m)
{
    Mat* const result        = allocate_matrix(as_cidx(m + 1, 0), 1);
   int x, y, index = 0;

    for (y = 0; y <= m; ++y)
    {
        for (x = 0; x <= y; ++x)
        {
            *mat_eltptr(result, index, 0) = c_mat_elt(mat, x, y);
            ++index;
        }
    }

    Q_ASSERT(index == as_cidx(m + 1, 0));

    return (result);
}

CMat* RefocusMatrix::copy_cvec2mat(const Mat* const cvec, const int m)
{
    CMat* const result = allocate_c_mat(m);
   int x, y;

    for (y = -m; y <= m; ++y)
    {
        for (x = -m; x <= m; ++x)
        {
            *c_mat_eltptr(result, x, y) = mat_elt(cvec, as_cidx(x, y), 0);
        }
    }

    return (result);
}

CMat* RefocusMatrix::copy_vec2mat(const Mat* const cvec, const int m)
{
    CMat* const result = allocate_c_mat(m);
   int x, y;

    for (y = -m; y <= m; ++y)
    {
        for (x = -m; x <= m; ++x)
        {
            *c_mat_eltptr(result, x, y) = mat_elt(cvec, as_idx(x, y, m), 0);
        }
    }

    return (result);
}

CMat* RefocusMatrix::compute_g(const CMat* const convolution, const int m, const double gamma,
                               const double noise_factor, const double musq, const bool symmetric)
{
    CMat h_conv_ruv, a, corr;
    CMat* result = 0;
    Mat* b       = 0;
    Mat* s       = 0;
//    int status;

    init_c_mat(&h_conv_ruv, 3 * m);
    fill_matrix2(&corr, 4 * m, correlation, gamma, musq);
    convolve_mat(&h_conv_ruv, convolution, &corr);
    init_c_mat(&a, 2 * m);
    convolve_star_mat(&a, convolution, &h_conv_ruv);

    if (symmetric)
    {
        s = make_s_cmatrix(&a, m, noise_factor);
        b = copy_cvec(&h_conv_ruv, m);
    }
    else
    {
        s = make_s_matrix(&a, m, noise_factor);
        b = copy_vec(&h_conv_ruv, m);
    }

#ifdef RF_DEBUG
    qCDebug(DIGIKAM_DIMG_LOG) << "Convolution:";
    print_refocus_c_mat(convolution);
    qCDebug(DIGIKAM_DIMG_LOG) << "h_conv_ruv:";
    print_refocus_c_mat(&h_conv_ruv);
    qCDebug(DIGIKAM_DIMG_LOG) << "Value of s:";
    print_refocus_matrix(s);
#endif // RF_DEBUG

    Q_ASSERT(s->cols == s->rows);
    Q_ASSERT(s->rows == b->rows);

    //    status =
    //  dgesv(s->rows, 1, s->data, s->rows, b->data, b->rows);

    // Solve Ax=B using Eigen:

    // Those are mappings, ie 'eigen' views of the data, there is NO
    // copy involved. Digikam and Eigen both default to column-major.
    Eigen::Map<Eigen::MatrixXd> A(s->data,s->rows, s->rows);
    Eigen::Map<Eigen::VectorXd> B(b->data,b->rows);
    // there's no "solveinplace" for lu in Eigen, use a temporary matrix
    Eigen::VectorXd X(b->rows);
    X = A.lu().solve(B);
    B = X; // maps are writeable, too.

    if (symmetric)
    {
        result = copy_cvec2mat(b, m);
    }
    else
    {
        result = copy_vec2mat(b, m);
    }

#ifdef RF_DEBUG
    qCDebug(DIGIKAM_DIMG_LOG) << "Deconvolution:";
    print_refocus_c_mat(result);
#endif // RF_DEBUG

    finish_c_mat(&a);
    finish_c_mat(&h_conv_ruv);
    finish_c_mat(&corr);
    finish_and_free_matrix(s);
    finish_and_free_matrix(b);
    return (result);
}

CMat* RefocusMatrix::compute_g_matrix(const CMat* const convolution, const int m,
                                      const double gamma, const double noise_factor,
                                      const double musq, const bool symmetric)
{
#ifdef RF_DEBUG
    qCDebug(DIGIKAM_DIMG_LOG) << "matrix size: " << m;
    qCDebug(DIGIKAM_DIMG_LOG) << "correlation: " << gamma;
    qCDebug(DIGIKAM_DIMG_LOG) << "noise: "       << noise_factor;
#endif // RF_DEBUG

    CMat* const g = compute_g(convolution, m, gamma, noise_factor, musq, symmetric);
    double sum    = 0.0;
    int r, c;

    /* Determine sum of array */
    for (r = -g->radius; r <= g->radius; ++r)
    {
        for (c = -g->radius; c <= g->radius; ++c)
        {
            sum += c_mat_elt(g, r, c);
        }
    }

    for (r = -g->radius; r <= g->radius; ++r)
    {
        for (c = -g->radius; c <= g->radius; ++c)
        {
            *c_mat_eltptr(g, r, c) /= sum;
        }
    }

    return (g);
}

void RefocusMatrix::fill_matrix(CMat* const matrix, const int m,
                                double f(const int, const int, const double),
                                const double fun_arg)
{
   int x, y;
    init_c_mat(matrix, m);

    for (y = -m; y <= m; ++y)
    {
        for (x = -m; x <= m; ++x)
        {
            *c_mat_eltptr(matrix, x, y) = f(x, y, fun_arg);
        }
    }
}

void RefocusMatrix::fill_matrix2(CMat* const matrix, const int m,
                                 double f(const int, const int, const double, const double),
                                 const double fun_arg1, const double fun_arg2)
{
   int x, y;
    init_c_mat(matrix, m);

    for (y = -m; y <= m; ++y)
    {
        for (x = -m; x <= m; ++x)
        {
            *c_mat_eltptr(matrix, x, y) = f(x, y, fun_arg1, fun_arg2);
        }
    }
}

void RefocusMatrix::make_gaussian_convolution(const double gradius, CMat* const convolution, const int m)
{
   int x, y;

#ifdef RF_DEBUG
    qCDebug(DIGIKAM_DIMG_LOG) << "gauss: " << gradius;
#endif // RF_DEBUG

    init_c_mat(convolution, m);

    if (SQR(gradius) <= 1.0e10F / 3.40282347e28F)
    {
        for (y = -m; y <= m; ++y)
        {
            for (x = -m; x <= m; ++x)
            {
                *c_mat_eltptr(convolution, x, y) = 0;
            }
        }

        *c_mat_eltptr(convolution, 0, 0) = 1;
    }
    else
    {
        const double alpha = log(2.0) / SQR(gradius);

        for (y = -m; y <= m; ++y)
        {
            for (x = -m; x <= m; ++x)
            {
                *c_mat_eltptr(convolution, x, y) = exp(-alpha * (SQR(x) + SQR(y)));
            }
        }
    }
}

/** Return the integral of sqrt(radius^2 - z^2) for z = 0 to x. */

double RefocusMatrix::circle_integral(const double x, const double radius)
{
    if (radius == 0)
    {
        // Perhaps some epsilon must be added here.
        return (0);
    }
    else
    {
        const double sin = x / radius;
        const double sq_diff = SQR(radius) - SQR(x);
        // From a mathematical point of view the following is redundant.
        // Numerically they are not equivalent!

        if ((sq_diff < 0.0) || (sin < -1.0) || (sin > 1.0))
        {
            if (sin < 0)
            {
                return (-0.25 * SQR(radius) * M_PI);
            }
            else
            {
                return (0.25 * SQR(radius) * M_PI);
            }
        }
        else
        {
            return (0.5 * x * sqrt(sq_diff) + 0.5 * SQR(radius) * asin(sin));
        }
    }
}

double RefocusMatrix::circle_intensity(const int x, const int y, const double radius)
{
    if (radius == 0)
    {
        return (((x == 0) && (y == 0)) ? 1 : 0);
    }
    else
    {
       double xlo = qAbs(x) - 0.5, xhi = qAbs(x) + 0.5,
                        ylo = qAbs(y) - 0.5, yhi = qAbs(y) + 0.5;
       double symmetry_factor = 1, xc1, xc2;

        if (xlo < 0)
        {
            xlo = 0;
            symmetry_factor *= 2;
        }

        if (ylo < 0)
        {
            ylo = 0;
            symmetry_factor *= 2;
        }

        if (SQR(xlo) + SQR(yhi) > SQR(radius))
        {
            xc1 = xlo;
        }
        else if (SQR(xhi) + SQR(yhi) > SQR(radius))
        {
            xc1 = sqrt(SQR(radius) - SQR(yhi));
        }
        else
        {
            xc1 = xhi;
        }

        if (SQR(xlo) + SQR(ylo) > SQR(radius))
        {
            xc2 = xlo;
        }
        else if (SQR(xhi) + SQR(ylo) > SQR(radius))
        {
            xc2 = sqrt(SQR(radius) - SQR(ylo));
        }
        else
        {
            xc2 = xhi;
        }

        return (((yhi - ylo) * (xc1 - xlo) +
                 circle_integral(xc2, radius) - circle_integral(xc1, radius) -
                 (xc2 - xc1) * ylo) * symmetry_factor / (M_PI * SQR(radius)));
    }
}

void RefocusMatrix::make_circle_convolution(const double radius, CMat* const convolution, const int m)
{
#ifdef RF_DEBUG
    qCDebug(DIGIKAM_DIMG_LOG) << "radius: " << radius;
#endif // RF_DEBUG

    fill_matrix(convolution, m, circle_intensity, radius);
}

}  // namespace Digikam
