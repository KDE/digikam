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



