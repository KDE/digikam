/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2007-16-01
 * Description : white balance color correction.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Guillaume Castagnino <casta at xwing dot info>
 *
 * Some parts are inspired from RawPhoto implementation copyrighted 
 * 2004-2005 by Pawel T. Jochym <jochym at ifj edu pl>
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

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

// C++ includes.

#include <cmath>

// Qt includes.

#include <qcolor.h>

// Local includes.

#include "ddebug.h"
#include "imagehistogram.h"
#include "whitebalance.h"

namespace Digikam
{

class WhiteBalancePriv
{
    
public:

    WhiteBalancePriv()
    {
        // Obsolete in algorithm since over/under exposure indicators
        // are implemented directly with preview widget.
        WBind       = false;
        overExp     = false;         

        clipSat     = true;
        mr          = 1.0;
        mg          = 1.0;
        mb          = 1.0;
        BP          = 0;

        // Neutral color temperature settings.
        dark        = 0.5;
        black       = 0.0;
        exposition  = 0.0;
        gamma       = 1.0;  
        saturation  = 1.0;  
        green       = 1.0;
        temperature = 6500.0;
    }

    bool   clipSat;
    bool   overExp;
    bool   WBind;

    double saturation;
    double temperature;    
    double gamma;
    double black;
    double exposition;
    double dark;
    double green;

    int    BP;
    int    WP;
    
    uint   rgbMax;
    
