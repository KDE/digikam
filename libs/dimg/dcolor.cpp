/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-12-02
 * Description : 16 bits color management class
 *
 * Copyright 2005 by Gilles Caulier
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
 * ============================================================ */

// C++ includes.

#include <cmath>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dcolor.h"

namespace Digikam
{

DColor::DColor()
{
    m_sixteenBit = false;
    m_red        = 0;
    m_green      = 0;
    m_blue       = 0;
    m_alpha      = 0;
}

DColor::DColor(const DColor& color)
{
    m_red        = color.m_red;
    m_green      = color.m_green;
    m_blue       = color.m_blue;
    m_alpha      = color.m_alpha;
    m_sixteenBit = color.m_sixteenBit;
}

DColor::DColor(int red, int green, int blue, int alpha, bool sixteenBit)
{
    m_sixteenBit = sixteenBit;
    m_red        = red;
    m_green      = green;
    m_blue       = blue;
    m_alpha      = alpha;
}

DColor::DColor(uchar *data, bool sixteenBit)
{
    m_sixteenBit = sixteenBit;

    if (!sixteenBit)          // 8 bits image
    {
        setBlue (data[0]);
        setGreen(data[1]);
        setRed  (data[2]);
        setAlpha(data[3]);
    }
    else                      // 16 bits image
    {
        unsigned short* data16 = (unsigned short*)data;
        setBlue (data16[0]);
        setGreen(data16[1]);
        setRed  (data16[2]);
        setAlpha(data16[3]);
    }
}

DColor::~DColor()
{
}

int DColor::red()
{
    return m_red;
}

int DColor::green()
{
    return m_green;
}

int DColor::blue()
{
    return m_blue;
}

int DColor::alpha()
{
    return m_alpha;
}

bool DColor::sixteenBit()
{
    return m_sixteenBit;
}

DColor& DColor::operator=(const DColor& color)
{
    m_red        = color.m_red;
    m_green      = color.m_green;
    m_blue       = color.m_blue;
    m_alpha      = color.m_alpha;
    m_sixteenBit = color.m_sixteenBit;
    return *this;
}

void DColor::setRed(int red)
{
    m_red = red;
}

void DColor::setGreen(int green)
{
    m_green = green;
}

void DColor::setBlue (int blue)
{
    m_blue = blue;
}

void DColor::setAlpha(int alpha)
{
    m_alpha = alpha;
}

void DColor::setSixteenBit(bool sixteenBit)
{
    m_sixteenBit = sixteenBit;
}

// FIXME : untested code !!!

// This algorithm come from http://www.paris-pc-gis.com/MI_Enviro/Colors/color_models.htm
// hue range : 0 - 360 (°)
// sat range : 0 - 100 (%)
// lig range : 0 - 100 (%)

void DColor::getHSL(int* hue, int* sat, int* lig)
{
    /* 1 - Convert the RBG values to the range 0-1
       Example: from the video colors page, colorbar red has R=83%, B=7%, G=7%, or in this scale, R=.83, B=.07, G=.07*/

    double H, S, L;
    int segments = m_sixteenBit ? 65535 : 255;
    double R = m_red   / (double)segments;
    double G = m_green / (double)segments;
    double B = m_blue  / (double)segments;

    /* 2 - Find min and max values of R, B, G
       In the example, maxcolor = .83, mincolor=.07*/

    double mincolor = QMIN (QMIN(R, G), B);
    double maxcolor = QMAX (QMAX(R, G), B);

    /* 3 - L = (maxcolor + mincolor)/2
       For the example, L = (.83+.07)/2 = .45*/

    L = (maxcolor + mincolor) / 2.0;

    /* 4 - If the max and min colors are the same (i.e. the color is some kind of grey), S is defined to be 0,
       and H is undefined but in programs usually written as 0. (all is done)*/

    if (maxcolor == mincolor)
    {
        *lig = (int)(L * 100.0);
        *sat = 0;
        *hue = 0;      // Hue undefined.
        return;
    }

    /* 5 - Otherwise, test L.
       If L < 0.5, S=(maxcolor-mincolor)/(maxcolor+mincolor)
       If L >=0.5, S=(maxcolor-mincolor)/(2.0-maxcolor-mincolor)
       For the example, L=0.45 so S=(.83-.07)/(.83+.07) = .84*/

    if (L < 0.5)
    {
        S = (maxcolor - mincolor) / (maxcolor + mincolor);
    }

    if (L >= 0.5)
    {
        S = (maxcolor - mincolor) / ( 2.0 - maxcolor - mincolor);
    }

    /* 6 - Compute
    If R=maxcolor, H = (G-B)/(maxcolor-mincolor)
    If G=maxcolor, H = 2.0 + (B-R)/(maxcolor-mincolor)
    If B=maxcolor, H = 4.0 + (R-G)/(maxcolor-mincolor)
    For the example, R=maxcolor so H = (.07-.07)/(.83-.07) = 0*/

    if (R == maxcolor)
    {
        H = (G - B)/(maxcolor - mincolor);
    }

    if (G == maxcolor)
    {
        H = 2.0 + (B - R) / (maxcolor - mincolor);
    }

    if (B == maxcolor)
    {
        H = 4.0 + (R - G) / (maxcolor - mincolor);
    }

    /* 7 - To use the scaling shown in the video color page, convert L and S back to percentages,
       and H into an angle in degrees (ie scale it from 0-360).*/

    *lig = (int)(L *100.0);
    *sat = (int)(S *100.0);
    //H    = 360.0 * H;

    /* 8 - From the computation in step 6, H will range from 0-6. RGB space is a cube, and HSL space is a double hexacone,
       where L is the principal diagonal of the RGB cube. Thus corners of the RGB cube; red, yellow, green, cyan, blue,
       and magenta, become the vertices of the HSL hexagon. Then the value 0-6 for H tells you which section of the hexagon you are in.
       H is most commonly given as in degrees, so to convert H = H*60.0*/

    H = H * 60.0;

    /* 9 - If H is negative, add 360 to complete the conversion.*/

    if (H < 0.0)
    {
        *hue = (int)(H + 360.0);
    }
    else
    {
        *hue = (int)(H);
    }
}

// FIXME : untested code !!!

// This algorithm come from http://www.paris-pc-gis.com/MI_Enviro/Colors/color_models.htm
// hue range : 0 - 360 (°)
// sat range : 0 - 100 (%)
// lig range : 0 - 100 (%)

void DColor::setRGB(int hue, int sat, int lig, bool sixteenBit)
{
    double temp1, temp2, Rtemp3, Gtemp3, Btemp3, colorR, colorG, colorB;
    double L = lig / 100.0;
    double S = sat / 100.0;
    double H;
    
    m_sixteenBit = sixteenBit;
    int segments = m_sixteenBit ? 65535 : 255;

    /* 1 - If S=0, define R, G, and B all to L (all is done)*/

    if (S == 0)
    {
        m_red   = (int)(L * (double)segments);
        m_green = (int)(L * (double)segments);
        m_blue  = (int)(L * (double)segments);
    }

    /* 2 - Otherwise, test L.
       If L < 0.5, temp2=L*(1.0+S)
       If L >= 0.5, temp2=L+S - L*S
    
       In the colorbar example for colorbar green, H=120, L=52, S=79, so converting to the range 0-1, L=.52, S=.79,
       so temp2=(.52+.79) - (.52*.79) = .899*/

    if (L < 0.5)
    {
        temp2 = L * (1.0 + S);
    }

    if (L >= 0.5)
    {
        temp2 = L + S - L * S;
    }

    /* 3 - Compute
       temp1 = 2.0*L - temp2
       In the example, temp1 = 2.0*.52 - .899 = .141*/

    temp1 = 2.0 * L - temp2;

    /* 4 - Convert H to the range 0-1
       In the example, H=120/360 = .33*/

    H = hue / 360.0;
    
    /* 5 - For each of R, G, B, compute another temporary value, temp3, as follows:
       for R, temp3=H+1.0/3.0
       for G, temp3=H
       for B, temp3=H-1.0/3.0
       if temp3 < 0, temp3 = temp3 + 1.0
       if temp3 > 1, temp3 = temp3 - 1.0
    
       In the example, Rtemp3=.33+.33 = .66, Gtemp3=.33, Btemp3=.33-.33=0*/
    
    Rtemp3 = H + 1.0 / 3.0;
    if (Rtemp3 < 0.0) Rtemp3 = Rtemp3 + 1.0;
    if (Rtemp3 > 1.0) Rtemp3 = Rtemp3 - 1.0;
    
    Gtemp3 = H;
    if (Gtemp3 < 0.0) Gtemp3 = Gtemp3 + 1.0;
    if (Gtemp3 > 1.0) Gtemp3 = Gtemp3 - 1.0;
    
    Btemp3 = H - 1.0 / 3.0;
    if (Btemp3 < 0.0) Btemp3 = Btemp3 + 1.0;
    if (Btemp3 > 1.0) Btemp3 = Btemp3 - 1.0;

    /* 7 - For each of R, G, B, do the following test:
       If 6.0*temp3 < 1, color=temp1+(temp2-temp1)*6.0*temp3
       Else if 2.0*temp3 < 1, color=temp2
       Else if 3.0*temp3 < 2, color=temp1+(temp2-temp1)*((2.0/3.0)-temp3)*6.0
       Else color=temp1
    
       In the example,    
       3.0*Rtemp3 < 2 so R=.141+(.899-.141)*((2.0/3.0-.66)*6.0=.141 
       2.0*Gtemp3 < 1 so G=.899
       6.0*Btemp3 < 1 so B=.141+(.899-.141)*6.0*0=.141*/

    if (6.0 * Rtemp3 < 1.0) colorR = temp1 + (temp2 - temp1) * 6.0 * Rtemp3;
    else if (2.0 * Rtemp3 < 1.0) colorR = temp2;
    else if (3.0 * Rtemp3 < 2.0) colorR = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - Rtemp3) * 6.0;
    else colorR = temp1;
    
    if (6.0 * Gtemp3 < 1.0) colorG = temp1 + (temp2 - temp1) * 6.0 * Gtemp3;
    else if (2.0 * Gtemp3 < 1.0) colorG = temp2;
    else if (3.0 * Gtemp3 < 2.0) colorG = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - Gtemp3) * 6.0;
    else colorG = temp1;
        
    if (6.0 * Gtemp3 < 1.0) colorB = temp1 + (temp2 - temp1) * 6.0 * Gtemp3;
    else if (2.0 * Gtemp3 < 1.0) colorB = temp2;
    else if (3.0 * Gtemp3 < 2.0) colorB = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - Gtemp3) * 6.0;
    else colorB = temp1;
            
    /* 8 - Scale back to the range 0-100 to use the scaling shown in the video color page
       For the example, R=14, G=90, B=14  Color in video*/

    m_red   = (int)(Rtemp3 * 100.0 * ((double)segments/255.0));
    m_green = (int)(Gtemp3 * 100.0 * ((double)segments/255.0));
    m_blue  = (int)(Btemp3 * 100.0 * ((double)segments/255.0));
    m_alpha = 0;
    
}

}  // NameSpace Digikam
