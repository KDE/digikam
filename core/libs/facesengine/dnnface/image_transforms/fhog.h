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

#ifndef _DLIB_FHOG_H_
#define _DLIB_FHOG_H_

#include "matrix.h"
#include "matrix_utilities.h"
#include "matrix_subexp.h"
#include "matrix_math_functions.h"
#include "matrix_generic_image.h"
#include "array2d_kernel.h"
#include "serialize_pixel_overloads.h"
#include "array2d_generic_image.h"
#include "array_kernel.h"
#include "rectangle.h"
#include "drectangle.h"
#include "dnn_vector.h"
#include "assign_image.h"
//#include "draw.h"
#include "interpolation.h"
#include "simd4f.h"


template <
    typename image_type,
    typename pixel_type
    >
void draw_line (
    long x1,
    long y1,
    long x2,
    long y2,
    image_type& c_,
    const pixel_type& val
)
{
    image_view<image_type> c(c_);
    if (x1 == x2)
    {
        // make sure y1 comes before y2
        if (y1 > y2)
            swap(y1,y2);

        if (x1 < 0 || x1 >= c.nc())
            return;


        // this is a vertical line
        for (long y = y1; y <= y2; ++y)
        {
            if (y < 0 || y >= c.nr())
                continue;

            assign_pixel(c[y][x1], val);
        }
    }
    else if (y1 == y2)
    {

        // make sure x1 comes before x2
        if (x1 > x2)
            swap(x1,x2);

        if (y1 < 0 || y1 >= c.nr())
            return;

        // this is a horizontal line
        for (long x = x1; x <= x2; ++x)
        {
            if (x < 0 || x >= c.nc())
                continue;

            assign_pixel(c[y1][x] , val);
        }
    }
    else
    {
        // This part is a little more complicated because we are going to perform alpha
        // blending so the diagonal lines look nice.
        const rectangle valid_area = get_rect(c);
        rgb_alpha_pixel alpha_pixel;
        assign_pixel(alpha_pixel, val);
        const unsigned char max_alpha = alpha_pixel.alpha;

        const long rise = (((long)y2) - ((long)y1));
        const long run = (((long)x2) - ((long)x1));
        if (std::abs(rise) < std::abs(run))
        {
            const double slope = ((double)rise)/run;


            double first, last;


            if (x1 > x2)
            {
                first = std::max(x2,valid_area.left());
                last = std::min(x1,valid_area.right());
            }
            else
            {
                first = std::max(x1,valid_area.left());
                last = std::min(x2,valid_area.right());
            }

            long y;
            long x;
            const double x1f = x1;
            const double y1f = y1;
            for (double i = first; i <= last; ++i)
            {
                const double dy = slope*(i-x1f) + y1f;
                const double dx = i;

                y = static_cast<long>(dy);
                x = static_cast<long>(dx);


                if (y >= valid_area.top() && y <= valid_area.bottom())
                {
                    alpha_pixel.alpha = static_cast<unsigned char>((1.0-(dy-y))*max_alpha);
                    assign_pixel(c[y][x], alpha_pixel);
                }
                if (y+1 >= valid_area.top() && y+1 <= valid_area.bottom())
                {
                    alpha_pixel.alpha = static_cast<unsigned char>((dy-y)*max_alpha);
                    assign_pixel(c[y+1][x], alpha_pixel);
                }
            }
        }
        else
        {
            const double slope = ((double)run)/rise;


            double first, last;


            if (y1 > y2)
            {
                first = std::max(y2,valid_area.top());
                last = std::min(y1,valid_area.bottom());
            }
            else
            {
                first = std::max(y1,valid_area.top());
                last = std::min(y2,valid_area.bottom());
            }

            long x;
            long y;
            const double x1f = x1;
            const double y1f = y1;
            for (double i = first; i <= last; ++i)
            {
                const double dx = slope*(i-y1f) + x1f;
                const double dy = i;

                y = static_cast<long>(dy);
                x = static_cast<long>(dx);

                if (x >= valid_area.left() && x <= valid_area.right())
                {
                    alpha_pixel.alpha = static_cast<unsigned char>((1.0-(dx-x))*max_alpha);
                    assign_pixel(c[y][x], alpha_pixel);
                }
                if (x+1 >= valid_area.left() && x+1 <= valid_area.right())
                {
                    alpha_pixel.alpha = static_cast<unsigned char>((dx-x)*max_alpha);
                    assign_pixel(c[y][x+1], alpha_pixel);
                }
            }
        }
    }

}


// ----------------------------------------------------------------------------------------

template <
    typename image_type,
    typename pixel_type
    >
void draw_line (
    image_type& c,
    const point& p1,
    const point& p2,
    const pixel_type& val
)
{
    draw_line(p1.x(),p1.y(),p2.x(),p2.y(),c,val);
}

class border_enumerator
{
public:
    border_enumerator(
    )
    {
        reset();
    }

    border_enumerator(
        const rectangle& rect_,
        unsigned long border_size
    ) :
        rect(rect_),
        inner_rect(shrink_rect(rect_, border_size))
    {
        reset();
    }

    border_enumerator(
        const rectangle& rect_,
        const rectangle& non_border_region
    ) :
        rect(rect_),
        inner_rect(non_border_region.intersect(rect))
    {
        reset();
    }

    void reset (
    )
    {
        // make the four rectangles that surround inner_rect and intersect them
        // with rect.
        bleft   = rect.intersect(rectangle(std::numeric_limits<long>::min(),
                                           std::numeric_limits<long>::min(),
                                           inner_rect.left()-1,
                                           std::numeric_limits<long>::max()));

        bright  = rect.intersect(rectangle(inner_rect.right()+1,
                                           std::numeric_limits<long>::min(),
                                           std::numeric_limits<long>::max(),
                                           std::numeric_limits<long>::max()));

        btop    = rect.intersect(rectangle(inner_rect.left(),
                                           std::numeric_limits<long>::min(),
                                           inner_rect.right(),
                                           inner_rect.top()-1));

        bbottom = rect.intersect(rectangle(inner_rect.left(),
                                           inner_rect.bottom()+1,
                                           inner_rect.right(),
                                           std::numeric_limits<long>::max()));

        p = bleft.tl_corner();
        p.x() -= 1;

        mode = atleft;
    }

    bool at_start (
    ) const
    {
        point temp = bleft.tl_corner();
        temp.x() -=1;
        return temp == p;
    }

    bool current_element_valid(
    ) const
    {
        return rect.contains(p);
    }

