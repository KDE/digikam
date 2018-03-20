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

#ifndef DLIB_SIMd_CHECK_Hh_
#define DLIB_SIMd_CHECK_Hh_

//#define DLIB_DO_NOT_USE_SIMD

// figure out which SIMD instructions we can use.
#ifndef DLIB_DO_NOT_USE_SIMD
    #if defined(_MSC_VER) 
        #ifdef __AVX__
            #ifndef DLIB_HAVE_SSE2
                #define DLIB_HAVE_SSE2
            #endif 
            #ifndef DLIB_HAVE_SSE3
                #define DLIB_HAVE_SSE3
            #endif
            #ifndef DLIB_HAVE_SSE41
                #define DLIB_HAVE_SSE41
            #endif
            #ifndef DLIB_HAVE_AVX
                #define DLIB_HAVE_AVX
            #endif
        #endif
        #if (defined( _M_X64) || defined(_M_IX86_FP) && _M_IX86_FP >= 2) && !defined(DLIB_HAVE_SSE2)
            #define DLIB_HAVE_SSE2
        #endif
    #else
        #ifdef __SSE2__
            #ifndef DLIB_HAVE_SSE2
                #define DLIB_HAVE_SSE2
            #endif 
        #endif
        #ifdef __SSSE3__
            #ifndef DLIB_HAVE_SSE3
                #define DLIB_HAVE_SSE3
            #endif
        #endif
        #ifdef __SSE4_1__
            #ifndef DLIB_HAVE_SSE41
                #define DLIB_HAVE_SSE41
            #endif
        #endif
        #ifdef __AVX__
            #ifndef DLIB_HAVE_AVX
                #define DLIB_HAVE_AVX
            #endif
        #endif
        #ifdef __AVX2__
            #ifndef DLIB_HAVE_AVX2
                #define DLIB_HAVE_AVX2
            #endif
        #endif
        #ifdef __ALTIVEC__
            #ifndef DLIB_HAVE_ALTIVEC
                #define DLIB_HAVE_ALTIVEC
            #endif
        #endif
        #ifdef __VSX__
            #ifndef DLIB_HAVE_VSX
                #define DLIB_HAVE_VSX
            #endif
        #endif
        #ifdef __VEC__ // __VEC__ = 10206
            #ifndef DLIB_HAVE_POWER_VEC	// vector and vec_ intrinsics
                #define DLIB_HAVE_POWER_VEC
            #endif
        #endif

    #endif
#endif

 
// ----------------------------------------------------------------------------------------


#ifdef DLIB_HAVE_ALTIVEC
#include <altivec.h>
#endif

#ifdef DLIB_HAVE_SSE2
    #include <xmmintrin.h>
    #include <emmintrin.h>
    #include <mmintrin.h>
#endif
#ifdef DLIB_HAVE_SSE3
    #include <pmmintrin.h> // SSE3
    #include <tmmintrin.h>
#endif
#ifdef DLIB_HAVE_SSE41
    #include <smmintrin.h> // SSE4
#endif
#ifdef DLIB_HAVE_AVX
    #include <immintrin.h> // AVX
#endif
#ifdef DLIB_HAVE_AVX2
    #include <immintrin.h> // AVX
//    #include <avx2intrin.h>
#endif



#endif // DLIB_SIMd_CHECK_Hh_


