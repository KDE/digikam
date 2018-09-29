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

#ifndef DLIB_ARRAY2D_SERIALIZE_PIXEL_OvERLOADS_Hh_
#define DLIB_ARRAY2D_SERIALIZE_PIXEL_OvERLOADS_Hh_

#include "array2d_kernel.h"
#include "pixel.h"


// ----------------------------------------------------------------------------------------

    /*
        This file contains overloads of the serialize functions for array2d object
        for the case where they contain simple 8bit POD pixel types.  In these
        cases we can perform a much faster serialization by writing data in chunks
        instead of one pixel at a time (this avoids a lot of function call overhead
        inside the iostreams).
    */

// ----------------------------------------------------------------------------------------

template <
    typename mem_manager
    >
void deserialize (
    array2d<rgb_pixel,mem_manager>& item, 
    std::istream& in
)   
{
    try
    {
        COMPILE_TIME_ASSERT(sizeof(rgb_pixel) == 3);

        long nr, nc;
        deserialize(nr,in);
        deserialize(nc,in);

        // this is the newer serialization format
        if (nr < 0 || nc < 0)
        {
            nr *= -1;
            nc *= -1;
        }
        else
        {
            std::swap(nr,nc);
        }

        item.set_size(nr,nc);

        if (item.size() != 0)
            in.read((char*)&item[0][0], sizeof(rgb_pixel)*item.size());
    }
    catch (serialization_error e)
    { 
        item.clear();
        throw serialization_error(e.info + "\n   while deserializing object of type array2d"); 
    }
}

// ----------------------------------------------------------------------------------------

    template <
    typename mem_manager
    >
void deserialize (
    array2d<bgr_pixel,mem_manager>& item, 
    std::istream& in
)   
{
    try
    {
        COMPILE_TIME_ASSERT(sizeof(bgr_pixel) == 3);

        long nr, nc;
        deserialize(nr,in);
        deserialize(nc,in);

        // this is the newer serialization format
        if (nr < 0 || nc < 0)
        {
            nr *= -1;
            nc *= -1;
        }
        else
        {
            std::swap(nr,nc);
        }


        item.set_size(nr,nc);

        if (item.size() != 0)
            in.read((char*)&item[0][0], sizeof(bgr_pixel)*item.size());
    }
    catch (serialization_error e)
    { 
        item.clear();
        throw serialization_error(e.info + "\n   while deserializing object of type array2d"); 
    }
}

// ----------------------------------------------------------------------------------------

template <
    typename mem_manager
    >
void deserialize (
    array2d<hsi_pixel,mem_manager>& item, 
    std::istream& in
)   
{
    try
    {
        COMPILE_TIME_ASSERT(sizeof(hsi_pixel) == 3);

        long nr, nc;
        deserialize(nr,in);
        deserialize(nc,in);

        // this is the newer serialization format
        if (nr < 0 || nc < 0)
        {
            nr *= -1;
            nc *= -1;
        }
        else
        {
            std::swap(nr,nc);
        }


        item.set_size(nr,nc);

        if (item.size() != 0)
            in.read((char*)&item[0][0], sizeof(hsi_pixel)*item.size());
    }
    catch (serialization_error e)
    { 
        item.clear();
        throw serialization_error(e.info + "\n   while deserializing object of type array2d"); 
    }
}

// ----------------------------------------------------------------------------------------

template <
    typename mem_manager
    >
void deserialize (
    array2d<rgb_alpha_pixel,mem_manager>& item, 
    std::istream& in
)   
{
    try
    {
        COMPILE_TIME_ASSERT(sizeof(rgb_alpha_pixel) == 4);

        long nr, nc;
        deserialize(nr,in);
        deserialize(nc,in);

        // this is the newer serialization format
        if (nr < 0 || nc < 0)
        {
            nr *= -1;
            nc *= -1;
        }
        else
        {
            std::swap(nr,nc);
        }


        item.set_size(nr,nc);

        if (item.size() != 0)
            in.read((char*)&item[0][0], sizeof(rgb_alpha_pixel)*item.size());
    }
    catch (serialization_error e)
    { 
        item.clear();
        throw serialization_error(e.info + "\n   while deserializing object of type array2d"); 
    }
}

// ----------------------------------------------------------------------------------------

template <
    typename mem_manager
    >
void deserialize (
    array2d<unsigned char,mem_manager>& item, 
    std::istream& in
)   
{
    try
    {
        long nr, nc;
        deserialize(nr,in);
        deserialize(nc,in);
        // this is the newer serialization format
        if (nr < 0 || nc < 0)
        {
            nr *= -1;
            nc *= -1;
        }
        else
        {
            std::swap(nr,nc);
        }


        item.set_size(nr,nc);

        if (item.size() != 0)
            in.read((char*)&item[0][0], sizeof(unsigned char)*item.size());
    }
    catch (serialization_error e)
    { 
        item.clear();
        throw serialization_error(e.info + "\n   while deserializing object of type array2d"); 
    }
}



#endif // DLIB_ARRAY2D_SERIALIZE_PIXEL_OvERLOADS_Hh_