    bool move_next()
    {
        if (mode == atleft)
        {
            if (advance_point(bleft, p))
                return true;

            mode = attop;
            p = btop.tl_corner();
            p.x() -= 1;
        }
        if (mode == attop)
        {
            if (advance_point(btop, p))
                return true;

            mode = atright;
            p = bright.tl_corner();
            p.x() -= 1;
        }
        if (mode == atright)
        {
            if (advance_point(bright, p))
                return true;

            mode = atbottom;
            p = bbottom.tl_corner();
            p.x() -= 1;
        }

        if (advance_point(bbottom, p))
            return true;

        // put p outside rect since there are no more points to enumerate
        p = rect.br_corner();
        p.x() += 1;

        return false;
    }

    unsigned long size (
    ) const
    {
        return rect.area() - inner_rect.area();
    }

    const point& element (
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(current_element_valid(),
            "\t point border_enumerator::element()"
            << "\n\t This function can't be called unless the element is valid."
            << "\n\t this: " << this
            );

        return p;
    }

private:

    bool advance_point (
        const rectangle& r,
        point& p
    ) const
    {
        p.x() += 1;
        if (p.x() > r.right())
        {
            p.x() = r.left();
            p.y() += 1;
        }

        return r.contains(p);
    }

    point p;
    rectangle rect;
    rectangle inner_rect;  // the non-border regions of rect

    enum emode
    {
        atleft,
        atright,
        atbottom,
        attop
    };

    emode mode;

    rectangle btop, bleft, bright, bbottom;
};




namespace impl_fhog
{
    template <typename image_type, typename T>
    inline typename enable_if_c<pixel_traits<typename image_type::pixel_type>::rgb>::type get_gradient (
        const int r,
        const int c,
        const image_type& img,
        matrix<T,2,1>& grad,
        T& len
    )
    {
        matrix<T, 2, 1> grad2, grad3;
        // get the red gradient
        grad(0) = (int)img[r][c+1].red-(int)img[r][c-1].red;
        grad(1) = (int)img[r+1][c].red-(int)img[r-1][c].red;
        len = length_squared(grad);

        // get the green gradient
        grad2(0) = (int)img[r][c+1].green-(int)img[r][c-1].green;
        grad2(1) = (int)img[r+1][c].green-(int)img[r-1][c].green;
        T v2 = length_squared(grad2);

        // get the blue gradient
        grad3(0) = (int)img[r][c+1].blue-(int)img[r][c-1].blue;
        grad3(1) = (int)img[r+1][c].blue-(int)img[r-1][c].blue;
        T v3 = length_squared(grad3);

        // pick color with strongest gradient
        if (v2 > len)
        {
            len = v2;
            grad = grad2;
        }
        if (v3 > len)
        {
            len = v3;
            grad = grad3;
        }
    }

    template <typename image_type>
    inline typename enable_if_c<pixel_traits<typename image_type::pixel_type>::rgb>::type get_gradient (
        const int r,
        const int c,
        const image_type& img,
        simd4f& grad_x,
        simd4f& grad_y,
        simd4f& len
    )
    {
        simd4i rleft((int)img[r][c-1].red,
                    (int)img[r][c].red,
                    (int)img[r][c+1].red,
                    (int)img[r][c+2].red);
        simd4i rright((int)img[r][c+1].red,
                     (int)img[r][c+2].red,
                     (int)img[r][c+3].red,
                     (int)img[r][c+4].red);
        simd4i rtop((int)img[r-1][c].red,
                   (int)img[r-1][c+1].red,
                   (int)img[r-1][c+2].red,
                   (int)img[r-1][c+3].red);
        simd4i rbottom((int)img[r+1][c].red,
                      (int)img[r+1][c+1].red,
                      (int)img[r+1][c+2].red,
                      (int)img[r+1][c+3].red);

        simd4i gleft((int)img[r][c-1].green,
                    (int)img[r][c].green,
                    (int)img[r][c+1].green,
                    (int)img[r][c+2].green);
        simd4i gright((int)img[r][c+1].green,
                     (int)img[r][c+2].green,
                     (int)img[r][c+3].green,
                     (int)img[r][c+4].green);
        simd4i gtop((int)img[r-1][c].green,
                   (int)img[r-1][c+1].green,
                   (int)img[r-1][c+2].green,
                   (int)img[r-1][c+3].green);
        simd4i gbottom((int)img[r+1][c].green,
                      (int)img[r+1][c+1].green,
                      (int)img[r+1][c+2].green,
                      (int)img[r+1][c+3].green);

        simd4i bleft((int)img[r][c-1].blue,
                    (int)img[r][c].blue,
                    (int)img[r][c+1].blue,
                    (int)img[r][c+2].blue);
        simd4i bright((int)img[r][c+1].blue,
                     (int)img[r][c+2].blue,
                     (int)img[r][c+3].blue,
                     (int)img[r][c+4].blue);
        simd4i btop((int)img[r-1][c].blue,
                   (int)img[r-1][c+1].blue,
                   (int)img[r-1][c+2].blue,
                   (int)img[r-1][c+3].blue);
        simd4i bbottom((int)img[r+1][c].blue,
                      (int)img[r+1][c+1].blue,
                      (int)img[r+1][c+2].blue,
                      (int)img[r+1][c+3].blue);

        simd4i grad_x_red   = rright-rleft;
        simd4i grad_y_red   = rbottom-rtop;
        simd4i grad_x_green = gright-gleft;
        simd4i grad_y_green = gbottom-gtop;
        simd4i grad_x_blue  = bright-bleft;
        simd4i grad_y_blue  = bbottom-btop;

        simd4i rlen = grad_x_red*grad_x_red + grad_y_red*grad_y_red;
        simd4i glen = grad_x_green*grad_x_green + grad_y_green*grad_y_green;
        simd4i blen = grad_x_blue*grad_x_blue + grad_y_blue*grad_y_blue;

        simd4i cmp = rlen>glen;
        simd4i tgrad_x = select(cmp,grad_x_red,grad_x_green);
        simd4i tgrad_y = select(cmp,grad_y_red,grad_y_green);
        simd4i tlen = select(cmp,rlen,glen);

        cmp = tlen>blen;
        grad_x = select(cmp,tgrad_x,grad_x_blue);
        grad_y = select(cmp,tgrad_y,grad_y_blue);
        len = select(cmp,tlen,blen);
    }

    // ------------------------------------------------------------------------------------

