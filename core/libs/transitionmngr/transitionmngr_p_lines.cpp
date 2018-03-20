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

int TransitionMngr::Private::transitionSweep(bool aInit)
{
    if (aInit)
    {
        // subtype: 0=sweep right to left, 1=sweep left to right
        //          2=sweep bottom to top, 3=sweep top to bottom
        eff_subType = qrand() % 4;
        eff_w       = eff_outSize.width();
        eff_h       = eff_outSize.height();
        eff_dx      = (eff_subType == 1 ? 16 : -16);
        eff_dy      = (eff_subType == 3 ? 16 : -16);
        eff_x       = (eff_subType == 1 ? 0  : eff_w);
        eff_y       = (eff_subType == 3 ? 0  : eff_h);
    }

    if (eff_subType == 0 || eff_subType == 1)
    {
        // horizontal sweep
        if ((eff_subType == 0 && eff_x < -64) || (eff_subType == 1 && eff_x > eff_w + 64))
        {
            eff_curFrame = eff_outImage;
            return -1;
        }

        int w;
        int x;
        int i;

        for (w = 2, i = 4, x = eff_x ; i > 0 ; i--, w <<= 1, x -= eff_dx)
        {
            eff_px  = x;
            eff_py  = 0;
            eff_psx = w;
            eff_psy = eff_h;

            QPainter bufferPainter(&eff_curFrame);
            bufferPainter.fillRect(eff_px, eff_py, eff_psx, eff_psy, QBrush(eff_outImage));
            bufferPainter.end();
        }

        eff_x += eff_dx;
    }
    else
    {
        // vertical sweep
        if ((eff_subType == 2 && eff_y < -64) || (eff_subType == 3 && eff_y > eff_h + 64))
        {
            eff_curFrame = eff_outImage;
            return -1;
        }

        int h;
        int y;
        int i;

        for (h = 2, i = 4, y = eff_y ; i > 0 ; i--, h <<= 1, y -= eff_dy)
        {
            eff_px  = 0;
            eff_py  = y;
            eff_psx = eff_w;
            eff_psy = h;

            QPainter bufferPainter(&eff_curFrame);
            bufferPainter.fillRect(eff_px, eff_py, eff_psx, eff_psy, QBrush(eff_outImage));
            bufferPainter.end();
        }

        eff_y += eff_dy;
    }

    return 20;
}

int TransitionMngr::Private::transitionHorizLines(bool aInit)
{
    static int iyPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        eff_w = eff_outSize.width();
        eff_h = eff_outSize.height();
        eff_i = 0;
    }

    if (iyPos[eff_i] < 0)
        return -1;

    int until    = eff_h;

    QPainter bufferPainter(&eff_curFrame);
    QBrush brush = QBrush(eff_outImage);

    for (int iPos = iyPos[eff_i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(0, iPos, eff_w, 1, brush);

    bufferPainter.end();

    eff_i++;

    if (iyPos[eff_i] >= 0)
        return 160;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionVertLines(bool aInit)
{
    static int ixPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        eff_w = eff_outSize.width();
        eff_h = eff_outSize.height();
        eff_i = 0;
    }

    if (ixPos[eff_i] < 0)
        return -1;

    int iPos;
    int until = eff_w;

    QPainter bufferPainter(&eff_curFrame);
    QBrush brush = QBrush(eff_outImage);

    for (iPos = ixPos[eff_i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(iPos, 0, 1, eff_h, brush);

    bufferPainter.end();

    eff_i++;

    if (ixPos[eff_i] >= 0)
        return 160;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionMultiCircleOut(bool aInit)
{
    int x, y, i;
    double alpha;

    if (aInit)
    {
        eff_w     = eff_outSize.width();
        eff_h     = eff_outSize.height();
        eff_x     = eff_w;
        eff_y     = eff_h >> 1;
        eff_pa.setPoint(0, eff_w >> 1, eff_h >> 1);
        eff_pa.setPoint(3, eff_w >> 1, eff_h >> 1);
        eff_fy    = sqrt((double)eff_w * eff_w + eff_h * eff_h) / 2;
        eff_i     = qrand() % 15 + 2;
        eff_fd    = M_PI * 2 / eff_i;
        eff_alpha = eff_fd;
        eff_wait  = 10 * eff_i;
        eff_fx    = M_PI / 32;  // divisor must be powers of 8
    }

    if (eff_alpha < 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    for (alpha = eff_alpha, i = eff_i ; i >= 0 ; i--, alpha += eff_fd)
    {
        x     = (eff_w >> 1) + (int)(eff_fy * cos(-alpha));
        y     = (eff_h >> 1) + (int)(eff_fy * sin(-alpha));
        eff_x = (eff_w >> 1) + (int)(eff_fy * cos(-alpha + eff_fx));
        eff_y = (eff_h >> 1) + (int)(eff_fy * sin(-alpha + eff_fx));

        eff_pa.setPoint(1, x, y);
        eff_pa.setPoint(2, eff_x, eff_y);

        QPainterPath painterPath;
        painterPath.addPolygon(QPolygon(eff_pa));

        QPainter bufferPainter(&eff_curFrame);
        bufferPainter.fillPath(painterPath, QBrush(eff_outImage));
        bufferPainter.end();
    }

    eff_alpha -= eff_fx;

    return eff_wait;
}

int TransitionMngr::Private::transitionCircleOut(bool aInit)
{
    int x, y;

    if (aInit)
    {
        eff_w     = eff_outSize.width();
        eff_h     = eff_outSize.height();
        eff_x     = eff_w;
        eff_y     = eff_h >> 1;
        eff_alpha = 2 * M_PI;
        eff_pa.setPoint(0, eff_w >> 1, eff_h >> 1);
        eff_pa.setPoint(3, eff_w >> 1, eff_h >> 1);
        eff_fx    = M_PI / 16;                       // divisor must be powers of 8
        eff_fy    = sqrt((double)eff_w * eff_w + eff_h * eff_h) / 2;
    }

    if (eff_alpha < 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    x          = eff_x;
    y          = eff_y;
    eff_x      = (eff_w >> 1) + (int)(eff_fy * cos(eff_alpha));
    eff_y      = (eff_h >> 1) + (int)(eff_fy * sin(eff_alpha));
    eff_alpha -= eff_fx;

    eff_pa.setPoint(1, x, y);
    eff_pa.setPoint(2, eff_x, eff_y);

    QPainterPath painterPath;
    painterPath.addPolygon(QPolygon(eff_pa));
    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.fillPath(painterPath, QBrush(eff_outImage));
    bufferPainter.end();

    return 20;
}

}  // namespace Digikam
