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

#ifndef DLIB_FLOAT_DEtAILS_Hh_
#define DLIB_FLOAT_DEtAILS_Hh_

#include <cmath>
#include "algs.h"
#include <limits> 

struct float_details
{
    /*!
        WHAT THIS OBJECT REPRESENTS
            This object is a tool for converting floating point numbers into an
            explicit integer representation and then also converting back.  In
            particular, a float_details object represents a floating point number with
            a 64 bit mantissa and 16 bit exponent.  These are stored in the public
            fields of the same names.

            The main use of this object is to convert floating point values into a
            known uniform representation so they can be serialized to an output stream.
            This allows dlib serialization code to work on any system, regardless of
            the floating point representation used by the hardware.  It also means
            that, for example, a double can be serialized and then deserialized into a
            float and it will perform the appropriate conversion.


            In more detail, this object represents a floating point value equal to
            mantissa*pow(2,exponent), except when exponent takes on any of the
            following special values: 
                - is_inf
                - is_ninf
                - is_nan
            These values are used to indicate that the floating point value should be
            either infinity, negative infinity, or not-a-number respectively.
    !*/

    float_details(
        dint64 man,
        int16 exp
    ) : mantissa(man), exponent(exp) {}
    /*!
        ensures
            - #mantissa == man
            - #exponent == exp
    !*/

    float_details() :
        mantissa(0), exponent(0)
    {}
    /*!
        ensures
            - this object represents a floating point value of 0
    !*/

    float_details ( const double&      val) { *this = val; }
    float_details ( const float&       val) { *this = val; }
    float_details ( const long double& val) { *this = val; }
    /*!
        ensures
            - converts the given value into a float_details representation.  This 
              means that converting #*this back into a floating point number should
              recover the input val.
    !*/

    float_details& operator= ( const double&      val) { convert_from_T(val); return *this; }
    float_details& operator= ( const float&       val) { convert_from_T(val); return *this; }
    float_details& operator= ( const long double& val) { convert_from_T(val); return *this; }
    /*!
        ensures
            - converts the given value into a float_details representation.  This 
              means that converting #*this back into a floating point number should
              recover the input val.
    !*/

    operator double      () const { return convert_to_T<double>(); }
    operator float       () const { return convert_to_T<float>(); }
    operator long double () const { return convert_to_T<long double>(); }
    /*!
        ensures
            - converts the contents of this float_details object into a floating point number.
    !*/

    const static int16 is_inf  = 32000;
    const static int16 is_ninf = 32001;
    const static int16 is_nan  = 32002;

    dint64 mantissa;
    int16 exponent;


private:


// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
//                                  IMPLEMENTATION DETAILS 
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    template <typename T>
    void convert_from_T (
        const T& val
    )
    {
        mantissa = 0;

        const int digits = tmin<std::numeric_limits<T>::digits, 63>::value;

        if (val == std::numeric_limits<T>::infinity())
        {
            exponent = is_inf;
        }
        else if (val == -std::numeric_limits<T>::infinity())
        {
            exponent = is_ninf;
        }
        else if (val < std::numeric_limits<T>::infinity())
        {
            int exp;
            mantissa = static_cast<dint64>(std::frexp(val, &exp)*(((duint64)1)<<digits));
            exponent = exp - digits;

            // Compact the representation a bit by shifting off any low order bytes 
            // which are zero in the mantissa.  This makes the numbers in mantissa and
            // exponent generally smaller which can make serialization and other things
            // more efficient in some cases.
            for (int i = 0; i < 8 && ((mantissa&0xFF)==0); ++i)
            {
                mantissa >>= 8;
                exponent += 8;
            }
        }
        else
        {
            exponent = is_nan;
        }
    }

    template <typename T>
    T convert_to_T (
    ) const
    {
        if (exponent < is_inf)
            return std::ldexp((T)mantissa, exponent);
        else if (exponent == is_inf)
            return std::numeric_limits<T>::infinity();
        else if (exponent == is_ninf)
            return -std::numeric_limits<T>::infinity();
        else
            return std::numeric_limits<T>::quiet_NaN();
    }

};



#endif // DLIB_FLOAT_DEtAILS_Hh_