    template <typename image_type>
    inline typename enable_if_c<pixel_traits<typename image_type::pixel_type>::rgb>::type get_gradient(
        const int r,
        const int c,
        const image_type& img,
        simd8f& grad_x,
        simd8f& grad_y,
        simd8f& len
        )
    {
        simd8i rleft((int)img[r][c - 1].red,
            (int)img[r][c].red,
            (int)img[r][c + 1].red,
            (int)img[r][c + 2].red,
            (int)img[r][c + 3].red,
            (int)img[r][c + 4].red,
            (int)img[r][c + 5].red,
            (int)img[r][c + 6].red);
        simd8i rright((int)img[r][c + 1].red,
            (int)img[r][c + 2].red,
            (int)img[r][c + 3].red,
            (int)img[r][c + 4].red,
            (int)img[r][c + 5].red,
            (int)img[r][c + 6].red,
            (int)img[r][c + 7].red,
            (int)img[r][c + 8].red);
        simd8i rtop((int)img[r - 1][c].red,
            (int)img[r - 1][c + 1].red,
            (int)img[r - 1][c + 2].red,
            (int)img[r - 1][c + 3].red,
            (int)img[r - 1][c + 4].red,
            (int)img[r - 1][c + 5].red,
            (int)img[r - 1][c + 6].red,
            (int)img[r - 1][c + 7].red);
        simd8i rbottom((int)img[r + 1][c].red,
            (int)img[r + 1][c + 1].red,
            (int)img[r + 1][c + 2].red,
            (int)img[r + 1][c + 3].red,
            (int)img[r + 1][c + 4].red,
            (int)img[r + 1][c + 5].red,
            (int)img[r + 1][c + 6].red,
            (int)img[r + 1][c + 7].red);

        simd8i gleft((int)img[r][c - 1].green,
            (int)img[r][c].green,
            (int)img[r][c + 1].green,
            (int)img[r][c + 2].green,
            (int)img[r][c + 3].green,
            (int)img[r][c + 4].green,
            (int)img[r][c + 5].green,
            (int)img[r][c + 6].green);
        simd8i gright((int)img[r][c + 1].green,
            (int)img[r][c + 2].green,
            (int)img[r][c + 3].green,
            (int)img[r][c + 4].green,
            (int)img[r][c + 5].green,
            (int)img[r][c + 6].green,
            (int)img[r][c + 7].green,
            (int)img[r][c + 8].green);
        simd8i gtop((int)img[r - 1][c].green,
            (int)img[r - 1][c + 1].green,
            (int)img[r - 1][c + 2].green,
            (int)img[r - 1][c + 3].green,
            (int)img[r - 1][c + 4].green,
            (int)img[r - 1][c + 5].green,
            (int)img[r - 1][c + 6].green,
            (int)img[r - 1][c + 7].green);
        simd8i gbottom((int)img[r + 1][c].green,
            (int)img[r + 1][c + 1].green,
            (int)img[r + 1][c + 2].green,
            (int)img[r + 1][c + 3].green,
            (int)img[r + 1][c + 4].green,
            (int)img[r + 1][c + 5].green,
            (int)img[r + 1][c + 6].green,
            (int)img[r + 1][c + 7].green);

        simd8i bleft((int)img[r][c - 1].blue,
            (int)img[r][c].blue,
            (int)img[r][c + 1].blue,
            (int)img[r][c + 2].blue,
            (int)img[r][c + 3].blue,
            (int)img[r][c + 4].blue,
            (int)img[r][c + 5].blue,
            (int)img[r][c + 6].blue);
        simd8i bright((int)img[r][c + 1].blue,
            (int)img[r][c + 2].blue,
            (int)img[r][c + 3].blue,
            (int)img[r][c + 4].blue,
            (int)img[r][c + 5].blue,
            (int)img[r][c + 6].blue,
            (int)img[r][c + 7].blue,
            (int)img[r][c + 8].blue);
        simd8i btop((int)img[r - 1][c].blue,
            (int)img[r - 1][c + 1].blue,
            (int)img[r - 1][c + 2].blue,
            (int)img[r - 1][c + 3].blue,
            (int)img[r - 1][c + 4].blue,
            (int)img[r - 1][c + 5].blue,
            (int)img[r - 1][c + 6].blue,
            (int)img[r - 1][c + 7].blue);
        simd8i bbottom((int)img[r + 1][c].blue,
            (int)img[r + 1][c + 1].blue,
            (int)img[r + 1][c + 2].blue,
            (int)img[r + 1][c + 3].blue,
            (int)img[r + 1][c + 4].blue,
            (int)img[r + 1][c + 5].blue,
            (int)img[r + 1][c + 6].blue,
            (int)img[r + 1][c + 7].blue);

        simd8i grad_x_red = rright - rleft;
        simd8i grad_y_red = rbottom - rtop;
        simd8i grad_x_green = gright - gleft;
        simd8i grad_y_green = gbottom - gtop;
        simd8i grad_x_blue = bright - bleft;
        simd8i grad_y_blue = bbottom - btop;

        simd8i rlen = grad_x_red*grad_x_red + grad_y_red*grad_y_red;
        simd8i glen = grad_x_green*grad_x_green + grad_y_green*grad_y_green;
        simd8i blen = grad_x_blue*grad_x_blue + grad_y_blue*grad_y_blue;

        simd8i cmp = rlen > glen;
        simd8i tgrad_x = select(cmp, grad_x_red, grad_x_green);
        simd8i tgrad_y = select(cmp, grad_y_red, grad_y_green);
        simd8i tlen = select(cmp, rlen, glen);

        cmp = tlen > blen;
        grad_x = select(cmp, tgrad_x, grad_x_blue);
        grad_y = select(cmp, tgrad_y, grad_y_blue);
        len = select(cmp, tlen, blen);
    }

    // ------------------------------------------------------------------------------------

    template <typename image_type, typename T>
    inline typename disable_if_c<pixel_traits<typename image_type::pixel_type>::rgb>::type get_gradient (
        const int r,
        const int c,
        const image_type& img,
        matrix<T, 2, 1>& grad,
        T& len
    )
    {
        grad(0) = (int)get_pixel_intensity(img[r][c+1])-(int)get_pixel_intensity(img[r][c-1]);
        grad(1) = (int)get_pixel_intensity(img[r+1][c])-(int)get_pixel_intensity(img[r-1][c]);
        len = length_squared(grad);
    }

    template <typename image_type>
    inline typename disable_if_c<pixel_traits<typename image_type::pixel_type>::rgb>::type get_gradient (
        int r,
        int c,
        const image_type& img,
        simd4f& grad_x,
        simd4f& grad_y,
        simd4f& len
    )
    {
        simd4i left((int)get_pixel_intensity(img[r][c-1]),
                    (int)get_pixel_intensity(img[r][c]),
                    (int)get_pixel_intensity(img[r][c+1]),
                    (int)get_pixel_intensity(img[r][c+2]));
        simd4i right((int)get_pixel_intensity(img[r][c+1]),
                     (int)get_pixel_intensity(img[r][c+2]),
                     (int)get_pixel_intensity(img[r][c+3]),
                     (int)get_pixel_intensity(img[r][c+4]));

        simd4i top((int)get_pixel_intensity(img[r-1][c]),
                   (int)get_pixel_intensity(img[r-1][c+1]),
                   (int)get_pixel_intensity(img[r-1][c+2]),
                   (int)get_pixel_intensity(img[r-1][c+3]));
        simd4i bottom((int)get_pixel_intensity(img[r+1][c]),
                      (int)get_pixel_intensity(img[r+1][c+1]),
                      (int)get_pixel_intensity(img[r+1][c+2]),
                      (int)get_pixel_intensity(img[r+1][c+3]));

        grad_x = right-left;
        grad_y = bottom-top;

        len = (grad_x*grad_x + grad_y*grad_y);
    }

