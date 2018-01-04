/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-08-08
 * Description : Base functions for dnn module, can be used for face recognition, 
 *               all codes are ported from dlib library (http://dlib.net/)
 *
 * Copyright (C) 2006-2016 by Davis E. King <davis at dlib dot net>
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#undef DLIB_MATRIx_CONV_ABSTRACT_Hh_
#ifdef DLIB_MATRIx_CONV_ABSTRACT_Hh_

#include "matrix_abstract.h"

//namespace dlib
//{

// ----------------------------------------------------------------------------------------

    const matrix_exp conv (
        const matrix_exp& m1,
        const matrix_exp& m2
    );
    /*!
        requires
            - m1 and m2 both contain elements of the same type
        ensures
            - returns a matrix R such that:
                - R is the convolution of m1 with m2.  In particular, this function is 
                  equivalent to performing the following in matlab: R = conv2(m1,m2).
                - R::type == the same type that was in m1 and m2.
                - R.nr() == m1.nr()+m2.nr()-1
                - R.nc() == m1.nc()+m2.nc()-1
    !*/

// ----------------------------------------------------------------------------------------

    const matrix_exp xcorr (
        const matrix_exp& m1,
        const matrix_exp& m2
    );
    /*!
        requires
            - m1 and m2 both contain elements of the same type
        ensures
            - returns a matrix R such that:
                - R is the cross-correlation of m1 with m2.  In particular, this
                  function returns conv(m1,flip(m2)) if the matrices contain real
                  elements and conv(m1,flip(conj(m2))) if they are complex.
                - R::type == the same type that was in m1 and m2.
                - R.nr() == m1.nr()+m2.nr()-1
                - R.nc() == m1.nc()+m2.nc()-1
    !*/

// ----------------------------------------------------------------------------------------

    const matrix_exp conv_same (
        const matrix_exp& m1,
        const matrix_exp& m2
    );
    /*!
        requires
            - m1 and m2 both contain elements of the same type
        ensures
            - returns a matrix R such that:
                - R is the convolution of m1 with m2.  In particular, this function is 
                  equivalent to performing the following in matlab: R = conv2(m1,m2,'same').
                  In particular, this means the result will have the same dimensions as m1 and will
                  contain the central part of the full convolution.  Therefore, conv_same(m1,m2) is 
                  equivalent to subm(conv(m1,m2), m2.nr()/2, m2.nc()/2, m1.nr(), m1.nc()).
                - R::type == the same type that was in m1 and m2.
                - R.nr() == m1.nr()
                - R.nc() == m1.nc()
    !*/

// ----------------------------------------------------------------------------------------

    const matrix_exp xcorr_same (
        const matrix_exp& m1,
        const matrix_exp& m2
    );
    /*!
        requires
            - m1 and m2 both contain elements of the same type
        ensures
            - returns a matrix R such that:
                - R is the cross-correlation of m1 with m2.  In particular, this
                  function returns conv_same(m1,flip(m2)) if the matrices contain real
                  elements and conv_same(m1,flip(conj(m2))) if they are complex.
                - R::type == the same type that was in m1 and m2.
                - R.nr() == m1.nr()
                - R.nc() == m1.nc()
    !*/

// ----------------------------------------------------------------------------------------

    const matrix_exp conv_valid (
        const matrix_exp& m1,
        const matrix_exp& m2
    );
    /*!
        requires
            - m1 and m2 both contain elements of the same type
        ensures
            - returns a matrix R such that:
                - R is the convolution of m1 with m2.  In particular, this function is 
                  equivalent to performing the following in matlab: R = conv2(m1,m2,'valid').
                  In particular, this means only elements of the convolution which don't require 
                  zero padding are included in the result.
                - R::type == the same type that was in m1 and m2.
                - if (m1 has larger dimensions than m2) then
                    - R.nr() == m1.nr()-m2.nr()+1
                    - R.nc() == m1.nc()-m2.nc()+1
                - else
                    - R.nr() == 0
                    - R.nc() == 0
    !*/

// ----------------------------------------------------------------------------------------

    const matrix_exp xcorr_valid (
        const matrix_exp& m1,
        const matrix_exp& m2
    );
    /*!
        requires
            - m1 and m2 both contain elements of the same type
        ensures
            - returns a matrix R such that:
                - R is the cross-correlation of m1 with m2.  In particular, this
                  function returns conv_valid(m1,flip(m2)) if the matrices contain real
                  elements and conv_valid(m1,flip(conj(m2))) if they are complex.
                - R::type == the same type that was in m1 and m2.
                - if (m1 has larger dimensions than m2) then
                    - R.nr() == m1.nr()-m2.nr()+1
                    - R.nc() == m1.nc()-m2.nc()+1
                - else
                    - R.nr() == 0
                    - R.nc() == 0
    !*/

// ----------------------------------------------------------------------------------------

//}

#endif // DLIB_MATRIx_CONV_ABSTRACT_Hh_


