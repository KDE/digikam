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


