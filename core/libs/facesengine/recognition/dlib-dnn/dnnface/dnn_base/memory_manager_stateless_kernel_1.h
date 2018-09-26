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

#ifndef DLIB_MEMORY_MANAGER_STATELESs_1_
#define DLIB_MEMORY_MANAGER_STATELESs_1_

//#include "memory_manager_stateless_kernel_abstract.h"

template <
    typename T
    >
class memory_manager_stateless_kernel_1
{
    /*!      
        this implementation just calls new and delete directly
    !*/
    
    public:

        typedef T type;
        const static bool is_stateless = true;

        template <typename U>
        struct rebind {
            typedef memory_manager_stateless_kernel_1<U> other;
        };

        memory_manager_stateless_kernel_1(
        )
        {}

        virtual ~memory_manager_stateless_kernel_1(
        ) {}

        T* allocate (
        )
        {
            return new T; 
        }

        void deallocate (
            T* item
        )
        {
            delete item;
        }

        T* allocate_array (
            unsigned long size
        ) 
        { 
            return new T[size];
        }

        void deallocate_array (
            T* item
        ) 
        { 
            delete [] item;
        }

        void swap (memory_manager_stateless_kernel_1&)
        {}

    private:

        // restricted functions
        memory_manager_stateless_kernel_1(memory_manager_stateless_kernel_1&);        // copy constructor
        memory_manager_stateless_kernel_1& operator=(memory_manager_stateless_kernel_1&);    // assignment operator
};

template <
    typename T
    >
inline void swap (
    memory_manager_stateless_kernel_1<T>& a, 
    memory_manager_stateless_kernel_1<T>& b 
) { a.swap(b); }   


#endif // DLIB_MEMORY_MANAGER_STATELESs_1_



