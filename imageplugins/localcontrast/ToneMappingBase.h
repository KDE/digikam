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

#ifndef TONE_MAPPING_BASE_H
#define TONE_MAPPING_BASE_H
#include <math.h>
#include "ToneMappingParameters.h"
class ToneMappingBase{
    public:
        ToneMappingBase();
           virtual ~ToneMappingBase();

        REALTYPE func(REALTYPE x1,REALTYPE x2);

        virtual void set_blur(int nstage,REALTYPE value);//1..5000
        virtual void set_power(int nstage,REALTYPE value);//0..100.0
        virtual void set_low_saturation(int value);//0..100
        virtual void set_high_saturation(int value);//0..100
        virtual void set_stretch_contrast(bool value);
        virtual void set_function_id (int value);//0..1
        void set_enabled(int nstage,bool enabled){
            par.stage[nstage].enabled=enabled;
        };
        void set_info_fast_mode(bool value){
            par.info_fast_mode=value;
        };

        void set_unsharp_mask_enabled(bool value){
            par.unsharp_mask.enabled=value;
        };
        void set_unsharp_mask_power(float value){
            if (value<0.0) value=0.0;
            if (value>100.0) value=100.0;
            par.unsharp_mask.power=value;
        };
        void set_unsharp_mask_blur(float value){
            if (value<0.0) value=0.0;
            if (value>5000.0) value=5000.0;
            par.unsharp_mask.blur=value;
        };
        void set_unsharp_mask_threshold(int value){
            if (value<0) value=0;
            if (value>100) value=100;
            par.unsharp_mask.threshold=value;
        };


        virtual void process_8bit_rgb_image(unsigned char *img,int sizex,int sizey)=0;

        virtual void update_preprocessed_values()=0;

        void apply_parameters(ToneMappingParameters inpar);

        ToneMappingParameters get_parameters(){
            return par;
        };

        REALTYPE get_enabled(int nstage){
            return par.stage[nstage].enabled;
        };
        REALTYPE get_blur(int nstage){
            return par.stage[nstage].blur;
        };
        REALTYPE get_power(int nstage){
            return par.stage[nstage].power;
        };
        int get_low_saturation(){
            return par.low_saturation;
        };
        int get_high_saturation(){
            return par.high_saturation;
        };
        bool get_stretch_contrast(){
            return par.stretch_contrast;
        };
        int get_function_id(){
            return par.function_id;
        };

        bool get_info_fast_mode(){
            return par.info_fast_mode;
        };

        bool get_unsharp_mask_enabled(bool /*value*/){
            return par.unsharp_mask.enabled;
        };
        float get_unsharp_mask_power(float /*value*/){
            return par.unsharp_mask.power;
        };
        float get_unsharp_mask_(float /*value*/){
            return par.unsharp_mask.blur;
        };
        int get_unsharp_mask_threshold(int /*value*/){
            return par.unsharp_mask.threshold;
        };

        void set_current_stage(int nstage){
            current_process_power_value=par.get_power(nstage);
        };
        void save_parameters(const char *filename);
        bool load_parameters(const char *filename);

        void set_preview_zoom(REALTYPE val){
            if ((val>0.001)&&(val<1000.0)) preview_zoom=val;
        };
    protected:
        REALTYPE preview_zoom;//used for zoom on previews
        ToneMappingParameters par;

        //preprocessed values
        REALTYPE current_process_power_value;
};

#endif

