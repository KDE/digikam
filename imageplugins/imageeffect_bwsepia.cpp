/* ============================================================
 * File  : imageeffect_bwsepia.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <imageiface.h>

#include "imageeffect_bwsepia.h"

void ImageEffect_BWSepia::convertTOBW()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth();
    int   h    = iface.originalHeight();

    if (!data || !w || !h)
        return;

    int   r1,g1,b1,a1;
    int   r2,g2,b2;
    float gr;

    uint* ptr = data;
    
    for (int i=0; i<w*h; i++) {

        a1 = (*ptr >> 24) & 0xff;
        r1 = (*ptr >> 16) & 0xff;
        g1 = (*ptr >> 8)  & 0xff;
        b1 = (*ptr)       & 0xff;
        
        // grayscale 
        gr = 0.34 * r1 + 0.5 * g1 + 0.16 * b1;
             
        r2 = QMIN((int)gr,255); 
        g2 = QMIN((int)gr,255);
        b2 = QMIN((int)gr,255);

        *ptr = a1 << 24 | r2 << 16 | g2 << 8 | b2; 
        ptr++;
    }

    iface.putOriginalData(data);

    delete [] data;
}

void ImageEffect_BWSepia::convertTOSepia()
{
    Digikam::ImageIface iface(0, 0);

    uint* data   = iface.getOriginalData();
    int   width  = iface.originalWidth();
    int   height = iface.originalHeight();

    if (!data || !width || !height)
        return;

    /*
    int   r1,g1,b1,a1;
    int   r2,g2,b2;
    float gr;

    uint* ptr = data;
    
    for (int i=0; i<w*h; i++) {

        a1 = (*ptr >> 24) & 0xff;
        r1 = (*ptr >> 16) & 0xff;
        g1 = (*ptr >> 8)  & 0xff;
        b1 = (*ptr)       & 0xff;

        // grayscale 
        gr = 0.34 * r1 + 0.5 * g1 + 0.16 * b1;
             
        //r2 = QMIN((int)(gr * 1.23),255); 
        //g2 = QMIN((int)(gr * 0.84),255);
        //b2 = QMIN((int)(gr * 0.52),255);

        r2 = QMIN((int)(gr * 1.11),255); 
        g2 = QMIN((int)(gr * 0.84),255);
        b2 = QMIN((int)(gr * 0.52),255);

        *ptr = a1 << 24 | r2 << 16 | g2 << 8 | b2; 
        ptr++;
    }
    */

    int   r,g,b,h,s,v;
    float gr;
    unsigned char* c;

    unsigned int* ptr = data;

    h = 162;
    s = 132;
    v = 101;
    Digikam::rgb_to_hsl(h, s, v);

    for (int i=0; i<width*height; i++) {

         c = (unsigned char*) ptr;

         b = c[0];
         g = c[1];
         r = c[2];
        
        // grayscale 
        gr = 0.3 * r + 0.59 * g + 0.11 * b;
        r = ROUND (gr);
        g = r;
        b = r;

        r = h;
        g = s;
        
        Digikam::hsl_to_rgb(r, g, b);
        
        c[0] = b;
        c[1] = g;
        c[2] = r;
        ptr++;
    }

    iface.putOriginalData(data);

    delete [] data;
}
