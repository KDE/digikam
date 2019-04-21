/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Bitwise operations.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimg_p.h"

namespace Digikam
{

void DImg::bitBltImage(const DImg* const src, int dx, int dy)
{
    bitBltImage(src, 0, 0, src->width(), src->height(), dx, dy);
}

void DImg::bitBltImage(const DImg* const src, int sx, int sy, int dx, int dy)
{
    bitBltImage(src, sx, sy, src->width() - sx, src->height() - sy, dx, dy);
}

void DImg::bitBltImage(const DImg* const src, int sx, int sy, int w, int h, int dx, int dy)
{
    if (isNull())
    {
        return;
    }

    if (src->sixteenBit() != sixteenBit())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Blitting from 8-bit to 16-bit or vice versa is not supported";
        return;
    }

    if (w == -1 && h == -1)
    {
        w = src->width();
        h = src->height();
    }

    bitBlt(src->bits(), bits(), sx, sy, w, h, dx, dy,
           src->width(), src->height(), width(), height(), sixteenBit(), src->bytesDepth(), bytesDepth());
}

void DImg::bitBltImage(const uchar* const src, int sx, int sy, int w, int h, int dx, int dy,
                       uint swidth, uint sheight, int sdepth)
{
    if (isNull())
    {
        return;
    }

    if (bytesDepth() != sdepth)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Blitting from 8-bit to 16-bit or vice versa is not supported";
        return;
    }

    if (w == -1 && h == -1)
    {
        w = swidth;
        h = sheight;
    }

    bitBlt(src, bits(), sx, sy, w, h, dx, dy, swidth, sheight, width(), height(), sixteenBit(), sdepth, bytesDepth());
}

bool DImg::normalizeRegionArguments(int& sx, int& sy, int& w, int& h, int& dx, int& dy,
                                    uint swidth, uint sheight, uint dwidth, uint dheight)
{
    if (sx < 0)
    {
        // sx is negative, so + is - and - is +
        dx -= sx;
        w  += sx;
        sx = 0;
    }

    if (sy < 0)
    {
        dy -= sy;
        h  += sy;
        sy = 0;
    }

    if (dx < 0)
    {
        sx -= dx;
        w  += dx;
        dx = 0;
    }

    if (dy < 0)
    {
        sy -= dy;
        h  += dy;
        dy = 0;
    }

    if (sx + w > (int)swidth)
    {
        w = swidth - sx;
    }

    if (sy + h > (int)sheight)
    {
        h = sheight - sy;
    }

    if (dx + w > (int)dwidth)
    {
        w = dwidth - dx;
    }

    if (dy + h > (int)dheight)
    {
        h = dheight - dy;
    }

    // Nothing left to copy
    if (w <= 0 || h <= 0)
    {
        return false;
    }

    return true;
}

void DImg::bitBlt(const uchar* const src, uchar* const dest,
                  int sx, int sy, int w, int h, int dx, int dy,
                  uint swidth, uint sheight, uint dwidth, uint dheight,
                  bool /*sixteenBit*/, int sdepth, int ddepth)
{
    // Normalize
    if (!normalizeRegionArguments(sx, sy, w, h, dx, dy, swidth, sheight, dwidth, dheight))
    {
        return;
    }

    // Same pixels
    if (src == dest && dx == sx && dy == sy)
    {
        return;
    }

    const uchar* sptr  = 0;
    uchar* dptr        = 0;
    uint   slinelength = swidth * sdepth;
    uint   dlinelength = dwidth * ddepth;
    int scurY          = sy;
    int dcurY          = dy;
    int sdepthlength   = w * sdepth;

    for (int j = 0 ; j < h ; ++j, ++scurY, ++dcurY)
    {
        sptr  = &src [ scurY * slinelength ] + sx * sdepth;
        dptr  = &dest[ dcurY * dlinelength ] + dx * ddepth;

        // plain and simple bitBlt
        for (int i = 0; i < sdepthlength ; ++i, ++sptr, ++dptr)
        {
            *dptr = *sptr;
        }
    }
}


