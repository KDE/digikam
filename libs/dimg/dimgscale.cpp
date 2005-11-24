/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-14
 * Copyright 2005 by Renchi Raju
 *
 * This is the normal smoothscale method, based on Imlib2's smoothscale.
 *
 * added smoothScaleSection - scaling only of a section of a image
 * added 16bit image support
 *
 * Ported to C++/QImage by Daniel M. Duley
 * Following modification are (C) Daniel M. Duley
 * Changes include formatting, namespaces and other C++'ings, removal of old
 * #ifdef'ed code, and removal of unneeded border calculation code.
 *
 * Imlib2 is (C) Carsten Haitzler and various contributors. The MMX code
 * is by Willem Monsuwe <willem@stack.nl>. 
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

// C ansi includes.

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Local includes.

#include "dimgprivate.h"
#include "dimg.h"

typedef uint64_t ullong;
typedef int64_t  llong;

namespace Digikam
{

namespace DImgScale
{
    typedef struct __dimg_scale_info
    {
        int     *xpoints;
        uint   **ypoints;
        ullong **ypoints16;
        int     *xapoints;
        int     *yapoints;
        int      xup_yup;
    } DImgScaleInfo;

    uint**   dimgCalcYPoints(uint *src, int sw, int sh, int dh);
    ullong** dimgCalcYPoints16(ullong *src, int sw, int sh, int dh);
    int*     dimgCalcXPoints(int sw, int dw);
    int*     dimgCalcApoints(int s, int d, int up);

    DImgScaleInfo* dimgFreeScaleInfo(DImgScaleInfo *isi);
    DImgScaleInfo *dimgCalcScaleInfo(const DImg &img, 
					 int sw, int sh,
                                         int dw, int dh, 
					 bool sixteenBit, 
					 bool aa);

    void dimgSampleRGBA(DImgScaleInfo *isi, unsigned int *dest, int dxx,
                          int dyy, int dx, int dy, int dw, int dh, int dow);
    void dimgScaleAARGBA(DImgScaleInfo *isi, unsigned int *dest, int dxx,
                           int dyy, int dx, int dy, int dw, int dh, int dow,
                           int sow);
    void dimgScaleAARGB(DImgScaleInfo *isi, unsigned int *dest, int dxx,
                          int dyy, int dx, int dy, int dw, int dh, int dow, int
                          sow);

