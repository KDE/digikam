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

#ifndef DLIB_uNSERIALIZE_Hh_
#define DLIB_uNSERIALIZE_Hh_

//#include "unserialize_abstract.h"

#include "serialize.h"
#include "algs.h"
#include "vectorstream.h"




class unserialize : public std::istream
{
    class mystreambuf : public std::streambuf
    {
        typedef std::vector<char>::size_type size_type;
        size_type read_pos; // buffer[read_pos] == next byte to read from buffer
    public:
        std::vector<char> buffer;
        std::istream& str;

        template <typename T>
        mystreambuf(
            const T& item,
            std::istream& str_
        ) :
            read_pos(0),
            str(str_) 
        {
            // put the item into our buffer.
            vectorstream vstr(buffer);
            serialize(item, vstr);
        }


        // ------------------------ INPUT FUNCTIONS ------------------------

        int_type underflow( 
        )
        {
            if (read_pos < buffer.size())
                return static_cast<unsigned char>(buffer[read_pos]);
            else
                return str.peek();
        }

        int_type uflow( 
        )
        {   
            if (read_pos < buffer.size())
                return static_cast<unsigned char>(buffer[read_pos++]);
            else
                return str.get();
        }

        std::streamsize xsgetn (
            char* s, 
            std::streamsize n
        )
        { 
            if (read_pos < buffer.size())
            {
                const size_type num = std::min<size_type>(n, buffer.size()-read_pos);
                std::memcpy(s, &buffer[read_pos], num);
                read_pos += num;
                return num;
            }
            else
            {
                return str.rdbuf()->sgetn(s,n);
            }
            return 0;
        }

    };

public:

    template <typename T>
    unserialize (
        const T& item,
        std::istream& str 
    ) :
        std::istream(&buf),
        buf(item, str)
    {}

private:
    mystreambuf buf;
};

#endif // DLIB_uNSERIALIZE_Hh_

