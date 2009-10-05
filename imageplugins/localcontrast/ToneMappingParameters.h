/*
        LDR ToneMapper

        Copyright (C) 2009 Nasca Octavian Paul
        Author: Nasca Octavian Paul

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef TONE_MAPPING_PARAMETERS_H
#define TONE_MAPPING_PARAMETERS_H

#ifndef REALTYPE
#define REALTYPE float
#endif

#define TONEMAPPING_MAX_STAGES 4

class ToneMappingParameters
{
public:

    ToneMappingParameters();
    ~ToneMappingParameters();

    REALTYPE get_power(int nstage);
    REALTYPE get_blur(int nstage);

    REALTYPE get_unsharp_mask_power();
    REALTYPE get_unsharp_mask_blur();

    void save_parameters(const char *filename);
    bool load_parameters(const char *filename);

    bool info_fast_mode;

    //parameters
    int low_saturation;
    int high_saturation;
    bool stretch_contrast;
    int function_id;

    struct
    {
        bool enabled;
        REALTYPE power;
        REALTYPE blur;
    } stage[TONEMAPPING_MAX_STAGES];

    struct
    {
        bool enabled;
        REALTYPE power;
        REALTYPE blur;
        int threshold;
    } unsharp_mask;
};

#endif // TONE_MAPPING_PARAMETERS_H
