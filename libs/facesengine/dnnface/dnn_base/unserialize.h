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