    // ------------------------------------------------------------------------------------

    template <typename image_type>
    inline typename disable_if_c<pixel_traits<typename image_type::pixel_type>::rgb>::type get_gradient(
        int r,
        int c,
        const image_type& img,
        simd8f& grad_x,
        simd8f& grad_y,
        simd8f& len
        )
    {
        simd8i left((int)get_pixel_intensity(img[r][c - 1]),
            (int)get_pixel_intensity(img[r][c]),
            (int)get_pixel_intensity(img[r][c + 1]),
            (int)get_pixel_intensity(img[r][c + 2]),
            (int)get_pixel_intensity(img[r][c + 3]),
            (int)get_pixel_intensity(img[r][c + 4]),
            (int)get_pixel_intensity(img[r][c + 5]),
            (int)get_pixel_intensity(img[r][c + 6]));
        simd8i right((int)get_pixel_intensity(img[r][c + 1]),
            (int)get_pixel_intensity(img[r][c + 2]),
            (int)get_pixel_intensity(img[r][c + 3]),
            (int)get_pixel_intensity(img[r][c + 4]),
            (int)get_pixel_intensity(img[r][c + 5]),
            (int)get_pixel_intensity(img[r][c + 6]),
            (int)get_pixel_intensity(img[r][c + 7]),
            (int)get_pixel_intensity(img[r][c + 8]));

        simd8i top((int)get_pixel_intensity(img[r - 1][c]),
            (int)get_pixel_intensity(img[r - 1][c + 1]),
            (int)get_pixel_intensity(img[r - 1][c + 2]),
            (int)get_pixel_intensity(img[r - 1][c + 3]),
            (int)get_pixel_intensity(img[r - 1][c + 4]),
            (int)get_pixel_intensity(img[r - 1][c + 5]),
            (int)get_pixel_intensity(img[r - 1][c + 6]),
            (int)get_pixel_intensity(img[r - 1][c + 7]));
        simd8i bottom((int)get_pixel_intensity(img[r + 1][c]),
            (int)get_pixel_intensity(img[r + 1][c + 1]),
            (int)get_pixel_intensity(img[r + 1][c + 2]),
            (int)get_pixel_intensity(img[r + 1][c + 3]),
            (int)get_pixel_intensity(img[r + 1][c + 4]),
            (int)get_pixel_intensity(img[r + 1][c + 5]),
            (int)get_pixel_intensity(img[r + 1][c + 6]),
            (int)get_pixel_intensity(img[r + 1][c + 7]));

        grad_x = right - left;
        grad_y = bottom - top;

        len = (grad_x*grad_x + grad_y*grad_y);
    }

    // ------------------------------------------------------------------------------------

    template <typename T, typename mm1, typename mm2>
    inline void set_hog (
        array<array2d<T,mm1>,mm2>& hog,
        int o,
        int x,
        int y,
        const float& value
    )
    {
        hog[o][y][x] = value;
    }

    template <typename T, typename mm1, typename mm2>
    void init_hog (
        array<array2d<T,mm1>,mm2>& hog,
        int hog_nr,
        int hog_nc,
        int filter_rows_padding,
        int filter_cols_padding
    )
    {
        const int num_hog_bands = 27+4;
        hog.resize(num_hog_bands);
        for (int i = 0; i < num_hog_bands; ++i)
        {
            hog[i].set_size(hog_nr+filter_rows_padding-1, hog_nc+filter_cols_padding-1);
            rectangle rect = get_rect(hog[i]);
            rect.top() +=   (filter_rows_padding-1)/2;
            rect.left() +=  (filter_cols_padding-1)/2;
            rect.right() -= filter_cols_padding/2;
            rect.bottom() -= filter_rows_padding/2;
            zero_border_pixels(hog[i],rect);
        }
    }

    template <typename T, typename mm1, typename mm2>
    void init_hog_zero_everything (
        array<array2d<T,mm1>,mm2>& hog,
        int hog_nr,
        int hog_nc,
        int filter_rows_padding,
        int filter_cols_padding
    )
    {
        const int num_hog_bands = 27+4;
        hog.resize(num_hog_bands);
        for (int i = 0; i < num_hog_bands; ++i)
        {
            hog[i].set_size(hog_nr+filter_rows_padding-1, hog_nc+filter_cols_padding-1);
            assign_all_pixels(hog[i], 0);
        }
    }

// ------------------------------------------------------------------------------------

    template <typename T, typename mm>
    inline void set_hog (
        array2d<matrix<T,31,1>,mm>& hog,
        int o,
        int x,
        int y,
        const float& value
    )
    {
        hog[y][x](o) = value;
    }

    template <typename T, typename mm>
    void init_hog (
        array2d<matrix<T,31,1>,mm>& hog,
        int hog_nr,
        int hog_nc,
        int filter_rows_padding,
        int filter_cols_padding
    )
    {
        hog.set_size(hog_nr+filter_rows_padding-1, hog_nc+filter_cols_padding-1);

        // now zero out the border region
        rectangle rect = get_rect(hog);
        rect.top() +=   (filter_rows_padding-1)/2;
        rect.left() +=  (filter_cols_padding-1)/2;
        rect.right() -= filter_cols_padding/2;
        rect.bottom() -= filter_rows_padding/2;
        border_enumerator be(get_rect(hog),rect);
        while (be.move_next())
        {
            const point p = be.element();
            set_all_elements(hog[p.y()][p.x()], 0);
        }
    }

