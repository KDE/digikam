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

