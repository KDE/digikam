/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : This is the normal smoothscale method,
 *               based on Imlib2's smoothscale. Added
 *               smoothScaleSection - Scaling only of a
 *               section of a image. Added 16bit image support
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Ported to C++/QImage by Daniel M. Duley
 * Following modification are (C) Daniel M. Duley
 * Changes include formatting, namespaces and other C++'ings, removal of old
 * #ifdef'ed code, and removal of unneeded border calculation code.
 * Original implementation: http://trac.enlightenment.org/e/browser/trunk/imlib2/src/lib/scale.c
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
 *
 * ============================================================ */

// C ANSI includes

extern "C"
{
#include <stdint.h>
}

// C++ includes

#include <cstring>
#include <cstdlib>
#include <cstdio>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dimg_p.h"

typedef uint64_t ullong;
typedef int64_t  llong;

namespace Digikam
{

namespace DImgScale
{

class DImgScaleInfo
{
public:

    DImgScaleInfo()
    {
        xpoints = 0;
        ypoints = 0;
        ypoints16 = 0;
        xapoints  = 0;
        yapoints  = 0;
        xup_yup   = 0;
    }

    ~DImgScaleInfo()
    {
        delete [] xpoints;
        delete [] ypoints;
        delete [] ypoints16;
        delete [] xapoints;
        delete [] yapoints;
    }

