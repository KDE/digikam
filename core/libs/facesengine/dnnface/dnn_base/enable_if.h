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

