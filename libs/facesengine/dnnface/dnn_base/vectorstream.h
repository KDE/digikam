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

#ifndef DLIB_VECTORStREAM_Hh_
#define DLIB_VECTORStREAM_Hh_



#include <cstring>
#include <iostream>
#include <streambuf>
#include <vector>
#include <cstdio>
#include "algs.h"


#ifdef _MSC_VER
// Disable the warning about inheriting from std::iostream 'via dominance' since this warning is a warning about
// visual studio conforming to the standard and is ignorable.  See http://connect.microsoft.com/VisualStudio/feedback/details/733720/inheriting-from-std-fstream-produces-c4250-warning
// for further details if interested.
#pragma warning(disable : 4250)
#endif // _MSC_VER



class vectorstream : public std::iostream
{
    class vector_streambuf : public std::streambuf
    {
        typedef std::vector<char>::size_type size_type;
        size_type read_pos; // buffer[read_pos] == next byte to read from buffer
    public:
        std::vector<char>& buffer;

        vector_streambuf(
            std::vector<char>& buffer_
        ) :
            read_pos(0),
            buffer(buffer_) 
        {}


        void seekg(size_type pos)
        {
            read_pos = pos;
        }

        // ------------------------ OUTPUT FUNCTIONS ------------------------

        int_type overflow ( int_type c)
        {
            if (c != EOF) buffer.push_back(static_cast<char>(c));
            return c;
        }

        std::streamsize xsputn ( const char* s, std::streamsize num)
        {
            buffer.insert(buffer.end(), s, s+num);
            return num;
        }

        // ------------------------ INPUT FUNCTIONS ------------------------

        int_type underflow( 
        )
        {
            if (read_pos < buffer.size())
                return static_cast<unsigned char>(buffer[read_pos]);
            else
                return EOF;
        }

        int_type uflow( 
        )
        {   
            if (read_pos < buffer.size())
                return static_cast<unsigned char>(buffer[read_pos++]);
            else
                return EOF;
        }

        int_type pbackfail(
            int_type c
        )
        {  
            // if they are trying to push back a character that they didn't read last
            // that is an error
            const unsigned long prev = read_pos-1;
            if (c != EOF && prev < buffer.size() && 
                c != static_cast<unsigned char>(buffer[prev]))
            {
                return EOF;
            }

            read_pos = prev;
            return 1;
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
            return 0;
        }

    };

public:

    vectorstream (
        std::vector<char>& buffer
    ) :
        std::iostream(&buf),
        buf(buffer)
    {}

    std::istream& seekg (
        std::streampos pos
    )
    {
        buf.seekg(pos);
        return *this;
    }

private:
    vector_streambuf buf;
};

#endif // DLIB_VECTORStREAM_Hh_

