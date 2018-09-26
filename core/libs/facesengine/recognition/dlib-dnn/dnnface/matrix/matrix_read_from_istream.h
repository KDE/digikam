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

#ifndef DLIB_MATRIx_READ_FROM_ISTREAM_H_h_
#define DLIB_MATRIx_READ_FROM_ISTREAM_H_h_

#include "matrix.h"
#include <vector>
#include <iostream>

//namespace dlib
//{

// ----------------------------------------------------------------------------------------

    namespace impl
    {
        inline bool next_is_whitespace (
            std::istream& in
        )
        {
            return in.peek() == '\n' ||
                in.peek() == ' ' || 
                in.peek() == ',' || 
                in.peek() == '\t' ||
                in.peek() == '\r';
        }
    }

    template <typename T, long NR, long NC, typename MM, typename L>
    std::istream& operator>> (
        std::istream& in,
        matrix<T,NR,NC,MM,L>& m
    )
    {
        using namespace impl;
        long num_rows = 0;
        std::vector<T> buf;
        buf.reserve(100);

        // eat any leading whitespace
        while (next_is_whitespace(in))
            in.get();

        bool at_start_of_line = true;
        bool stop = false;
        while(!stop && in.peek() != EOF)
        {
            T temp;
            in >> temp;
            if (!in)
                return in;

            buf.push_back(temp);
            if (at_start_of_line)
            {
                at_start_of_line = false;
                ++num_rows;
            }

            // Eat next block of whitespace but also note if we hit the start of the next
            // line. 
            while (next_is_whitespace(in))
            {
                if (at_start_of_line && in.peek() == '\n')
                {
                    stop = true;
                    break;
                }

                if (in.get() == '\n')
                    at_start_of_line = true;
            }
        }

        // It's an error for there to not be any matrix data in the input stream
        if (num_rows == 0)
        {
            in.clear(in.rdstate() | std::ios::failbit);
            return in;
        }

        const long num_cols = buf.size()/num_rows;
        // It's also an error if the sizes don't make sense.
        if (num_rows*num_cols != (long)buf.size() ||
            (NR != 0 && NR != num_rows) ||
            (NC != 0 && NC != num_cols))
        {
            in.clear(in.rdstate() | std::ios::failbit);
            return in;
        }


        m = reshape(mat(buf),num_rows, buf.size()/num_rows);

        if (in.eof())
        {
            // Clear the eof and fail bits since this is caused by peeking at the EOF.
            // But in the current case, we have successfully read the matrix.
            in.clear(in.rdstate() & (~(std::ios::eofbit | std::ios::failbit)));
        }
        return in;
    }
//}

// ----------------------------------------------------------------------------------------

#endif // DLIB_MATRIx_READ_FROM_ISTREAM_H_h_

