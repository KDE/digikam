/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * Copyright (C) 2009 by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
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

// C++ includes.

#include <cstdio>
#include <cstdlib>
#include <cmath>

// Local includes.

#include "ToneMappingFloat.h"

namespace DigikamLocalContrastImagesPlugin
{

ToneMappingFloat::ToneMappingFloat()
                : ToneMappingBase()
{
    par.info_fast_mode = false;
}

ToneMappingFloat::~ToneMappingFloat()
{
}

void ToneMappingFloat::process_rgb_image(REALTYPE *img, int sizex, int sizey)
{
    update_preprocessed_values();
    int size            = sizex*sizey;
    REALTYPE *blurimage = new REALTYPE[size];
    REALTYPE *srcimg    = new REALTYPE[size*3];

    for (int i=0 ; i < (size*3) ; i++)
        srcimg[i] = img[i];

    if (par.stretch_contrast)
    {
        stretch_contrast(img,size*3);
    };

    int pos=0;
    for (int nstage=0 ; par.cancel() && (nstage < TONEMAPPING_MAX_STAGES) ; nstage++)
    {
        if (!par.stage[nstage].enabled) continue;

        //compute the desatured image
        pos=0;

        for (int i=0 ; par.cancel() && (i < size) ; i++)
        {
            blurimage[i] = (img[pos]+img[pos+1]+img[pos+2])/3.0;
            pos += 3;
        };

        current_process_power_value = par.get_power(nstage);

        //blur
        inplace_blur(blurimage, sizex,sizey, par.get_blur(nstage));

        pos=0;
        for (int i=0 ; par.cancel() && (i<size) ; i++)
        {
            REALTYPE src_r  = img[pos];
            REALTYPE src_g  = img[pos+1];
            REALTYPE src_b  = img[pos+2];

            REALTYPE blur   = blurimage[i];

            REALTYPE dest_r = func(src_r,blur);
            REALTYPE dest_g = func(src_g,blur);
            REALTYPE dest_b = func(src_b,blur);

            img[pos]        = dest_r;
            img[pos+1]      = dest_g;
            img[pos+2]      = dest_b;

            pos+=3;
        };
    };

    int high_saturation_value = 100-par.high_saturation;
    int low_saturation_value  = 100-par.low_saturation;

    if ((par.high_saturation != 100) || (par.low_saturation != 100))
    {
        int pos=0;
        for (int i=0 ; par.cancel() && (i < size) ; i++)
        {
            REALTYPE src_h, src_s, src_v;
            REALTYPE dest_h, dest_s, dest_v;
            rgb2hsv(srcimg[pos], srcimg[pos+1], srcimg[pos+2], src_h, src_s, src_v);
            rgb2hsv(img[pos], img[pos+1], img[pos+2], dest_h, dest_s, dest_v);

            REALTYPE dest_saturation = (src_s*high_saturation_value+dest_s*(100.0-high_saturation_value))*0.01;
            if (dest_v>src_v)
            {
                REALTYPE s1     = dest_saturation*src_v/(dest_v+1.0/255.0);
                dest_saturation = (low_saturation_value*s1+par.low_saturation*dest_saturation)*0.01;
            };

            hsv2rgb(dest_h, dest_saturation, dest_v, img[pos], img[pos+1], img[pos+2]);

            pos+=3;
        };
    };

    //Unsharp Mask filter
    if (par.unsharp_mask.enabled)
    {
        REALTYPE *val = new REALTYPE[size];
        //compute the desatured image
        int pos = 0;
        for (int i=0 ; par.cancel() && (i < size) ; i++)
        {
            val[i] = blurimage[i] = (img[pos]+img[pos+1]+img[pos+2])/3.0;
            //val[i]=blurimage[i]=max3(img[pos],img[pos+1],img[pos+2]);
            pos += 3;
        };

        REALTYPE blur_value = par.get_unsharp_mask_blur();
        inplace_blur(blurimage,sizex,sizey,blur_value);

        pos                 = 0;
        REALTYPE pow        = 2.5*par.get_unsharp_mask_power();
        REALTYPE threshold  = par.unsharp_mask.threshold*pow/250.0;
        REALTYPE threshold2 = threshold/2;

        for (int i=0 ; par.cancel() && (i < size) ; i++)
        {
            REALTYPE dval     = (val[i]-blurimage[i])*pow;
            REALTYPE abs_dval = fabs(dval);
            if (abs_dval < threshold)
            {
                if (abs_dval > threshold2)
                {
                    bool sign = (dval < 0.0);
                    dval      = (abs_dval-threshold2)*2.0;
                    if (sign) dval =- dval;
                }
                else
                {
                    dval=0;
                };
            };

            REALTYPE r   = img[pos]  +dval;
            REALTYPE g   = img[pos+1]+dval;
            REALTYPE b   = img[pos+2]+dval;

            if (r<0.0) r = 0.0;
            if (r>1.0) r = 1.0;
            if (g<0.0) g = 0.0;
            if (g>1.0) g = 1.0;
            if (b<0.0) b = 0.0;
            if (b>1.0) b = 1.0;

            img[pos]     = r;
            img[pos+1]   = g;
            img[pos+2]   = b;

            pos += 3;
        };

        delete [] val;
    };

    delete [] srcimg;
    delete [] blurimage;
}

void ToneMappingFloat::update_preprocessed_values()
{
}

void ToneMappingFloat::process_16bit_rgb_image(unsigned short int *img, int sizex, int sizey)
{
    int size              = sizex*sizey;
    REALTYPE *tmpimage    = new REALTYPE[size*3];
    const float inv_65536 = 1.0/65536.0;

    for (int i=0 ; par.cancel() && (i < size*3) ; i++)
    {
        //convert to floating point
        tmpimage[i] = img[i]/65535.0;
    };

    process_rgb_image(tmpimage, sizex, sizey);

    //convert back to 8 bits (with dithering)
    int pos = 0;

    for (int i=0 ; par.cancel() && (i < size) ; i++)
    {
        REALTYPE dither = ((rand()/65536)%65536)*inv_65536;
        img[pos]        = (int)(tmpimage[pos]  *65535.0+dither);
        img[pos+1]      = (int)(tmpimage[pos+1]*65535.0+dither);
        img[pos+2]      = (int)(tmpimage[pos+2]*65535.0+dither);
        pos+=3;
    };

    delete [] tmpimage;
}

void ToneMappingFloat::process_8bit_rgb_image(unsigned char *img, int sizex, int sizey)
{
    int size            = sizex*sizey;
    REALTYPE *tmpimage  = new REALTYPE[size*3];
    const float inv_256 = 1.0/256.0;

    for (int i=0 ; par.cancel() && (i < size*3) ; i++)
    {
        //convert to floating point
        tmpimage[i]=img[i]/255.0;
    };

    process_rgb_image(tmpimage, sizex, sizey);

    //convert back to 8 bits (with dithering)
    int pos=0;

    for (int i=0 ; par.cancel() && (i < size) ; i++)
    {
        REALTYPE dither = ((rand()/256)%256)*inv_256;
        img[pos]        = (int)(tmpimage[pos]  *255.0+dither);
        img[pos+1]      = (int)(tmpimage[pos+1]*255.0+dither);
        img[pos+2]      = (int)(tmpimage[pos+2]*255.0+dither);
        pos+=3;
    };

    delete [] tmpimage;
}

void ToneMappingFloat::inplace_blur(REALTYPE *data, int sizex, int sizey, REALTYPE blur)
{
    blur /= preview_zoom;

    if (blur < 0.3) return;

    REALTYPE a = exp(log(0.25)/blur);

    if ((a <= 0.0) || (a >= 1.0)) return;

    a *= a;
    REALTYPE denormal_remove = 1e-15;

    for (int stage=0 ; par.cancel() && (stage < 2) ; stage++)
    {
        for (int y=0 ; par.cancel() && (y < sizey) ; y++)
        {
            int pos      = y*sizex;
            REALTYPE old = data[pos];
            pos++;

            for (int x=1 ; par.cancel() && (x < sizex) ; x++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos++;
            };

            pos = y*sizex+sizex-1;

            for (int x=1 ; par.cancel() && (x < sizex) ; x++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos--;
            };
        };

        for (int x=0 ; par.cancel() && (x < sizex) ; x++)
        {
            int pos      = x;
            REALTYPE old = data[pos];

            for (int y=1 ; par.cancel() && (y < sizey) ; y++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos += sizex;
            };

            pos = x+sizex*(sizey-1);

            for (int y=1 ; par.cancel() && (y < sizey) ; y++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos -= sizex;
            };
        };
    };
}

void ToneMappingFloat::stretch_contrast(REALTYPE *data, int datasize)
{
    //stretch the contrast
    const unsigned int histogram_size=256;
    //first, we compute the histogram
    unsigned int histogram[histogram_size];

    for (unsigned int i=0 ; i < histogram_size ; i++) histogram[i] = 0;

    for (unsigned int i=0 ; par.cancel() && (i < (unsigned int)datasize) ; i++)
    {
        int m = (int)(data[i]*(histogram_size-1));
        if (m < 0) m = 0;
        if (m > (int)(histogram_size-1)) m = histogram_size-1;
        histogram[m]++;
    };

    //I want to strip the lowest and upper 0.1 procents (in the histogram) of the pixels
    int          min = 0,max = 255;
    unsigned int desired_sum = datasize/1000;
    unsigned int sum_min     = 0;
    unsigned int sum_max     = 0;

    for (unsigned int i=0 ; par.cancel() && (i < histogram_size) ; i++)
    {
        sum_min += histogram[i];
        if (sum_min > desired_sum)
        {
            min = i;
            break;
        };
    };

    for (int i = histogram_size-1 ; par.cancel() && (i >= 0) ; i--)
    {
        sum_max += histogram[i];
        if (sum_max > desired_sum)
        {
            max = i;
            break;
        };
    };

    if (min >= max)
    {
        min = 0;
        max = 255;
    };

    REALTYPE min_src_val = min/255.0;
    REALTYPE max_src_val = max/255.0;

    for (int i=0 ; par.cancel() && (i < datasize) ; i++)
    {
        //stretch the contrast
        REALTYPE x = data[i];
        x          = (x-min_src_val)/(max_src_val-min_src_val);
        if (x < 0.0) x = 0.0;
        if (x > 1.0) x = 1.0;
        data[i]    = x;
    };
}

} // namespace DigikamNoiseReductionImagesPlugin
