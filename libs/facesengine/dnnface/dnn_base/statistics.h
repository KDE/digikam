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

#ifndef _DLIB_STATISTICS_H_
#define _DLIB_STATISTICS_H_

#include <limits>
#include <cmath>
#include "algs.h"
#include "matrix.h"
#include "matrix_utilities.h"
#include "matrix_subexp.h"
#include "matrix_math_functions.h"
#include "matrix_generic_image.h"



template <
    typename T
    >
class running_stats
{
public:

    running_stats()
    {
        clear();

        COMPILE_TIME_ASSERT ((
                is_same_type<float,T>::value ||
                is_same_type<double,T>::value ||
                is_same_type<long double,T>::value 
        ));
    }

    void clear()
    {
        sum = 0;
        sum_sqr  = 0;
        sum_cub  = 0;
        sum_four = 0;

        n = 0;
        min_value = std::numeric_limits<T>::infinity();
        max_value = -std::numeric_limits<T>::infinity();
    }

    void add (
        const T& val
    )
    {
        sum      += val;
        sum_sqr  += val*val;
        sum_cub  += cubed(val);
        sum_four += quaded(val);

        if (val < min_value)
            min_value = val;
        if (val > max_value)
            max_value = val;

        ++n;
    }

    T current_n (
    ) const
    {
        return n;
    }

    T mean (
    ) const
    {
        if (n != 0)
            return sum/n;
        else
            return 0;
    }

    T max (
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(current_n() > 0,
            "\tT running_stats::max"
            << "\n\tyou have to add some numbers to this object first"
            << "\n\tthis: " << this
            );

        return max_value;
    }

    T min (
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(current_n() > 0,
            "\tT running_stats::min"
            << "\n\tyou have to add some numbers to this object first"
            << "\n\tthis: " << this
            );

        return min_value;
    }

    T variance (
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(current_n() > 1,
            "\tT running_stats::variance"
            << "\n\tyou have to add some numbers to this object first"
            << "\n\tthis: " << this
            );

        T temp = 1/(n-1);
        temp = temp*(sum_sqr - sum*sum/n);
        // make sure the variance is never negative.  This might
        // happen due to numerical errors.
        if (temp >= 0)
            return temp;
        else
            return 0;
    }

    T stddev (
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(current_n() > 1,
            "\tT running_stats::stddev"
            << "\n\tyou have to add some numbers to this object first"
            << "\n\tthis: " << this
            );

        return std::sqrt(variance());
    }

    T skewness (
    ) const
    {  
        // make sure requires clause is not broken
        DLIB_ASSERT(current_n() > 2,
            "\tT running_stats::skewness"
            << "\n\tyou have to add some numbers to this object first"
            << "\n\tthis: " << this
        );

        T temp  = 1/n;
        T temp1 = std::sqrt(n*(n-1))/(n-2); 
        temp    = temp1*temp*(sum_cub - 3*sum_sqr*sum*temp + 2*cubed(sum)*temp*temp)/
                  (std::sqrt(std::pow(temp*(sum_sqr-sum*sum*temp),3)));

        return temp; 
    }

    T ex_kurtosis (
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(current_n() > 3,
            "\tT running_stats::kurtosis"
            << "\n\tyou have to add some numbers to this object first"
            << "\n\tthis: " << this
        );

        T temp = 1/n;
        T m4   = temp*(sum_four - 4*sum_cub*sum*temp+6*sum_sqr*sum*sum*temp*temp
                 -3*quaded(sum)*cubed(temp));
        T m2   = temp*(sum_sqr-sum*sum*temp);
        temp   = (n-1)*((n+1)*m4/(m2*m2)-3*(n-1))/((n-2)*(n-3));

        return temp; 
    }

    T scale (
        const T& val
    ) const
    {
        // make sure requires clause is not broken
        DLIB_ASSERT(current_n() > 1,
            "\tT running_stats::variance"
            << "\n\tyou have to add some numbers to this object first"
            << "\n\tthis: " << this
            );
        return (val-mean())/std::sqrt(variance());
    }

    running_stats operator+ (
        const running_stats& rhs
    ) const
    {
        running_stats temp(*this);

        temp.sum += rhs.sum;
        temp.sum_sqr += rhs.sum_sqr;
        temp.sum_cub += rhs.sum_cub;
        temp.sum_four += rhs.sum_four;
        temp.n += rhs.n;
        temp.min_value = std::min(rhs.min_value, min_value);
        temp.max_value = std::max(rhs.max_value, max_value);
        return temp;
    }

    template <typename U>
    friend void serialize (
        const running_stats<U>& item, 
        std::ostream& out 
    );

    template <typename U>
    friend void deserialize (
        running_stats<U>& item, 
        std::istream& in
    ); 

private:
    T sum;
    T sum_sqr;
    T sum_cub;
    T sum_four;
    T n;
    T min_value;
    T max_value;

    T cubed  (const T& val) const {return val*val*val; }
    T quaded (const T& val) const {return val*val*val*val; }
};


#endif // _DLIB_STATISTICS_H_
