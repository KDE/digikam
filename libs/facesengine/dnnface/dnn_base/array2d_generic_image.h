/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-08-08
 * @brief   Base functions for dnn module, can be used for face recognition, 
 *          all codes are ported from dlib library (http://dlib.net/)
 *
 * @section DESCRIPTION
 *
 * @author Copyright (C) 2017 by Yingjie Liu
 *         <a href="mailto:yingjiewudi at gmail dot com">yingjiewudi at gmail dot com</a>
 *
 * @section LICENSE
 *
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 *
 * ============================================================ */
#ifndef DLIB_ARRAY2D_GENERIC_iMAGE_Hh_
#define DLIB_ARRAY2D_GENERIC_iMAGE_Hh_

#include "array2d_kernel.h"
#include "generic_image.h"


template <typename T, typename mm>
struct image_traits<array2d<T,mm> >
{
    typedef T pixel_type;
};

template <typename T, typename mm>
struct image_traits<const array2d<T,mm> >
{
    typedef T pixel_type;
};

template <typename T, typename mm>
inline long num_rows( const array2d<T,mm>& img) { return img.nr(); }

template <typename T, typename mm>
inline long num_columns( const array2d<T,mm>& img) { return img.nc(); }

template <typename T, typename mm>
inline void set_image_size(
    array2d<T,mm>& img,
    long rows,
    long cols 
) { img.set_size(rows,cols); }

template <typename T, typename mm>
inline void* image_data(
    array2d<T,mm>& img
)
{
    if (img.size() != 0)
        return &img[0][0];
    else
        return 0;
}

template <typename T, typename mm>
inline const void* image_data(
    const array2d<T,mm>& img
)
{
    if (img.size() != 0)
        return &img[0][0];
    else
        return 0;
}

template <typename T, typename mm>
inline long width_step(
    const array2d<T,mm>& img
) 
{ 
    return img.width_step(); 
}



#endif // DLIB_ARRAY2D_GENERIC_iMAGE_Hh_

