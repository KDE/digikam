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
#ifndef DLIB_BOOST_UTILITY_ENABLE_IF_HPP
#define DLIB_BOOST_UTILITY_ENABLE_IF_HPP



template <bool B, class T = void>
struct enable_if_c {
  typedef T type;
};

template <class T>
struct enable_if_c<false, T> {};

template <class Cond, class T = void> 
struct enable_if : public enable_if_c<Cond::value, T> {};



template <bool B, class T>
struct lazy_enable_if_c {
  typedef typename T::type type;
};

template <class T>
struct lazy_enable_if_c<false, T> {};

template <class Cond, class T> 
struct lazy_enable_if : public lazy_enable_if_c<Cond::value, T> {};


template <bool B, class T = void>
struct disable_if_c {
  typedef T type;
};

template <class T>
struct disable_if_c<true, T> {};

template <class Cond, class T = void> 
struct disable_if : public disable_if_c<Cond::value, T> {};

template <bool B, class T>
struct lazy_disable_if_c {
  typedef typename T::type type;
};

template <class T>
struct lazy_disable_if_c<true, T> {};

template <class Cond, class T> 
struct lazy_disable_if : public lazy_disable_if_c<Cond::value, T> {};



#endif // DLIB_BOOST_UTILITY_ENABLE_IF_HPP

