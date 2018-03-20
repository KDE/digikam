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

#ifndef DLIB_IMAGE_PYRaMID_Hh_
#define DLIB_IMAGE_PYRaMID_Hh_

#include "dnn_assert.h"
#include "rectangle.h"
#include "drectangle.h"

//using namespace dlib;

template <
        typename pyramid_type
        >
    drectangle tiled_pyramid_to_image (
        const std::vector<rectangle>& rects,
        drectangle r 
    )
    {
        DLIB_CASSERT(rects.size() > 0);

        size_t pyramid_down_iter = nearest_rect(rects, dcenter(r));

        dpoint origin = rects[pyramid_down_iter].tl_corner();
        r = drectangle(r.tl_corner()-origin, r.br_corner()-origin);
        pyramid_type pyr;
        return pyr.rect_up(r, pyramid_down_iter);
    }

 // ----------------------------------------------------------------------------------------

    template <
        typename pyramid_type
        >
    drectangle image_to_tiled_pyramid (
        const std::vector<rectangle>& rects,
        double scale,
        drectangle r
    )
    {
        DLIB_ASSERT(rects.size() > 0);
        DLIB_ASSERT(0 < scale && scale <= 1);
        return drectangle(image_to_tiled_pyramid<pyramid_type>(rects, scale, r.tl_corner()),
                          image_to_tiled_pyramid<pyramid_type>(rects, scale, r.br_corner()));
    }

// ----------------------------------------------------------------------------------------

    template <
        typename pyramid_type,
        typename image_type1,
        typename image_type2
        >
    void create_tiled_pyramid (
        const image_type1& img,
        image_type2& out_img,
        std::vector<rectangle>& rects,
        const unsigned long padding = 10
    )
    {
        DLIB_ASSERT(!is_same_object(img, out_img));

        rects.clear();
        if (num_rows(img)*num_columns(img) == 0)
        {
            set_image_size(out_img,0,0);
            return;
        }

        const long min_height = 5;
        pyramid_type pyr;
        std::vector<matrix<rgb_pixel>> pyramid;
        matrix<rgb_pixel> temp;
        assign_image(temp, img);
        pyramid.push_back(std::move(temp));
        // build the whole pyramid
        while(true)
        {
            matrix<rgb_pixel> temp;
            pyr(pyramid.back(), temp);
            if (temp.size() == 0 || temp.nr() < min_height)
                break;
            pyramid.push_back(std::move(temp));
        }

        // figure out output image size
        long total_height = 0;
        for (auto&& i : pyramid)
            total_height += i.nr()+padding;
        total_height -= padding*2; // don't add unnecessary padding to the very right side.
        long height = 0;
        long prev_width = 0;
        for (auto&& i : pyramid)
        {
            // Figure out how far we go on the first column.  We go until the next image can
            // fit next to the previous one, which means we can double back for the second
            // column of images.
            if (i.nc() <= img.nc()-prev_width-(long)padding && 
                (height-img.nr())*2 >= (total_height-img.nr()))
            {
                break;
            }
            height += i.nr() + padding;
            prev_width = i.nc();
        }
        height -= padding; // don't add unnecessary padding to the very right side.

        set_image_size(out_img,height,img.nc());
        assign_all_pixels(out_img, 0);

        long y = 0;
        size_t i = 0;
        while(y < height)
        {
            rectangle rect = translate_rect(get_rect(pyramid[i]),point(0,y));
            DLIB_ASSERT(get_rect(out_img).contains(rect));
            rects.push_back(rect);
            auto si = sub_image(out_img, rect);
            assign_image(si, pyramid[i]);
            y += pyramid[i].nr()+padding;
            ++i;
        }
        y -= padding;
        while (i < pyramid.size())
        {
            point p1(img.nc()-1,y-1);
            point p2 = p1 - get_rect(pyramid[i]).br_corner();
            rectangle rect(p1,p2);
            DLIB_ASSERT(get_rect(out_img).contains(rect));
            // don't keep going on the last row if it would intersect the original image.
            if (!get_rect(img).intersect(rect).is_empty())
                break;
            rects.push_back(rect);
            auto si = sub_image(out_img, rect);
            assign_image(si, pyramid[i]);
            y -= pyramid[i].nr()+padding;
            ++i;
        }

    }
template <
    unsigned int N
    >
class pyramid_down : noncopyable
{
public:

    COMPILE_TIME_ASSERT(N > 0);

    template <typename T>
    dvector<double,2> point_down (
        const dvector<T,2>& p
    ) const
    {
        const double ratio = (N-1.0)/N;
        //return (p - 0.3)*ratio;
        double new_x = (p.x()-0.3)*ratio, new_y = (p.y()-0.3)*ratio;
        return dvector<double,2>(new_x, new_y);

    }

    template <typename T>
    dvector<double,2> point_up (
        const dvector<T,2>& p
    ) const
    {
        const double ratio = N/(N-1.0);
        //return p*ratio + 0.3;
        double new_x = p.x()*ratio+0.3, new_y = p.y()*ratio+0.3;
        return dvector<double,2>(new_x, new_y);
    }

// -----------------------------

    template <typename T>
    dvector<double,2> point_down (
        const dvector<T,2>& p,
        unsigned int levels
    ) const
    {
        dvector<double,2> temp = p;
        for (unsigned int i = 0; i < levels; ++i)
            temp = point_down(temp);
        return temp;
    }

    template <typename T>
    dvector<double,2> point_up (
        const dvector<T,2>& p,
        unsigned int levels
    ) const
    {
        dvector<double,2> temp = p;
        for (unsigned int i = 0; i < levels; ++i)
            temp = point_up(temp);
        return temp;
    }

// -----------------------------

    drectangle rect_up (
        const drectangle& rect
    ) const
    {
        return drectangle(point_up(rect.tl_corner()), point_up(rect.br_corner()));
    }

    drectangle rect_up (
        const drectangle& rect,
        unsigned int levels
    ) const
    {
        return drectangle(point_up(rect.tl_corner(),levels), point_up(rect.br_corner(),levels));
    }

// -----------------------------

    drectangle rect_down (
        const drectangle& rect
    ) const
    {
        return drectangle(point_down(rect.tl_corner()), point_down(rect.br_corner()));
    }

    drectangle rect_down (
        const drectangle& rect,
        unsigned int levels
    ) const
    {
        return drectangle(point_down(rect.tl_corner(),levels), point_down(rect.br_corner(),levels));
    }

    template <
        typename in_image_type,
        typename out_image_type
        >
    void operator() (
        const in_image_type& original,
        out_image_type& down
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(is_same_object(original, down) == false, 
                    "\t void pyramid_down::operator()"
                    << "\n\t is_same_object(original, down): " << is_same_object(original, down) 
                    << "\n\t this:                           " << this
                    );

        typedef typename image_traits<in_image_type>::pixel_type in_pixel_type;
        typedef typename image_traits<out_image_type>::pixel_type out_pixel_type;
        COMPILE_TIME_ASSERT( pixel_traits<in_pixel_type>::has_alpha == false );
        COMPILE_TIME_ASSERT( pixel_traits<out_pixel_type>::has_alpha == false );


        set_image_size(down, ((N-1)*num_rows(original))/N+0.5, ((N-1)*num_columns(original))/N+0.5);
        resize_image(original, down);
    }

    template <
        typename image_type
        >
    void operator() (
        image_type& img
    ) const
    {
        image_type temp;
        (*this)(img, temp);
        swap(temp, img);
    }
};


#endif
