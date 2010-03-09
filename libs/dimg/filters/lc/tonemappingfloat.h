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

#ifndef TONE_MAPPING_FLOAT_H
#define TONE_MAPPING_FLOAT_H

// Local includes.

#include "digikam_export.h"
#include "tonemappingbase.h"

namespace Digikam
{

class DIGIKAM_EXPORT ToneMappingFloat : public ToneMappingBase
{

public:

    ToneMappingFloat();
    ~ToneMappingFloat();

    void process_8bit_rgb_image(unsigned char* img, int sizex, int sizey);
    void process_16bit_rgb_image(unsigned short int* img, int sizex, int sizey);
    void process_rgb_image(float* img, int sizex, int sizey);
    void update_preprocessed_values();

private:

    void inplace_blur(float* data, int sizex, int sizey, float blur);
    void stretch_contrast(float* data, int datasize);

    inline void rgb2hsv(const float& r, const float& g, const float& b,
                        float& h, float& s, float& v)
    {
        float maxrg = (r>g) ? r : g;
        float max   = (maxrg>b) ? maxrg : b;

        float minrg = (r<g) ? r : g;
        float min   = (minrg<b) ? minrg : b;

        float delta = max-min;

        //hue
        if (min == max)
        {
            h = 0.0;
        }
        else
        {
            if (max == r)
            {
                h = (float)(fmod(60.0*(g-b)/delta+360.0, 360.0));
            }
            else
            {
                if (max == g)
                {
                    h = (float)(60.0*(b-r)/delta+120.0);
                }
                else
                {
                    //max==b
                    h = (float)(60.0*(r-g)/delta+240.0);
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
            s = (float)(1.0-min/max);
        }

        //value
        v = max;
    };

    inline void hsv2rgb(const float& h, const float& s, const float& v,
                        float& r, float& g, float& b)
    {
        float hfi = (float)(floor(h/60.0));
        float f   = (float)((h/60.0)-hfi);
        int hi       = ((int)hfi)%6;

        float p   = (float)(v*(1.0-s));
        float q   = (float)(v*(1.0-f*s));
        float t   = (float)(v*(1.0-(1.0-f)*s));

        switch (hi)
        {
            case 0:
                r = v ; g = t ; b = p;
                break;
            case 1:
                r = q ; g = v ; b = p;
                break;
            case 2:
                r = p ; g = v ; b = t;
                break;
            case 3:
                r = p ; g = q ; b = v;
                break;
            case 4:
                r = t ; g = p; b = v;
                break;
            case 5:
                r = v ; g = p ; b = q;
                break;
        }
    }
};

} // namespace Digikam

#endif // TONE_MAPPING_FLOAT_H
