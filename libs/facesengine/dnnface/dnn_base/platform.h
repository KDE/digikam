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