    template <typename T, typename mm>
    void init_hog_zero_everything (
        array2d<matrix<T,31,1>,mm>& hog,
        int hog_nr,
        int hog_nc,
        int filter_rows_padding,
        int filter_cols_padding
    )
    {
        hog.set_size(hog_nr+filter_rows_padding-1, hog_nc+filter_cols_padding-1);

        for (long r = 0; r < hog.nr(); ++r)
        {
            for (long c = 0; c < hog.nc(); ++c)
            {
                set_all_elements(hog[r][c], 0);
            }
        }
    }

// ------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename out_type
        >
    void impl_extract_fhog_features_cell_size_1(
        const image_type& img_,
        out_type& hog,
        int filter_rows_padding,
        int filter_cols_padding
    )
    {
        const_image_view<image_type> img(img_);
        // make sure requires clause is not broken
        DLIB_ASSERT( filter_rows_padding > 0 &&
                     filter_cols_padding > 0 ,
            "\t void extract_fhog_features()"
            << "\n\t Invalid inputs were given to this function. "
            << "\n\t filter_rows_padding: " << filter_rows_padding
            << "\n\t filter_cols_padding: " << filter_cols_padding
            );

        /*
            This function is an optimized version of impl_extract_fhog_features() for
            the case where cell_size == 1.
        */


        // unit vectors used to compute gradient orientation
        matrix<float,2,1> directions[9];
        directions[0] =  1.0000, 0.0000;
        directions[1] =  0.9397, 0.3420;
        directions[2] =  0.7660, 0.6428;
        directions[3] =  0.500,  0.8660;
        directions[4] =  0.1736, 0.9848;
        directions[5] = -0.1736, 0.9848;
        directions[6] = -0.5000, 0.8660;
        directions[7] = -0.7660, 0.6428;
        directions[8] = -0.9397, 0.3420;



        if (img.nr() <= 2 || img.nc() <= 2)
        {
            hog.clear();
            return;
        }

        array2d<unsigned char> angle(img.nr(), img.nc());

        array2d<float> norm(img.nr(), img.nc());
        zero_border_pixels(norm,1,1);

        // memory for HOG features
        const long hog_nr = img.nr()-2;
        const long hog_nc = img.nc()-2;

        const int padding_rows_offset = (filter_rows_padding-1)/2;
        const int padding_cols_offset = (filter_cols_padding-1)/2;
        init_hog_zero_everything(hog, hog_nr, hog_nc, filter_rows_padding, filter_cols_padding);


        const int visible_nr = img.nr()-1;
        const int visible_nc = img.nc()-1;

        // First populate the gradient histograms
        for (int y = 1; y < visible_nr; y++)
        {
            int x;
            for (x = 1; x < visible_nc - 7; x += 8)
            {
                // v will be the length of the gradient vectors.
                simd8f grad_x, grad_y, v;
                get_gradient(y, x, img, grad_x, grad_y, v);

                float _vv[8];
                v.store(_vv);

                // Now snap the gradient to one of 18 orientations
                simd8f best_dot = 0;
                simd8f best_o = 0;
                for (int o = 0; o < 9; o++)
                {
                    simd8f dot = grad_x*directions[o](0) + grad_y*directions[o](1);
                    simd8f_bool cmp = dot>best_dot;
                    best_dot = select(cmp, dot, best_dot);
                    dot *= -1;
                    best_o = select(cmp, o, best_o);

                    cmp = dot > best_dot;
                    best_dot = select(cmp, dot, best_dot);
                    best_o = select(cmp, o + 9, best_o);
                }

                int32 _best_o[8]; simd8i(best_o).store(_best_o);

                norm[y][x + 0] = _vv[0];
                norm[y][x + 1] = _vv[1];
                norm[y][x + 2] = _vv[2];
                norm[y][x + 3] = _vv[3];
                norm[y][x + 4] = _vv[4];
                norm[y][x + 5] = _vv[5];
                norm[y][x + 6] = _vv[6];
                norm[y][x + 7] = _vv[7];

                angle[y][x + 0] = _best_o[0];
                angle[y][x + 1] = _best_o[1];
                angle[y][x + 2] = _best_o[2];
                angle[y][x + 3] = _best_o[3];
                angle[y][x + 4] = _best_o[4];
                angle[y][x + 5] = _best_o[5];
                angle[y][x + 6] = _best_o[6];
                angle[y][x + 7] = _best_o[7];
            }
            // Now process the right columns that don't fit into simd registers.
            for (; x < visible_nc; x++)
            {
                matrix<float,2,1> grad;
                float v;
                get_gradient(y,x,img,grad,v);

                // snap to one of 18 orientations
                float best_dot = 0;
                int best_o = 0;
                for (int o = 0; o < 9; o++)
                {
                    const float dotf = dot(directions[o], grad);
                    if (dotf > best_dot)
                    {
                        best_dot = dotf;
                        best_o = o;
                    }
                    else if (-dotf > best_dot)
                    {
                        best_dot = -dotf;
                        best_o = o+9;
                    }
                }

                norm[y][x] = v;
                angle[y][x] = best_o;
            }
        }

        const float eps = 0.0001;
        // compute features
        for (int y = 0; y < hog_nr; y++)
        {
            const int yy = y+padding_rows_offset;
            for (int x = 0; x < hog_nc; x++)
            {
                const simd4f z1(norm[y+1][x+1],
                                norm[y][x+1],
                                norm[y+1][x],
                                norm[y][x]);

                const simd4f z2(norm[y+1][x+2],
                                norm[y][x+2],
                                norm[y+1][x+1],
                                norm[y][x+1]);

                const simd4f z3(norm[y+2][x+1],
                                norm[y+1][x+1],
                                norm[y+2][x],
                                norm[y+1][x]);

                const simd4f z4(norm[y+2][x+2],
                                norm[y+1][x+2],
                                norm[y+2][x+1],
                                norm[y+1][x+1]);

                const simd4f temp0 = std::sqrt(norm[y+1][x+1]);
                const simd4f nn = 0.2*sqrt(z1+z2+z3+z4+eps);
                const simd4f n = 0.1/nn;

                simd4f t = 0;

                const int xx = x+padding_cols_offset;

                simd4f h0 = min(temp0,nn)*n;
                const float vv = sum(h0);
                set_hog(hog,angle[y+1][x+1],xx,yy,   vv);
                t += h0;

                t *= 2*0.2357;

                // contrast-insensitive features
                set_hog(hog,angle[y+1][x+1]%9+18,xx,yy, vv);


                float temp[4];
                t.store(temp);

                // texture features
                set_hog(hog,27,xx,yy, temp[0]);
                set_hog(hog,28,xx,yy, temp[1]);
                set_hog(hog,29,xx,yy, temp[2]);
                set_hog(hog,30,xx,yy, temp[3]);
            }
        }
    }

// ------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename out_type
        >
    void impl_extract_fhog_features(
        const image_type& img_,
        out_type& hog,
        int cell_size,
        int filter_rows_padding,
        int filter_cols_padding
    )
    {
        const_image_view<image_type> img(img_);
        // make sure requires clause is not broken
        DLIB_ASSERT( cell_size > 0 &&
                     filter_rows_padding > 0 &&
                     filter_cols_padding > 0 ,
            "\t void extract_fhog_features()"
            << "\n\t Invalid inputs were given to this function. "
            << "\n\t cell_size: " << cell_size
            << "\n\t filter_rows_padding: " << filter_rows_padding
            << "\n\t filter_cols_padding: " << filter_cols_padding
            );

        /*
            This function implements the HOG feature extraction method described in
            the paper:
                P. Felzenszwalb, R. Girshick, D. McAllester, D. Ramanan
                Object Detection with Discriminatively Trained Part Based Models
                IEEE Transactions on Pattern Analysis and Machine Intelligence, Vol. 32, No. 9, Sep. 2010

            Moreover, this function is derived from the HOG feature extraction code
            from the features.cc file in the voc-releaseX code (see
            http://people.cs.uchicago.edu/~rbg/latent/) which is has the following
            license (note that the code has been modified to work with grayscale and
            color as well as planar and interlaced input and output formats):

            Copyright (C) 2011, 2012 Ross Girshick, Pedro Felzenszwalb
            Copyright (C) 2008, 2009, 2010 Pedro Felzenszwalb, Ross Girshick
            Copyright (C) 2007 Pedro Felzenszwalb, Deva Ramanan

            Permission is hereby granted, free of charge, to any person obtaining
            a copy of this software and associated documentation files (the
            "Software"), to deal in the Software without restriction, including
            without limitation the rights to use, copy, modify, merge, publish,
            distribute, sublicense, and/or sell copies of the Software, and to
            permit persons to whom the Software is furnished to do so, subject to
            the following conditions:

            The above copyright notice and this permission notice shall be
            included in all copies or substantial portions of the Software.

            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
            MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
            LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
            OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
            WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
        */

        if (cell_size == 1)
        {
            impl_extract_fhog_features_cell_size_1(img_,hog,filter_rows_padding,filter_cols_padding);
            return;
        }

        // unit vectors used to compute gradient orientation
        matrix<float,2,1> directions[9];
        directions[0] =  1.0000, 0.0000;
        directions[1] =  0.9397, 0.3420;
        directions[2] =  0.7660, 0.6428;
        directions[3] =  0.500,  0.8660;
        directions[4] =  0.1736, 0.9848;
        directions[5] = -0.1736, 0.9848;
        directions[6] = -0.5000, 0.8660;
        directions[7] = -0.7660, 0.6428;
        directions[8] = -0.9397, 0.3420;



        // First we allocate memory for caching orientation histograms & their norms.
        const int cells_nr = (int)((float)img.nr()/(float)cell_size + 0.5);
        const int cells_nc = (int)((float)img.nc()/(float)cell_size + 0.5);

        if (cells_nr == 0 || cells_nc == 0)
        {
            hog.clear();
            return;
        }

        // We give hist extra padding around the edges (1 cell all the way around the
        // edge) so we can avoid needing to do boundary checks when indexing into it
        // later on.  So some statements assign to the boundary but those values are
        // never used.
        array2d<matrix<float,18,1> > hist(cells_nr+2, cells_nc+2);
        for (long r = 0; r < hist.nr(); ++r)
        {
            for (long c = 0; c < hist.nc(); ++c)
            {
                hist[r][c] = 0;
            }
        }

        array2d<float> norm(cells_nr, cells_nc);
        assign_all_pixels(norm, 0);

        // memory for HOG features
        const int hog_nr = std::max(cells_nr-2, 0);
        const int hog_nc = std::max(cells_nc-2, 0);
        if (hog_nr == 0 || hog_nc == 0)
        {
            hog.clear();
            return;
        }
        const int padding_rows_offset = (filter_rows_padding-1)/2;
        const int padding_cols_offset = (filter_cols_padding-1)/2;
        init_hog(hog, hog_nr, hog_nc, filter_rows_padding, filter_cols_padding);

        const int visible_nr = std::min((long)cells_nr*cell_size,img.nr())-1;
        const int visible_nc = std::min((long)cells_nc*cell_size,img.nc())-1;

        // First populate the gradient histograms
        for (int y = 1; y < visible_nr; y++)
        {
            const float yp = ((float)y+0.5)/(float)cell_size - 0.5;
            const int iyp = (int)std::floor(yp);
            const float vy0 = yp - iyp;
            const float vy1 = 1.0 - vy0;
            int x;
            for (x = 1; x < visible_nc - 7; x += 8)
            {
                simd8f xx(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
                // v will be the length of the gradient vectors.
                simd8f grad_x, grad_y, v;
                get_gradient(y, x, img, grad_x, grad_y, v);

                // We will use bilinear interpolation to add into the histogram bins.
                // So first we precompute the values needed to determine how much each
                // pixel votes into each bin.
                simd8f xp = (xx + 0.5) / (float)cell_size + 0.5;
                simd8i ixp = simd8i(xp);
                simd8f vx0 = xp - ixp;
                simd8f vx1 = 1.0f - vx0;

                v = sqrt(v);

                // Now snap the gradient to one of 18 orientations
                simd8f best_dot = 0;
                simd8f best_o = 0;
                for (int o = 0; o < 9; o++)
                {
                    simd8f dot = grad_x*directions[o](0) + grad_y*directions[o](1);
                    simd8f_bool cmp = dot>best_dot;
                    best_dot = select(cmp, dot, best_dot);
                    dot *= -1;
                    best_o = select(cmp, o, best_o);

                    cmp = dot > best_dot;
                    best_dot = select(cmp, dot, best_dot);
                    best_o = select(cmp, o + 9, best_o);
                }


                // Add the gradient magnitude, v, to 4 histograms around pixel using
                // bilinear interpolation.
                vx1 *= v;
                vx0 *= v;
                // The amounts for each bin
                simd8f v11 = vy1*vx1;
                simd8f v01 = vy0*vx1;
                simd8f v10 = vy1*vx0;
                simd8f v00 = vy0*vx0;

                int32 _best_o[8]; simd8i(best_o).store(_best_o);
                int32 _ixp[8];    ixp.store(_ixp);
                float _v11[8];    v11.store(_v11);
                float _v01[8];    v01.store(_v01);
                float _v10[8];    v10.store(_v10);
                float _v00[8];    v00.store(_v00);

                hist[iyp + 1][_ixp[0]](_best_o[0]) += _v11[0];
                hist[iyp + 1 + 1][_ixp[0]](_best_o[0]) += _v01[0];
                hist[iyp + 1][_ixp[0] + 1](_best_o[0]) += _v10[0];
                hist[iyp + 1 + 1][_ixp[0] + 1](_best_o[0]) += _v00[0];

                hist[iyp + 1][_ixp[1]](_best_o[1]) += _v11[1];
                hist[iyp + 1 + 1][_ixp[1]](_best_o[1]) += _v01[1];
                hist[iyp + 1][_ixp[1] + 1](_best_o[1]) += _v10[1];
                hist[iyp + 1 + 1][_ixp[1] + 1](_best_o[1]) += _v00[1];

                hist[iyp + 1][_ixp[2]](_best_o[2]) += _v11[2];
                hist[iyp + 1 + 1][_ixp[2]](_best_o[2]) += _v01[2];
                hist[iyp + 1][_ixp[2] + 1](_best_o[2]) += _v10[2];
                hist[iyp + 1 + 1][_ixp[2] + 1](_best_o[2]) += _v00[2];

                hist[iyp + 1][_ixp[3]](_best_o[3]) += _v11[3];
                hist[iyp + 1 + 1][_ixp[3]](_best_o[3]) += _v01[3];
                hist[iyp + 1][_ixp[3] + 1](_best_o[3]) += _v10[3];
                hist[iyp + 1 + 1][_ixp[3] + 1](_best_o[3]) += _v00[3];

                hist[iyp + 1][_ixp[4]](_best_o[4]) += _v11[4];
                hist[iyp + 1 + 1][_ixp[4]](_best_o[4]) += _v01[4];
                hist[iyp + 1][_ixp[4] + 1](_best_o[4]) += _v10[4];
                hist[iyp + 1 + 1][_ixp[4] + 1](_best_o[4]) += _v00[4];

                hist[iyp + 1][_ixp[5]](_best_o[5]) += _v11[5];
                hist[iyp + 1 + 1][_ixp[5]](_best_o[5]) += _v01[5];
                hist[iyp + 1][_ixp[5] + 1](_best_o[5]) += _v10[5];
                hist[iyp + 1 + 1][_ixp[5] + 1](_best_o[5]) += _v00[5];

                hist[iyp + 1][_ixp[6]](_best_o[6]) += _v11[6];
                hist[iyp + 1 + 1][_ixp[6]](_best_o[6]) += _v01[6];
                hist[iyp + 1][_ixp[6] + 1](_best_o[6]) += _v10[6];
                hist[iyp + 1 + 1][_ixp[6] + 1](_best_o[6]) += _v00[6];

                hist[iyp + 1][_ixp[7]](_best_o[7]) += _v11[7];
                hist[iyp + 1 + 1][_ixp[7]](_best_o[7]) += _v01[7];
                hist[iyp + 1][_ixp[7] + 1](_best_o[7]) += _v10[7];
                hist[iyp + 1 + 1][_ixp[7] + 1](_best_o[7]) += _v00[7];
            }
            // Now process the right columns that don't fit into simd registers.
            for (; x < visible_nc; x++)
            {
                matrix<float, 2, 1> grad;
                float v;
                get_gradient(y,x,img,grad,v);

                // snap to one of 18 orientations
                float best_dot = 0;
                int best_o = 0;
                for (int o = 0; o < 9; o++)
                {
                    const float dotf = dot(directions[o], grad);
                    if (dotf > best_dot)
                    {
                        best_dot = dotf;
                        best_o = o;
                    }
                    else if (-dotf > best_dot)
                    {
                        best_dot = -dotf;
                        best_o = o+9;
                    }
                }

                v = std::sqrt(v);
                // add to 4 histograms around pixel using bilinear interpolation
                const float xp = ((double)x + 0.5) / (double)cell_size - 0.5;
                const int ixp = (int)std::floor(xp);
                const float vx0 = xp - ixp;
                const float vx1 = 1.0 - vx0;

                hist[iyp+1][ixp+1](best_o) += vy1*vx1*v;
                hist[iyp+1+1][ixp+1](best_o) += vy0*vx1*v;
                hist[iyp+1][ixp+1+1](best_o) += vy1*vx0*v;
                hist[iyp+1+1][ixp+1+1](best_o) += vy0*vx0*v;
            }
        }

        // compute energy in each block by summing over orientations
        for (int r = 0; r < cells_nr; ++r)
        {
            for (int c = 0; c < cells_nc; ++c)
            {
                for (int o = 0; o < 9; o++)
                {
                    norm[r][c] += (hist[r+1][c+1](o) + hist[r+1][c+1](o+9)) * (hist[r+1][c+1](o) + hist[r+1][c+1](o+9));
                }
            }
        }

        const float eps = 0.0001;
        // compute features
        for (int y = 0; y < hog_nr; y++)
        {
            const int yy = y+padding_rows_offset;
            for (int x = 0; x < hog_nc; x++)
            {
                const simd4f z1(norm[y+1][x+1],
                                norm[y][x+1],
                                norm[y+1][x],
                                norm[y][x]);

                const simd4f z2(norm[y+1][x+2],
                                norm[y][x+2],
                                norm[y+1][x+1],
                                norm[y][x+1]);

                const simd4f z3(norm[y+2][x+1],
                                norm[y+1][x+1],
                                norm[y+2][x],
                                norm[y+1][x]);

                const simd4f z4(norm[y+2][x+2],
                                norm[y+1][x+2],
                                norm[y+2][x+1],
                                norm[y+1][x+1]);

                const simd4f nn = 0.2*sqrt(z1+z2+z3+z4+eps);
                const simd4f n = 0.1/nn;

                simd4f t = 0;

                const int xx = x+padding_cols_offset;

                // contrast-sensitive features
                for (int o = 0; o < 18; o+=3)
                {
                    simd4f temp0(hist[y+1+1][x+1+1](o));
                    simd4f temp1(hist[y+1+1][x+1+1](o+1));
                    simd4f temp2(hist[y+1+1][x+1+1](o+2));
                    simd4f h0 = min(temp0,nn)*n;
                    simd4f h1 = min(temp1,nn)*n;
                    simd4f h2 = min(temp2,nn)*n;
                    set_hog(hog,o,xx,yy,   sum(h0));
                    set_hog(hog,o+1,xx,yy, sum(h1));
                    set_hog(hog,o+2,xx,yy, sum(h2));
                    t += h0+h1+h2;
                }

                t *= 2*0.2357;

                // contrast-insensitive features
                for (int o = 0; o < 9; o+=3)
                {
                    simd4f temp0 = hist[y+1+1][x+1+1](o)   + hist[y+1+1][x+1+1](o+9);
                    simd4f temp1 = hist[y+1+1][x+1+1](o+1) + hist[y+1+1][x+1+1](o+9+1);
                    simd4f temp2 = hist[y+1+1][x+1+1](o+2) + hist[y+1+1][x+1+1](o+9+2);
                    simd4f h0 = min(temp0,nn)*n;
                    simd4f h1 = min(temp1,nn)*n;
                    simd4f h2 = min(temp2,nn)*n;
                    set_hog(hog,o+18,xx,yy, sum(h0));
                    set_hog(hog,o+18+1,xx,yy, sum(h1));
                    set_hog(hog,o+18+2,xx,yy, sum(h2));
                }


                float temp[4];
                t.store(temp);

                // texture features
                set_hog(hog,27,xx,yy, temp[0]);
                set_hog(hog,28,xx,yy, temp[1]);
                set_hog(hog,29,xx,yy, temp[2]);
                set_hog(hog,30,xx,yy, temp[3]);
            }
        }
    }
} // end namespace impl_fhog



inline point image_to_fhog (
    point p,
    int cell_size = 8,
    int filter_rows_padding = 1,
    int filter_cols_padding = 1
)
{
    // make sure requires clause is not broken
    DLIB_ASSERT( cell_size > 0 &&
        filter_rows_padding > 0 &&
        filter_cols_padding > 0 ,
        "\t point image_to_fhog()"
        << "\n\t Invalid inputs were given to this function. "
        << "\n\t cell_size: " << cell_size
        << "\n\t filter_rows_padding: " << filter_rows_padding
        << "\n\t filter_cols_padding: " << filter_cols_padding
    );

    // There is a one pixel border around the image.
    p -= point(1,1);
    // There is also a 1 "cell" border around the HOG image formation.
    return p/cell_size - point(1,1) + point((filter_cols_padding-1)/2,(filter_rows_padding-1)/2);
}
inline rectangle image_to_fhog (
    const rectangle& rect,
    int cell_size = 8,
    int filter_rows_padding = 1,
    int filter_cols_padding = 1
)
{
    // make sure requires clause is not broken
    DLIB_ASSERT( cell_size > 0 &&
        filter_rows_padding > 0 &&
        filter_cols_padding > 0 ,
        "\t rectangle image_to_fhog()"
        << "\n\t Invalid inputs were given to this function. "
        << "\n\t cell_size: " << cell_size
        << "\n\t filter_rows_padding: " << filter_rows_padding
        << "\n\t filter_cols_padding: " << filter_cols_padding
    );

    return rectangle(image_to_fhog(rect.tl_corner(),cell_size,filter_rows_padding,filter_cols_padding),
                     image_to_fhog(rect.br_corner(),cell_size,filter_rows_padding,filter_cols_padding));
}
inline point fhog_to_image (
    point p,
    int cell_size = 8,
    int filter_rows_padding = 1,
    int filter_cols_padding = 1
)
{
    // make sure requires clause is not broken
    DLIB_ASSERT( cell_size > 0 &&
        filter_rows_padding > 0 &&
        filter_cols_padding > 0 ,
        "\t point fhog_to_image()"
        << "\n\t Invalid inputs were given to this function. "
        << "\n\t cell_size: " << cell_size
        << "\n\t filter_rows_padding: " << filter_rows_padding
        << "\n\t filter_cols_padding: " << filter_cols_padding
    );

    // Convert to image space and then set to the center of the cell.
    point offset;

    p = (p+point(1,1)-point((filter_cols_padding-1)/2,(filter_rows_padding-1)/2))*cell_size + point(1,1);
    if (p.x() >= 0 && p.y() >= 0) offset = point(cell_size/2,cell_size/2);
    if (p.x() <  0 && p.y() >= 0) offset = point(-cell_size/2,cell_size/2);
    if (p.x() >= 0 && p.y() <  0) offset = point(cell_size/2,-cell_size/2);
    if (p.x() <  0 && p.y() <  0) offset = point(-cell_size/2,-cell_size/2);
    return p + offset;
}

// ----------------------------------------------------------------------------------------

inline rectangle fhog_to_image (
    const rectangle& rect,
    int cell_size = 8,
    int filter_rows_padding = 1,
    int filter_cols_padding = 1
)
{
    // make sure requires clause is not broken
    DLIB_ASSERT( cell_size > 0 &&
        filter_rows_padding > 0 &&
        filter_cols_padding > 0 ,
        "\t rectangle fhog_to_image()"
        << "\n\t Invalid inputs were given to this function. "
        << "\n\t cell_size: " << cell_size
        << "\n\t filter_rows_padding: " << filter_rows_padding
        << "\n\t filter_cols_padding: " << filter_cols_padding
    );

    return rectangle(fhog_to_image(rect.tl_corner(),cell_size,filter_rows_padding,filter_cols_padding),
                     fhog_to_image(rect.br_corner(),cell_size,filter_rows_padding,filter_cols_padding));
}

// ----------------------------------------------------------------------------------------

