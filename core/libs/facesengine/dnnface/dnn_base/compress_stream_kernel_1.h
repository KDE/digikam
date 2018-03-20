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

#ifndef DLIB_COMPRESS_STREAM_KERNEl_1_
#define DLIB_COMPRESS_STREAM_KERNEl_1_

#include "algs.h"
#include <iostream>
#include <streambuf>
#include <cstdio>



template <
    typename fce,
    typename fcd,
    typename crc32
    >
class compress_stream_kernel_1
{
    /*!
        REQUIREMENTS ON fce
            is an implementation of entropy_encoder_model/entropy_encoder_model_kernel_abstract.h
            the alphabet_size of fce must be 257.
            fce and fcd share the same kernel number.

        REQUIREMENTS ON fcd
            is an implementation of entropy_decoder_model/entropy_decoder_model_kernel_abstract.h
            the alphabet_size of fcd must be 257.
            fce and fcd share the same kernel number.

        REQUIREMENTS ON crc32
            is an implementation of crc32/crc32_kernel_abstract.h



        INITIAL VALUE
            this object has no state

        CONVENTION
            this object has no state
    !*/

    const static unsigned long eof_symbol = 256;

public:

    class decompression_error : public error
    { 
        public: 
            decompression_error(
                const char* i
            ) :
                error(std::string(i))
            {}

            decompression_error(
                const std::string& i
            ) :
                error(i)
            {}
    };


    compress_stream_kernel_1 (
    )
    {}

    ~compress_stream_kernel_1 (
    )
    {}

    void compress (
        std::istream& in,
        std::ostream& out
    ) const;

    void decompress (
        std::istream& in,
        std::ostream& out
    ) const;

private:

    // restricted functions
    compress_stream_kernel_1(compress_stream_kernel_1&);        // copy constructor
    compress_stream_kernel_1& operator=(compress_stream_kernel_1&);    // assignment operator

};

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// member function definitions
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

template <
    typename fce,
    typename fcd,
    typename crc32
    >
void compress_stream_kernel_1<fce,fcd,crc32>::
compress (
    std::istream& in_,
    std::ostream& out_
) const
{
    std::streambuf::int_type temp;

    std::streambuf& in = *in_.rdbuf();

    typename fce::entropy_encoder_type coder;
    coder.set_stream(out_);

    fce model(coder);

    crc32 crc;

    unsigned long count = 0;

    while (true)
    {
        // write out a known value every 20000 symbols
        if (count == 20000)
        {
            count = 0;
            coder.encode(1500,1501,8000);
        }
        ++count;

        // get the next character
        temp = in.sbumpc();

        // if we have hit EOF then encode the marker symbol
        if (temp != EOF)  
        {
            // encode the symbol
            model.encode(static_cast<unsigned long>(temp));
            crc.add(static_cast<unsigned char>(temp));
            continue;
        }
        else
        {
            model.encode(eof_symbol);

            // now write the checksum
            unsigned long checksum = crc.get_checksum();
            unsigned char byte1 = static_cast<unsigned char>((checksum>>24)&0xFF);
            unsigned char byte2 = static_cast<unsigned char>((checksum>>16)&0xFF);
            unsigned char byte3 = static_cast<unsigned char>((checksum>>8)&0xFF);
            unsigned char byte4 = static_cast<unsigned char>((checksum)&0xFF);

            model.encode(byte1);
            model.encode(byte2);
            model.encode(byte3);
            model.encode(byte4);

            break;
        }
    }      
}

// ----------------------------------------------------------------------------------------

template <
    typename fce,
    typename fcd,
    typename crc32
    >
void compress_stream_kernel_1<fce,fcd,crc32>::
decompress (
    std::istream& in_,
    std::ostream& out_
) const
{

    std::streambuf& out = *out_.rdbuf();

    typename fcd::entropy_decoder_type coder;
    coder.set_stream(in_);

    fcd model(coder);

    unsigned long symbol;
    unsigned long count = 0;

    crc32 crc;

    // decode until we hit the marker symbol
    while (true)
    {
        // make sure this is the value we expect
        if (count == 20000)
        {
            if (coder.get_target(8000) != 1500)
            {
                throw decompression_error("Error detected in compressed data stream.");
            }
            count = 0;
            coder.decode(1500,1501);
        }
        ++count;

        // decode the next symbol
        model.decode(symbol);
        if (symbol != eof_symbol)
        {
            crc.add(static_cast<unsigned char>(symbol));
            // write this symbol to out
            if (out.sputc(static_cast<char>(symbol)) != static_cast<int>(symbol))
            {
                throw std::ios::failure("error occurred in compress_stream_kernel_1::decompress");
            }
            continue;
        }
        else
        {
            // we read eof from the encoded data.  now we just have to check the checksum and we are done.
            unsigned char byte1;
            unsigned char byte2;
            unsigned char byte3;
            unsigned char byte4;

            model.decode(symbol); byte1 = static_cast<unsigned char>(symbol);
            model.decode(symbol); byte2 = static_cast<unsigned char>(symbol);
            model.decode(symbol); byte3 = static_cast<unsigned char>(symbol);
            model.decode(symbol); byte4 = static_cast<unsigned char>(symbol);

            unsigned long checksum = byte1;
            checksum <<= 8;
            checksum |= byte2;
            checksum <<= 8;
            checksum |= byte3;
            checksum <<= 8;
            checksum |= byte4;

            if (checksum != crc.get_checksum())
                throw decompression_error("Error detected in compressed data stream.");

            break;
        }
    } // while (true)

}

// ----------------------------------------------------------------------------------------


#endif // DLIB_COMPRESS_STREAM_KERNEl_1_

