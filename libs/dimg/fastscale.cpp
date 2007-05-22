/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-04
 * Description : fast smooth QImage based on Bresenham method
 *
 * Copyright (C) 2002-2007 Antonio Larrosa <larrosa at kde dot org>
 * Copyright (C)      2007 Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "fastscale.h"

namespace Digikam
{

QImage FastScale::fastScaleQImage(const QImage &img, int width, int height)
{
    QImage tgt(width, height, 32);
    tgt.setAlphaBuffer(img.hasAlphaBuffer());
    fastScaleRectAvg( (Q_UINT32 *)tgt.bits(), (Q_UINT32 *)img.bits(),
                      img.width(), img.height(), tgt.width(), tgt.height() );
    return tgt;
}

void FastScale::fastScaleQImage(const QImage &img, QImage &tgt)
{
    fastScaleRectAvg( (Q_UINT32 *)tgt.bits(), (Q_UINT32 *)img.bits(),
                      img.width(), img.height(), tgt.width(), tgt.height() );
}

void FastScale::fastScaleLineAvg(Q_UINT32 *Target, Q_UINT32 *Source, int SrcWidth, int TgtWidth)
{
    int NumPixels = TgtWidth;
    int IntPart   = SrcWidth / TgtWidth;
    int FractPart = SrcWidth % TgtWidth;
    int Mid       = TgtWidth / 2;
    int E         = 0;
    int skip;
    Q_UINT32 p;
    
    skip = (TgtWidth < SrcWidth) ? 0 : TgtWidth / (2*SrcWidth) + 1;
    NumPixels -= skip;
    
    while (NumPixels-- > 0) 
    {
        p = *Source;

        if (E >= Mid)
            p = fastScaleAverage(p, *(Source+1));

        *Target++ = p;
        Source += IntPart;
        E += FractPart;

        if (E >= TgtWidth) 
        {
            E -= TgtWidth;
            Source++;
        } 
    } 

    while (skip-- > 0)
    {
        *Target++ = *Source;
    }
}

void FastScale::fastScaleRectAvg(Q_UINT32 *Target, Q_UINT32 *Source, int SrcWidth, int SrcHeight,
                                 int TgtWidth, int TgtHeight)
{
    int NumPixels = TgtHeight;
    int IntPart   = (SrcHeight / TgtHeight) * SrcWidth;
    int FractPart = SrcHeight % TgtHeight;
    int Mid       = TgtHeight / 2;
    int E         = 0;
    int skip;
    Q_UINT32 *PrevSource      = NULL;
    Q_UINT32 *PrevSourceAhead = NULL;
    
    skip = (TgtHeight < SrcHeight) ? 0 : TgtHeight / (2*SrcHeight) + 1;
    NumPixels -= skip;
    
    Q_UINT32 *ScanLine      = new Q_UINT32[TgtWidth];
    Q_UINT32 *ScanLineAhead = new Q_UINT32[TgtWidth];
    
    while (NumPixels-- > 0) 
    {
        if (Source != PrevSource) 
        {
            if (Source == PrevSourceAhead) 
            {
                /* the next scan line has already been scaled and stored in
                * ScanLineAhead; swap the buffers that ScanLine and ScanLineAhead
                * point to
                */
                Q_UINT32 *tmp = ScanLine;
                ScanLine      = ScanLineAhead;
                ScanLineAhead = tmp;
            }
            else 
            {
                fastScaleLineAvg(ScanLine, Source, SrcWidth, TgtWidth);
            }

            PrevSource = Source;
        }

        if (E >= Mid && PrevSourceAhead != Source+SrcWidth) 
        {
            int x;
            fastScaleLineAvg(ScanLineAhead, Source+SrcWidth, SrcWidth, TgtWidth);

            for (x = 0; x < TgtWidth; x++)
                ScanLine[x] = fastScaleAverage(ScanLine[x], ScanLineAhead[x]);

            PrevSourceAhead = Source + SrcWidth;
        } 

        memcpy(Target, ScanLine, TgtWidth*sizeof(Q_UINT32));
        Target += TgtWidth;
        Source += IntPart;
        E      += FractPart;

        if (E >= TgtHeight) 
        {
            E -= TgtHeight;
            Source += SrcWidth;
        } 
    } 
    
    if (skip > 0 && Source != PrevSource)
        fastScaleLineAvg(ScanLine, Source, SrcWidth, TgtWidth);

    while (skip-- > 0) 
    {
        memcpy(Target, ScanLine, TgtWidth*sizeof(Q_UINT32));
        Target += TgtWidth;
    }
    
    delete [] ScanLine;
    delete [] ScanLineAhead;
}

#define CLIP(x, y, w, h, xx, yy, ww, hh) \
if (x < (xx)) {w += (x - (xx)); x = (xx);} \
if (y < (yy)) {h += (y - (yy)); y = (yy);} \
if ((x + w) > ((xx) + (ww))) {w = (ww) - (x - xx);} \
if ((y + h) > ((yy) + (hh))) {h = (hh) - (y - yy);}

QImage FastScale::fastScaleSectionQImage(const QImage &img, int sx, int sy, int sw, int sh, int dw, int dh)
{
    uint w = img.width();
    uint h = img.height();

    // sanity checks
    if ((dw <= 0) || (dh <= 0))
        return QImage();

    if ((sw <= 0) || (sh <= 0))
        return QImage();

    // clip the source rect to be within the actual image 
    int  psx, psy, psw, psh;
    psx = sx;
    psy = sy;
    psw = sw;
    psh = sh;
    CLIP(sx, sy, sw, sh, 0, 0, (int)w, (int)h);
    
    // clip output coords to clipped input coords 
    if (psw != sw)
        dw = (dw * sw) / psw;
    if (psh != sh)
        dh = (dh * sh) / psh;

    // do a second check to see if we now have invalid coords 
    // do not do anything if we have a 0 widht or height image to render 
    if ((dw <= 0) || (dh <= 0))
        return QImage();

    // if the input rect size < 0 do not render either 
    if ((sw <= 0) || (sh <= 0))
        return QImage();

    // do we actually need to scale?
    if ((sw == dw) && (sh == dh))
    {
        return img.copy(sx, sy, sw, sh);
    }

    // FIXME : this code is not optimized and can give artifact.

    QImage section = img.copy(sx-sw/10, sy-sh/10, sw+sw/5, sh+sh/5);
    QImage scaled  = FastScale::fastScaleQImage(section, dw+dw/5, dh+dh/5);
    return (scaled.copy(dw/10, dh/10, dw, dh));
}

}  // NameSpace Digikam
