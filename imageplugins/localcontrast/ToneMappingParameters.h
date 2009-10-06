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

#ifndef TONE_MAPPING_PARAMETERS_H
#define TONE_MAPPING_PARAMETERS_H

#ifndef REALTYPE
#define REALTYPE float
#endif

#define TONEMAPPING_MAX_STAGES 4

namespace DigikamLocalContrastImagesPlugin
{

class ToneMappingParameters
{

public:

    ToneMappingParameters();
    ~ToneMappingParameters();

    bool     cancel();
    void     setCancel(bool *b);

    REALTYPE get_power(int nstage);
    REALTYPE get_blur(int nstage);

    REALTYPE get_unsharp_mask_power();
    REALTYPE get_unsharp_mask_blur();

    void     save_parameters(const char *filename);
    bool     load_parameters(const char *filename);

public:

    /** To cancel computation from user interface.
    */
    bool* info_cancel;

    bool  info_fast_mode;
    bool  stretch_contrast;

    int   low_saturation;
    int   high_saturation;
    int   function_id;

    struct
    {
        bool     enabled;
        REALTYPE power;
        REALTYPE blur;
    } stage[TONEMAPPING_MAX_STAGES];

    struct
    {
        bool     enabled;
        REALTYPE power;
        REALTYPE blur;
        int      threshold;
    } unsharp_mask;
};

} // namespace DigikamNoiseReductionImagesPlugin

#endif // TONE_MAPPING_PARAMETERS_H
