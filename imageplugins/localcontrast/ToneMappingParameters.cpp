/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * Copyright (C) 2009 by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ToneMappingParameters.h"

namespace DigikamLocalContrastImagesPlugin
{

ToneMappingParameters::ToneMappingParameters()
{
    info_cancel      = 0;
    info_fast_mode   = true;
    high_saturation  = 100;
    low_saturation   = 100;
    stretch_contrast = true;
    function_id      = 0;
    info_callBack    = 0;
    info_data        = 0;

    for (int i=0 ; i < TONEMAPPING_MAX_STAGES ; i++)
    {
        stage[i].enabled = (i == 0);
        stage[i].power   = 30.0;
        stage[i].blur    = 80;
    }

    unsharp_mask.enabled   = false;
    unsharp_mask.power     = 30;
    unsharp_mask.blur      = 4.0;
    unsharp_mask.threshold = 0;
}

ToneMappingParameters::~ToneMappingParameters()
{
}

bool ToneMappingParameters::cancel()
{
    if (info_cancel)
        return *info_cancel;

    return false;
}

void ToneMappingParameters::setCancel(bool *b)
{
    info_cancel = b;
}

void ToneMappingParameters::setProgressCallBackFunction(void *data, ToneMappingCallbackPtr cb)
{
    info_callBack = cb;
    info_data     = data;
}

void ToneMappingParameters::postProgress(int progress)
{
    info_callBack(info_data, progress);
}

REALTYPE ToneMappingParameters::get_power(int nstage)
{
    REALTYPE power = stage[nstage].power;
    power          = pow(power/100.0,1.5)*100.0;
    return power;
}

REALTYPE ToneMappingParameters::get_blur(int nstage)
{
    return stage[nstage].blur;
}

REALTYPE ToneMappingParameters::get_unsharp_mask_power()
{
    REALTYPE power = unsharp_mask.power;
    power          = pow(power/100.0,3.0)*10.0;
    return power;
}

REALTYPE ToneMappingParameters::get_unsharp_mask_blur()
{
    return unsharp_mask.blur;
}

void ToneMappingParameters::save_parameters(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "Tonemapping_by_PAUL\n");
    fprintf(f, "1\n");//version

    fprintf(f, "info_fast_mode %d\n",   info_fast_mode);
    fprintf(f, "low_saturation %d\n",   low_saturation);
    fprintf(f, "high_saturation %d\n",  high_saturation);
    fprintf(f, "stretch_contrast %d\n", stretch_contrast);
    fprintf(f, "function_id %d\n",      function_id);

    for (int i=0 ; i < TONEMAPPING_MAX_STAGES ; i++)
    {
        fprintf(f, "STAGE %d\n",   i);
        fprintf(f, "enabled %d\n", stage[i].enabled);
        fprintf(f, "power %g\n",   stage[i].power);
        fprintf(f, "blur %g\n",    stage[i].blur);
    }

    fprintf(f, "unsharp_mask_enabled %d\n",   unsharp_mask.enabled);
    fprintf(f, "unsharp_mask_power %g\n",     unsharp_mask.power);
    fprintf(f, "unsharp_mask_blur %g\n",      unsharp_mask.blur);
    fprintf(f, "unsharp_mask_threshold %d\n", unsharp_mask.threshold);

    fclose(f);
}

bool ToneMappingParameters::load_parameters(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return false;

    const int max_line = 1024;
    char line[max_line];
    line[0] = 0;

    fgets(line, max_line, f);
    if (feof(f)) return false;
    if (strstr(line, "Tonemapping_by_PAUL") != line) return false;
    fgets(line, max_line, f);//version
    int current_stage = 0;

    while (!feof(f))
    {
        for (int i=0 ; i < max_line ; i++)
            line[i] = 0;

        fgets(line, max_line-1,f);

        if (strlen(line)<3) continue;

        int space = 0;
        for (int i=0 ; i < max_line ; i++)
        {
            if (line[i] == ' ')
            {
                line[i] = 0;
                space = i+1;
                break;
            }
        }

        const char *par  = line;
        const char *sval = &line[space];
        int ipar         = atoi(sval);
        REALTYPE fpar    = atof(sval);

        if (strstr(par, "info_fast_mode") == par)   info_fast_mode   = ipar;
        if (strstr(par, "low_saturation") == par)   low_saturation   = ipar;
        if (strstr(par, "high_saturation") == par)  high_saturation  = ipar;
        if (strstr(par, "stretch_contrast") == par) stretch_contrast = ipar;
        if (strstr(par, "function_id") == par)      function_id      = ipar;

        if (strstr(par, "STAGE") == par)
        {
            if (ipar < 0) ipar = 0;
            if (ipar > (TONEMAPPING_MAX_STAGES-1)) ipar = TONEMAPPING_MAX_STAGES-1;
            current_stage = ipar;
        }

        if (strstr(par, "enabled") == par) stage[current_stage].enabled = ipar;
        if (strstr(par, "power") == par)   stage[current_stage].power   = fpar;
        if (strstr(par, "blur") == par)    stage[current_stage].blur    = fpar;

        if (strstr(par, "unsharp_mask_enabled") == par)   unsharp_mask.enabled   = ipar;
        if (strstr(par, "unsharp_mask_power") == par)     unsharp_mask.power     = fpar;
        if (strstr(par, "unsharp_mask_blur") == par)      unsharp_mask.blur      = fpar;
        if (strstr(par, "unsharp_mask_threshold") == par) unsharp_mask.threshold = ipar;
    }

    fclose(f);

    return true;
}

} // namespace DigikamNoiseReductionImagesPlugin