    template <
        typename image_type, 
        typename T, 
        typename mm1, 
        typename mm2
        >
    void extract_fhog_features(
        const image_type& img, 
        array<array2d<T,mm1>,mm2>& hog,
        int cell_size = 8,
        int filter_rows_padding = 1,
        int filter_cols_padding = 1
    ) 
    {
        impl_fhog::impl_extract_fhog_features(img, hog, cell_size, filter_rows_padding, filter_cols_padding);
        // If the image is too small then the above function outputs an empty feature map.
        // But to make things very uniform in usage we require the output to still have the
        // 31 planes (but they are just empty).
        if (hog.size() == 0)
            hog.resize(31);
    }

    template <
        typename image_type, 
        typename T, 
        typename mm
        >
    void extract_fhog_features(
        const image_type& img, 
        array2d<matrix<T,31,1>,mm>& hog, 
        int cell_size = 8,
        int filter_rows_padding = 1,
        int filter_cols_padding = 1
    ) 
    {
        impl_fhog::impl_extract_fhog_features(img, hog, cell_size, filter_rows_padding, filter_cols_padding);
    }

// ----------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename T
        >
    void extract_fhog_features(
        const image_type& img, 
        matrix<T,0,1>& feats,
        int cell_size = 8,
        int filter_rows_padding = 1,
        int filter_cols_padding = 1
    )
    {
        array<array2d<T> > hog;
        extract_fhog_features(img, hog, cell_size, filter_rows_padding, filter_cols_padding);
        feats.set_size(hog.size()*hog[0].size());
        for (unsigned long i = 0; i < hog.size(); ++i)
        {
            const long size = hog[i].size();
            set_rowm(feats, range(i*size, (i+1)*size-1)) = reshape_to_column_vector(mat(hog[i]));
        }
    }

// ----------------------------------------------------------------------------------------

    template <
        typename image_type
        >
    matrix<double,0,1> extract_fhog_features(
        const image_type& img, 
        int cell_size = 8,
        int filter_rows_padding = 1,
        int filter_cols_padding = 1
    )
    {
        matrix<double, 0, 1> feats;
        extract_fhog_features(img, feats, cell_size, filter_rows_padding, filter_cols_padding);
        return feats;
    }

#endif // _DLIB_FHOG_H_