    int*     xpoints;
    uint**   ypoints;
    ullong** ypoints16;
    int*     xapoints;
    int*     yapoints;
    int      xup_yup;
};

uint**   dimgCalcYPoints(uint* const src, int sw, int sh, int dh);
ullong** dimgCalcYPoints16(ullong* const src, int sw, int sh, int dh);
int*     dimgCalcXPoints(int sw, int dw);
int*     dimgCalcApoints(int s, int d, int up);

DImgScaleInfo* dimgCalcScaleInfo(const DImg& img,
                                 int sw, int sh,
                                 int dw, int dh,
                                 bool sixteenBit,
                                 bool aa);

// 8 bit, not smoothed
void dimgSampleRGBA(DImgScaleInfo* const isi, uint* const dest,
                    int dxx, int dyy, int dw, int dh, int dow);

void dimgSampleRGBA(DImgScaleInfo* const isi, uint* const dest,
                    int dxx, int dyy, int dw, int dh, int dow,
                    int clip_dx, int clip_dy, int clip_dw, int clip_dh);

// 16 bit, not smoothed
void dimgSampleRGBA16(DImgScaleInfo* const isi, ullong* const dest,
                      int dxx, int dyy, int dw, int dh, int dow);

void dimgSampleRGBA16(DImgScaleInfo* const isi, ullong* const dest,
                      int dxx, int dyy, int dw, int dh, int dow,
                      int clip_dx, int clip_dy, int clip_dw, int clip_dh);

// 8 bit, RGBA
void dimgScaleAARGBA(DImgScaleInfo* const isi, uint* const dest,
                     int dxx, int dyy, int dw, int dh, int dow, int sow);

void dimgScaleAARGBA(DImgScaleInfo* const isi, uint* const dest,
                     int dxx, int dyy, int dw, int dh, int dow, int sow,
                     int clip_dx, int clip_dy, int clip_dw, int clip_dh);

// 8 bit, RGB
void dimgScaleAARGB(DImgScaleInfo* const isi, uint* const dest,
                    int dxx, int dyy, int dw, int dh, int dow, int sow);

void dimgScaleAARGB(DImgScaleInfo* const isi, uint* const dest,
                    int dxx, int dyy, int dw, int dh, int dow, int sow,
                    int clip_dx, int clip_dy, int clip_dw, int clip_dh);

// 16 bit, RGBA
void dimgScaleAARGBA16(DImgScaleInfo* const isi, ullong* const dest,
                       int dxx, int dyy, int dw, int dh,
                       int dow, int sow);

void dimgScaleAARGBA16(DImgScaleInfo* const isi, ullong* const dest,
                       int dxx, int dyy, int dw, int dh,
                       int dow, int sow,
                       int clip_dx, int clip_dy, int clip_dw, int clip_dh);

// 16 bit, RGB
void dimgScaleAARGB16(DImgScaleInfo* const isi, ullong* const dest,
                      int dxx, int dyy, int dw, int dh,
                      int dow, int sow);

void dimgScaleAARGB16(DImgScaleInfo* const isi, ullong* const dest,
                      int dxx, int dyy, int dw, int dh,
                      int dow, int sow,
                      int clip_dx, int clip_dy, int clip_dw, int clip_dh);
}

using namespace DImgScale;

/*
#define CLIP(x, y, w, h, xx, yy, ww, hh) \
if (x < (xx)) {w += (x - (xx)); x = (xx);} \
if (y < (yy)) {h += (y - (yy)); y = (yy);} \
if ((x + w) > ((xx) + (ww))) {w = (ww) - (x - xx);} \
if ((y + h) > ((yy) + (hh))) {h = (hh) - (y - yy);}
*/

DImg DImg::smoothScale(const QSize& destSize, Qt::AspectRatioMode aspectRatioMode) const
{
    QSize scaleSize = size();
    scaleSize.scale(destSize, aspectRatioMode);

    if (scaleSize.isEmpty())
    {
        return DImg();
    }

    return smoothScaleClipped(scaleSize, QRect(QPoint(0,0), scaleSize));
}

DImg DImg::smoothScale(int dw, int dh, Qt::AspectRatioMode aspectRatioMode) const
{
    return smoothScale(QSize(dw, dh), aspectRatioMode);
}

DImg DImg::smoothScaleClipped(const QSize& destSize, const QRect& clip) const
{
    return DImg::smoothScaleClipped(destSize.width(), destSize.height(),
                                    clip.x(), clip.y(), clip.width(), clip.height());
}

DImg DImg::smoothScaleClipped(int dw, int dh, int clipx, int clipy, int clipw, int cliph) const
{
    if (dw <= 0 || dh <= 0 || clipw <=0 || cliph <=0 || isNull())
    {
        return DImg();
    }

    uint w = width();
    uint h = height();

    if (w <= 0 || h <= 0)
    {
        return DImg();
    }

    // ensure clip is valid
    if (!Private::clipped(clipx, clipy, clipw, cliph, dw, dh))
    {
        return DImg();
    }

    // do we actually need to scale?
    if ((w == (uint)dw) && (h == (uint)dh))
    {
        if (clipw == dw && cliph == dh)
        {
            return copy();
        }
        else
        {
            return copy(clipx, clipy, clipw, cliph);
        }
    }

    DImgScaleInfo* scaleinfo = dimgCalcScaleInfo(*this, w, h, dw, dh, sixteenBit(), true);

    DImg buffer(*this, clipw, cliph);

    if (sixteenBit())
    {
        if (hasAlpha())
        {
            dimgScaleAARGBA16(scaleinfo, reinterpret_cast<ullong*>(buffer.bits()),
                              0, 0, dw, dh, clipw, w,
                              clipx, clipy, clipw, cliph);
        }
        else
        {
            dimgScaleAARGB16(scaleinfo, reinterpret_cast<ullong*>(buffer.bits()),
                             0, 0, dw, dh, clipw, w,
                             clipx, clipy, clipw, cliph);
        }
    }
    else
    {
        if (hasAlpha())
        {
            dimgScaleAARGBA(scaleinfo, reinterpret_cast<uint*>(buffer.bits()),
                            0, 0, dw, dh, clipw, w,
                            clipx, clipy, clipw, cliph);
        }
        else
        {
            dimgScaleAARGB(scaleinfo, reinterpret_cast<uint*>(buffer.bits()),
                           0, 0, dw, dh, clipw, w,
                           clipx, clipy, clipw, cliph);
        }
    }

    delete scaleinfo;

    return buffer;
}

DImg DImg::smoothScaleSection(const QRect& sourceRect, const QSize& destSize) const
{
    return smoothScaleSection(sourceRect.x(), sourceRect.y(), sourceRect.width(), sourceRect.height(),
                              destSize.width(), destSize.height());
}

DImg DImg::smoothScaleSection(int sx, int sy,
                              int sw, int sh,
                              int dw, int dh) const
{
    uint w = width();
    uint h = height();

    // sanity checks
    if ((dw <= 0) || (dh <= 0))
    {
        return DImg();
    }

    if ((sw <= 0) || (sh <= 0))
    {
        return DImg();
    }

    // clip the source rect to be within the actual image
    int  /*psx, psy,*/ psw, psh;
//    psx = sx;
//    psy = sy;
    psw = sw;
    psh = sh;

    if (!Private::clipped(sx, sy, sw, sh, w, h))
    {
        return DImg();
    }

    // clip output coords to clipped input coords
    if (psw != sw)
    {
        dw = (dw * sw) / psw;
    }

    if (psh != sh)
    {
        dh = (dh * sh) / psh;
    }

    // do a second check to see if we now have invalid coords
    // do not do anything if we have a 0 width or height image to render
    if ((dw <= 0) || (dh <= 0))
    {
        return DImg();
    }

    // if the input rect size < 0 do not render either
    if ((sw <= 0) || (sh <= 0))
    {
        return DImg();
    }

    // do we actually need to scale?
    if ((sw == dw) && (sh == dh))
    {
        return copy(sx, sy, sw, sh);
    }

    // calculate scaleinfo
    DImgScaleInfo* scaleinfo = dimgCalcScaleInfo(*this, sw, sh, dw, dh, sixteenBit(), true);

    DImg buffer(*this, dw, dh);

    if (sixteenBit())
    {
        if (hasAlpha())
        {
            dimgScaleAARGBA16(scaleinfo, reinterpret_cast<ullong*>(buffer.bits()),
                              ((sx * dw) / sw),
                              ((sy * dh) / sh),
                              dw, dh,
                              dw, w);
        }
        else
        {
            dimgScaleAARGB16(scaleinfo, reinterpret_cast<ullong*>(buffer.bits()),
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
                            reinterpret_cast<uint*>(buffer.bits()),
                            ((sx * dw) / sw),
                            ((sy * dh) / sh),
                            dw, dh,
                            dw, w);
        }
        else
        {
            dimgScaleAARGB(scaleinfo,
                           reinterpret_cast<uint*>(buffer.bits()),
                           ((sx * dw) / sw),
                           ((sy * dh) / sh),
                           dw, dh,
                           dw, w);
        }
    }

    delete scaleinfo;

    return buffer;
}

//
// Code ported from Imlib2...
//

// FIXME: replace with mRed, etc... These work on pointers to pixels, not
// pixel values
#define A_VAL(p) ((unsigned char*)(p))[3]
#define R_VAL(p) ((unsigned char*)(p))[2]
#define G_VAL(p) ((unsigned char*)(p))[1]
#define B_VAL(p) ((unsigned char*)(p))[0]

#define INV_XAP  (256 - xapoints[x])
#define XAP      (xapoints[x])
#define INV_YAP  (256 - yapoints[dyy + y])
#define YAP      (yapoints[dyy + y])

uint** DImgScale::dimgCalcYPoints(uint* const src, int sw, int sh, int dh)
{
    uint** p = 0;
    int i, j = 0;
    ullong val, inc;

    p = new uint* [dh+1];

    val = 0;
    inc = (((ullong)sh) << 16) / dh;

    for (i = 0; i < dh; ++i)
    {
        p[j++] = src + ((val >> 16) * sw);
        val += inc;
    }

    return(p);
}

ullong** DImgScale::dimgCalcYPoints16(ullong* const src, int sw, int sh, int dh)
{
    ullong** p = 0;
    int i, j   = 0;
    ullong val, inc;

    p = new ullong*[(dh+1)];

    val = 0;
    inc = (((ullong)sh) << 16) / dh;

    for (i = 0; i < dh; ++i)
    {
        p[j++] = src + ((val >> 16) * sw);
        val += inc;
    }

    return p;
}

int* DImgScale::dimgCalcXPoints(int sw, int dw)
{
    int* p=0, i, j = 0;
    ullong val, inc;

    p = new int[dw+1];

    val = 0;
    inc = (((ullong)sw) << 16) / dw;

    for (i = 0; i < dw; ++i)
    {
        p[j++] = (val >> 16);
        val    += inc;
    }

    return(p);
}

int* DImgScale::dimgCalcApoints(int s, int d, int up)
{
    int* p=0, i, j = 0;

    p = new int[d];

    /* scaling up */
    if (up)
    {
        ullong val, inc;

        val = 0;
        inc = (((ullong)s) << 16) / d;

        for (i = 0; i < d; ++i)
        {
            p[j++] = (val >> 8) - ((val >> 8) & 0xffffff00);

            if ((int)(val >> 16) >= (s - 1))
            {
                p[j - 1] = 0;
            }

            val    += inc;
        }
    }
    /* scaling down */
    else
    {
        ullong val, inc;
        int ap, Cp;
        val = 0;
        inc = (((ullong)s) << 16) / d;
        Cp  = ((d << 14) / s) + 1;

        for (i = 0; i < d; ++i)
        {
            ap   = ((0x100 - ((val >> 8) & 0xff)) * Cp) >> 8;
            p[j] = ap | (Cp << 16);
            ++j;
            val  += inc;
        }
    }

    return(p);
}

DImgScaleInfo* DImgScale::dimgCalcScaleInfo(const DImg& img,
        int sw, int sh,
        int dw, int dh,
        bool /*sixteenBit*/,
        bool aa)
{
    DImgScaleInfo* isi = new DImgScaleInfo;
    int scw, sch;

    scw = dw * img.width()  / sw;
    sch = dh * img.height() / sh;

    isi->xup_yup = (abs(dw) >= sw) + ((abs(dh) >= sh) << 1);

    isi->xpoints = dimgCalcXPoints(img.width(), scw);

    if (img.sixteenBit())
    {
        isi->ypoints   = 0;
        isi->ypoints16 = dimgCalcYPoints16(reinterpret_cast<ullong*>(img.bits()), img.width(), img.height(), sch);
    }
    else
    {
        isi->ypoints16 = 0;
        isi->ypoints   = dimgCalcYPoints(reinterpret_cast<uint*>(img.bits()), img.width(), img.height(), sch);
    }

    if (aa)
    {
        isi->xapoints = dimgCalcApoints(img.width(), scw, isi->xup_yup & 1);

        isi->yapoints = dimgCalcApoints(img.height(), sch, isi->xup_yup & 2);
    }
    else
    {
        isi->xapoints = new int[scw];

        for (int i = 0; i < scw; ++i)
        {
            isi->xapoints[i] = 0;
        }

        isi->yapoints = new int[sch];

        for (int i = 0; i < sch; ++i)
        {
            isi->yapoints[i] = 0;
        }
    }

    return isi;
}

/** scale by pixel sampling only */
void DImgScale::dimgSampleRGBA(DImgScaleInfo* const isi, uint* const dest,
                               int dxx, int dyy, int dw, int dh, int dow)
{
    dimgSampleRGBA(isi, dest, dxx, dyy, dw, dh, dow,
                   0, 0, dw, dh);
}

void DImgScale::dimgSampleRGBA(DImgScaleInfo* const isi, uint* const dest,
                               int dxx, int dyy, int dw, int dh, int dow,
                               int clip_dx, int clip_dy, int clip_dw, int clip_dh)
{
    Q_UNUSED(dw);
    Q_UNUSED(dh);
    uint* sptr=0;
    uint* dptr=0;
    int x, y;
    uint** ypoints = isi->ypoints;
    int* xpoints   = isi->xpoints;

    const int x_begin = dxx + clip_dx;     // no clip set = dxx
    const int x_end   = x_begin + clip_dw; // no clip set = dxx + dw
    const int y_begin = clip_dy;           // no clip set = 0
    const int y_end   = clip_dy + clip_dh; // no clip set = dh

    /* go through every scanline in the output buffer */
    for (y = y_begin; y < y_end; ++y)
    {
        /* get the pointer to the start of the destination scanline */
        dptr = dest + (y - y_begin) * dow;
        /* calculate the source line we'll scan from */
        sptr = ypoints[dyy + y];

        /* go through the scanline and copy across */
        for (x = x_begin; x < x_end; ++x)
        {
            *dptr++ = sptr[xpoints[x]];
        }
    }
}

void DImgScale::dimgSampleRGBA16(DImgScaleInfo* const isi, ullong* const dest,
                                 int dxx, int dyy, int dw, int dh, int dow)
{
    dimgSampleRGBA16(isi, dest, dxx, dyy, dw, dh, dow,
                     0, 0, dw, dh);
}

void DImgScale::dimgSampleRGBA16(DImgScaleInfo* const isi, ullong* const dest,
                                 int dxx, int dyy, int dw, int dh, int dow,
                                 int clip_dx, int clip_dy, int clip_dw, int clip_dh)
{
    Q_UNUSED(dw);
    Q_UNUSED(dh);
    ullong* sptr=0;
    ullong* dptr=0;
    int x, y;
    ullong** ypoints = isi->ypoints16;
    int* xpoints   = isi->xpoints;

    const int x_begin = dxx + clip_dx;     // no clip set = dxx
    const int x_end   = x_begin + clip_dw; // no clip set = dxx + dw
    const int y_begin = clip_dy;           // no clip set = 0
    const int y_end   = clip_dy + clip_dh; // no clip set = dh

    /* go through every scanline in the output buffer */
    for (y = y_begin; y < y_end; ++y)
    {
        /* get the pointer to the start of the destination scanline */
        dptr = dest + (y - y_begin) * dow;
        /* calculate the source line we'll scan from */
        sptr = ypoints[dyy + y];

        /* go through the scanline and copy across */
        for (x = x_begin; x < x_end; ++x)
        {
            *dptr++ = sptr[xpoints[x]];
        }
    }
}

/* FIXME: NEED to optimize ScaleAARGBA - currently its "ok" but needs work*/

/**
dimgScaleAARGBA : scale by area sampling. Arguments:
  DImgScaleInfo* isi,              // scaleinfo
  uint*  dest,             // destination img data
  int            dxx,              // destination x location corresponding to start x of src section
  int            dyy,              // destination y location corresponding to start y of src section
  int            dw,               // destination width
  int            dh,               // destination height
  int            dow,              // destination scanline width
  int            sow);             // src scanline width
*/

void DImgScale::dimgScaleAARGBA(DImgScaleInfo* const isi, uint* const dest,
                                int dxx, int dyy,
                                int dw, int dh,
                                int dow, int sow
                               )
{
    dimgScaleAARGBA(isi, dest, dxx, dyy, dw, dh, dow, sow,
                    0, 0, dw, dh);
}

void DImgScale::dimgScaleAARGBA(DImgScaleInfo* const isi, uint* const dest,
                                int dxx, int dyy,
                                int dw, int dh,
                                int dow, int sow,
                                int clip_dx, int clip_dy,
                                int clip_dw, int clip_dh
                               )
{
    Q_UNUSED(dw);
    Q_UNUSED(dh);
    uint* sptr=0;
    uint* dptr=0;
    int x, y;
    uint** ypoints = isi->ypoints;
    int* xpoints           = isi->xpoints;
    int* xapoints          = isi->xapoints;
    int* yapoints          = isi->yapoints;

    const int x_begin = dxx + clip_dx;     // no clip set = dxx
    const int x_end   = x_begin + clip_dw; // no clip set = dxx + dw
    const int y_begin = clip_dy;           // no clip set = 0
    const int y_end   = clip_dy + clip_dh; // no clip set = dh

    /* scaling up both ways */
    if (isi->xup_yup == 3)
    {
        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            /* calculate the source line we'll scan from */
            dptr = dest + (y - y_begin) * dow;
            sptr = ypoints[dyy + y];

            if (YAP > 0)
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    int r, g, b, a;
                    uint* pix=0;

                    if (XAP > 0)
                    {
                        int rr, gg, bb, aa;

                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL(pix) * INV_XAP;
                        g   = G_VAL(pix) * INV_XAP;
                        b   = B_VAL(pix) * INV_XAP;
                        a   = A_VAL(pix) * INV_XAP;
                        ++pix;
                        r   += R_VAL(pix) * XAP;
                        g   += G_VAL(pix) * XAP;
                        b   += B_VAL(pix) * XAP;
                        a   += A_VAL(pix) * XAP;
                        pix += sow;
                        rr  = R_VAL(pix) * XAP;
                        gg  = G_VAL(pix) * XAP;
                        bb  = B_VAL(pix) * XAP;
                        aa  = A_VAL(pix) * XAP;
                        --pix;
                        rr += R_VAL(pix) * INV_XAP;
                        gg += G_VAL(pix) * INV_XAP;
                        bb += B_VAL(pix) * INV_XAP;
                        aa += A_VAL(pix) * INV_XAP;
                        r  = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g  = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b  = ((bb * YAP) + (b * INV_YAP)) >> 16;
                        a  = ((aa * YAP) + (a * INV_YAP)) >> 16;

                        A_VAL(dptr) = a;
                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;

                        ++dptr;
                    }
                    else
                    {
                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL(pix) * INV_YAP;
                        g   = G_VAL(pix) * INV_YAP;
                        b   = B_VAL(pix) * INV_YAP;
                        a   = A_VAL(pix) * INV_YAP;
                        pix += sow;
                        r   += R_VAL(pix) * YAP;
                        g   += G_VAL(pix) * YAP;
                        b   += B_VAL(pix) * YAP;
                        a   += A_VAL(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;

                        A_VAL(dptr) = a;
                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;

                        ++dptr;
                    }
                }
            }
            else
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    uint* pix=0;

                    if (XAP > 0)
                    {
                        int r, g, b, a;

                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL(pix) * INV_XAP;
                        g   = G_VAL(pix) * INV_XAP;
                        b   = B_VAL(pix) * INV_XAP;
                        a   = A_VAL(pix) * INV_XAP;
                        ++pix;
                        r   += R_VAL(pix) * XAP;
                        g   += G_VAL(pix) * XAP;
                        b   += B_VAL(pix) * XAP;
                        a   += A_VAL(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;

                        A_VAL(dptr) = a;
                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;

                        ++dptr;
                    }
                    else
                    {
                        *dptr++ = sptr[xpoints[x] ];
                    }
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if (isi->xup_yup == 1)
    {
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        uint* pix=0;
        int r, g, b, a, rr, gg, bb, aa;
        int yap;

        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL(pix) * yap) >> 10;
                g   = (G_VAL(pix) * yap) >> 10;
                b   = (B_VAL(pix) * yap) >> 10;
                a   = (A_VAL(pix) * yap) >> 10;

                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    pix += sow;
                    r   += (R_VAL(pix) * Cy) >> 10;
                    g   += (G_VAL(pix) * Cy) >> 10;
                    b   += (B_VAL(pix) * Cy) >> 10;
                    a   += (A_VAL(pix) * Cy) >> 10;
                }

                if (j > 0)
                {
                    pix += sow;
                    r   += (R_VAL(pix) * j) >> 10;
                    g   += (G_VAL(pix) * j) >> 10;
                    b   += (B_VAL(pix) * j) >> 10;
                    a   += (A_VAL(pix) * j) >> 10;
                }

                if (XAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr  = (R_VAL(pix) * yap) >> 10;
                    gg  = (G_VAL(pix) * yap) >> 10;
                    bb  = (B_VAL(pix) * yap) >> 10;
                    aa  = (A_VAL(pix) * yap) >> 10;

                    for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                    {
                        pix += sow;
                        rr  += (R_VAL(pix) * Cy) >> 10;
                        gg  += (G_VAL(pix) * Cy) >> 10;
                        bb  += (B_VAL(pix) * Cy) >> 10;
                        aa  += (A_VAL(pix) * Cy) >> 10;
                    }

                    if (j > 0)
                    {
                        pix += sow;
                        rr  += (R_VAL(pix) * j) >> 10;
                        gg  += (G_VAL(pix) * j) >> 10;
                        bb  += (B_VAL(pix) * j) >> 10;
                        aa  += (A_VAL(pix) * j) >> 10;
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                A_VAL(dptr) = a;
                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;

                ++dptr;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if (isi->xup_yup == 2)
    {
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        uint* pix=0;
        int r, g, b, a, rr, gg, bb, aa;
        int xap;

        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx  = XAP >> 16;
                xap = XAP & 0xffff;
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL(pix) * xap) >> 10;
                g   = (G_VAL(pix) * xap) >> 10;
                b   = (B_VAL(pix) * xap) >> 10;
                a   = (A_VAL(pix) * xap) >> 10;

                for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                {
                    ++pix;
                    r += (R_VAL(pix) * Cx) >> 10;
                    g += (G_VAL(pix) * Cx) >> 10;
                    b += (B_VAL(pix) * Cx) >> 10;
                    a += (A_VAL(pix) * Cx) >> 10;
                }

                if (j > 0)
                {
                    ++pix;
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                    a += (A_VAL(pix) * j) >> 10;
                }

                if (YAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr  = (R_VAL(pix) * xap) >> 10;
                    gg  = (G_VAL(pix) * xap) >> 10;
                    bb  = (B_VAL(pix) * xap) >> 10;
                    aa  = (A_VAL(pix) * xap) >> 10;

                    for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                    {
                        ++pix;
                        rr += (R_VAL(pix) * Cx) >> 10;
                        gg += (G_VAL(pix) * Cx) >> 10;
                        bb += (B_VAL(pix) * Cx) >> 10;
                        aa += (A_VAL(pix) * Cx) >> 10;
                    }

                    if (j > 0)
                    {
                        ++pix;
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                A_VAL(dptr) = a;
                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;

                ++dptr;
            }
        }
    }
    /* if we're scaling down horizontally & vertically */
    else
    {
        /*\ 'Correct' version, with math units prepared for MMXification:
        |*|  The operation 'b = (b * c) >> 16' translates to pmulhw,
        |*|  so the operation 'b = (b * c) >> d' would translate to
        |*|  psllw (16 - d), %mmb; pmulh %mmc, %mmb
        \*/
        int Cx, Cy, i, j;
        uint* pix=0;
        int a, r, g, b, ax, rx, gx, bx;
        int xap, yap;

        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx  = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix  = sptr;
                sptr += sow;
                rx   = (R_VAL(pix) * xap) >> 9;
                gx   = (G_VAL(pix) * xap) >> 9;
                bx   = (B_VAL(pix) * xap) >> 9;
                ax   = (A_VAL(pix) * xap) >> 9;
                ++pix;

                for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                {
                    rx += (R_VAL(pix) * Cx) >> 9;
                    gx += (G_VAL(pix) * Cx) >> 9;
                    bx += (B_VAL(pix) * Cx) >> 9;
                    ax += (A_VAL(pix) * Cx) >> 9;
                    ++pix;
                }

                if (i > 0)
                {
                    rx += (R_VAL(pix) * i) >> 9;
                    gx += (G_VAL(pix) * i) >> 9;
                    bx += (B_VAL(pix) * i) >> 9;
                    ax += (A_VAL(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;
                a = (ax * yap) >> 14;

                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL(pix) * xap) >> 9;
                    gx   = (G_VAL(pix) * xap) >> 9;
                    bx   = (B_VAL(pix) * xap) >> 9;
                    ax   = (A_VAL(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ax += (A_VAL(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
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

                if (j > 0)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL(pix) * xap) >> 9;
                    gx   = (G_VAL(pix) * xap) >> 9;
                    bx   = (B_VAL(pix) * xap) >> 9;
                    ax   = (A_VAL(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ax += (A_VAL(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
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
                ++dptr;
            }
        }
    }
}

void DImgScale::dimgScaleAARGB(DImgScaleInfo* const isi, uint* const dest,
                               int dxx, int dyy,
                               int dw, int dh,
                               int dow, int sow
                              )
{
    dimgScaleAARGB(isi, dest, dxx, dyy, dw, dh, dow, sow,
                   0, 0, dw, dh);
}

/** scale by area sampling - IGNORE the ALPHA byte */
void DImgScale::dimgScaleAARGB(DImgScaleInfo* const isi, uint* const dest,
                               int dxx, int dyy,
                               int dw, int dh,
                               int dow, int sow,
                               int clip_dx, int clip_dy,
                               int clip_dw, int clip_dh)
{
    Q_UNUSED(dw);
    Q_UNUSED(dh);
    uint* sptr=0;
    uint* dptr=0;
    int x, y;
    uint** ypoints = isi->ypoints;
    int* xpoints           = isi->xpoints;
    int* xapoints          = isi->xapoints;
    int* yapoints          = isi->yapoints;

    const int x_begin = dxx + clip_dx;     // no clip set = dxx
    const int x_end   = x_begin + clip_dw; // no clip set = dxx + dw
    const int y_begin = clip_dy;           // no clip set = 0
    const int y_end   = clip_dy + clip_dh; // no clip set = dh

    /* scaling up both ways */
    if (isi->xup_yup == 3)
    {
        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            /* calculate the source line we'll scan from */
            dptr = dest + (y - y_begin) * dow;
            sptr = ypoints[dyy + y];

            if (YAP > 0)
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    int   r = 0, g = 0, b = 0;
                    uint* pix=0;

                    if (XAP > 0)
                    {
                        int rr = 0, gg = 0, bb = 0;

                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL(pix) * INV_XAP;
                        g   = G_VAL(pix) * INV_XAP;
                        b   = B_VAL(pix) * INV_XAP;
                        ++pix;
                        r   += R_VAL(pix) * XAP;
                        g   += G_VAL(pix) * XAP;
                        b   += B_VAL(pix) * XAP;
                        pix += sow;
                        rr  = R_VAL(pix) * XAP;
                        gg  = G_VAL(pix) * XAP;
                        bb  = B_VAL(pix) * XAP;
                        pix--;
                        rr += R_VAL(pix) * INV_XAP;
                        gg += G_VAL(pix) * INV_XAP;
                        bb += B_VAL(pix) * INV_XAP;
                        r  = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g  = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b  = ((bb * YAP) + (b * INV_YAP)) >> 16;

                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;
                        A_VAL(dptr) = 0xFF;

                        ++dptr;
                    }
                    else
                    {
                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL(pix) * INV_YAP;
                        g   = G_VAL(pix) * INV_YAP;
                        b   = B_VAL(pix) * INV_YAP;
                        pix += sow;
                        r   += R_VAL(pix) * YAP;
                        g   += G_VAL(pix) * YAP;
                        b   += B_VAL(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;
                        A_VAL(dptr) = 0xFF;

                        ++dptr;
                    }
                }
            }
            else
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    uint* pix=0;

                    if (XAP > 0)
                    {
                        int r = 0, g = 0, b = 0;

                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL(pix) * INV_XAP;
                        g   = G_VAL(pix) * INV_XAP;
                        b   = B_VAL(pix) * INV_XAP;
                        ++pix;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

                        R_VAL(dptr) = r;
                        G_VAL(dptr) = g;
                        B_VAL(dptr) = b;
                        A_VAL(dptr) = 0xFF;

                        ++dptr;
                    }
                    else
                    {
                        *dptr++ = sptr[xpoints[x] ];
                    }
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if (isi->xup_yup == 1)
    {
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        uint* pix=0;
        int r, g, b, rr, gg, bb;
        int yap;

        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL(pix) * yap) >> 10;
                g   = (G_VAL(pix) * yap) >> 10;
                b   = (B_VAL(pix) * yap) >> 10;
                pix += sow;

                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    r   += (R_VAL(pix) * Cy) >> 10;
                    g   += (G_VAL(pix) * Cy) >> 10;
                    b   += (B_VAL(pix) * Cy) >> 10;
                    pix += sow;
                }

                if (j > 0)
                {
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                }

                if (XAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr  = (R_VAL(pix) * yap) >> 10;
                    gg  = (G_VAL(pix) * yap) >> 10;
                    bb  = (B_VAL(pix) * yap) >> 10;
                    pix += sow;

                    for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                    {
                        rr  += (R_VAL(pix) * Cy) >> 10;
                        gg  += (G_VAL(pix) * Cy) >> 10;
                        bb  += (B_VAL(pix) * Cy) >> 10;
                        pix += sow;
                    }

                    if (j > 0)
                    {
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;
                A_VAL(dptr) = 0xFF;

                ++dptr;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if (isi->xup_yup == 2)
    {
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        uint* pix=0;
        int r, g, b, rr, gg, bb;
        int xap;

        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx  = XAP >> 16;
                xap = XAP & 0xffff;
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL(pix) * xap) >> 10;
                g   = (G_VAL(pix) * xap) >> 10;
                b   = (B_VAL(pix) * xap) >> 10;
                ++pix;

                for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                {
                    r += (R_VAL(pix) * Cx) >> 10;
                    g += (G_VAL(pix) * Cx) >> 10;
                    b += (B_VAL(pix) * Cx) >> 10;
                    ++pix;
                }

                if (j > 0)
                {
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                }

                if (YAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr  = (R_VAL(pix) * xap) >> 10;
                    gg  = (G_VAL(pix) * xap) >> 10;
                    bb  = (B_VAL(pix) * xap) >> 10;
                    ++pix;

                    for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                    {
                        rr += (R_VAL(pix) * Cx) >> 10;
                        gg += (G_VAL(pix) * Cx) >> 10;
                        bb += (B_VAL(pix) * Cx) >> 10;
                        ++pix;
                    }

                    if (j > 0)
                    {
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

                R_VAL(dptr) = r;
                G_VAL(dptr) = g;
                B_VAL(dptr) = b;
                A_VAL(dptr) = 0xFF;

                ++dptr;
            }
        }
    }
    /* fully optimized (i think) - only change of algorithm can help */
    /* if we're scaling down horizontally & vertically */
    else
    {
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, Cy, i, j;
        uint* pix=0;
        int r, g, b, rx, gx, bx;
        int xap, yap;

        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx   = XAP >> 16;
                xap  = XAP & 0xffff;
                sptr = ypoints[dyy + y] + xpoints[x];
                pix  = sptr;
                sptr += sow;
                rx   = (R_VAL(pix) * xap) >> 9;
                gx   = (G_VAL(pix) * xap) >> 9;
                bx   = (B_VAL(pix) * xap) >> 9;
                ++pix;

                for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                {
                    rx += (R_VAL(pix) * Cx) >> 9;
                    gx += (G_VAL(pix) * Cx) >> 9;
                    bx += (B_VAL(pix) * Cx) >> 9;
                    ++pix;
                }

                if (i > 0)
                {
                    rx += (R_VAL(pix) * i) >> 9;
                    gx += (G_VAL(pix) * i) >> 9;
                    bx += (B_VAL(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;

                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL(pix) * xap) >> 9;
                    gx   = (G_VAL(pix) * xap) >> 9;
                    bx   = (B_VAL(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                }

                if (j > 0)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL(pix) * xap) >> 9;
                    gx   = (G_VAL(pix) * xap) >> 9;
                    bx   = (B_VAL(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
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
                A_VAL(dptr) = 0xFF;
                ++dptr;
            }
        }
    }
}

#define A_VAL16(p) ((ushort*)(p))[3]
#define R_VAL16(p) ((ushort*)(p))[2]
#define G_VAL16(p) ((ushort*)(p))[1]
#define B_VAL16(p) ((ushort*)(p))[0]

void DImgScale::dimgScaleAARGB16(DImgScaleInfo* const isi, ullong* const dest,
                                 int dxx, int dyy,
                                 int dw, int dh,
                                 int dow, int sow)
{
    dimgScaleAARGB16(isi, dest, dxx, dyy, dw, dh, dow, sow,
                     0, 0, dw, dh);
}

/** scale by area sampling - IGNORE the ALPHA byte*/
void DImgScale::dimgScaleAARGB16(DImgScaleInfo* const isi, ullong* const dest,
                                 int dxx, int dyy, int dw, int dh,
                                 int dow, int sow,
                                 int clip_dx, int clip_dy,
                                 int clip_dw, int clip_dh)
{
    Q_UNUSED(dw);
    Q_UNUSED(dh);
    ullong* sptr=0;
    ullong* dptr=0;
    int x, y;
    ullong** ypoints  = isi->ypoints16;
    int*     xpoints  = isi->xpoints;
    int*     xapoints = isi->xapoints;
    int*     yapoints = isi->yapoints;

    const int x_begin = dxx + clip_dx;     // no clip set = dxx
    const int x_end   = x_begin + clip_dw; // no clip set = dxx + dw
    const int y_begin = clip_dy;           // no clip set = 0
    const int y_end   = clip_dy + clip_dh; // no clip set = dh

    // scaling up both ways
    if (isi->xup_yup == 3)
    {
        // go through every scanline in the output buffer
        for (y = y_begin; y < y_end; ++y)
        {
            // calculate the source line we'll scan from
            dptr = dest + (y - y_begin) * dow;
            sptr = ypoints[dyy + y];

            if (YAP > 0)
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    llong r = 0, g = 0, b = 0;
                    ullong* pix=0;

                    if (XAP > 0)
                    {
                        llong rr = 0, gg = 0, bb = 0;

                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL16(pix) * INV_XAP;
                        g   = G_VAL16(pix) * INV_XAP;
                        b   = B_VAL16(pix) * INV_XAP;
                        ++pix;
                        r   += R_VAL16(pix) * XAP;
                        g   += G_VAL16(pix) * XAP;
                        b   += B_VAL16(pix) * XAP;
                        pix += sow;
                        rr  = R_VAL16(pix) * XAP;
                        gg  = G_VAL16(pix) * XAP;
                        bb  = B_VAL16(pix) * XAP;
                        pix--;
                        rr  += R_VAL16(pix) * INV_XAP;
                        gg  += G_VAL16(pix) * INV_XAP;
                        bb  += B_VAL16(pix) * INV_XAP;
                        r   = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g   = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b   = ((bb * YAP) + (b * INV_YAP)) >> 16;

                        R_VAL16(dptr) = r;
                        G_VAL16(dptr) = g;
                        B_VAL16(dptr) = b;
                        A_VAL16(dptr) = 0xFFFF;

                        ++dptr;
                    }
                    else
                    {
                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL16(pix) * INV_YAP;
                        g   = G_VAL16(pix) * INV_YAP;
                        b   = B_VAL16(pix) * INV_YAP;
                        pix += sow;
                        r   += R_VAL16(pix) * YAP;
                        g   += G_VAL16(pix) * YAP;
                        b   += B_VAL16(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

                        R_VAL16(dptr) = r;
                        G_VAL16(dptr) = g;
                        B_VAL16(dptr) = b;
                        A_VAL16(dptr) = 0xFFFF;

                        ++dptr;
                    }
                }
            }
            else
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    ullong* pix=0;

                    if (XAP > 0)
                    {
                        llong r = 0, g = 0, b = 0;

                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL16(pix) * INV_XAP;
                        g   = G_VAL16(pix) * INV_XAP;
                        b   = B_VAL16(pix) * INV_XAP;
                        ++pix;
                        r += R_VAL16(pix) * XAP;
                        g += G_VAL16(pix) * XAP;
                        b += B_VAL16(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;

                        R_VAL16(dptr) = r;
                        G_VAL16(dptr) = g;
                        B_VAL16(dptr) = b;
                        A_VAL16(dptr) = 0xFFFF;

                        ++dptr;
                    }
                    else
                    {
                        *dptr++ = sptr[xpoints[x] ];
                    }
                }
            }
        }
    }
    // if we're scaling down vertically
    else if (isi->xup_yup == 1)
    {
        // 'Correct' version, with math units prepared for MMXification
        int Cy, j;
        ullong* pix=0;
        llong r, g, b, rr, gg, bb;
        int yap;

        // go through every scanline in the output buffer
        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL16(pix) * yap) >> 10;
                g   = (G_VAL16(pix) * yap) >> 10;
                b   = (B_VAL16(pix) * yap) >> 10;
                pix += sow;

                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    r   += (R_VAL16(pix) * Cy) >> 10;
                    g   += (G_VAL16(pix) * Cy) >> 10;
                    b   += (B_VAL16(pix) * Cy) >> 10;
                    pix += sow;
                }

                if (j > 0)
                {
                    r += (R_VAL16(pix) * j) >> 10;
                    g += (G_VAL16(pix) * j) >> 10;
                    b += (B_VAL16(pix) * j) >> 10;
                }

                if (XAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr  = (R_VAL16(pix) * yap) >> 10;
                    gg  = (G_VAL16(pix) * yap) >> 10;
                    bb  = (B_VAL16(pix) * yap) >> 10;
                    pix += sow;

                    for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                    {
                        rr  += (R_VAL16(pix) * Cy) >> 10;
                        gg  += (G_VAL16(pix) * Cy) >> 10;
                        bb  += (B_VAL16(pix) * Cy) >> 10;
                        pix += sow;
                    }

                    if (j > 0)
                    {
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

                R_VAL16(dptr) = r;
                G_VAL16(dptr) = g;
                B_VAL16(dptr) = b;
                A_VAL16(dptr) = 0xFFFF;
                ++dptr;
            }
        }
    }
    // if we're scaling down horizontally
    else if (isi->xup_yup == 2)
    {
        // 'Correct' version, with math units prepared for MMXification
        int Cx, j;
        ullong* pix=0;
        llong r, g, b, rr, gg, bb;
        int xap;

        // go through every scanline in the output buffer
        for (y = y_begin; y < y_end; ++y)
        {
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx  = XAP >> 16;
                xap = XAP & 0xffff;
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL16(pix) * xap) >> 10;
                g   = (G_VAL16(pix) * xap) >> 10;
                b   = (B_VAL16(pix) * xap) >> 10;
                ++pix;

                for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                {
                    r += (R_VAL16(pix) * Cx) >> 10;
                    g += (G_VAL16(pix) * Cx) >> 10;
                    b += (B_VAL16(pix) * Cx) >> 10;
                    ++pix;
                }

                if (j > 0)
                {
                    r += (R_VAL16(pix) * j) >> 10;
                    g += (G_VAL16(pix) * j) >> 10;
                    b += (B_VAL16(pix) * j) >> 10;
                }

                if (YAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr  = (R_VAL16(pix) * xap) >> 10;
                    gg  = (G_VAL16(pix) * xap) >> 10;
                    bb  = (B_VAL16(pix) * xap) >> 10;
                    ++pix;

                    for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                    {
                        rr += (R_VAL16(pix) * Cx) >> 10;
                        gg += (G_VAL16(pix) * Cx) >> 10;
                        bb += (B_VAL16(pix) * Cx) >> 10;
                        ++pix;
                    }

                    if (j > 0)
                    {
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }

                R_VAL16(dptr) = r;
                G_VAL16(dptr) = g;
                B_VAL16(dptr) = b;
                A_VAL16(dptr) = 0xFFFF;
                ++dptr;
            }
        }
    }
    // fully optimized (i think) - only change of algorithm can help
    // if we're scaling down horizontally & vertically
    else
    {
        // 'Correct' version, with math units prepared for MMXification
        int Cx, Cy, i, j;
        ullong* pix=0;
        llong r, g, b, rx, gx, bx;
        int xap, yap;

        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx   = XAP >> 16;
                xap  = XAP & 0xffff;
                sptr = ypoints[dyy + y] + xpoints[x];
                pix  = sptr;
                sptr += sow;
                rx   = (R_VAL16(pix) * xap) >> 9;
                gx   = (G_VAL16(pix) * xap) >> 9;
                bx   = (B_VAL16(pix) * xap) >> 9;
                ++pix;

                for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                {
                    rx += (R_VAL16(pix) * Cx) >> 9;
                    gx += (G_VAL16(pix) * Cx) >> 9;
                    bx += (B_VAL16(pix) * Cx) >> 9;
                    ++pix;
                }

                if (i > 0)
                {
                    rx += (R_VAL16(pix) * i) >> 9;
                    gx += (G_VAL16(pix) * i) >> 9;
                    bx += (B_VAL16(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;

                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL16(pix) * xap) >> 9;
                    gx   = (G_VAL16(pix) * xap) >> 9;
                    bx   = (B_VAL16(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
                        rx += (R_VAL16(pix) * i) >> 9;
                        gx += (G_VAL16(pix) * i) >> 9;
                        bx += (B_VAL16(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                }

                if (j > 0)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL16(pix) * xap) >> 9;
                    gx   = (G_VAL16(pix) * xap) >> 9;
                    bx   = (B_VAL16(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
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
                A_VAL16(dptr) = 0xFFFF;
                ++dptr;
            }
        }
    }
}

void DImgScale::dimgScaleAARGBA16(DImgScaleInfo* const isi, ullong* const dest,
                                  int dxx, int dyy,
                                  int dw, int dh,
                                  int dow, int sow
                                 )
{
    dimgScaleAARGBA16(isi, dest, dxx, dyy, dw, dh, dow, sow,
                      0, 0, dw, dh);
}

/* scale by area sampling */
void DImgScale::dimgScaleAARGBA16(DImgScaleInfo* const isi, ullong* const dest,
                                  int dxx, int dyy,
                                  int dw,  int dh,
                                  int dow, int sow,
                                  int clip_dx, int clip_dy,
                                  int clip_dw, int clip_dh)
{
    Q_UNUSED(dw);
    Q_UNUSED(dh);
    ullong* sptr=0;
    ullong* dptr=0;
    int x, y;
    ullong** ypoints = isi->ypoints16;
    int* xpoints     = isi->xpoints;
    int* xapoints    = isi->xapoints;
    int* yapoints    = isi->yapoints;

    const int x_begin = dxx + clip_dx;     // no clip set = dxx
    const int x_end   = x_begin + clip_dw; // no clip set = dxx + dw
    const int y_begin = clip_dy;           // no clip set = 0
    const int y_end   = clip_dy + clip_dh; // no clip set = dh

    /* scaling up both ways */
    if (isi->xup_yup == 3)
    {
        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            /* calculate the source line we'll scan from */
            dptr = dest + (y - y_begin) * dow;
            sptr = ypoints[dyy + y];

            if (YAP > 0)
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    llong r, g, b, a;
                    llong rr, gg, bb, aa;
                    ullong* pix=0;

                    if (XAP > 0)
                    {
                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL16(pix) * INV_XAP;
                        g   = G_VAL16(pix) * INV_XAP;
                        b   = B_VAL16(pix) * INV_XAP;
                        a   = A_VAL16(pix) * INV_XAP;
                        ++pix;
                        r   += R_VAL16(pix) * XAP;
                        g   += G_VAL16(pix) * XAP;
                        b   += B_VAL16(pix) * XAP;
                        a   += A_VAL16(pix) * XAP;
                        pix += sow;
                        rr  = R_VAL16(pix) * XAP;
                        gg  = G_VAL16(pix) * XAP;
                        bb  = B_VAL16(pix) * XAP;
                        aa  = A_VAL16(pix) * XAP;
                        --pix;
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

                        ++dptr;
                    }
                    else
                    {
                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL16(pix) * INV_YAP;
                        g   = G_VAL16(pix) * INV_YAP;
                        b   = B_VAL16(pix) * INV_YAP;
                        a   = A_VAL16(pix) * INV_YAP;
                        pix += sow;
                        r   += R_VAL16(pix) * YAP;
                        g   += G_VAL16(pix) * YAP;
                        b   += B_VAL16(pix) * YAP;
                        a   += A_VAL16(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;

                        R_VAL16(dptr) = r;
                        G_VAL16(dptr) = g;
                        B_VAL16(dptr) = b;
                        A_VAL16(dptr) = a;

                        ++dptr;
                    }
                }
            }
            else
            {
                for (x = x_begin; x < x_end; ++x)
                {
                    llong r, g, b, a;
                    ullong* pix=0;

                    if (XAP > 0)
                    {
                        pix = ypoints[dyy + y] + xpoints[x];
                        r   = R_VAL16(pix) * INV_XAP;
                        g   = G_VAL16(pix) * INV_XAP;
                        b   = B_VAL16(pix) * INV_XAP;
                        a   = A_VAL16(pix) * INV_XAP;
                        ++pix;
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

                        ++dptr;
                    }
                    else
                    {
                        *dptr++ = sptr[xpoints[x] ];
                    }
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if (isi->xup_yup == 1)
    {
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        ullong* pix=0;
        llong r, g, b, a, rr, gg, bb, aa;
        int yap;

        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL16(pix) * yap) >> 10;
                g   = (G_VAL16(pix) * yap) >> 10;
                b   = (B_VAL16(pix) * yap) >> 10;
                a   = (A_VAL16(pix) * yap) >> 10;

                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    pix += sow;
                    r   += (R_VAL16(pix) * Cy) >> 10;
                    g   += (G_VAL16(pix) * Cy) >> 10;
                    b   += (B_VAL16(pix) * Cy) >> 10;
                    a   += (A_VAL16(pix) * Cy) >> 10;
                }

                if (j > 0)
                {
                    pix += sow;
                    r   += (R_VAL16(pix) * j) >> 10;
                    g   += (G_VAL16(pix) * j) >> 10;
                    b   += (B_VAL16(pix) * j) >> 10;
                    a   += (A_VAL16(pix) * j) >> 10;
                }

                if (XAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr  = (R_VAL16(pix) * yap) >> 10;
                    gg  = (G_VAL16(pix) * yap) >> 10;
                    bb  = (B_VAL16(pix) * yap) >> 10;
                    aa  = (A_VAL16(pix) * yap) >> 10;

                    for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                    {
                        pix += sow;
                        rr  += (R_VAL16(pix) * Cy) >> 10;
                        gg  += (G_VAL16(pix) * Cy) >> 10;
                        bb  += (B_VAL16(pix) * Cy) >> 10;
                        aa  += (A_VAL16(pix) * Cy) >> 10;
                    }

                    if (j > 0)
                    {
                        pix += sow;
                        rr  += (R_VAL16(pix) * j) >> 10;
                        gg  += (G_VAL16(pix) * j) >> 10;
                        bb  += (B_VAL16(pix) * j) >> 10;
                        aa  += (A_VAL16(pix) * j) >> 10;
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                R_VAL16(dptr) = r;
                G_VAL16(dptr) = g;
                B_VAL16(dptr) = b;
                A_VAL16(dptr) = a;

                ++dptr;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if (isi->xup_yup == 2)
    {
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        ullong* pix=0;
        llong r, g, b, a, rr, gg, bb, aa;
        int xap;

        /* go through every scanline in the output buffer */
        for (y = y_begin; y < y_end; ++y)
        {
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx  = XAP >> 16;
                xap = XAP & 0xffff;
                pix = ypoints[dyy + y] + xpoints[x];
                r   = (R_VAL16(pix) * xap) >> 10;
                g   = (G_VAL16(pix) * xap) >> 10;
                b   = (B_VAL16(pix) * xap) >> 10;
                a   = (A_VAL16(pix) * xap) >> 10;

                for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                {
                    ++pix;
                    r += (R_VAL16(pix) * Cx) >> 10;
                    g += (G_VAL16(pix) * Cx) >> 10;
                    b += (B_VAL16(pix) * Cx) >> 10;
                    a += (A_VAL16(pix) * Cx) >> 10;
                }

                if (j > 0)
                {
                    ++pix;
                    r += (R_VAL16(pix) * j) >> 10;
                    g += (G_VAL16(pix) * j) >> 10;
                    b += (B_VAL16(pix) * j) >> 10;
                    a += (A_VAL16(pix) * j) >> 10;
                }

                if (YAP > 0)
                {
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr = (R_VAL16(pix) * xap) >> 10;
                    gg = (G_VAL16(pix) * xap) >> 10;
                    bb = (B_VAL16(pix) * xap) >> 10;
                    aa = (A_VAL16(pix) * xap) >> 10;

                    for (j = (1 << 14) - xap; j > Cx; j -= Cx)
                    {
                        ++pix;
                        rr += (R_VAL16(pix) * Cx) >> 10;
                        gg += (G_VAL16(pix) * Cx) >> 10;
                        bb += (B_VAL16(pix) * Cx) >> 10;
                        aa += (A_VAL16(pix) * Cx) >> 10;
                    }

                    if (j > 0)
                    {
                        ++pix;
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
                else
                {
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }

                R_VAL16(dptr) = r;
                G_VAL16(dptr) = g;
                B_VAL16(dptr) = b;
                A_VAL16(dptr) = a;

                ++dptr;
            }
        }
    }
    /* if we're scaling down horizontally & vertically */
    else
    {
        /*\ 'Correct' version, with math units prepared for MMXification:
        |*|  The operation 'b = (b * c) >> 16' translates to pmulhw,
        |*|  so the operation 'b = (b * c) >> d' would translate to
        |*|  psllw (16 - d), %mmb; pmulh %mmc, %mmb
        \*/
        int Cx, Cy, i, j;
        ullong* pix=0;
        llong a, r, g, b, ax, rx, gx, bx;
        int xap, yap;

        for (y = y_begin; y < y_end; ++y)
        {
            Cy   = YAP >> 16;
            yap  = YAP & 0xffff;
            dptr = dest + (y - y_begin) * dow;

            for (x = x_begin; x < x_end; ++x)
            {
                Cx   = XAP >> 16;
                xap  = XAP & 0xffff;
                sptr = ypoints[dyy + y] + xpoints[x];
                pix  = sptr;
                sptr += sow;
                rx   = (R_VAL16(pix) * xap) >> 9;
                gx   = (G_VAL16(pix) * xap) >> 9;
                bx   = (B_VAL16(pix) * xap) >> 9;
                ax   = (A_VAL16(pix) * xap) >> 9;
                ++pix;

                for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                {
                    rx += (R_VAL16(pix) * Cx) >> 9;
                    gx += (G_VAL16(pix) * Cx) >> 9;
                    bx += (B_VAL16(pix) * Cx) >> 9;
                    ax += (A_VAL16(pix) * Cx) >> 9;
                    ++pix;
                }

                if (i > 0)
                {
                    rx += (R_VAL16(pix) * i) >> 9;
                    gx += (G_VAL16(pix) * i) >> 9;
                    bx += (B_VAL16(pix) * i) >> 9;
                    ax += (A_VAL16(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;
                a = (ax * yap) >> 14;


                for (j = (1 << 14) - yap; j > Cy; j -= Cy)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL16(pix) * xap) >> 9;
                    gx   = (G_VAL16(pix) * xap) >> 9;
                    bx   = (B_VAL16(pix) * xap) >> 9;
                    ax   = (A_VAL16(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        ax += (A_VAL16(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
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

                if (j > 0)
                {
                    pix  = sptr;
                    sptr += sow;
                    rx   = (R_VAL16(pix) * xap) >> 9;
                    gx   = (G_VAL16(pix) * xap) >> 9;
                    bx   = (B_VAL16(pix) * xap) >> 9;
                    ax   = (A_VAL16(pix) * xap) >> 9;
                    ++pix;

                    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
                    {
                        rx += (R_VAL16(pix) * Cx) >> 9;
                        gx += (G_VAL16(pix) * Cx) >> 9;
                        bx += (B_VAL16(pix) * Cx) >> 9;
                        ax += (A_VAL16(pix) * Cx) >> 9;
                        ++pix;
                    }

                    if (i > 0)
                    {
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
                ++dptr;
            }
        }
    }
}

}  // namespace Digikam
