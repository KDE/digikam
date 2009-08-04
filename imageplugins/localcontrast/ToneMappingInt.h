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

#ifndef TONE_MAPPING_INT_H
#define TONE_MAPPING_INT_H
#include "ToneMappingBase.h"

class ToneMappingInt:public ToneMappingBase{
public:
    ToneMappingInt();
    ~ToneMappingInt();
    void set_power(int nstage,REALTYPE value);
    void set_function_id(int value);

    void process_8bit_rgb_image(unsigned char *img,int sizex,int sizey);
    void recompute_func_table(int nstage);
    void update_preprocessed_values();

    void get_min_max_data(unsigned char *img,int size,int &min,int &max);
    void stretch_contrast_8bit_rgb_image(unsigned char *img,int sizex,int sizey,int min,int max,unsigned char *stretch_contrast_table=NULL);
private:
    void inplace_blur_8bit_process(unsigned char *data,int sizex, int sizey,float blur);
    inline unsigned char fast_func(unsigned char x1, unsigned char x2){
        return current_func_lookup_table[(((int)x1)<<8)+x2];
    };
    inline unsigned int max3(unsigned int x1,unsigned int x2,unsigned int x3){
        unsigned int max=x1;
        if (x2>max) max=x2;
        if (x3>max) max=x3;
        return max;
    };
    inline unsigned int min3(unsigned int x1,unsigned int x2,unsigned int x3){
        unsigned int min=x1;
        if (x2<min) min=x2;
        if (x3<min) min=x3;
        return min;
    };
    inline void rgb2hsv(unsigned char r,unsigned char g,unsigned char b,
                        unsigned int &h,unsigned int &s,unsigned int &v){
        unsigned char min=min3(r,g,b);
        unsigned char max=max3(r,g,b);
        int diff=max-min;
        if (max==0) {
            h=v=s=0;
            return;
        };
        //value
        v=max;

        //saturation
        s=(255*diff)/v;

        if (diff==0){
            h=0;
            return;
        };
        //hue
        if (max==r) {
            h=(4096*((int)g-(int)b)/diff+24576)%24576;
            return;
        };
        if (max==g) {
            h=8192+4096*((int)b-(int)r)/diff;
            return;
        };
        if (max==b) {
            h=16384+4096*((int)r-(int)g)/diff;
            return;
        };
    };
    inline void hsv2rgb(unsigned int h,unsigned int s,unsigned int v,
                        unsigned char &r,unsigned char &g,unsigned char &b){
        unsigned int hi=(h>>12)%6;
        unsigned int f=(h&4095)>>4;

        unsigned char p=(v*(255^s))>>8;
        unsigned char q=( v*(65535^(f*s)))>>16;
        unsigned char t=(v*(65535^(255^f)*s))>>16;

        switch(hi){
            case 0:
                r=v;g=t;b=p;
                break;
            case 1:
                r=q;g=v;b=p;
                break;
            case 2:
                r=p;g=v;b=t;
                break;
            case 3:
                r=p;g=q;b=v;
                break;
            case 4:
                r=t;g=p;b=v;
                break;
            case 5:
                r=v;g=p;b=q;
                break;
        };
    };
    struct {
        bool changed;
        unsigned char *func_lookup_table;
    }precomputed[TONEMAPPING_MAX_STAGES];
    unsigned char *current_func_lookup_table;
};

#endif