    void dimgScaleAARGBA16(DImgScaleInfo *isi, ullong *dest,
                             int dxx, int dyy, int dw, int dh, 
                             int dow, int sow);
    void dimgScaleAARGB16(DImgScaleInfo *isi, ullong *dest,
                            int dxx, int dyy, int dw, int dh, 
			    int dow, int sow);
};

using namespace DImgScale;

DImg DImg::smoothScale(uint dw, uint dh)
{
    uint w = width();
    uint h = height();

    // do we actually need to scale?
    if ((w == dw) && (h == dh))
    {
        return copy(0, 0, w, h);
    }

    DImgScale::DImgScaleInfo *scaleinfo = dimgCalcScaleInfo(*this, w, h, dw, dh, 
						     sixteenBit(), true);
    if (!scaleinfo)
        return *this;

    DImg buffer(dw, dh, sixteenBit());
    buffer.m_priv->alpha = hasAlpha();    

    if (sixteenBit())
    {
        if (hasAlpha())
        {
            dimgScaleAARGBA16(scaleinfo, (ullong*) buffer.bits(),
                                0, 0, dw, dh, dw, w);
        }
        else
        {
            dimgScaleAARGB16(scaleinfo, (ullong*) buffer.bits(), 
                               0, 0, dw, dh, dw, w);
        }
    }
    else
    {
        if (hasAlpha())
        {
            dimgScaleAARGBA(scaleinfo, (unsigned int *)buffer.bits(),
                              0, 0, 0, 0, dw, dh, dw, w);
        }
        else
        {
            dimgScaleAARGB(scaleinfo, (unsigned int *)buffer.bits(),
                             0, 0, 0, 0, dw, dh, dw, w);
        }
    }
    
    dimgFreeScaleInfo(scaleinfo);

    return buffer;
}

#define CLIP(x, y, w, h, xx, yy, ww, hh) \
if (x < (xx)) {w += (x - (xx)); x = (xx);} \
if (y < (yy)) {h += (y - (yy)); y = (yy);} \
if ((x + w) > ((xx) + (ww))) {w = (ww) - (x - xx);} \
if ((y + h) > ((yy) + (hh))) {h = (hh) - (y - yy);}

DImg DImg::smoothScaleSection(uint sx, uint sy,
                                  uint sw, uint sh,
                                  uint dw, uint dh)
{
    uint w = width();
    uint h = height();

    // sanity checks
    if ((dw <= 0) || (dh <= 0))
        return DImg();

    if ((sw <= 0) || (sh <= 0))
        return DImg();

    // clip the source rect to be within the actual image 
    int  psx, psy, psw, psh;
    psx = sx;
    psy = sy;
    psw = sw;
    psh = sh;
    CLIP(sx, sy, sw, sh, 0, 0, w, h);
    
    // clip output coords to clipped input coords 
    if (psw != sw)
        dw = (dw * sw) / psw;
    if (psh != sh)
        dh = (dh * sh) / psh;

    // do a second check to see if we now have invalid coords 
    // dont do anything if we have a 0 widht or height image to render 
    if ((dw <= 0) || (dh <= 0))
        return DImg();

    // if the input rect size < 0 dont render either 
    if ((sw <= 0) || (sh <= 0))
        return DImg();

    // do we actually need to scale?
    if ((sw == dw) && (sh == dh))
    {
        return copy(sx, sy, sw, sh);
    }
    
    // calculate scaleinfo
    DImgScaleInfo *scaleinfo = dimgCalcScaleInfo(*this, sw, sh, dw, dh, 
						     sixteenBit(), true);
    if (!scaleinfo)
        return DImg();

    DImg buffer(dw, dh, sixteenBit());
    buffer.m_priv->alpha = hasAlpha();    

    if (sixteenBit())
    {
        if (hasAlpha())
        {
            dimgScaleAARGBA16(scaleinfo, (ullong*) buffer.bits(), 
                                ((sx * dw) / sw),
                                ((sy * dh) / sh),
                                dw, dh, 
                                dw, w);
        }
        else
        {
            dimgScaleAARGB16(scaleinfo, (ullong*) buffer.bits(), 
			      ((sx * dw) / sw),
			      ((sy * dh) / sh),
			       dw, dh, 
			       dw, w);
        }
    }
    else
    {
	if (hasAlpha())
        { 
	    dimgScaleAARGBA(scaleinfo,
			      (uint *)buffer.bits(),
			      ((sx * dw) / sw),
			      ((sy * dh) / sh),
			      0, 0,
			      dw, dh,
			      dw, w);
	}
	else
	{
	    dimgScaleAARGB(scaleinfo,
			     (uint *)buffer.bits(),
			     ((sx * dw) / sw),
			     ((sy * dh) / sh),
			     0, 0,
			     dw, dh,
			     dw, w);
	}
    }

    dimgFreeScaleInfo(scaleinfo);

    return buffer;
}


//
// Code ported from Imlib...
//

// FIXME: replace with mRed, etc... These work on pointers to pixels, not
// pixel values
#define A_VAL(p) ((unsigned char *)(p))[3]
#define R_VAL(p) ((unsigned char *)(p))[2]
#define G_VAL(p) ((unsigned char *)(p))[1]
#define B_VAL(p) ((unsigned char *)(p))[0]

#define INV_XAP                   (256 - xapoints[x])
#define XAP                       (xapoints[x])
#define INV_YAP                   (256 - yapoints[dyy + y])
#define YAP                       (yapoints[dyy + y])

unsigned int** DImgScale::dimgCalcYPoints(unsigned int *src,
                                              int sw, int sh, int dh)
{
    unsigned int **p;
    int i, j = 0;
    int val, inc;

    p = new unsigned int* [dh+1];

    val = 0;
    inc = (sh << 16) / dh;
    for(i = 0; i < dh; i++){
        p[j++] = src + ((val >> 16) * sw);
        val += inc;
    }

    return(p);
}

ullong** DImgScale::dimgCalcYPoints16(ullong* src, int sw, int sh, int dh)
{
    ullong** p;
    int i, j = 0;
    int val, inc;

    p = new ullong*[(dh+1)];

    val = 0;
    inc = (sh << 16) / dh;
    for(i = 0; i < dh; i++)
    {
        p[j++] = src + ((val >> 16) * sw);
        val += inc;
    }

    return p;
}

int* DImgScale::dimgCalcXPoints(int sw, int dw)
{
    int *p, i, j = 0;
    int val, inc;

    p = new int[dw+1];

    val = 0;
    inc = (sw << 16) / dw;
    for(i = 0; i < dw; i++){
        p[j++] = (val >> 16);
        val += inc;
    }

   return(p);
}

int* DImgScale::dimgCalcApoints(int s, int d, int up)
{
    int *p, i, j = 0;

    p = new int[d];

    /* scaling up */
    if(up){
        int val, inc;

        val = 0;
        inc = (s << 16) / d;
        for(i = 0; i < d; i++){
            p[j++] = (val >> 8) - ((val >> 8) & 0xffffff00);
            if((val >> 16) >= (s - 1))
                p[j - 1] = 0;
            val += inc;
        }
    }
    /* scaling down */
    else{
        int val, inc, ap, Cp;
        val = 0;
        inc = (s << 16) / d;
        Cp = ((d << 14) / s) + 1;

        for(i = 0; i < d; i++){
            ap = ((0x100 - ((val >> 8) & 0xff)) * Cp) >> 8;
            p[j] = ap | (Cp << 16);
            j++;
            val += inc;
        }
    }

    return(p);
}

DImgScaleInfo* DImgScale::dimgFreeScaleInfo(DImgScaleInfo *isi)
{
    if(isi)
    {
        delete [] isi->xpoints;
        delete [] isi->ypoints;
        delete [] isi->ypoints16;
        delete [] isi->xapoints;
        delete [] isi->yapoints;
        delete isi;
    }
    
    return 0;
}

DImgScaleInfo* DImgScale::dimgCalcScaleInfo(const DImg &img, 
						  int sw, int sh,
                                                  int dw, int dh, 
						  bool sixteenBit,
						  bool aa)
{
    DImgScaleInfo *isi;
    int scw, sch;

    scw = dw * img.width() / sw;
    sch = dh * img.height() / sh;

    isi = new DImgScaleInfo;
    if(!isi)
        return(NULL);
    memset(isi, 0, sizeof(DImgScaleInfo));

    isi->xup_yup = (abs(dw) >= sw) + ((abs(dh) >= sh) << 1);

    isi->xpoints = dimgCalcXPoints(img.width(), scw);
    if(!isi->xpoints)
        return(dimgFreeScaleInfo(isi));

    if (img.sixteenBit())
    {
        isi->ypoints   = 0;
        isi->ypoints16 = dimgCalcYPoints16((ullong*)img.bits(),
					     img.width(), img.height(), sch);
	if (!isi->ypoints16)
	    return(dimgFreeScaleInfo(isi));
    }
    else
    {
        isi->ypoints16 = 0;
        isi->ypoints = dimgCalcYPoints((uint*)img.bits(),
					 img.width(), img.height(), sch);
	if (!isi->ypoints)
	    return(dimgFreeScaleInfo(isi));
    }

    if (aa)
    {
        isi->xapoints = dimgCalcApoints(img.width(), scw, isi->xup_yup & 1);
        if(!isi->xapoints)
            return(dimgFreeScaleInfo(isi));
        isi->yapoints = dimgCalcApoints(img.height(), sch, isi->xup_yup & 2);
        if(!isi->yapoints)
            return(dimgFreeScaleInfo(isi));
    }

    return(isi);
}

/* scale by pixel sampling only */
void DImgScale::dimgSampleRGBA(DImgScaleInfo *isi, unsigned int *dest,
                                   int dxx, int dyy, int dx, int dy, int dw,
                                   int dh, int dow)
{
    unsigned int *sptr, *dptr;
    int x, y, end;
    unsigned int **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;

    /* whats the last pixel ont he line so we stop there */
    end = dxx + dw;
    /* go through every scanline in the output buffer */
    for(y = 0; y < dh; y++){
        /* get the pointer to the start of the destination scanline */
        dptr = dest + dx + ((y + dy) * dow);
        /* calculate the source line we'll scan from */
        sptr = ypoints[dyy + y];
        /* go thru the scanline and copy across */
        for(x = dxx; x < end; x++)
            *dptr++ = sptr[xpoints[x]];
    }
}

/* FIXME: NEED to optimise ScaleAARGBA - currently its "ok" but needs work*/

/* scale by area sampling */
void DImgScale::dimgScaleAARGBA(DImgScaleInfo *isi, unsigned int *dest,
                                    int dxx, int dyy, int dx, int dy, int dw,
                                    int dh, int dow, int sow)
{
    unsigned int *sptr, *dptr;
    int x, y, end;
    unsigned int **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;
    int *xapoints = isi->xapoints;
    int *yapoints = isi->yapoints;

    end = dxx + dw;
    /* scaling up both ways */
    if(isi->xup_yup == 3){
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            /* calculate the source line we'll scan from */
            dptr = dest + dx + ((y + dy) * dow);
            sptr = ypoints[dyy + y];
            if(YAP > 0){
                for(x = dxx; x < end; x++){
                    int r, g, b, a;
                    int rr, gg, bb, aa;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        a = A_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        a += A_VAL(pix) * XAP;
                        pix += sow;
                        rr = R_VAL(pix) * XAP;
                        gg = G_VAL(pix) * XAP;
                        bb = B_VAL(pix) * XAP;
                        aa = A_VAL(pix) * XAP;
                        pix--;
                        rr += R_VAL(pix) * INV_XAP;
                        gg += G_VAL(pix) * INV_XAP;
                        bb += B_VAL(pix) * INV_XAP;
                        aa += A_VAL(pix) * INV_XAP;
                        r = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b = ((bb * YAP) + (b * INV_YAP)) >> 16;
                        a = ((aa * YAP) + (a * INV_YAP)) >> 16;

                        A_VAL(dptr) = a;
                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;

                        dptr++;
                    }
                    else{
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_YAP;
                        g = G_VAL(pix) * INV_YAP;
                        b = B_VAL(pix) * INV_YAP;
                        a = A_VAL(pix) * INV_YAP;
                        pix += sow;
                        r += R_VAL(pix) * YAP;
                        g += G_VAL(pix) * YAP;
                        b += B_VAL(pix) * YAP;
                        a += A_VAL(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;

                        A_VAL(dptr) = a;
                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;

                        dptr++;
                    }
                }
            }
            else{
                for(x = dxx; x < end; x++){
                    int r, g, b, a;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        a = A_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        a += A_VAL(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;

                        A_VAL(dptr) = a;
                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;

                        dptr++;
                    }
                    else
                        *dptr++ = sptr[xpoints[x] ];
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if(isi->xup_yup == 1){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        unsigned int *pix;
        int r, g, b, a, rr, gg, bb, aa;
        int yap;
		 
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * yap) >> 10;
                g = (G_VAL(pix) * yap) >> 10;
                b = (B_VAL(pix) * yap) >> 10;
                a = (A_VAL(pix) * yap) >> 10;
                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix += sow;
                    r += (R_VAL(pix) * Cy) >> 10;
                    g += (G_VAL(pix) * Cy) >> 10;
                    b += (B_VAL(pix) * Cy) >> 10;
                    a += (A_VAL(pix) * Cy) >> 10;
                }
                if(j > 0){
                    pix += sow;
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                    a += (A_VAL(pix) * j) >> 10;
                }
                if(XAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr = (R_VAL(pix) * yap) >> 10;
                    gg = (G_VAL(pix) * yap) >> 10;
                    bb = (B_VAL(pix) * yap) >> 10;
                    aa = (A_VAL(pix) * yap) >> 10;
                    for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                        pix += sow;
                        rr += (R_VAL(pix) * Cy) >> 10;
                        gg += (G_VAL(pix) * Cy) >> 10;
                        bb += (B_VAL(pix) * Cy) >> 10;
                        aa += (A_VAL(pix) * Cy) >> 10;
                    }
                    if(j > 0){
                        pix += sow;
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                        aa += (A_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_XAP;
                    g = g * INV_XAP;
                    b = b * INV_XAP;
                    a = a * INV_XAP;
                    r = (r + ((rr * XAP))) >> 12;
                    g = (g + ((gg * XAP))) >> 12;
                    b = (b + ((bb * XAP))) >> 12;
                    a = (a + ((aa * XAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                A_VAL(dptr) = a;
                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;
                
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if(isi->xup_yup == 2){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        unsigned int *pix;
        int r, g, b, a, rr, gg, bb, aa;
        int xap;

        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * xap) >> 10;
                g = (G_VAL(pix) * xap) >> 10;
                b = (B_VAL(pix) * xap) >> 10;
                a = (A_VAL(pix) * xap) >> 10;
                for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                    pix++;
                    r += (R_VAL(pix) * Cx) >> 10;
                    g += (G_VAL(pix) * Cx) >> 10;
                    b += (B_VAL(pix) * Cx) >> 10;
                    a += (A_VAL(pix) * Cx) >> 10;
                }
                if(j > 0){
                    pix++;
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                    a += (A_VAL(pix) * j) >> 10;
                }
                if(YAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr = (R_VAL(pix) * xap) >> 10;
                    gg = (G_VAL(pix) * xap) >> 10;
                    bb = (B_VAL(pix) * xap) >> 10;
                    aa = (A_VAL(pix) * xap) >> 10;
                    for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                        pix++;
                        rr += (R_VAL(pix) * Cx) >> 10;
                        gg += (G_VAL(pix) * Cx) >> 10;
                        bb += (B_VAL(pix) * Cx) >> 10;
                        aa += (A_VAL(pix) * Cx) >> 10;
                    }
                    if(j > 0){
                        pix++;
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                        aa += (A_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_YAP;
                    g = g * INV_YAP;
                    b = b * INV_YAP;
                    a = a * INV_YAP;
                    r = (r + ((rr * YAP))) >> 12;
                    g = (g + ((gg * YAP))) >> 12;
                    b = (b + ((bb * YAP))) >> 12;
                    a = (a + ((aa * YAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                A_VAL(dptr) = a;
                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;
                
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally & vertically */
    else{
         /*\ 'Correct' version, with math units prepared for MMXification:
         |*|  The operation 'b = (b * c) >> 16' translates to pmulhw,
         |*|  so the operation 'b = (b * c) >> d' would translate to
         |*|  psllw (16 - d), %mmb; pmulh %mmc, %mmb
         \*/
        int Cx, Cy, i, j;
        unsigned int *pix;
        int a, r, g, b, ax, rx, gx, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix = sptr;
                sptr += sow;
                rx = (R_VAL(pix) * xap) >> 9;
                gx = (G_VAL(pix) * xap) >> 9;
                bx = (B_VAL(pix) * xap) >> 9;
                ax = (A_VAL(pix) * xap) >> 9;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    rx += (R_VAL(pix) * Cx) >> 9;
                    gx += (G_VAL(pix) * Cx) >> 9;
                    bx += (B_VAL(pix) * Cx) >> 9;
                    ax += (A_VAL(pix) * Cx) >> 9;
                    pix++;
                }
                if(i > 0){
                    rx += (R_VAL(pix) * i) >> 9;
                    gx += (G_VAL(pix) * i) >> 9;
                    bx += (B_VAL(pix) * i) >> 9;
                    ax += (A_VAL(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;
                a = (ax * yap) >> 14;


                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    ax = (A_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ax += (A_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                        ax += (A_VAL(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                    a += (ax * Cy) >> 14;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    ax = (A_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ax += (A_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                        ax += (A_VAL(pix) * i) >> 9;
                    }

                    r += (rx * j) >> 14;
                    g += (gx * j) >> 14;
                    b += (bx * j) >> 14;
                    a += (ax * j) >> 14;
                }

                R_VAL(dptr) = r >> 5;
                G_VAL(dptr) = g >> 5;
                B_VAL(dptr) = b >> 5;
                A_VAL(dptr) = a >> 5;
                dptr++;
            }
        }
    }
}

/* scale by area sampling - IGNORE the ALPHA byte*/
void DImgScale::dimgScaleAARGB(DImgScaleInfo *isi, unsigned int *dest,
                                   int dxx, int dyy, int dx, int dy, int dw,
                                   int dh, int dow, int sow)
{
    unsigned int *sptr, *dptr;
    int x, y, end;
    unsigned int **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;
    int *xapoints = isi->xapoints;
    int *yapoints = isi->yapoints;

    end = dxx + dw;
    /* scaling up both ways */
    if(isi->xup_yup == 3){
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            /* calculate the source line we'll scan from */
            dptr = dest + dx + ((y + dy) * dow);
            sptr = ypoints[dyy + y];
            if(YAP > 0){
                for(x = dxx; x < end; x++){
                    int r = 0, g = 0, b = 0;
                    int rr = 0, gg = 0, bb = 0;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        pix += sow;
                        rr = R_VAL(pix) * XAP;
                        gg = G_VAL(pix) * XAP;
                        bb = B_VAL(pix) * XAP;
                        pix --;
                        rr += R_VAL(pix) * INV_XAP;
                        gg += G_VAL(pix) * INV_XAP;
                        bb += B_VAL(pix) * INV_XAP;
                        r = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b = ((bb * YAP) + (b * INV_YAP)) >> 16;

                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;
                        
                        dptr++;
                    }
                    else{
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_YAP;
                        g = G_VAL(pix) * INV_YAP;
                        b = B_VAL(pix) * INV_YAP;
                        pix += sow;
                        r += R_VAL(pix) * YAP;
                        g += G_VAL(pix) * YAP;
                        b += B_VAL(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;
                        
                        dptr++;
                    }
                }
            }
            else{
                for(x = dxx; x < end; x++){
                    int r = 0, g = 0, b = 0;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;
                        
                        dptr++;
                    }
                    else
                        *dptr++ = sptr[xpoints[x] ];
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if(isi->xup_yup == 1){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        unsigned int *pix;
        int r, g, b, rr, gg, bb;
        int yap;

        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * yap) >> 10;
                g = (G_VAL(pix) * yap) >> 10;
                b = (B_VAL(pix) * yap) >> 10;
                pix += sow;
                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    r += (R_VAL(pix) * Cy) >> 10;
                    g += (G_VAL(pix) * Cy) >> 10;
                    b += (B_VAL(pix) * Cy) >> 10;
                    pix += sow;
                }
                if(j > 0){
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                }
                if(XAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr = (R_VAL(pix) * yap) >> 10;
                    gg = (G_VAL(pix) * yap) >> 10;
                    bb = (B_VAL(pix) * yap) >> 10;
                    pix += sow;
                    for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                        rr += (R_VAL(pix) * Cy) >> 10;
                        gg += (G_VAL(pix) * Cy) >> 10;
                        bb += (B_VAL(pix) * Cy) >> 10;
                        pix += sow;
                    }
                    if(j > 0){
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_XAP;
                    g = g * INV_XAP;
                    b = b * INV_XAP;
                    r = (r + ((rr * XAP))) >> 12;
                    g = (g + ((gg * XAP))) >> 12;
                    b = (b + ((bb * XAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;
                
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if(isi->xup_yup == 2){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        unsigned int *pix;
        int r, g, b, rr, gg, bb;
        int xap;

        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * xap) >> 10;
                g = (G_VAL(pix) * xap) >> 10;
                b = (B_VAL(pix) * xap) >> 10;
                pix++;
                for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                    r += (R_VAL(pix) * Cx) >> 10;
                    g += (G_VAL(pix) * Cx) >> 10;
                    b += (B_VAL(pix) * Cx) >> 10;
                    pix++;
                }
                if(j > 0){
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                }
                if(YAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr = (R_VAL(pix) * xap) >> 10;
                    gg = (G_VAL(pix) * xap) >> 10;
                    bb = (B_VAL(pix) * xap) >> 10;
                    pix++;
                    for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                        rr += (R_VAL(pix) * Cx) >> 10;
                        gg += (G_VAL(pix) * Cx) >> 10;
                        bb += (B_VAL(pix) * Cx) >> 10;
                        pix++;
                    }
                    if(j > 0){
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_YAP;
                    g = g * INV_YAP;
                    b = b * INV_YAP;
                    r = (r + ((rr * YAP))) >> 12;
                    g = (g + ((gg * YAP))) >> 12;
                    b = (b + ((bb * YAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;
                
                dptr++;
            }
        }
    }
    /* fully optimized (i think) - onyl change of algorithm can help */
    /* if we're scaling down horizontally & vertically */
    else{
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, Cy, i, j;
        unsigned int *pix;
        int r, g, b, rx, gx, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix = sptr;
                sptr += sow;
                rx = (R_VAL(pix) * xap) >> 9;
                gx = (G_VAL(pix) * xap) >> 9;
                bx = (B_VAL(pix) * xap) >> 9;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    rx += (R_VAL(pix) * Cx) >> 9;
                    gx += (G_VAL(pix) * Cx) >> 9;
                    bx += (B_VAL(pix) * Cx) >> 9;
                    pix++;
                }
                if(i > 0){
                    rx += (R_VAL(pix) * i) >> 9;
                    gx += (G_VAL(pix) * i) >> 9;
                    bx += (B_VAL(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;

                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                    }

                    r += (rx * j) >> 14;
                    g += (gx * j) >> 14;
                    b += (bx * j) >> 14;
                }

                R_VAL(dptr) = r >> 5;
                G_VAL(dptr) = g >> 5;
                B_VAL(dptr) = b >> 5;
                dptr++;
            }
        }
    }
}

#define A_VAL16(p) ((ushort *)(p))[3]
#define R_VAL16(p) ((ushort *)(p))[2]
#define G_VAL16(p) ((ushort *)(p))[1]
#define B_VAL16(p) ((ushort *)(p))[0]

// scale by area sampling - IGNORE the ALPHA byte
void DImgScale::dimgScaleAARGB16(DImgScaleInfo *isi, ullong *dest,
                                     int dxx, int dyy, int dw, int dh, 
				     int dow, int sow)
{
    ullong *sptr, *dptr;
    int x, y, end;
    ullong **ypoints  = isi->ypoints16;
    int     *xpoints  = isi->xpoints;
    int     *xapoints = isi->xapoints;
    int     *yapoints = isi->yapoints;
    
    end = dxx + dw;

    // scaling up both ways 
    if(isi->xup_yup == 3){
        
        // go through every scanline in the output buffer 
        for(y = 0; y < dh; y++){
            // calculate the source line we'll scan from 
            dptr = dest + (y * dow);
            sptr = ypoints[dyy + y];
            if(YAP > 0){
                for(x = dxx; x < end; x++){
                    llong r = 0, g = 0, b = 0;
                    llong rr = 0, gg = 0, bb = 0;
                    ullong *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL16(pix) * INV_XAP;
                        g = G_VAL16(pix) * INV_XAP;
                        b = B_VAL16(pix) * INV_XAP;
                        pix++;
                        r += R_VAL16(pix) * XAP;
                        g += G_VAL16(pix) * XAP;
                        b += B_VAL16(pix) * XAP;
                        pix += sow;
                        rr = R_VAL16(pix) * XAP;
                        gg = G_VAL16(pix) * XAP;
                        bb = B_VAL16(pix) * XAP;
                        pix --;
                        rr += R_VAL16(pix) * INV_XAP;
                        gg += G_VAL16(pix) * INV_XAP;
                        bb += B_VAL16(pix) * INV_XAP;
                        r = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b = ((bb * YAP) + (b * INV_YAP)) >> 16;

			R_VAL16(dptr) = r;
			G_VAL16(dptr) = g;
			B_VAL16(dptr) = b;

			dptr++;
                    }
                    else{
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL16(pix) * INV_YAP;
                        g = G_VAL16(pix) * INV_YAP;
                        b = B_VAL16(pix) * INV_YAP;
                        pix += sow;
                        r += R_VAL16(pix) * YAP;
                        g += G_VAL16(pix) * YAP;
                        b += B_VAL16(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

			R_VAL16(dptr) = r;
			G_VAL16(dptr) = g;
			B_VAL16(dptr) = b;

			dptr++;
                    }
                }
            }
            else{
                for(x = dxx; x < end; x++){
                    llong r = 0, g = 0, b = 0;
                    ullong *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL16(pix) * INV_XAP;
                        g = G_VAL16(pix) * INV_XAP;
                        b = B_VAL16(pix) * INV_XAP;
                        pix++;
                        r += R_VAL16(pix) * XAP;
                        g += G_VAL16(pix) * XAP;
                        b += B_VAL16(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

			R_VAL16(dptr) = r;
			G_VAL16(dptr) = g;
			B_VAL16(dptr) = b;

			dptr++;
                    }
                    else
                        *dptr++ = sptr[xpoints[x] ];
                }
            }
        }
    } 
    // if we're scaling down vertically 
    else if(isi->xup_yup == 1){

        // 'Correct' version, with math units prepared for MMXification 
        int Cy, j;
        ullong *pix;
        llong r, g, b, rr, gg, bb;
        int yap;

        // go through every scanline in the output buffer 
        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + y * dow;
            for(x = dxx; x < end; x++){
                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL16(pix) * yap) >> 10;
                g = (G_VAL16(pix) * yap) >> 10;
                b = (B_VAL16(pix) * yap) >> 10;
                pix += sow;
                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    r += (R_VAL16(pix) * Cy) >> 10;
                    g += (G_VAL16(pix) * Cy) >> 10;
                    b += (B_VAL16(pix) * Cy) >> 10;
                    pix += sow;
                }
                if(j > 0){
                    r += (R_VAL16(pix) * j) >> 10;
                    g += (G_VAL16(pix) * j) >> 10;
                    b += (B_VAL16(pix) * j) >> 10;
                }
                if(XAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr = (R_VAL16(pix) * yap) >> 10;
                    gg = (G_VAL16(pix) * yap) >> 10;
                    bb = (B_VAL16(pix) * yap) >> 10;
                    pix += sow;
                    for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                        rr += (R_VAL16(pix) * Cy) >> 10;
                        gg += (G_VAL16(pix) * Cy) >> 10;
                        bb += (B_VAL16(pix) * Cy) >> 10;
                        pix += sow;
                    }
                    if(j > 0){
                        rr += (R_VAL16(pix) * j) >> 10;
                        gg += (G_VAL16(pix) * j) >> 10;
                        bb += (B_VAL16(pix) * j) >> 10;
                    }
                    r = r * INV_XAP;
                    g = g * INV_XAP;
                    b = b * INV_XAP;
                    r = (r + ((rr * XAP))) >> 12;
                    g = (g + ((gg * XAP))) >> 12;
                    b = (b + ((bb * XAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

		R_VAL16(dptr) = r;
		G_VAL16(dptr) = g;
		B_VAL16(dptr) = b;
                dptr++;
            }
        }
    }
    // if we're scaling down horizontally 
    else if(isi->xup_yup == 2){
        // 'Correct' version, with math units prepared for MMXification 
        int Cx, j;
        ullong *pix;
        llong r, g, b, rr, gg, bb;
        int xap;

        // go through every scanline in the output buffer 
        for(y = 0; y < dh; y++){
            dptr = dest + y * dow;
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL16(pix) * xap) >> 10;
                g = (G_VAL16(pix) * xap) >> 10;
                b = (B_VAL16(pix) * xap) >> 10;
                pix++;
                for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                    r += (R_VAL16(pix) * Cx) >> 10;
                    g += (G_VAL16(pix) * Cx) >> 10;
                    b += (B_VAL16(pix) * Cx) >> 10;
                    pix++;
                }
                if(j > 0){
                    r += (R_VAL16(pix) * j) >> 10;
                    g += (G_VAL16(pix) * j) >> 10;
                    b += (B_VAL16(pix) * j) >> 10;
                }
                if(YAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr = (R_VAL16(pix) * xap) >> 10;
                    gg = (G_VAL16(pix) * xap) >> 10;
                    bb = (B_VAL16(pix) * xap) >> 10;
                    pix++;
                    for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                        rr += (R_VAL16(pix) * Cx) >> 10;
                        gg += (G_VAL16(pix) * Cx) >> 10;
                        bb += (B_VAL16(pix) * Cx) >> 10;
                        pix++;
                    }
                    if(j > 0){
                        rr += (R_VAL16(pix) * j) >> 10;
                        gg += (G_VAL16(pix) * j) >> 10;
                        bb += (B_VAL16(pix) * j) >> 10;
                    }
                    r = r * INV_YAP;
                    g = g * INV_YAP;
                    b = b * INV_YAP;
                    r = (r + ((rr * YAP))) >> 12;
                    g = (g + ((gg * YAP))) >> 12;
                    b = (b + ((bb * YAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

		R_VAL16(dptr) = r;
		G_VAL16(dptr) = g;
		B_VAL16(dptr) = b;
                dptr++;
            }
        }
    }
    // fully optimized (i think) - onyl change of algorithm can help 
    // if we're scaling down horizontally & vertically 
    else
    {
        // 'Correct' version, with math units prepared for MMXification
        int Cx, Cy, i, j;
        ullong *pix;
        llong r, g, b, rx, gx, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

	    dptr = dest + y * dow;

            for(x = dxx; x < end; x++) {

                Cx  = XAP >> 16;
                xap = XAP & 0xffff;

                sptr  = ypoints[dyy + y] + xpoints[x];
                pix   = sptr;
                sptr += sow;

                rx = (R_VAL16(pix) * xap) >> 9;
                gx = (G_VAL16(pix) * xap) >> 9;
                bx = (B_VAL16(pix) * xap) >> 9;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    rx += (R_VAL16(pix) * Cx) >> 9;
                    gx += (G_VAL16(pix) * Cx) >> 9;
                    bx += (B_VAL16(pix) * Cx) >> 9;
                    pix++;
                }
                if(i > 0){
                    rx += (R_VAL16(pix) * i) >> 9;
                    gx += (G_VAL16(pix) * i) >> 9;
                    bx += (B_VAL16(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;

                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL16(pix) * xap) >> 9;
                    gx = (G_VAL16(pix) * xap) >> 9;
                    bx = (B_VAL16(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL16(pix) * i) >> 9;
                        gx += (G_VAL16(pix) * i) >> 9;
                        bx += (B_VAL16(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL16(pix) * xap) >> 9;
                    gx = (G_VAL16(pix) * xap) >> 9;
                    bx = (B_VAL16(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL16(pix) * i) >> 9;
                        gx += (G_VAL16(pix) * i) >> 9;
                        bx += (B_VAL16(pix) * i) >> 9;
                    }

                    r += (rx * j) >> 14;
                    g += (gx * j) >> 14;
                    b += (bx * j) >> 14;
                }

                R_VAL16(dptr) = r >> 5;
                G_VAL16(dptr) = g >> 5;
                B_VAL16(dptr) = b >> 5;
                dptr++;
            }
        }
    }
}

/* scale by area sampling */
void DImgScale::dimgScaleAARGBA16(DImgScaleInfo *isi, ullong *dest,
                                      int dxx, int dyy,
                                      int dw,  int dh,
                                      int dow, int sow)
{
    ullong *sptr, *dptr;
    int x, y, end;
    ullong **ypoints = isi->ypoints16;
    int *xpoints = isi->xpoints;
    int *xapoints = isi->xapoints;
    int *yapoints = isi->yapoints;

    end = dxx + dw;
    /* scaling up both ways */
    if(isi->xup_yup == 3){
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            /* calculate the source line we'll scan from */
            dptr = dest + (y * dow);
            sptr = ypoints[dyy + y];
            if(YAP > 0){
                for(x = dxx; x < end; x++){
                    llong r, g, b, a;
                    llong rr, gg, bb, aa;
                    ullong *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL16(pix) * INV_XAP;
                        g = G_VAL16(pix) * INV_XAP;
                        b = B_VAL16(pix) * INV_XAP;
                        a = A_VAL16(pix) * INV_XAP;
                        pix++;
                        r += R_VAL16(pix) * XAP;
                        g += G_VAL16(pix) * XAP;
                        b += B_VAL16(pix) * XAP;
                        a += A_VAL16(pix) * XAP;
                        pix += sow;
                        rr = R_VAL16(pix) * XAP;
                        gg = G_VAL16(pix) * XAP;
                        bb = B_VAL16(pix) * XAP;
                        aa = A_VAL16(pix) * XAP;
                        pix--;
                        rr += R_VAL16(pix) * INV_XAP;
                        gg += G_VAL16(pix) * INV_XAP;
                        bb += B_VAL16(pix) * INV_XAP;
                        aa += A_VAL16(pix) * INV_XAP;
                        r = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b = ((bb * YAP) + (b * INV_YAP)) >> 16;
                        a = ((aa * YAP) + (a * INV_YAP)) >> 16;

			R_VAL16(dptr) = r;
			G_VAL16(dptr) = g;
			B_VAL16(dptr) = b;
                        A_VAL16(dptr) = a;

			dptr++;
                    }
                    else{
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL16(pix) * INV_YAP;
                        g = G_VAL16(pix) * INV_YAP;
                        b = B_VAL16(pix) * INV_YAP;
                        a = A_VAL16(pix) * INV_YAP;
                        pix += sow;
                        r += R_VAL16(pix) * YAP;
                        g += G_VAL16(pix) * YAP;
                        b += B_VAL16(pix) * YAP;
                        a += A_VAL16(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;
                        
			R_VAL16(dptr) = r;
			G_VAL16(dptr) = g;
			B_VAL16(dptr) = b;
                        A_VAL16(dptr) = a;

			dptr++;
                    }
                }
            }
            else{
                for(x = dxx; x < end; x++){
                    llong r, g, b, a;
                    ullong *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL16(pix) * INV_XAP;
                        g = G_VAL16(pix) * INV_XAP;
                        b = B_VAL16(pix) * INV_XAP;
                        a = A_VAL16(pix) * INV_XAP;
                        pix++;
                        r += R_VAL16(pix) * XAP;
                        g += G_VAL16(pix) * XAP;
                        b += B_VAL16(pix) * XAP;
                        a += A_VAL16(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;

			R_VAL16(dptr) = r;
			G_VAL16(dptr) = g;
			B_VAL16(dptr) = b;
                        A_VAL16(dptr) = a;

			dptr++;
                    }
                    else
                        *dptr++ = sptr[xpoints[x] ];
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if(isi->xup_yup == 1){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        ullong *pix;
        llong r, g, b, a, rr, gg, bb, aa;
        int yap;
		 
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + (y * dow);
            for(x = dxx; x < end; x++){
                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL16(pix) * yap) >> 10;
                g = (G_VAL16(pix) * yap) >> 10;
                b = (B_VAL16(pix) * yap) >> 10;
                a = (A_VAL16(pix) * yap) >> 10;
                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix += sow;
                    r += (R_VAL16(pix) * Cy) >> 10;
                    g += (G_VAL16(pix) * Cy) >> 10;
                    b += (B_VAL16(pix) * Cy) >> 10;
                    a += (A_VAL16(pix) * Cy) >> 10;
                }
                if(j > 0){
                    pix += sow;
                    r += (R_VAL16(pix) * j) >> 10;
                    g += (G_VAL16(pix) * j) >> 10;
                    b += (B_VAL16(pix) * j) >> 10;
                    a += (A_VAL16(pix) * j) >> 10;
                }
                if(XAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr = (R_VAL16(pix) * yap) >> 10;
                    gg = (G_VAL16(pix) * yap) >> 10;
                    bb = (B_VAL16(pix) * yap) >> 10;
                    aa = (A_VAL16(pix) * yap) >> 10;
                    for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                        pix += sow;
                        rr += (R_VAL16(pix) * Cy) >> 10;
                        gg += (G_VAL16(pix) * Cy) >> 10;
                        bb += (B_VAL16(pix) * Cy) >> 10;
                        aa += (A_VAL16(pix) * Cy) >> 10;
                    }
                    if(j > 0){
                        pix += sow;
                        rr += (R_VAL16(pix) * j) >> 10;
                        gg += (G_VAL16(pix) * j) >> 10;
                        bb += (B_VAL16(pix) * j) >> 10;
                        aa += (A_VAL16(pix) * j) >> 10;
                    }
                    r = r * INV_XAP;
                    g = g * INV_XAP;
                    b = b * INV_XAP;
                    a = a * INV_XAP;
                    r = (r + ((rr * XAP))) >> 12;
                    g = (g + ((gg * XAP))) >> 12;
                    b = (b + ((bb * XAP))) >> 12;
                    a = (a + ((aa * XAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                R_VAL16(dptr) = r;
                G_VAL16(dptr) = g;
                B_VAL16(dptr) = b;
                A_VAL16(dptr) = a;
                
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if(isi->xup_yup == 2){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        ullong *pix;
        llong r, g, b, a, rr, gg, bb, aa;
        int xap;

        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            dptr = dest + y * dow;
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL16(pix) * xap) >> 10;
                g = (G_VAL16(pix) * xap) >> 10;
                b = (B_VAL16(pix) * xap) >> 10;
                a = (A_VAL16(pix) * xap) >> 10;
                for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                    pix++;
                    r += (R_VAL16(pix) * Cx) >> 10;
                    g += (G_VAL16(pix) * Cx) >> 10;
                    b += (B_VAL16(pix) * Cx) >> 10;
                    a += (A_VAL16(pix) * Cx) >> 10;
                }
                if(j > 0){
                    pix++;
                    r += (R_VAL16(pix) * j) >> 10;
                    g += (G_VAL16(pix) * j) >> 10;
                    b += (B_VAL16(pix) * j) >> 10;
                    a += (A_VAL16(pix) * j) >> 10;
                }
                if(YAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr = (R_VAL16(pix) * xap) >> 10;
                    gg = (G_VAL16(pix) * xap) >> 10;
                    bb = (B_VAL16(pix) * xap) >> 10;
                    aa = (A_VAL16(pix) * xap) >> 10;
                    for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                        pix++;
                        rr += (R_VAL16(pix) * Cx) >> 10;
                        gg += (G_VAL16(pix) * Cx) >> 10;
                        bb += (B_VAL16(pix) * Cx) >> 10;
                        aa += (A_VAL16(pix) * Cx) >> 10;
                    }
                    if(j > 0){
                        pix++;
                        rr += (R_VAL16(pix) * j) >> 10;
                        gg += (G_VAL16(pix) * j) >> 10;
                        bb += (B_VAL16(pix) * j) >> 10;
                        aa += (A_VAL16(pix) * j) >> 10;
                    }
                    r = r * INV_YAP;
                    g = g * INV_YAP;
                    b = b * INV_YAP;
                    a = a * INV_YAP;
                    r = (r + ((rr * YAP))) >> 12;
                    g = (g + ((gg * YAP))) >> 12;
                    b = (b + ((bb * YAP))) >> 12;
                    a = (a + ((aa * YAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                R_VAL16(dptr) = r;
                G_VAL16(dptr) = g;
                B_VAL16(dptr) = b;
                A_VAL16(dptr) = a;
                
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally & vertically */
    else{
         /*\ 'Correct' version, with math units prepared for MMXification:
         |*|  The operation 'b = (b * c) >> 16' translates to pmulhw,
         |*|  so the operation 'b = (b * c) >> d' would translate to
         |*|  psllw (16 - d), %mmb; pmulh %mmc, %mmb
         \*/
        int Cx, Cy, i, j;
        ullong *pix;
        llong a, r, g, b, ax, rx, gx, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + y * dow;
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix = sptr;
                sptr += sow;
                rx = (R_VAL16(pix) * xap) >> 9;
                gx = (G_VAL16(pix) * xap) >> 9;
                bx = (B_VAL16(pix) * xap) >> 9;
                ax = (A_VAL16(pix) * xap) >> 9;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    rx += (R_VAL16(pix) * Cx) >> 9;
                    gx += (G_VAL16(pix) * Cx) >> 9;
                    bx += (B_VAL16(pix) * Cx) >> 9;
                    ax += (A_VAL16(pix) * Cx) >> 9;
                    pix++;
                }
                if(i > 0){
                    rx += (R_VAL16(pix) * i) >> 9;
                    gx += (G_VAL16(pix) * i) >> 9;
                    bx += (B_VAL16(pix) * i) >> 9;
                    ax += (A_VAL16(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;
                a = (ax * yap) >> 14;


                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL16(pix) * xap) >> 9;
                    gx = (G_VAL16(pix) * xap) >> 9;
                    bx = (B_VAL16(pix) * xap) >> 9;
                    ax = (A_VAL16(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        ax += (A_VAL16(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL16(pix) * i) >> 9;
                        gx += (G_VAL16(pix) * i) >> 9;
                        bx += (B_VAL16(pix) * i) >> 9;
                        ax += (A_VAL16(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                    a += (ax * Cy) >> 14;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL16(pix) * xap) >> 9;
                    gx = (G_VAL16(pix) * xap) >> 9;
                    bx = (B_VAL16(pix) * xap) >> 9;
                    ax = (A_VAL16(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        ax += (A_VAL16(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL16(pix) * i) >> 9;
                        gx += (G_VAL16(pix) * i) >> 9;
                        bx += (B_VAL16(pix) * i) >> 9;
                        ax += (A_VAL16(pix) * i) >> 9;
                    }

                    r += (rx * j) >> 14;
                    g += (gx * j) >> 14;
                    b += (bx * j) >> 14;
                    a += (ax * j) >> 14;
                }

                R_VAL16(dptr) = r >> 5;
                G_VAL16(dptr) = g >> 5;
                B_VAL16(dptr) = b >> 5;
                A_VAL16(dptr) = a >> 5;
                dptr++;
            }
        }
    }
}

/*
//Documentation of the cryptic dimgScaleAARGBA
dimgScaleAARGBA(
DImgScaleInfo *isi, // scaleinfo
unsigned int *dest,   // destination img data
int dxx,              // destination x location corresponding to start x of src section
int dyy,              // destination y location corresponding to start y of src section
int dx,               // destination x start location
int dy,               // destination y start location
int dw,               // destination width
int dh,               // destination height
int dow,              // destination scanline width
int sow);             // src scanline width
*/

}  // NameSpace Digikam

