/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : images transition manager.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "transitionmngr_p.h"

namespace Digikam
{

int TransitionMngr::Private::transitionChessboard(bool aInit)
{
    if (aInit)
    {
        eff_w    = eff_outSize.width();
        eff_h    = eff_outSize.height();
        eff_dx   = 8;                                // width of one tile
        eff_dy   = 8;                                // height of one tile
        eff_j    = (eff_w + eff_dx - 1) / eff_dx;    // number of tiles
        eff_x    = eff_j * eff_dx;                   // shrinking x-offset from screen border
        eff_ix   = 0;                                // growing x-offset from screen border
        eff_iy   = 0;                                // 0 or eff_dy for growing tiling effect
        eff_y    = (eff_j & 1) ? 0 : eff_dy;         // 0 or eff_dy for shrinking tiling effect
        eff_wait = 800 / eff_j;                      // timeout between effects
    }

    if (eff_ix >= eff_w)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    eff_ix += eff_dx;
    eff_x  -= eff_dx;
    eff_iy  = eff_iy ? 0 : eff_dy;
    eff_y   = eff_y  ? 0 : eff_dy;

    QPainter bufferPainter(&eff_curFrame);
    QBrush brush = QBrush(eff_outImage);

    for (int y = 0 ; y < eff_w ; y += (eff_dy << 1))
    {
        bufferPainter.fillRect(eff_ix, y + eff_iy, eff_dx, eff_dy, brush);
        bufferPainter.fillRect(eff_x, y + eff_y, eff_dx, eff_dy, brush);
    }

    return eff_wait;
}

int TransitionMngr::Private::transitionGrowing(bool aInit)
{
    if (aInit)
    {
        eff_w  = eff_outSize.width();
        eff_h  = eff_outSize.height();
        eff_x  = eff_w >> 1;
        eff_y  = eff_h >> 1;
        eff_i  = 0;
        eff_fx = eff_x / 100.0;
        eff_fy = eff_y / 100.0;
    }

    eff_x = (eff_w >> 1) - (int)(eff_i * eff_fx);
    eff_y = (eff_h >> 1) - (int)(eff_i * eff_fy);
    eff_i++;

    if (eff_x < 0 || eff_y < 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    eff_px  = eff_x;
    eff_py  = eff_y;
    eff_psx = eff_w - (eff_x << 1);
    eff_psy = eff_h - (eff_y << 1);

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.fillRect(eff_px, eff_py, eff_psx, eff_psy, QBrush(eff_outImage));
    bufferPainter.end();

    return 20;
}

int TransitionMngr::Private::transitionSpiralIn(bool aInit)
{
    if (aInit)
    {
        eff_w  = eff_outSize.width();
        eff_h  = eff_outSize.height();
        eff_ix = eff_w / 8;
        eff_iy = eff_h / 8;
        eff_x0 = 0;
        eff_x1 = eff_w - eff_ix;
        eff_y0 = eff_iy;
        eff_y1 = eff_h - eff_iy;
        eff_dx = eff_ix;
        eff_dy = 0;
        eff_i  = 0;
        eff_j  = 16 * 16;
        eff_x  = 0;
        eff_y  = 0;
    }

    if (eff_i == 0 && eff_x0 >= eff_x1)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    if (eff_i == 0 && eff_x >= eff_x1)      // switch to: down on right side
    {
        eff_i   = 1;
        eff_dx  = 0;
        eff_dy  = eff_iy;
        eff_x1 -= eff_ix;
    }
    else if (eff_i == 1 && eff_y >= eff_y1) // switch to: right to left on bottom side
    {
        eff_i   = 2;
        eff_dx  = -eff_ix;
        eff_dy  = 0;
        eff_y1 -= eff_iy;
    }
    else if (eff_i == 2 && eff_x <= eff_x0) // switch to: up on left side
    {
        eff_i   = 3;
        eff_dx  = 0;
        eff_dy  = -eff_iy;
        eff_x0 += eff_ix;
    }
    else if (eff_i == 3 && eff_y <= eff_y0) // switch to: left to right on top side
    {
        eff_i   = 0;
        eff_dx  = eff_ix;
        eff_dy  = 0;
        eff_y0 += eff_iy;
    }

    eff_px  = eff_x;
    eff_py  = eff_y;
    eff_psx = eff_ix;
    eff_psy = eff_iy;

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.fillRect(eff_px, eff_py, eff_psx, eff_psy, QBrush(eff_outImage));
    bufferPainter.end();

    eff_x += eff_dx;
    eff_y += eff_dy;
    eff_j--;

    return 8;
}

}  // namespace Digikam
