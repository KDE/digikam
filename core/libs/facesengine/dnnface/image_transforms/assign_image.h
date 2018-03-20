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

#ifndef ASSIGN_IMAGE_H
#define ASSIGN_IMAGE_H

#include "pixel.h"
#include "generic_image.h"
#include "cv_image.h"

template <
        typename dest_image_type,
        typename src_image_type
        >
void impl_assign_image (
        image_view<dest_image_type>& dest,
        const src_image_type& src
        )
{
    dest.set_size(src.nr(),src.nc());
    for (long r = 0; r < src.nr(); ++r)
    {
        for (long c = 0; c < src.nc(); ++c)
        {
            assign_pixel(dest[r][c], src(r,c));
        }
    }
}

template <
        typename dest_image_type,
        typename src_image_type
        >
void impl_assign_image (
        dest_image_type& dest_,
        const src_image_type& src
        )
{
    image_view<dest_image_type> dest(dest_);
    impl_assign_image(dest, src);
}

template <
        typename dest_image_type,
        typename src_image_type
        >
void assign_image (
        dest_image_type& dest,
        const src_image_type& src
        )
{
    // check for the case where dest is the same object as src
    if (is_same_object(dest,src))
        return;

    impl_assign_image(dest, mat(src));
}

template <
    typename image_type
    >
void assign_border_pixels (
    image_view<image_type>& img,
    long x_border_size,
    long y_border_size,
    const typename image_traits<image_type>::pixel_type& p
)
{
    DLIB_ASSERT( x_border_size >= 0 && y_border_size >= 0,
        "\tvoid assign_border_pixels(img, p, border_size)"
        << "\n\tYou have given an invalid border_size"
        << "\n\tx_border_size: " << x_border_size
        << "\n\ty_border_size: " << y_border_size
        );

    y_border_size = std::min(y_border_size, img.nr()/2+1);
    x_border_size = std::min(x_border_size, img.nc()/2+1);

    // assign the top border
    for (long r = 0; r < y_border_size; ++r)
    {
        for (long c = 0; c < img.nc(); ++c)
        {
            img[r][c] = p;
        }
    }

    // assign the bottom border
    for (long r = img.nr()-y_border_size; r < img.nr(); ++r)
    {
        for (long c = 0; c < img.nc(); ++c)
        {
            img[r][c] = p;
        }
    }

    // now assign the two sides
    for (long r = y_border_size; r < img.nr()-y_border_size; ++r)
    {
        // left border
        for (long c = 0; c < x_border_size; ++c)
            img[r][c] = p;

        // right border
        for (long c = img.nc()-x_border_size; c < img.nc(); ++c)
            img[r][c] = p;
    }
}

template <
    typename image_type
    >
void assign_border_pixels (
    image_type& img_,
    long x_border_size,
    long y_border_size,
    const typename image_traits<image_type>::pixel_type& p
)
{
    image_view<image_type> img(img_);
    assign_border_pixels(img, x_border_size, y_border_size, p);
}

template <
    typename image_type
    >
void zero_border_pixels (
    image_type& img,
    long x_border_size,
    long y_border_size
)
{
    DLIB_ASSERT( x_border_size >= 0 && y_border_size >= 0,
        "\tvoid zero_border_pixels(img, p, border_size)"
        << "\n\tYou have given an invalid border_size"
        << "\n\tx_border_size: " << x_border_size
        << "\n\ty_border_size: " << y_border_size
        );

    typename image_traits<image_type>::pixel_type zero_pixel;
    assign_pixel_intensity(zero_pixel, 0);
    assign_border_pixels(img, x_border_size, y_border_size, zero_pixel);
}

template <
    typename dest_image_type,
    typename src_pixel_type
    >
void assign_all_pixels (
    image_view<dest_image_type>& dest_img,
    const src_pixel_type& src_pixel
)
{
    for (long r = 0; r < dest_img.nr(); ++r)
    {
        for (long c = 0; c < dest_img.nc(); ++c)
        {
            assign_pixel(dest_img[r][c], src_pixel);
        }
    }
}
// ----------------------------------------------------------------------------------------

template <
    typename dest_image_type,
    typename src_pixel_type
    >
void assign_all_pixels (
    dest_image_type& dest_img_,
    const src_pixel_type& src_pixel
)
{
    image_view<dest_image_type> dest_img(dest_img_);
    assign_all_pixels(dest_img, src_pixel);
}

// ----------------------------------------------------------------------------------------

template <
    typename image_type
    >
void zero_border_pixels (
    image_view<image_type>& img,
    rectangle inside
)
{
    inside = inside.intersect(get_rect(img));
    if (inside.is_empty())
    {
        assign_all_pixels(img, 0);
        return;
    }

    for (long r = 0; r < inside.top(); ++r)
    {
        for (long c = 0; c < img.nc(); ++c)
            assign_pixel(img[r][c], 0);
    }
    for (long r = inside.top(); r <= inside.bottom(); ++r)
    {
        for (long c = 0; c < inside.left(); ++c)
            assign_pixel(img[r][c], 0);
        for (long c = inside.right()+1; c < img.nc(); ++c)
            assign_pixel(img[r][c], 0);
    }
    for (long r = inside.bottom()+1; r < img.nr(); ++r)
    {
        for (long c = 0; c < img.nc(); ++c)
            assign_pixel(img[r][c], 0);
    }
}

// ----------------------------------------------------------------------------------------

template <
    typename image_type
    >
void zero_border_pixels (
    image_type& img_,
    const rectangle& inside
)
{
    image_view<image_type> img(img_);
    zero_border_pixels(img, inside);
}









#endif // ASSIGN_IMAGE_H
