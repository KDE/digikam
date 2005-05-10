/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
 *
 * Includes code from gimp version 2.0  
 * LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#ifndef IMAGEIFACE_H
#define IMAGEIFACE_H

#include <qglobal.h>
#include <klocale.h>
#include "digikam_export.h"
#define MAX3(a, b, c) (QMAX(QMAX(a,b),b))
#define MIN3(a, b, c) (QMIN(QMIN(a,b),b))
#define ROUND(x) ((int) ((x) + 0.5))

class QPaintDevice;
class QString;

namespace Digikam
{

class ImageIfacePriv;

class DIGIKAMIMAGEEDITOR_EXPORT ImageIface
{
public:

    ImageIface(int w=0, int h=0);
    ~ImageIface();

    uint* getPreviewData();
    uint* getOriginalData();
    uint* getSelectedData();

    void putPreviewData(uint* data);
    void putOriginalData(const QString &caller, uint* data, int w=-1, int h=-1);
    void putSelectedData(uint* data);

    int  previewWidth();
    int  previewHeight();

    int  originalWidth();
    int  originalHeight();

    // Get selected dimensions.
    int  selectedWidth();
    int  selectedHeight();
    
    // Get selected (X, Y) position on the top/left corner.
    int  selectedXOrg();
    int  selectedYOrg();
        
    void setPreviewBCG(double brightness, double contrast, double gamma);
    void setOriginalBCG(double brightness, double contrast, double gamma);
    
    void paint(QPaintDevice* device, int x, int y, int w, int h);
    
private:

    ImageIfacePriv* d;
};


inline static int hsl_value (double n1,
                      double n2,
                      double hue)
{
    double value;

    if (hue > 255)
	hue -= 255;
    else if (hue < 0)
	hue += 255;

    if (hue < 42.5)
	value = n1 + (n2 - n1) * (hue / 42.5);
    else if (hue < 127.5)
	value = n2;
    else if (hue < 170)
	value = n1 + (n2 - n1) * ((170 - hue) / 42.5);
    else
	value = n1;

    return ROUND(value * 255.0);
}

inline static void rgb_to_hsl (int& r, int& g, int& b)
{
    double h, s, l;
    int    min, max;
    int    delta;

    if (r > g)
    {
        max = QMAX (r, b);
        min = QMIN (g, b);
    }
    else
    {
        max = QMAX (g, b);
        min = QMIN (r, b);
    }

    l = (max + min) / 2.0;

    if (max == min)
    {
        s = 0.0;
        h = 0.0;
    }
    else
    {
        delta = (max - min);

        if (l < 128)
            s = 255 * (double) delta / (double) (max + min);
        else
            s = 255 * (double) delta / (double) (511 - max - min);

        if (r == max)
            h = (g - b) / (double) delta;
        else if (g == max)
            h = 2 + (b - r) / (double) delta;
        else
            h = 4 + (r - g) / (double) delta;

        h = h * 42.5;

        if (h < 0)
            h += 255;
        else if (h > 255)
            h -= 255;
    }

    r = ROUND (h);
    g = ROUND (s);
    b = ROUND (l);
}


inline static void hsl_to_rgb (int& hue, int& saturation, int& lightness)
{
    double h, s, l;

    h = hue;
    s = saturation;
    l = lightness;

    if (s == 0)
    {
        /*  achromatic case  */
        hue        = (int) l;
        lightness  = (int) l;
        saturation = (int) l;
    }
    else
    {
        double m1, m2;

        if (l < 128)
            m2 = (l * (255 + s)) / 65025.0;
        else
            m2 = (l + s - (l * s) / 255.0) / 255.0;

        m1 = (l / 127.5) - m2;

        /*  chromatic case  */
        hue        = hsl_value (m1, m2, h + 85);
        saturation = hsl_value (m1, m2, h);
        lightness  = hsl_value (m1, m2, h - 85);
    }
}


inline static int rgb_to_l (int red, int green, int blue)
{
    int min, max;

    if (red > green)
    {
        max = QMAX (red,   blue);
        min = QMIN (green, blue);
    }
    else
    {
        max = QMAX (green, blue);
        min = QMIN (red,   blue);
    }

    return ROUND ((max + min) / 2.0);
}


}

#endif /* IMAGEIFACE_H */
