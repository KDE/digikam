/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : Enhance image with local contrasts (as human eye does).
 *               LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>
 *
 * Copyright (C) 2009 by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "localcontrastfilter.h"

// KDE includes

#include <kdebug.h>

// Local includes

#include "randomnumbergenerator.h"

namespace Digikam
{

class LocalContrastFilterPriv
{
public:

    LocalContrastFilterPriv()
    {
        current_process_power_value = 20.0;
    }

    // preprocessed values
    float                  current_process_power_value;

    LocalContrastContainer par;

    RandomNumberGenerator  generator;
};

LocalContrastFilter::LocalContrastFilter(QObject* parent)
    : DImgThreadedFilter(parent),
      d(new LocalContrastFilterPriv)
{
    initFilter();
}

LocalContrastFilter::LocalContrastFilter(DImg* image, QObject* parent, const LocalContrastContainer& par)
    : DImgThreadedFilter(image, parent, "LocalContrast"),
      d(new LocalContrastFilterPriv)
{
    d->par = par;
    d->generator.seedByTime();
    initFilter();
}

LocalContrastFilter::~LocalContrastFilter()
{
    cancelFilter();
    delete d;
}

void LocalContrastFilter::filterImage()
{
    if (!m_orgImage.isNull())
    {
        int size = m_orgImage.width() * m_orgImage.height() * 3;
        int i, j;

        d->generator.reseed();

        if (m_orgImage.sixteenBit())
        {
            // sixteen bit image
            QScopedArrayPointer<unsigned short> data(new unsigned short[size]);
            unsigned short* dataImg = (unsigned short*)(m_orgImage.bits());

            for (i = 0, j = 0; runningFlag() && (i < size); i += 3, j += 4)
            {
                data[i]   = dataImg[j];
                data[i + 1] = dataImg[j + 1];
                data[i + 2] = dataImg[j + 2];
            }

            postProgress(10);

            process_16bit_rgb_image(data.data(), m_orgImage.width(), m_orgImage.height());

            for (uint x = 0; runningFlag() && (x < m_orgImage.width()); ++x)
            {
                for (uint y = 0; runningFlag() && (y < m_orgImage.height()); ++y)
                {
                    i = (m_orgImage.width() * y + x) * 3;
                    m_destImage.setPixelColor(x, y, DColor((unsigned short)data[i + 2],
                                                           (unsigned short)data[i + 1],
                                                           (unsigned short)data[i],
                                                           65535, true));
                }
            }
        }
        else
        {
            // eight bit image
            QScopedArrayPointer<uchar> data(new uchar[size]);

            for (i = 0, j = 0; runningFlag() && (i < size); i += 3, j += 4)
            {
                data[i]   = m_orgImage.bits()[j];
                data[i + 1] = m_orgImage.bits()[j + 1];
                data[i + 2] = m_orgImage.bits()[j + 2];
            }

            postProgress(10);

            process_8bit_rgb_image(data.data(), m_orgImage.width(), m_orgImage.height());

            for (uint x = 0; runningFlag() && (x < m_orgImage.width()); ++x)
            {
                for (uint y = 0; runningFlag() && (y < m_orgImage.height()); ++y)
                {
                    i = (m_orgImage.width() * y + x) * 3;
                    m_destImage.setPixelColor(x, y, DColor(data[i + 2], data[i + 1], data[i], 255, false));
                }
            }
        }
    }

    postProgress(100);
}

void LocalContrastFilter::process_8bit_rgb_image(unsigned char* img, int sizex, int sizey)
{
    int size = sizex * sizey;
    QScopedArrayPointer<float> tmpimage(new float[size * 3]);

    for (int i = 0 ; runningFlag() && (i < size * 3) ; ++i)
    {
        // convert to floating point
        tmpimage[i] = (float)(img[i] / 255.0);
    }

    process_rgb_image(tmpimage.data(), sizex, sizey);

    // convert back to 8 bits (with dithering)
    int pos = 0;

    for (int i = 0 ; runningFlag() && (i < size) ; ++i)
    {
        float dither = d->generator.number(0.0, 1.0);
        img[pos]     = (int)(tmpimage[pos]  * 255.0 + dither);
        img[pos + 1]   = (int)(tmpimage[pos + 1] * 255.0 + dither);
        img[pos + 2]   = (int)(tmpimage[pos + 2] * 255.0 + dither);
        pos += 3;
    }

    postProgress(90);
}

void LocalContrastFilter::process_16bit_rgb_image(unsigned short int* img, int sizex, int sizey)
{
    int size = sizex * sizey;
    QScopedArrayPointer<float> tmpimage(new float[size * 3]);

    for (int i = 0 ; runningFlag() && (i < size * 3) ; ++i)
    {
        // convert to floating point
        tmpimage[i] = (float)(img[i] / 65535.0);
    }

    process_rgb_image(tmpimage.data(), sizex, sizey);

    // convert back to 16 bits (with dithering)
    int pos = 0;

    for (int i = 0 ; runningFlag() && (i < size) ; ++i)
    {
        float dither = d->generator.number(0.0, 1.0);
        img[pos]     = (int)(tmpimage[pos]  * 65535.0 + dither);
        img[pos + 1]   = (int)(tmpimage[pos + 1] * 65535.0 + dither);
        img[pos + 2]   = (int)(tmpimage[pos + 2] * 65535.0 + dither);
        pos += 3;
    }

    postProgress(90);
}

float LocalContrastFilter::func(float x1, float x2)
{
    float result = 0.5;
    float p;

    /*
    //test function
    if (d->par.function_id==1)
    {
        p=pow(0.1,fabs((x2*2.0-1.0))*d->current_process_power_value*0.02);
        if (x2<0.5) result=pow(x1,p);
        else result=1.0-pow(1.0-x1,p);
        return result;
    };
    //test function
    if (function_id==1)
    {
        p=d->current_process_power_value*0.3+1e-4;
        x2=1.0/(1.0+exp(-(x2*2.0-1.0)*p*0.5));
        float f=1.0/(1.0+exp((1.0-(x1-x2+0.5)*2.0)*p));
        float m0=1.0/(1.0+exp((1.0-(-x2+0.5)*2.0)*p));
        float m1=1.0/(1.0+exp((1.0-(-x2+1.5)*2.0)*p));
        result=(f-m0)/(m1-m0);
        return result;
    };
    */

    switch (d->par.function_id)
    {
        case 0:  //power function
        {
            p = (float)(pow((double)10.0, (double)fabs((x2 * 2.0 - 1.0)) * d->current_process_power_value * 0.02));

            if (x2 >= 0.5)
            {
                result = pow(x1, p);
            }
            else
            {
                result = (float)(1.0 - pow((double)1.0 - x1, (double)p));
            }

            break;
        }

        case 1:  //linear function
        {
            p      = (float)(1.0 / (1 + exp(-(x2 * 2.0 - 1.0) * d->current_process_power_value * 0.04)));
            result = (x1 < p) ? (float)(x1 * (1.0 - p) / p) : (float)((1.0 - p) + (x1 - p) * p / (1.0 - p));
            break;
        }
    }

    return result;
}

void LocalContrastFilter::process_rgb_image(float* img, int sizex, int sizey)
{
    int size = sizex * sizey;
    QScopedArrayPointer<float> blurimage(new float[size]);
    QScopedArrayPointer<float> srcimg(new float[size * 3]);

    for (int i = 0 ; i < (size * 3) ; ++i)
    {
        srcimg[i] = img[i];
    }

    if (d->par.stretch_contrast)
    {
        stretch_contrast(img, size * 3);
    }

    int pos = 0;

    for (int nstage = 0 ; runningFlag() && (nstage < TONEMAPPING_MAX_STAGES) ; ++nstage)
    {
        if (d->par.stage[nstage].enabled)
        {
            // compute the desatured image

            pos = 0;

            for (int i = 0 ; runningFlag() && (i < size) ; ++i)
            {
                blurimage[i] = (float)((img[pos] + img[pos + 1] + img[pos + 2]) / 3.0);
                pos += 3;
            }

            d->current_process_power_value = d->par.get_power(nstage);

            // blur

            inplace_blur(blurimage.data(), sizex, sizey, d->par.get_blur(nstage));

            pos = 0;

            for (int i = 0 ; runningFlag() && (i < size) ; ++i)
            {
                float src_r  = img[pos];
                float src_g  = img[pos + 1];
                float src_b  = img[pos + 2];

                float blur   = blurimage[i];

                float dest_r = func(src_r, blur);
                float dest_g = func(src_g, blur);
                float dest_b = func(src_b, blur);

                img[pos]     = dest_r;
                img[pos + 1]   = dest_g;
                img[pos + 2]   = dest_b;

                pos += 3;
            }
        }

        postProgress(30 + nstage * 10);
    }

    int high_saturation_value = 100 - d->par.high_saturation;
    int low_saturation_value  = 100 - d->par.low_saturation;

    if ((d->par.high_saturation != 100) || (d->par.low_saturation != 100))
    {
        kDebug() << "high_saturation : " << d->par.high_saturation;
        kDebug() << "low_saturation : " << d->par.low_saturation;

        float src_h,  src_s,  src_v;
        float dest_h, dest_s, dest_v;
        float dest_saturation, s1;
        int   pos = 0;

        for (int i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            rgb2hsv(srcimg[pos], srcimg[pos + 1], srcimg[pos + 2], src_h, src_s, src_v);
            rgb2hsv(img[pos], img[pos + 1], img[pos + 2], dest_h, dest_s, dest_v);

            dest_saturation = (float)((src_s * high_saturation_value + dest_s * (100.0 - high_saturation_value)) * 0.01);

            if (dest_v > src_v)
            {
                s1              = (float)(dest_saturation * src_v / (dest_v + 1.0 / 255.0));
                dest_saturation = (float)((low_saturation_value * s1 + d->par.low_saturation * dest_saturation) * 0.01);
            }

            hsv2rgb(dest_h, dest_saturation, dest_v, img[pos], img[pos + 1], img[pos + 2]);

            pos += 3;
        }
    }

    postProgress(70);

    // Unsharp Mask filter

    if (d->par.unsharp_mask.enabled)
    {
        QScopedArrayPointer<float> val(new float[size]);

        // compute the desatured image

        int pos = 0;

        for (int i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            val[i] = blurimage[i] = (float)((img[pos] + img[pos + 1] + img[pos + 2]) / 3.0);
            //val[i] = blurimage[i] = (float)(max3(img[pos],img[pos+1],img[pos+2]));
            pos += 3;
        }

        float blur_value = d->par.get_unsharp_mask_blur();
        inplace_blur(blurimage.data(), sizex, sizey, blur_value);

        pos              = 0;
        float pow        = (float)(2.5 * d->par.get_unsharp_mask_power());
        float threshold  = (float)(d->par.unsharp_mask.threshold * pow / 250.0);
        float threshold2 = threshold / 2;

        for (int i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            float dval     = (val[i] - blurimage[i]) * pow;
            float abs_dval = fabs(dval);

            if (abs_dval < threshold)
            {
                if (abs_dval > threshold2)
                {
                    bool sign = (dval < 0.0);
                    dval      = (float)((abs_dval - threshold2) * 2.0);

                    if (sign)
                    {
                        dval = - dval;
                    }
                }
                else
                {
                    dval = 0;
                }
            }

            float r   = img[pos]  + dval;
            float g   = img[pos + 1] + dval;
            float b   = img[pos + 2] + dval;

            if (r < 0.0)
            {
                r = 0.0;
            }

            if (r > 1.0)
            {
                r = 1.0;
            }

            if (g < 0.0)
            {
                g = 0.0;
            }

            if (g > 1.0)
            {
                g = 1.0;
            }

            if (b < 0.0)
            {
                b = 0.0;
            }

            if (b > 1.0)
            {
                b = 1.0;
            }

            img[pos]     = r;
            img[pos + 1] = g;
            img[pos + 2] = b;

            pos += 3;
        }
    }

    postProgress(80);
}

void LocalContrastFilter::inplace_blur(float* data, int sizex, int sizey, float blur)
{
    if (blur < 0.3)
    {
        return;
    }

    float a = (float)(exp(log(0.25) / blur));

    if ((a <= 0.0) || (a >= 1.0))
    {
        return;
    }

    a *= a;
    float denormal_remove = (float)(1e-15);

    for (int stage = 0 ; runningFlag() && (stage < 2) ; ++stage)
    {
        for (int y = 0 ; runningFlag() && (y < sizey) ; ++y)
        {
            int pos   = y * sizex;
            float old = data[pos];
            ++pos;

            for (int x = 1 ; runningFlag() && (x < sizex) ; ++x)
            {
                old       = (data[pos] * (1 - a) + old * a) + denormal_remove;
                data[pos] = old;
                ++pos;
            }

            pos = y * sizex + sizex - 1;

            for (int x = 1 ; runningFlag() && (x < sizex) ; ++x)
            {
                old       = (data[pos] * (1 - a) + old * a) + denormal_remove;
                data[pos] = old;
                pos--;
            }
        }

        for (int x = 0 ; runningFlag() && (x < sizex) ; ++x)
        {
            int pos   = x;
            float old = data[pos];

            for (int y = 1 ; runningFlag() && (y < sizey) ; ++y)
            {
                old       = (data[pos] * (1 - a) + old * a) + denormal_remove;
                data[pos] = old;
                pos       += sizex;
            }

            pos = x + sizex * (sizey - 1);

            for (int y = 1 ; runningFlag() && (y < sizey) ; ++y)
            {
                old       = (data[pos] * (1 - a) + old * a) + denormal_remove;
                data[pos] = old;
                pos       -= sizex;
            }
        }
    }
}

void LocalContrastFilter::stretch_contrast(float* data, int datasize)
{
    //stretch the contrast
    const unsigned int histogram_size = 256;
    //first, we compute the histogram
    unsigned int histogram[histogram_size];

    for (unsigned int i = 0 ; i < histogram_size ; ++i)
    {
        histogram[i] = 0;
    }

    for (unsigned int i = 0 ; runningFlag() && (i < (unsigned int)datasize) ; ++i)
    {
        int m = (int)(data[i] * (histogram_size - 1));

        if (m < 0)
        {
            m = 0;
        }

        if (m > (int)(histogram_size - 1))
        {
            m = histogram_size - 1;
        }

        histogram[m]++;
    }

    //I want to strip the lowest and upper 0.1 procents (in the histogram) of the pixels
    int          min         = 0;
    int          max         = 255;
    unsigned int desired_sum = datasize / 1000;
    unsigned int sum_min     = 0;
    unsigned int sum_max     = 0;

    for (unsigned int i = 0 ; runningFlag() && (i < histogram_size) ; ++i)
    {
        sum_min += histogram[i];

        if (sum_min > desired_sum)
        {
            min = i;
            break;
        }
    }

    for (int i = histogram_size - 1 ; runningFlag() && (i >= 0) ; i--)
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

    float min_src_val = (float)(min / 255.0);
    float max_src_val = (float)(max / 255.0);

    for (int i = 0 ; runningFlag() && (i < datasize) ; ++i)
    {
        //stretch the contrast
        float x = data[i];
        x       = (x - min_src_val) / (max_src_val - min_src_val);

        if (x < 0.0)
        {
            x = 0.0;
        }

        if (x > 1.0)
        {
            x = 1.0;
        }

        data[i] = x;
    }
}

void LocalContrastFilter::rgb2hsv(const float& r, const float& g, const float& b, float& h, float& s, float& v)
{
    float maxrg = (r > g) ? r : g;
    float max   = (maxrg > b) ? maxrg : b;
    float minrg = (r < g) ? r : g;
    float min   = (minrg < b) ? minrg : b;
    float delta = max - min;

    //hue
    if (min == max)
    {
        h = 0.0;
    }
    else
    {
        if (max == r)
        {
            h = (float)(fmod(60.0 * (g - b) / delta + 360.0, 360.0));
        }
        else
        {
            if (max == g)
            {
                h = (float)(60.0 * (b - r) / delta + 120.0);
            }
            else
            {
                //max==b
                h = (float)(60.0 * (r - g) / delta + 240.0);
            }
        }
    }

    //saturation
    if (max < 1e-6)
    {
        s = 0;
    }
    else
    {
        s = (float)(1.0 - min / max);
    }

    //value
    v = max;
}

void LocalContrastFilter::hsv2rgb(const float& h, const float& s, const float& v, float& r, float& g, float& b)
{
    float hfi = (float)(floor(h / 60.0));
    float f   = (float)((h / 60.0) - hfi);
    int hi    = ((int)hfi) % 6;
    float p   = (float)(v * (1.0 - s));
    float q   = (float)(v * (1.0 - f * s));
    float t   = (float)(v * (1.0 - (1.0 - f) * s));

    switch (hi)
    {
        case 0:
            r = v ;
            g = t ;
            b = p;
            break;

        case 1:
            r = q ;
            g = v ;
            b = p;
            break;

        case 2:
            r = p ;
            g = v ;
            b = t;
            break;

        case 3:
            r = p ;
            g = q ;
            b = v;
            break;

        case 4:
            r = t ;
            g = p;
            b = v;
            break;

        case 5:
            r = v ;
            g = p ;
            b = q;
            break;
    }
}

FilterAction LocalContrastFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("function_id", d->par.function_id);
    action.addParameter("high_saturation", d->par.high_saturation);
    action.addParameter("low_saturation", d->par.low_saturation);
    action.addParameter("stretch_contrast", d->par.stretch_contrast);

