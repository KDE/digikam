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


#ifndef DLIB_UINtn_
#define DLIB_UINtn_

#include "dnn_assert.h"



    /*!
        duint64 is a typedef for an unsigned integer that is exactly 64 bits wide.
        uint32 is a typedef for an unsigned integer that is exactly 32 bits wide.
        uint16 is a typedef for an unsigned integer that is exactly 16 bits wide.
        uint8  is a typedef for an unsigned integer that is exactly 8  bits wide.

        dint64 is a typedef for an integer that is exactly 64 bits wide.
        int32 is a typedef for an integer that is exactly 32 bits wide.
        int16 is a typedef for an integer that is exactly 16 bits wide.
        int8  is a typedef for an integer that is exactly 8  bits wide.
    !*/


#ifdef __GNUC__
    typedef unsigned long long duint64;
    typedef long long dint64;
#elif defined(__BORLANDC__)
    typedef unsigned __int64 duint64;
    typedef __int64 dint64;
#elif defined(_MSC_VER)
    typedef unsigned __int64 duint64;
    typedef __int64 dint64;
#else
    typedef unsigned long long duint64;
    typedef long long dint64;
#endif

    typedef unsigned short uint16;
    typedef unsigned int   uint32;
    typedef unsigned char  uint8;

    typedef short int16;
    typedef int   int32;
    typedef char  int8;


    // make sure these types have the right sizes on this platform
/*
    COMPILE_TIME_ASSERT(sizeof(uint8)  == 1);
    COMPILE_TIME_ASSERT(sizeof(uint16) == 2);
    COMPILE_TIME_ASSERT(sizeof(uint32) == 4);
    COMPILE_TIME_ASSERT(sizeof(duint64) == 8);

    COMPILE_TIME_ASSERT(sizeof(int8)  == 1);
    COMPILE_TIME_ASSERT(sizeof(int16) == 2);
    COMPILE_TIME_ASSERT(sizeof(int32) == 4);
    COMPILE_TIME_ASSERT(sizeof(dint64) == 8);
*/


    template <typename T, size_t s = sizeof(T)>
    struct unsigned_type;
    template <typename T>
    struct unsigned_type<T,1> { typedef uint8 type; };
    template <typename T>
    struct unsigned_type<T,2> { typedef uint16 type; };
    template <typename T>
    struct unsigned_type<T,4> { typedef uint32 type; };
    template <typename T>
    struct unsigned_type<T,8> { typedef duint64 type; };
    /*!
        ensures
            - sizeof(unsigned_type<T>::type) == sizeof(T)
            - unsigned_type<T>::type is an unsigned integral type
    !*/

    template <typename T, typename U>
    T zero_extend_cast(
        const U val
    )
    /*!
        requires
            - U and T are integral types
        ensures
            - let ut be a typedef for unsigned_type<U>::type
            - return static_cast<T>(static_cast<ut>(val));
    !*/
    {
        typedef typename unsigned_type<U>::type ut;
        return static_cast<T>(static_cast<ut>(val));
    }


#endif // DLIB_UINtn_

