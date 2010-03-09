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

    void  process_8bit_rgb_image(unsigned char* img, int sizex, int sizey);
    void  process_16bit_rgb_image(unsigned short int* img, int sizex, int sizey);
    void  process_rgb_image(float* img, int sizex, int sizey);

    float func(float x1, float x2);
    void  apply_parameters(ToneMappingParameters* par);
    void  update_preprocessed_values();
    void  set_current_stage(int nstage);
    void  set_preview_zoom(float val);

    void  set_blur(int nstage, float value);      // 1..5000
    float get_blur(int nstage);

    void  set_power(int nstage, float value);     // 0..100.0
    float get_power(int nstage);

    void  set_low_saturation(int value);          // 0..100
    void  set_high_saturation(int value);         // 0..100

    void  set_stretch_contrast(bool value);
    bool  get_stretch_contrast();

    void  set_function_id (int value);            // 0..1
    int   get_function_id();

    void  inplace_blur(float* data, int sizex, int sizey, float blur);
    void  stretch_contrast(float* data, int datasize);

    ToneMappingParameters* get_parameters() const;

    void  set_info_fast_mode(bool value);
    bool  get_info_fast_mode();

    void  set_unsharp_mask_enabled(bool value);
    void  set_unsharp_mask_power(float value);
    void  set_unsharp_mask_blur(float value);
    void  set_unsharp_mask_threshold(int value);

    bool  get_unsharp_mask_enabled(bool /*value*/);
    float get_unsharp_mask_power(float /*value*/);
    float get_unsharp_mask_(float /*value*/);
    int   get_unsharp_mask_threshold(int /*value*/);

    void  set_enabled(int nstage, bool enabled);
    float get_enabled(int nstage);

    int   get_low_saturation();
    int   get_high_saturation();

private:

    inline void rgb2hsv(const float& r, const float& g, const float& b, float& h, float& s, float& v);
    inline void hsv2rgb(const float& h, const float& s, const float& v, float& r, float& g, float& b);

private:

    // used for zoom on previews
    float                  m_preview_zoom;

    ToneMappingParameters* m_par;

    // preprocessed values
    float                  m_current_process_power_value;
};

} // namespace Digikam

#endif // TONE_MAPPING_H
