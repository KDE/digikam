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


#ifdef DLIB_ALL_SOURCE_END
#include "dlib_basic_cpp_build_tutorial.txt"
#endif

#ifndef DLIB_PLATFORm_
#define DLIB_PLATFORm_


/*!
    This file ensures that:
        - if (we are compiling under a posix platform) then
            - POSIX will be defined
            - if (this is also Mac OS X) then
                - MACOSX will be defined
            - if (this is also HP-UX) then
                - HPUX will be defined
        - if (we are compiling under an MS Windows platform) then
            - WIN32 will be defined
!*/


/*
    A good reference for this sort of information is
    http://predef.sourceforge.net/
*/

// Define WIN32 if this is MS Windows
#ifndef WIN32
    #if defined( _MSC_VER) || defined(__BORLANDC__) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
    #define WIN32 
    #endif 
#endif

#ifndef WIN32
    // since this is the only other platform the library currently supports
    // just assume it is POSIX if it isn't WIN32
    #ifndef POSIX
        #define POSIX
    #endif
 
    #ifndef HPUX
       #if defined(__hpux ) || defined(hpux) || defined (_hpux)
       #define HPUX
       #endif	
    #endif

    #ifndef MACOSX
        #ifdef __MACOSX__
        #define MACOSX
        #endif 
        #ifdef __APPLE__
        #define MACOSX
        #endif
    #endif

#endif




#endif // DLIB_PLATFORm_