    for (int nstage = 0 ; nstage < TONEMAPPING_MAX_STAGES ; ++nstage)
    {
        QString stage = QString("stage[%1]:").arg(nstage);
        action.addParameter(stage + "enabled", d->par.stage[nstage].enabled);

        if (d->par.stage[nstage].enabled)
        {
            action.addParameter(stage + "power", d->par.stage[nstage].power);
            action.addParameter(stage + "blur", d->par.stage[nstage].blur);
        }
    }

    action.addParameter("unsharp_mask:enabled", d->par.unsharp_mask.enabled);

    if (d->par.unsharp_mask.enabled)
    {
        action.addParameter("unsharp_mask:blur", d->par.unsharp_mask.blur);
        action.addParameter("unsharp_mask:power", d->par.unsharp_mask.power);
        action.addParameter("unsharp_mask:threshold", d->par.unsharp_mask.threshold);
    }

    action.addParameter("randomSeed", d->generator.currentSeed());

    return action;
}

void LocalContrastFilter::readParameters(const Digikam::FilterAction& action)
{
    d->par.function_id = action.parameter("function_id").toInt();
    d->par.high_saturation = action.parameter("high_saturation").toInt();
    d->par.low_saturation = action.parameter("low_saturation").toInt();
    d->par.stretch_contrast = action.parameter("stretch_contrast").toBool();

    for (int nstage = 0 ; nstage < TONEMAPPING_MAX_STAGES ; ++nstage)
    {
        QString stage = QString("stage[%1]:").arg(nstage);
        d->par.stage[nstage].enabled = action.parameter(stage + "enabled").toBool();

        if (d->par.stage[nstage].enabled)
        {
            d->par.stage[nstage].power = action.parameter(stage + "power").toFloat();
            d->par.stage[nstage].blur  = action.parameter(stage + "blur").toFloat();
        }
    }

    d->par.unsharp_mask.enabled = action.parameter("unsharp_mask:enabled").toBool();

    if (d->par.unsharp_mask.enabled)
    {
        d->par.unsharp_mask.blur = action.parameter("unsharp_mask:blur").toFloat();
        d->par.unsharp_mask.power = action.parameter("unsharp_mask:power").toFloat();
        d->par.unsharp_mask.threshold = action.parameter("unsharp_mask:threshold").toInt();
    }

    d->generator.seed(action.parameter("randomSeed").toUInt());
}


} // namespace Digikam
