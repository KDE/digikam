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

#ifndef DLIB_MAP_PAIr_INTERFACE_
#define DLIB_MAP_PAIr_INTERFACE_


// ----------------------------------------------------------------------------------------
    
template <
    typename T1,
    typename T2
    >
class map_pair  
{
    /*!
        POINTERS AND REFERENCES TO INTERNAL DATA
            None of the functions in map_pair will invalidate
            pointers or references to internal data when called.

        WHAT THIS OBJECT REPRESENTS
            this object is used to return the key/value pair used in the 
            map and hash_map containers when using the enumerable interface.

            note that the enumerable interface is defined in
            interfaces/enumerable.h
    !*/

public:
    typedef T1 key_type;
    typedef T2 value_type;

    virtual ~map_pair(
    )=0;

    virtual const T1& key( 
    ) const =0;
    /*!
        ensures
            - returns a const reference to the key
    !*/

    virtual const T2& value(
    ) const =0;
    /*!
        ensures
            - returns a const reference to the value associated with key
    !*/

    virtual T2& value(
    )=0;
    /*!
        ensures
            - returns a non-const reference to the value associated with key
    !*/

protected:

    // restricted functions
    map_pair<T1,T2>& operator=(const map_pair<T1,T2>&) {return *this;} // no assignment operator

};

// destructor does nothing
template <typename T1,typename T2> 
map_pair<T1,T2>::~map_pair () {}

// ----------------------------------------------------------------------------------------


#endif // DLIB_MAP_PAIr_INTERFACE_

