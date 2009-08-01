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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ToneMappingBase.h"

ToneMappingBase::ToneMappingBase(){
	current_process_power_value=20.0;
	preview_zoom=1.0;
};

ToneMappingBase::~ToneMappingBase(){
};


void ToneMappingBase::set_blur(int nstage, REALTYPE value){
	if (value<0) value=0;
	if (value>10000.0) value=10000.0;
	par.stage[nstage].blur=value;
};

void ToneMappingBase::set_power(int nstage, REALTYPE value){
	if (value<0) value=0;
	if (value>100.0) value=100.0;
	par.stage[nstage].power=value;
};


void ToneMappingBase::set_low_saturation(int value){
	if (value<0) value=0;
	if (value>100) value=100;
	par.low_saturation=value;
};

void ToneMappingBase::set_high_saturation(int value){
	if (value<0) value=0;
	if (value>100) value=100;
	par.high_saturation=value;
};

void ToneMappingBase::set_stretch_contrast(bool value){
	par.stretch_contrast=value;
};

void ToneMappingBase::set_function_id (int value){
	if (value<0) value=0;
	if (value>1) value=1;
	par.function_id=value;
};


REALTYPE ToneMappingBase::func(REALTYPE x1,REALTYPE x2){
	REALTYPE result=0.5;
	REALTYPE p;
/*
	//test function
	if (par.function_id==1){
		p=pow(0.1,fabs((x2*2.0-1.0))*current_process_power_value*0.02);
		if (x2<0.5) result=pow(x1,p);
		else result=1.0-pow(1.0-x1,p);
		return result;
	};
	//test function
   if (function_id==1){
		p=current_process_power_value*0.3+1e-4;
		x2=1.0/(1.0+exp(-(x2*2.0-1.0)*p*0.5));
		REALTYPE f=1.0/(1.0+exp((1.0-(x1-x2+0.5)*2.0)*p));
		REALTYPE m0=1.0/(1.0+exp((1.0-(-x2+0.5)*2.0)*p));
		REALTYPE m1=1.0/(1.0+exp((1.0-(-x2+1.5)*2.0)*p));
		result=(f-m0)/(m1-m0);
		return result;
	};
*/

	switch (par.function_id){
			case 0://power function 
				p=pow(10.0,fabs((x2*2.0-1.0))*current_process_power_value*0.02);
				if (x2>=0.5) result=pow(x1,p);
					else result=1.0-pow(1.0-x1,p);
				break;
			case 1://linear function
				p=1.0/(1+exp(-(x2*2.0-1.0)*current_process_power_value*0.04));
				result=(x1<p)?(x1*(1.0-p)/p):((1.0-p)+(x1-p)*p/(1.0-p));
				break;
	};


	return result;
};

void ToneMappingBase::save_parameters(const char *filename){
	par.save_parameters(filename);
};

bool ToneMappingBase::load_parameters(const char *filename){
	if (!par.load_parameters(filename)) return false;	
	apply_parameters(par);
	return true;
};

void ToneMappingBase::apply_parameters(ToneMappingParameters inpar){
	par=inpar;
	set_low_saturation(par.low_saturation);
	set_high_saturation(par.high_saturation);
	set_stretch_contrast(par.stretch_contrast);
	set_function_id(par.function_id);

	for (int i=0;i<TONEMAPPING_MAX_STAGES;i++){
		set_power(i,par.stage[i].power);
		set_blur(i,par.stage[i].blur);
	};
	update_preprocessed_values();
};

