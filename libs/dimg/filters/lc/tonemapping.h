/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper.
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

#ifndef TONE_MAPPING_H
#define TONE_MAPPING_H

// C++ includes.

#include <cmath>

// Local includes.

#include "digikam_export.h"
#include "tonemappingparameters.h"

namespace Digikam
{

class DIGIKAM_EXPORT ToneMapping
{

public:

    ToneMapping();
    virtual ~ToneMapping();

    void process_8bit_rgb_image(unsigned char* img, int sizex, int sizey);
    void process_16bit_rgb_image(unsigned short int* img, int sizex, int sizey);
    void process_rgb_image(float* img, int sizex, int sizey);

    float func(float x1, float x2);
    void  apply_parameters(ToneMappingParameters* par);
    void  update_preprocessed_values();

    void set_blur(int nstage, float value);      // 1..5000
    void set_power(int nstage, float value);     // 0..100.0
    void set_low_saturation(int value);          // 0..100
    void set_high_saturation(int value);         // 0..100
    void set_stretch_contrast(bool value);
    void set_function_id (int value);            // 0..1

    void inplace_blur(float* data, int sizex, int sizey, float blur);
    void stretch_contrast(float* data, int datasize);

    ToneMappingParameters* get_parameters() const
    {
        return m_par;
    };

    void set_enabled(int nstage, bool enabled)
    {
        m_par->stage[nstage].enabled=enabled;
    };

    void set_info_fast_mode(bool value)
    {
        m_par->info_fast_mode=value;
    };

    void set_unsharp_mask_enabled(bool value)
    {
        m_par->unsharp_mask.enabled = value;
    };

    void set_unsharp_mask_power(float value)
    {
        if (value < 0.0) value = 0.0;
        if (value > 100.0) value = 100.0;
        m_par->unsharp_mask.power = value;
    };

    void set_unsharp_mask_blur(float value)
    {
        if (value < 0.0) value = 0.0;
        if (value > 5000.0) value = 5000.0;
        m_par->unsharp_mask.blur = value;
    };

    void set_unsharp_mask_threshold(int value)
    {
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        m_par->unsharp_mask.threshold = value;
    };

    float get_enabled(int nstage)
    {
        return m_par->stage[nstage].enabled;
    };

    float get_blur(int nstage)
    {
        return m_par->stage[nstage].blur;
    };

    float get_power(int nstage)
    {
        return m_par->stage[nstage].power;
    };

    int get_low_saturation()
    {
        return m_par->low_saturation;
    };

    int get_high_saturation()
    {
        return m_par->high_saturation;
    };

    bool get_stretch_contrast()
    {
        return m_par->stretch_contrast;
    };

    int get_function_id()
    {
        return m_par->function_id;
    };

    bool get_info_fast_mode()
    {
        return m_par->info_fast_mode;
    };

    bool get_unsharp_mask_enabled(bool /*value*/)
    {
        return m_par->unsharp_mask.enabled;
    };

    float get_unsharp_mask_power(float /*value*/)
    {
        return m_par->unsharp_mask.power;
    };

    float get_unsharp_mask_(float /*value*/)
    {
        return m_par->unsharp_mask.blur;
    };

    int get_unsharp_mask_threshold(int /*value*/)
    {
        return m_par->unsharp_mask.threshold;
    };

    void set_current_stage(int nstage)
    {
        m_current_process_power_value = m_par->get_power(nstage);
    };

    void set_preview_zoom(float val)
    {
        if ((val > 0.001) && (val < 1000.0)) m_preview_zoom = val;
    };

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

protected:

    // used for zoom on previews
    float                  m_preview_zoom;

    ToneMappingParameters* m_par;

    // preprocessed values
    float                  m_current_process_power_value;
};

} // namespace Digikam

#endif // TONE_MAPPING_H
