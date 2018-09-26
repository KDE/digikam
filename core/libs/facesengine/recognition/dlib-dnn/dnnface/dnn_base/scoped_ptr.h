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

#ifndef DLIB_SCOPED_PTr_
#define DLIB_SCOPED_PTr_ 

#include <algorithm>
#include "noncopyable.h"
#include "algs.h"




// ----------------------------------------------------------------------------------------

template <typename T>
struct default_deleter
{
    void operator() (T* item) const
    {
        delete item;
    }
};

template <typename T>
struct default_deleter<T[]>
{
    void operator() (T* item) const
    {
        delete [] item;
    }
};

// ----------------------------------------------------------------------------------------

template <
    typename T,
    typename deleter = default_deleter<T>
    > 
class scoped_ptr : noncopyable 
{
    /*!
        CONVENTION
            - get() == ptr
    !*/

public:
    typedef T element_type;
    typedef deleter deleter_type;

    explicit scoped_ptr (
        T* p = 0
    ) : ptr(p) { }

    ~scoped_ptr() 
    { 
        if (ptr) 
        {
            deleter del;
            del(ptr); 
        }
    }

    void reset (
        T* p = 0
    ) 
    { 
        if (ptr) 
        {
            deleter del;
            del(ptr); 
        }

        ptr = p;
    }

    T& operator*() const
    {
        /*
        DLIB_ASSERT(get() != 0,
                    "\tscoped_ptr::operator*()"
                    << "\n\tget() can't be null if you are going to dereference it"
                    << "\n\tthis: " << this
        );
        */

        return *ptr;
    }

    T* operator->() const
    {
        /*
        DLIB_ASSERT(get() != 0,
                    "\tscoped_ptr::operator*()"
                    << "\n\tget() can't be null"
                    << "\n\tthis: " << this
        );
        */

        return ptr;
    }

    T* get() const
    {
        return ptr;
    }

    operator bool() const
    {
        return (ptr != 0);
    }

    void swap(
        scoped_ptr& b
    )
    {
        std::swap(ptr,b.ptr);
    }

private:

    T* ptr;
};

// ----------------------------------------------------------------------------------------

template <
    typename T,
    typename deleter 
    > 
class scoped_ptr<T[],deleter> : noncopyable 
{
    /*!
        CONVENTION
            - get() == ptr
    !*/

public:
    typedef T element_type;

    explicit scoped_ptr (
        T* p = 0
    ) : ptr(p) { }

    ~scoped_ptr() 
    { 
        if (ptr) 
        {
            deleter del;
            del(ptr); 
        }
    }

    void reset (
        T* p = 0
    ) 
    { 
        if (ptr) 
        {
            deleter del;
            del(ptr); 
        }
        ptr = p;
    }

    T& operator[] (
        unsigned long idx
    ) const
    {
        /*
        DLIB_ASSERT(get() != 0,
                    "\tscoped_ptr::operator[]()"
                    << "\n\tget() can't be null if you are going to dereference it"
                    << "\n\tthis: " << this
        );
        */

        return ptr[idx];
    }

    T* get() const
    {
        return ptr;
    }

    operator bool() const
    {
        return (ptr != 0);
    }

    void swap(
        scoped_ptr& b
    )
    {
        std::swap(ptr,b.ptr);
    }

private:

    T* ptr;
};

// ----------------------------------------------------------------------------------------

template <
    typename T,
    typename deleter
    > 
void swap(
    scoped_ptr<T,deleter>& a, 
    scoped_ptr<T,deleter>& b
)
{
    a.swap(b);
}

// ----------------------------------------------------------------------------------------


#endif // DLIB_SCOPED_PTr_


