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

#ifndef DLIB_AnY_H_
#define DLIB_AnY_H_

#include "algs.h"
#include "scoped_ptr.h"
#include <typeinfo>

class bad_any_cast : public std::bad_cast
{
public:
      virtual const char * what() const throw()
      {
          return "bad_any_cast";
      }
};

// ----------------------------------------------------------------------------------------

class any
{

public:

    any()
    {
    }

    any (
        const any& item
    )
    {
        if (item.data)
        {
            item.data->copy_to(data);
        }
    }

    template <typename T>
    any (
        const T& item
    )
    {
        typedef typename basic_type<T>::type U;
        data.reset(new derived<U>(item));
    }

    void clear (
    )
    {
        data.reset();
    }

    template <typename T>
    bool contains (
    ) const
    {
        typedef typename basic_type<T>::type U;
        return dynamic_cast<derived<U>*>(data.get()) != 0;
    }

    bool is_empty(
    ) const
    {
        return data.get() == 0;
    }

    template <typename T>
    T& cast_to(
    )
    {
        typedef typename basic_type<T>::type U;
        derived<U>* d = dynamic_cast<derived<U>*>(data.get());
        if (d == 0)
        {
            throw bad_any_cast();
        }

        return d->item;
    }

    template <typename T>
    const T& cast_to(
    ) const
    {
        typedef typename basic_type<T>::type U;
        derived<U>* d = dynamic_cast<derived<U>*>(data.get());
        if (d == 0)
        {
            throw bad_any_cast();
        }

        return d->item;
    }

    template <typename T>
    T& get(
    )
    {
        typedef typename basic_type<T>::type U;
        derived<U>* d = dynamic_cast<derived<U>*>(data.get());
        if (d == 0)
        {
            d = new derived<U>();
            data.reset(d);
        }

        return d->item;
    }

    any& operator= (
        const any& item
    )
    {
        any(item).swap(*this);
        return *this;
    }

    void swap (
        any& item
    )
    {
        data.swap(item.data);
    }

private:

    struct base
    {
        virtual ~base() {}

        virtual void copy_to (
            scoped_ptr<base>& dest
        ) const = 0;
    };

    template <typename T>
    struct derived : public base
    {
        T item;
        derived() {}
        derived(const T& val) : item(val) {}

        virtual void copy_to (
            scoped_ptr<base>& dest
        ) const
        {
            dest.reset(new derived<T>(item));
        }
    };

    scoped_ptr<base> data;
};



// ----------------------------------------------------------------------------------------

inline void swap (
    any& a,
    any& b
) { a.swap(b); }

// ----------------------------------------------------------------------------------------

template <typename T> T& any_cast(any& a) { return a.cast_to<T>(); }
template <typename T> const T& any_cast(const any& a) { return a.cast_to<T>(); }

// ----------------------------------------------------------------------------------------


    



#endif // DLIB_AnY_H_
