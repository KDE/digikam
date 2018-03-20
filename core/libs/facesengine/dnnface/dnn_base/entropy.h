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
