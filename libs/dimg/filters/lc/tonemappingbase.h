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

#ifndef TONE_MAPPING_BASE_H
#define TONE_MAPPING_BASE_H

// C++ includes.

#include <cmath>

// Local includes.

#include "digikam_export.h"
#include "tonemappingparameters.h"

namespace Digikam
{

class DIGIKAM_EXPORT ToneMappingBase
{

public:

    ToneMappingBase();
    virtual ~ToneMappingBase();

    float func(float x1, float x2);
    void  apply_parameters(ToneMappingParameters* par);

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

    virtual void set_blur(int nstage, float value);      // 1..5000
    virtual void set_power(int nstage, float value);     // 0..100.0
    virtual void set_low_saturation(int value);          // 0..100
    virtual void set_high_saturation(int value);         // 0..100
    virtual void set_stretch_contrast(bool value);
    virtual void set_function_id (int value);            // 0..1

    virtual void process_8bit_rgb_image(unsigned char* img, int sizex, int sizey)=0;
    virtual void update_preprocessed_values()=0;

protected:

    // used for zoom on previews
    float                  m_preview_zoom;

    ToneMappingParameters* m_par;

    // preprocessed values
    float                  m_current_process_power_value;
};

} // namespace Digikam

#endif // TONE_MAPPING_BASE_H
