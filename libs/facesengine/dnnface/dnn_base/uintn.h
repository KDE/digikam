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

