/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * Copyright (C) 2009 by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <cstring>
#include <cstdlib>
#include <cmath>

// Local includes.

#include "tonemapping.h"

namespace Digikam
{

ToneMapping::ToneMapping()
{
    m_current_process_power_value = 20.0;
    m_preview_zoom                = 1.0;
}

ToneMapping::~ToneMapping()
{
    delete m_par;
}

void ToneMapping::set_blur(int nstage, float value)
{
    if (value < 0) value = 0;
    if (value > 10000.0) value = 10000.0;
    m_par->stage[nstage].blur = value;
}

void ToneMapping::set_power(int nstage, float value)
{
    if (value < 0) value = 0;
    if (value > 100.0) value = 100.0;
    m_par->stage[nstage].power = value;
}

void ToneMapping::set_low_saturation(int value)
{
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    m_par->low_saturation = value;
}

void ToneMapping::set_high_saturation(int value)
{
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    m_par->high_saturation = value;
}

void ToneMapping::set_stretch_contrast(bool value)
{
    m_par->stretch_contrast = value;
}

void ToneMapping::set_function_id (int value)
{
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    m_par->function_id = value;
}

float ToneMapping::func(float x1, float x2)
{
    float result = 0.5;
    float p;

    /*
    //test function
    if (m_par->function_id==1)
    {
        p=pow(0.1,fabs((x2*2.0-1.0))*m_current_process_power_value*0.02);
        if (x2<0.5) result=pow(x1,p);
        else result=1.0-pow(1.0-x1,p);
        return result;
    };
    //test function
    if (function_id==1)
    {
        p=m_current_process_power_value*0.3+1e-4;
        x2=1.0/(1.0+exp(-(x2*2.0-1.0)*p*0.5));
        float f=1.0/(1.0+exp((1.0-(x1-x2+0.5)*2.0)*p));
        float m0=1.0/(1.0+exp((1.0-(-x2+0.5)*2.0)*p));
        float m1=1.0/(1.0+exp((1.0-(-x2+1.5)*2.0)*p));
        result=(f-m0)/(m1-m0);
        return result;
    };
    */

    switch (m_par->function_id)
    {
        case 0:  //power function
            p = (float)(pow((double)10.0,(double)fabs((x2*2.0-1.0))*m_current_process_power_value*0.02));
            if (x2 >= 0.5) result = pow(x1,p);
            else result = (float)(1.0-pow((double)1.0-x1,(double)p));
            break;
        case 1:  //linear function
            p = (float)(1.0/(1+exp(-(x2*2.0-1.0)*m_current_process_power_value*0.04)));
            result = (x1 < p) ? (float)(x1*(1.0-p)/p) : (float)((1.0-p)+(x1-p)*p/(1.0-p));
            break;
    };

    return result;
}

void ToneMapping::apply_parameters(ToneMappingParameters* par)
{
    m_par = par;
    set_low_saturation(m_par->low_saturation);
    set_high_saturation(m_par->high_saturation);
    set_stretch_contrast(m_par->stretch_contrast);
    set_function_id(m_par->function_id);

    for (int i=0 ; i < TONEMAPPING_MAX_STAGES ; i++)
    {
        set_power(i, m_par->stage[i].power);
        set_blur(i, m_par->stage[i].blur);
    };

    update_preprocessed_values();
}

void ToneMapping::process_rgb_image(float* img, int sizex, int sizey)
{
    update_preprocessed_values();

    int size         = sizex*sizey;
    float* blurimage = new float[size];
    float* srcimg    = new float[size*3];

    for (int i=0 ; i < (size*3) ; i++)
        srcimg[i] = img[i];

    if (m_par->stretch_contrast)
    {
        stretch_contrast(img, size*3);
    }

    int pos = 0;

    for (int nstage=0 ; !m_par->cancel() && (nstage < TONEMAPPING_MAX_STAGES) ; nstage++)
    {
        if (m_par->stage[nstage].enabled)
        {
            // compute the desatured image

            pos = 0;

            for (int i=0 ; !m_par->cancel() && (i < size) ; i++)
            {
                blurimage[i] = (float)((img[pos]+img[pos+1]+img[pos+2])/3.0);
                pos += 3;
            }

            m_current_process_power_value = m_par->get_power(nstage);

            // blur

            inplace_blur(blurimage, sizex, sizey, m_par->get_blur(nstage));

            pos = 0;

            for (int i=0 ; !m_par->cancel() && (i<size) ; i++)
            {
                float src_r  = img[pos];
                float src_g  = img[pos+1];
                float src_b  = img[pos+2];

                float blur   = blurimage[i];

                float dest_r = func(src_r, blur);
                float dest_g = func(src_g, blur);
                float dest_b = func(src_b, blur);

                img[pos]     = dest_r;
                img[pos+1]   = dest_g;
                img[pos+2]   = dest_b;

                pos += 3;
            }
        }

        m_par->postProgress(30 + nstage*10);
    }

    int high_saturation_value = 100-m_par->high_saturation;
    int low_saturation_value  = 100-m_par->low_saturation;

    if ((m_par->high_saturation != 100) || (m_par->low_saturation != 100))
    {
        int pos = 0;

        for (int i=0 ; !m_par->cancel() && (i < size) ; i++)
        {
            float src_h, src_s, src_v;
            float dest_h, dest_s, dest_v;
            rgb2hsv(srcimg[pos], srcimg[pos+1], srcimg[pos+2], src_h, src_s, src_v);
            rgb2hsv(img[pos], img[pos+1], img[pos+2], dest_h, dest_s, dest_v);

            float dest_saturation = (float)((src_s*high_saturation_value+dest_s*(100.0-high_saturation_value))*0.01);
            if (dest_v>src_v)
            {
                float s1        = (float)(dest_saturation*src_v/(dest_v+1.0/255.0));
                dest_saturation = (float)((low_saturation_value*s1+m_par->low_saturation*dest_saturation)*0.01);
            }

            hsv2rgb(dest_h, dest_saturation, dest_v, img[pos], img[pos+1], img[pos+2]);

            pos += 3;
        }
    }

    m_par->postProgress(70);

    // Unsharp Mask filter

    if (m_par->unsharp_mask.enabled)
    {
        float* val = new float[size];

        // compute the desatured image

        int pos = 0;

        for (int i=0 ; !m_par->cancel() && (i < size) ; i++)
        {
            val[i] = blurimage[i] = (float)((img[pos]+img[pos+1]+img[pos+2])/3.0);
            //val[i] = blurimage[i] = (float)(max3(img[pos],img[pos+1],img[pos+2]));
            pos += 3;
        }

        float blur_value = m_par->get_unsharp_mask_blur();
        inplace_blur(blurimage, sizex, sizey, blur_value);

        pos              = 0;
        float pow        = (float)(2.5*m_par->get_unsharp_mask_power());
        float threshold  = (float)(m_par->unsharp_mask.threshold*pow/250.0);
        float threshold2 = threshold/2;

        for (int i=0 ; !m_par->cancel() && (i < size) ; i++)
        {
            float dval     = (val[i]-blurimage[i])*pow;
            float abs_dval = fabs(dval);
            if (abs_dval < threshold)
            {
                if (abs_dval > threshold2)
                {
                    bool sign = (dval < 0.0);
                    dval      = (float)((abs_dval-threshold2)*2.0);
                    if (sign) dval =- dval;
                }
                else
                {
                    dval = 0;
                }
            }

            float r   = img[pos]  +dval;
            float g   = img[pos+1]+dval;
            float b   = img[pos+2]+dval;

            if (r<0.0) r = 0.0;
            if (r>1.0) r = 1.0;
            if (g<0.0) g = 0.0;
            if (g>1.0) g = 1.0;
            if (b<0.0) b = 0.0;
            if (b>1.0) b = 1.0;

            img[pos]   = r;
            img[pos+1] = g;
            img[pos+2] = b;

            pos += 3;
        }

        delete [] val;
    }

    delete [] srcimg;
    delete [] blurimage;

    m_par->postProgress(80);
}

void ToneMapping::update_preprocessed_values()
{
    m_par->postProgress(20);
}

void ToneMapping::process_16bit_rgb_image(unsigned short int* img, int sizex, int sizey)
{
    int size              = sizex*sizey;
    float* tmpimage       = new float[size*3];
    const float inv_65536 = 1.0/65536.0;

    for (int i=0 ; !m_par->cancel() && (i < size*3) ; i++)
    {
        //convert to floating point
        tmpimage[i] = (float)(img[i]/65535.0);
    }

    process_rgb_image(tmpimage, sizex, sizey);

    //convert back to 8 bits (with dithering)
    int pos = 0;

    for (int i=0 ; !m_par->cancel() && (i < size) ; i++)
    {
        float dither = ((rand()/65536)%65536)*inv_65536;
        img[pos]     = (int)(tmpimage[pos]  *65535.0+dither);
        img[pos+1]   = (int)(tmpimage[pos+1]*65535.0+dither);
        img[pos+2]   = (int)(tmpimage[pos+2]*65535.0+dither);
        pos+=3;
    }

    delete [] tmpimage;

    m_par->postProgress(90);
}

void ToneMapping::process_8bit_rgb_image(unsigned char* img, int sizex, int sizey)
{
    int size            = sizex*sizey;
    float* tmpimage     = new float[size*3];
    const float inv_256 = 1.0/256.0;

    for (int i=0 ; !m_par->cancel() && (i < size*3) ; i++)
    {
        //convert to floating point
        tmpimage[i] = (float)(img[i]/255.0);
    }

    process_rgb_image(tmpimage, sizex, sizey);

    //convert back to 8 bits (with dithering)
    int pos=0;

    for (int i=0 ; !m_par->cancel() && (i < size) ; i++)
    {
        float dither = ((rand()/256)%256)*inv_256;
        img[pos]     = (int)(tmpimage[pos]  *255.0+dither);
        img[pos+1]   = (int)(tmpimage[pos+1]*255.0+dither);
        img[pos+2]   = (int)(tmpimage[pos+2]*255.0+dither);
        pos += 3;
    }

    delete [] tmpimage;
    m_par->postProgress(90);
}

void ToneMapping::inplace_blur(float* data, int sizex, int sizey, float blur)
{
    blur /= m_preview_zoom;

    if (blur < 0.3) return;

    float a = (float)(exp(log(0.25)/blur));

    if ((a <= 0.0) || (a >= 1.0)) return;

    a *= a;
    float denormal_remove = (float)(1e-15);

    for (int stage=0 ; !m_par->cancel() && (stage < 2) ; stage++)
    {
        for (int y=0 ; !m_par->cancel() && (y < sizey) ; y++)
        {
            int pos   = y*sizex;
            float old = data[pos];
            pos++;

            for (int x=1 ; !m_par->cancel() && (x < sizex) ; x++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos++;
            }

            pos = y*sizex+sizex-1;

            for (int x=1 ; !m_par->cancel() && (x < sizex) ; x++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos--;
            }
        }

        for (int x=0 ; !m_par->cancel() && (x < sizex) ; x++)
        {
            int pos   = x;
            float old = data[pos];

            for (int y=1 ; !m_par->cancel() && (y < sizey) ; y++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos += sizex;
            }

            pos = x+sizex*(sizey-1);

            for (int y=1 ; !m_par->cancel() && (y < sizey) ; y++)
            {
                old       = (data[pos]*(1-a)+old*a)+denormal_remove;
                data[pos] = old;
                pos -= sizex;
            }
        }
    }
}

void ToneMapping::stretch_contrast(float* data, int datasize)
{
    //stretch the contrast
    const unsigned int histogram_size=256;
    //first, we compute the histogram
    unsigned int histogram[histogram_size];

    for (unsigned int i=0 ; i < histogram_size ; i++)
    histogram[i] = 0;

    for (unsigned int i=0 ; !m_par->cancel() && (i < (unsigned int)datasize) ; i++)
    {
        int m = (int)(data[i]*(histogram_size-1));
        if (m < 0) m = 0;
        if (m > (int)(histogram_size-1)) m = histogram_size-1;
        histogram[m]++;
    }

    //I want to strip the lowest and upper 0.1 procents (in the histogram) of the pixels
    int          min         = 0;
    int          max         = 255;
    unsigned int desired_sum = datasize/1000;
    unsigned int sum_min     = 0;
    unsigned int sum_max     = 0;

    for (unsigned int i=0 ; !m_par->cancel() && (i < histogram_size) ; i++)
    {
        sum_min += histogram[i];
        if (sum_min > desired_sum)
        {
            min = i;
            break;
        }
    }

    for (int i = histogram_size-1 ; !m_par->cancel() && (i >= 0) ; i--)
    {
        sum_max += histogram[i];

        if (sum_max > desired_sum)
        {
            max = i;
            break;
        }
    }

    if (min >= max)
    {
        min = 0;
        max = 255;
    }

    float min_src_val = (float)(min/255.0);
    float max_src_val = (float)(max/255.0);

    for (int i=0 ; !m_par->cancel() && (i < datasize) ; i++)
    {
        //stretch the contrast
        float x = data[i];
        x       = (x-min_src_val)/(max_src_val-min_src_val);

        if (x < 0.0)
            x = 0.0;
        if (x > 1.0)
            x = 1.0;

        data[i] = x;
    }
}

} // namespace Digikam
