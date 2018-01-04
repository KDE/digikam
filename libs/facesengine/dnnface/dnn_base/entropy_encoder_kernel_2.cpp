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
#ifndef DLIB_ENTROPY_ENCODER_KERNEL_2_CPp_
#define DLIB_ENTROPY_ENCODER_KERNEL_2_CPp_
#include "entropy_encoder_kernel_2.h"
#include <iostream>
#include <streambuf>




// ----------------------------------------------------------------------------------------

entropy_encoder_kernel_2::
entropy_encoder_kernel_2(
) :
    initial_low(0x00000001),
    initial_high(0xffffffff),
    out(0),
    low(initial_low),
    high(initial_high)
{
    streambuf = 0;
}

// ----------------------------------------------------------------------------------------

entropy_encoder_kernel_2::
~entropy_encoder_kernel_2 (
)
{
    try {
        if (out != 0)
        {
            flush();
        }
    } catch (...) {}
}

// ----------------------------------------------------------------------------------------

void entropy_encoder_kernel_2::
clear(
)
{
    if (out != 0)
    {
        flush();
    }
    out = 0;
}

// ----------------------------------------------------------------------------------------

void entropy_encoder_kernel_2::
set_stream (
    std::ostream& out_
)
{
    if (out != 0)
    {
        // if a stream is currently set then flush the buffers to it before
        // we switch to the new stream
        flush();
    }

    out = &out_;
    streambuf = out_.rdbuf();

    // reset the encoder state
    low = initial_low;
    high = initial_high;
}

// ----------------------------------------------------------------------------------------

bool entropy_encoder_kernel_2::
stream_is_set (
) const
{
    if (out != 0)
        return true;
    else
        return false;
}

// ----------------------------------------------------------------------------------------

std::ostream& entropy_encoder_kernel_2::
get_stream (
) const
{
    return *out;
}

// ----------------------------------------------------------------------------------------

void entropy_encoder_kernel_2::
encode (
    uint32 low_count,
    uint32 high_count,
    uint32 total
)
{
    // note that we must add one because of the convention that
    // high == the real upper range minus 1
    uint32 r = (high-low+1)/total;                 

    // note that we must subtract 1 to preserve the convention that
    // high == the real upper range - 1
    high = low + r*high_count-1;
    low = low + r*low_count;



    while (true )
    {

        // if high and low don't have the same 8 high order bits
        if ((high&0xFF000000) != (low&0xFF000000)) 
        {   
            // if the distance between high and low is small and there aren't
            // any bits we can roll off then force high and low to have common high 
            // order bits.
            if ((high-low < 0x10000))
            {
                if (high-low > 0x1000)
                {
                    high>>=1;
                    low>>=1;
                    high = low = high+low;
                    high += 0xFF;
                    low -= 0xFF;
                } 
                else /**/
                {
                    high>>=1;
                    low>>=1;
                    high = low = high+low;
                }
            }
            else
            {
                // there are no bits to roll off and high and low are not
                // too close so just quit the loop
                break;
            }
            
        }  
        // else if there are 8 bits we can roll off
        else
        {
            // write the 8 high order bits from low into buf
            unsigned char buf = static_cast<unsigned char>(low>>24);


            // roll off the bits we just wrote to buf
            high <<= 8;  
            low <<= 8;               
            high |= 0xFF;  // note that it is ok to add 0xFF to high here because
                        // of the convention that high == real upper range - 1.
                        // so that means that if we want to shift the upper range
                        // left by one then we must shift a one into high also
                        // since real upper range == high + 0.999999999...

            // make sure low is never zero
            if (low == 0)
                low = 1;

            // write buf to the output stream
            if (streambuf->sputn(reinterpret_cast<char*>(&buf),1)==0)
            {
                throw std::ios_base::failure("error occured in the entropy_encoder object");
            }                   
            
        } 

    } // while (true)

}

// ----------------------------------------------------------------------------------------

void entropy_encoder_kernel_2::
flush (
)
{

    // flush low to the output stream


    unsigned char buf;


    buf = static_cast<unsigned char>((low >> 24)&0xFF);
    if (streambuf->sputn(reinterpret_cast<char*>(&buf),1) == 0)
        throw std::ios_base::failure("error occured in the entropy_encoder object");




    buf = static_cast<unsigned char>((low >> 16)&0xFF);
    if (streambuf->sputn(reinterpret_cast<char*>(&buf),1)==0)
        throw std::ios_base::failure("error occured in the entropy_encoder object");



    buf = static_cast<unsigned char>((low >> 8)&0xFF);
    if (streambuf->sputn(reinterpret_cast<char*>(&buf),1)==0)
        throw std::ios_base::failure("error occured in the entropy_encoder object");


    buf = static_cast<unsigned char>((low)&0xFF);
    if (streambuf->sputn(reinterpret_cast<char*>(&buf),1)==0)
        throw std::ios_base::failure("error occured in the entropy_encoder object");
    


    
    // make sure the stream buffer flushes to its I/O channel
    streambuf->pubsync();


    // reset the encoder state
    low = initial_low;
    high = initial_high;
}

// ----------------------------------------------------------------------------------------


#endif // DLIB_ENTROPY_ENCODER_KERNEL_2_CPp_

