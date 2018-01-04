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
#ifndef DLIB_ENTROPY_
#define DLIB_ENTROPY_


#include "entropy_decoder_kernel_2.h"
#include "entropy_decoder_model_kernel_5.h"
#include "entropy_encoder_kernel_2.h"
#include "entropy_encoder_model_kernel_5.h"




class entropy_decoder
{
    entropy_decoder() {}


public:

    typedef     entropy_decoder_kernel_2
                kernel_2a;
};

template <
    unsigned long alphabet_size,
    typename entropy_decoder
    >
class entropy_decoder_model
{
    entropy_decoder_model() {}

public:

    // kernel_5       
    typedef     entropy_decoder_model_kernel_5<alphabet_size,entropy_decoder,200000,4>
                kernel_5a;

};

class entropy_encoder
{
    entropy_encoder() {}


public:

    typedef     entropy_encoder_kernel_2
                kernel_2a;

};

template <
    unsigned long alphabet_size,
    typename entropy_encoder
    >
class entropy_encoder_model
{
    entropy_encoder_model() {}

public:        
    typedef     entropy_encoder_model_kernel_5<alphabet_size,entropy_encoder,200000,4>
                kernel_5a;



};

#endif