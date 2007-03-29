/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-16-01
 * Description : white balance color correction.
 * 
 * Copyright 2007 by Gilles Caulier
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
#include "blackbody.h"
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
        green       = 1.2;  
        temperature = 4.750;
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
            
    register int l, r, m;
    double sR, sG, sB, mRB, t;

    t   = QMAX( QMAX(tc.red(), tc.green()), tc.blue());
    sR  = tc.red()   / t;
    sG  = tc.green() / t;
    sB  = tc.blue()  / t;
    mRB = sR / sB;

    DDebug() << "Sums:  R:" << sR << " G:" << sG  << " B:" << sB << endl;

    l = 0;
    r = sizeof(blackBodyWhiteBalance)/(sizeof(float)*3);
    m = (r + l) / 2;

    for (l = 0, r = sizeof(blackBodyWhiteBalance)/(sizeof(float)*3), m = (l+r)/2 ; r-l > 1 ; m = (l+r)/2) 
    {
        if (blackBodyWhiteBalance[m][0]/blackBodyWhiteBalance[m][2] > mRB) 
            l = m;
        else
            r = m;

        DDebug() << "L,M,R:  " << l << " " << m << " " << r 
                 << " blackBodyWhiteBalance[m]=:" << blackBodyWhiteBalance[m][0]/blackBodyWhiteBalance[m][2]
                 << endl;
    }
    
    DDebug() << "Temperature (K):" << m*10.0+2000.0 << endl;

    t = (blackBodyWhiteBalance[m][1]/blackBodyWhiteBalance[m][0]) / (sG/sR);

    DDebug() << "Green component:" << t << endl;

    temperature = m*10.0+2000.0;
    green       = t;
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

void WhiteBalance::setRGBmult()
{
    int   t;
    float mi;

    if ( d->temperature > 7.0 ) d->temperature = 7.0;
    
    t     = (int)(d->temperature * 100.0 - 200.0);
    d->mr  = 1.0 / blackBodyWhiteBalance[t][0];
    d->mg  = 1.0 / blackBodyWhiteBalance[t][1];
    d->mb  = 1.0 / blackBodyWhiteBalance[t][2];
    d->mg *= d->green;
    
    // Normalize to at least 1.0, so we are not dimming colors only bumping.
    mi    = QMIN(d->mr, d->mg);
    mi    = QMIN(mi, d->mb);
    d->mr /= mi;
    d->mg /= mi;
    d->mb /= mi;
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
        d->curve[i]  = (i < d->BP) ? 0 : (d->rgbMax-1) * pow(x, gamma);
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
