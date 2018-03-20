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

#ifndef DLIB_MATRIX_GENERIC_iMAGE_Hh_
#define DLIB_MATRIX_GENERIC_iMAGE_Hh_

#include "matrix.h"
#include "../dnn_base/generic_image.h"

//namespace dlib
//{
    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    struct image_traits<matrix<T,NR,NC,MM> >
    {
        typedef T pixel_type;
    };

    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    struct image_traits<const matrix<T,NR,NC,MM> >
    {
        typedef T pixel_type;
    };

    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    inline long num_rows( const matrix<T,NR,NC,MM>& img) { return img.nr(); }

    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    inline long num_columns( const matrix<T,NR,NC,MM>& img) { return img.nc(); }

    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    inline void set_image_size(
        matrix<T,NR,NC,MM>& img,
        long rows,
        long cols 
    ) { img.set_size(rows,cols); }

    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    inline void* image_data(
        matrix<T,NR,NC,MM>& img
    )
    {
        if (img.size() != 0)
            return &img(0,0);
        else
            return 0;
    }

    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    inline const void* image_data(
        const matrix<T,NR,NC,MM>& img
    )
    {
        if (img.size() != 0)
            return &img(0,0);
        else
            return 0;
    }

    template <
        typename T,
        long NR,
        long NC,
        typename MM
        >
    inline long width_step(
        const matrix<T,NR,NC,MM>& img
    ) 
    { 
        return img.nc()*sizeof(T);
    }

//}

#endif // DLIB_MATRIX_GENERIC_iMAGE_Hh_