void DImg::bitBlendImage(DColorComposer* const composer, const DImg* const src,
                         int sx, int sy, int w, int h, int dx, int dy,
                         DColorComposer::MultiplicationFlags multiplicationFlags)
{
    if (isNull())
    {
        return;
    }

    if (src->sixteenBit() != sixteenBit())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Blending from 8-bit to 16-bit or vice versa is not supported";
        return;
    }

    bitBlend(composer, src->bits(), bits(), sx, sy, w, h, dx, dy,
             src->width(), src->height(), width(), height(), sixteenBit(),
             src->bytesDepth(), bytesDepth(), multiplicationFlags);
}

void DImg::bitBlend(DColorComposer* const composer, uchar* const src, uchar* const dest,
                    int sx, int sy, int w, int h, int dx, int dy,
                    uint swidth, uint sheight, uint dwidth, uint dheight,
                    bool sixteenBit, int sdepth, int ddepth,
                    DColorComposer::MultiplicationFlags multiplicationFlags)
{
    // Normalize
    if (!normalizeRegionArguments(sx, sy, w, h, dx, dy, swidth, sheight, dwidth, dheight))
    {
        return;
    }

    uchar* sptr      = 0;
    uchar* dptr      = 0;
    uint slinelength = swidth * sdepth;
    uint dlinelength = dwidth * ddepth;
    int scurY        = sy;
    int dcurY        = dy;

    for (int j = 0 ; j < h ; ++j, ++scurY, ++dcurY)
    {
        sptr = &src [ scurY * slinelength ] + sx * sdepth;
        dptr = &dest[ dcurY * dlinelength ] + dx * ddepth;

        // blend src and destination
        for (int i = 0 ; i < w ; ++i, sptr += sdepth, dptr += ddepth)
        {
            DColor src(sptr, sixteenBit);
            DColor dst(dptr, sixteenBit);

            // blend colors
            composer->compose(dst, src, multiplicationFlags);

            dst.setPixel(dptr);
        }
    }
}

void DImg::bitBlendImageOnColor(const DColor& color)
{
    bitBlendImageOnColor(color, 0, 0, width(), height());
}

void DImg::bitBlendImageOnColor(const DColor& color, int x, int y, int w, int h)
{
    // get composer for compositing rule
    DColorComposer* const composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
    // flags would be MultiplicationFlagsDImg for anything but PorterDuffNone
    bitBlendImageOnColor(composer, color, x, y, w, h, DColorComposer::NoMultiplication);

    delete composer;
}

void DImg::bitBlendImageOnColor(DColorComposer* const composer, const DColor& color,
                                int x, int y, int w, int h,
                                DColorComposer::MultiplicationFlags multiplicationFlags)
{
    if (isNull())
    {
        return;
    }

    DColor c = color;

    if (sixteenBit())
    {
        c.convertToSixteenBit();
    }
    else
    {
        c.convertToEightBit();
    }

    bitBlendOnColor(composer, c, bits(), x, y, w, h,
                    width(), height(), sixteenBit(), bytesDepth(), multiplicationFlags);
}

void DImg::bitBlendOnColor(DColorComposer* const composer, const DColor& color,
                           uchar* const data, int x, int y, int w, int h,
                           uint width, uint height, bool sixteenBit, int depth,
                           DColorComposer::MultiplicationFlags multiplicationFlags)
{
    // Normalize
    if (!normalizeRegionArguments(x, y, w, h, x, y, width, height, width, height))
    {
        return;
    }

    uchar* ptr      = 0;
    uint linelength = width * depth;
    int curY        = y;

    for (int j = 0 ; j < h ; ++j, ++curY)
    {
        ptr = &data[ curY * linelength ] + x * depth;

        // blend src and destination
        for (int i = 0 ; i < w ; ++i, ptr += depth)
        {
            DColor src(ptr, sixteenBit);
            DColor dst(color);

            // blend colors
            composer->compose(dst, src, multiplicationFlags);

            dst.setPixel(ptr);
        }
    }
}

} // namespace Digikam
