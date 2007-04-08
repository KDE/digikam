/* ============================================================
 * Authors: Antonio Larrosa <larrosa at kde dot org>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-06-04
 * Description : fast smooth QImage based on Bresenham method
 *
 * Copyright 2002-2007 Antonio Larrosa
 * Copyright      2007 Gilles Caulier
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

}  // NameSpace Digikam