    float  curve[65536];
    float  mr;
    float  mg;
    float  mb;
};

WhiteBalance::WhiteBalance(bool sixteenBit)
{
    d = new WhiteBalancePriv;
    d->WP     = sixteenBit ? 65536 : 256;
    d->rgbMax = sixteenBit ? 65536 : 256;
}

WhiteBalance::~WhiteBalance()
{ 
    delete d;
}

void WhiteBalance::whiteBalance(uchar *data, int width, int height, bool sixteenBit, 
                                double black, double exposition,
                                double temperature, double green, double dark, 
                                double gamma, double saturation)
{ 
    d->temperature = temperature;
    d->green       = green;
    d->dark        = dark;
    d->black       = black;
    d->exposition  = exposition;
    d->gamma       = gamma;
    d->saturation  = saturation;

    // Set final lut.
    setRGBmult();
    d->mr = d->mb = 1.0;
    if (d->clipSat) d->mg = 1.0; 
    setLUTv();
    setRGBmult();

    // Apply White balance adjustments.
    adjustWhiteBalance(data, width, height, sixteenBit);
}

void WhiteBalance::autoWBAdjustementFromColor(const QColor &tc, double &temperature, double &green)
{       
    // Calculate Temperature and Green component from color picked.
            
    double tmin, tmax, mBR;
    float mr, mg, mb;

    DDebug() << "Sums:  R:" << tc.red() << " G:" << tc.green() << " B:" << tc.blue() << endl;

    /* This is a dichotomic search based on Blue and Red layers ratio
       to find the matching temperature
       Adapted from ufraw (0.12.1) RGB_to_Temperature
    */
    tmin = 2000.0;
    tmax = 12000.0;
    mBR = (double)tc.blue() / (double)tc.red();
    green = 1.0;
    for (temperature = (tmin+tmax)/2; tmax-tmin > 10; temperature = (tmin+tmax)/2)
    {
        DDebug() << "Intermediate Temperature (K):" << temperature << endl;
        setRGBmult(temperature, green, mr, mg, mb);
        if (mr/mb > mBR)
            tmax = temperature;
        else
            tmin = temperature;
    }
    // Calculate the green level to neutralize picture
    green = (mr / mg) / ((double)tc.green() / (double)tc.red());

    DDebug() << "Temperature (K):" << temperature << endl;
    DDebug() << "Green component:" << green << endl;
}

void WhiteBalance::autoExposureAdjustement(uchar* data, int width, int height, bool sb,
                                           double &black, double &expo)
{
    // Create an histogram of original image.     

    ImageHistogram *histogram = new ImageHistogram(data, width, height, sb);
       
    // Calculate optimal exposition and black level 
    
    int    i;
    double sum, stop;
    uint   rgbMax = sb ? 65536 : 256;
        
    // Cutoff at 0.5% of the histogram.
    
    stop = width * height / 200;
    
    for (i = rgbMax, sum = 0; (i >= 0) && (sum < stop); i--)
        sum += histogram->getValue(Digikam::ImageHistogram::ValueChannel, i);
    
    expo = -log((float)(i+1) / rgbMax) / log(2);
    DDebug() << "White level at:" << i << endl;
    
    for (i = 1, sum = 0; (i < (int)rgbMax) && (sum < stop); i++)
        sum += histogram->getValue(Digikam::ImageHistogram::ValueChannel, i);
    
    black = (double)i / rgbMax;
    black /= 2;
    
    DDebug() << "Black:" << black << "  Exposition:" << expo << endl;

    delete histogram;
}

void WhiteBalance::setRGBmult(double &temperature, double &green, float &mr, float &mg, float &mb)
{
    float mi;
    double xD, yD, X, Y, Z;

    if ( temperature > 12000 ) temperature = 12000.0;
    
    /* Here starts the code picked from ufraw (0.12.1)
       to convert Temperature + green multiplier to RGB multipliers
    */
    /* Convert between Temperature and RGB.
     * Base on information from http://www.brucelindbloom.com/
     * The fit for D-illuminant between 4000K and 12000K are from CIE
     * The generalization to 2000K < T < 4000K and the blackbody fits
     * are my own and should be taken with a grain of salt.
     */
    const double XYZ_to_RGB[3][3] = {
        { 3.24071,  -0.969258,  0.0556352 },
        {-1.53726,  1.87599,    -0.203996 },
        {-0.498571, 0.0415557,  1.05707 } };
    // Fit for CIE Daylight illuminant
    if (temperature <= 4000)
    {
        xD = 0.27475e9/(temperature*temperature*temperature)
             - 0.98598e6/(temperature*temperature) 
             + 1.17444e3/temperature + 0.145986;
    }
    else if (temperature <= 7000)
    {
        xD = -4.6070e9/(temperature*temperature*temperature)
             + 2.9678e6/(temperature*temperature)
             + 0.09911e3/temperature + 0.244063;
    }
    else
    {
        xD = -2.0064e9/(temperature*temperature*temperature)
             + 1.9018e6/(temperature*temperature)
             + 0.24748e3/temperature + 0.237040;
    }
    yD = -3*xD*xD + 2.87*xD - 0.275;

    X = xD/yD;
    Y = 1;
    Z = (1-xD-yD)/yD;
    mr = X*XYZ_to_RGB[0][0] + Y*XYZ_to_RGB[1][0] + Z*XYZ_to_RGB[2][0];
    mg = X*XYZ_to_RGB[0][1] + Y*XYZ_to_RGB[1][1] + Z*XYZ_to_RGB[2][1];
    mb = X*XYZ_to_RGB[0][2] + Y*XYZ_to_RGB[1][2] + Z*XYZ_to_RGB[2][2];
    /* End of the code picked to ufraw
    */

    // Apply green multiplier
    mg = mg / green;

    mr  = 1.0 / mr;
    mg  = 1.0 / mg;
    mb  = 1.0 / mb;
    
    // Normalize to at least 1.0, so we are not dimming colors only bumping.
    mi  = QMIN(mr, QMIN(mg, mb));
    mr /= mi;
    mg /= mi;
    mb /= mi;
}

void WhiteBalance::setRGBmult()
{
    setRGBmult(d->temperature, d->green, d->mr, d->mg, d->mb);
}

void WhiteBalance::setLUTv()
{
    double b = d->mg * pow(2, d->exposition);
    d->BP    = (uint)(d->rgbMax * d->black);
    d->WP    = (uint)(d->rgbMax / b);
    
    if (d->WP - d->BP < 1) d->WP = d->BP + 1;

    DDebug() << "T(K): " << d->temperature
             << " => R:" << d->mr
             << " G:"    << d->mg
             << " B:"    << d->mb
             << " BP:"   << d->BP
             << " WP:"   << d->WP
             << endl;
    
    d->curve[0] = 0;
    
    // We will try to reproduce the same Gamma effect here than BCG tool.
    double gamma; 
    if (d->gamma >= 1.0)
        gamma = 0.335*(2.0-d->gamma) + 0.665;
    else 
        gamma = 1.8*(2.0-d->gamma) - 0.8; 
    
    for (int i = 1; i < (int)d->rgbMax; i++)
    {
        float x      = (float)(i - d->BP)/(d->WP - d->BP);
        d->curve[i]  = (i < d->BP) ? 0 : (d->rgbMax-1) * pow((double)x, gamma);
        d->curve[i] *= (1 - d->dark * exp(-x * x / 0.002));
        d->curve[i] /= (float)i;
    }
}

void WhiteBalance::adjustWhiteBalance(uchar *data, int width, int height, bool sixteenBit)
{  
    uint i, j;
         
    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue;
        uchar *ptr = data;
        
        for (j = 0 ; j < (uint)(width*height) ; j++)
        {
            int v, rv[3];

            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            rv[0] = (int)(blue  * d->mb);
            rv[1] = (int)(green * d->mg);
            rv[2] = (int)(red   * d->mr);
            v = QMAX(rv[0], rv[1]);
            v = QMAX(v, rv[2]);

            if (d->clipSat) v = QMIN(v, (int)d->rgbMax-1);
            i = v;

            ptr[0] = (uchar)pixelColor(rv[0], i, v);
            ptr[1] = (uchar)pixelColor(rv[1], i, v);
            ptr[2] = (uchar)pixelColor(rv[2], i, v);
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue;
        unsigned short *ptr = (unsigned short *)data;
        
        for (j = 0 ; j < (uint)(width*height) ; j++)
        {
            int v, rv[3];

            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];

            rv[0] = (int)(blue  * d->mb);
            rv[1] = (int)(green * d->mg);
            rv[2] = (int)(red   * d->mr);
            v     = QMAX(rv[0], rv[1]);
            v     = QMAX(v, rv[2]);

            if (d->clipSat) v = QMIN(v, (int)d->rgbMax-1);
            i = v;

            ptr[0] = pixelColor(rv[0], i, v);
            ptr[1] = pixelColor(rv[1], i, v);
            ptr[2] = pixelColor(rv[2], i, v);
            ptr += 4;
        }
    }
}

unsigned short WhiteBalance::pixelColor(int colorMult, int index, int value)
{
    int r = (d->clipSat && colorMult > (int)d->rgbMax) ? d->rgbMax : colorMult;

    if (value > d->BP && d->overExp && value > d->WP) 
    {
        if (d->WBind) 
           r = (colorMult > d->WP) ? 0 : r;
        else 
           r = 0;
    }
    
    return( (unsigned short)CLAMP((int)((index - d->saturation*(index - r)) * d->curve[index]), 
                                  0, (int)(d->rgbMax-1)) );
}               

}  // NameSpace Digikam
