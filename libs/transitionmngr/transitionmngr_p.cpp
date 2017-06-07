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

// C++ includes

#include <cmath>

// Qt includes

#include <QMatrix>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

void TransitionMngr::Private::registerEffects()
{
    eff_transList.insert(TransitionMngr::None,            &TransitionMngr::Private::effectNone);
    eff_transList.insert(TransitionMngr::ChessBoard,      &TransitionMngr::Private::effectChessboard);
    eff_transList.insert(TransitionMngr::MeltDown,        &TransitionMngr::Private::effectMeltdown);
    eff_transList.insert(TransitionMngr::Sweep,           &TransitionMngr::Private::effectSweep);
    eff_transList.insert(TransitionMngr::Mosaic,          &TransitionMngr::Private::effectMosaic);
    eff_transList.insert(TransitionMngr::Cubism,          &TransitionMngr::Private::effectCubism);
    eff_transList.insert(TransitionMngr::Growing,         &TransitionMngr::Private::effectGrowing);
    eff_transList.insert(TransitionMngr::HorizontalLines, &TransitionMngr::Private::effectHorizLines);
    eff_transList.insert(TransitionMngr::VerticalLines,   &TransitionMngr::Private::effectVertLines);
    eff_transList.insert(TransitionMngr::CircleOut,       &TransitionMngr::Private::effectCircleOut);
    eff_transList.insert(TransitionMngr::MultiCircleOut,  &TransitionMngr::Private::effectMultiCircleOut);
    eff_transList.insert(TransitionMngr::SpiralIn,        &TransitionMngr::Private::effectSpiralIn);
    eff_transList.insert(TransitionMngr::Blobs,           &TransitionMngr::Private::effectBlobs);
    eff_transList.insert(TransitionMngr::Fade,            &TransitionMngr::Private::effectFade);
    eff_transList.insert(TransitionMngr::SlideL2R,        &TransitionMngr::Private::effectSlideL2R);
    eff_transList.insert(TransitionMngr::SlideR2L,        &TransitionMngr::Private::effectSlideR2L);
    eff_transList.insert(TransitionMngr::SlideT2B,        &TransitionMngr::Private::effectSlideT2B);
    eff_transList.insert(TransitionMngr::SlideB2T,        &TransitionMngr::Private::effectSlideB2T);
    eff_transList.insert(TransitionMngr::PushL2R,         &TransitionMngr::Private::effectPushL2R);
    eff_transList.insert(TransitionMngr::PushR2L,         &TransitionMngr::Private::effectPushR2L);
    eff_transList.insert(TransitionMngr::PushT2B,         &TransitionMngr::Private::effectPushT2B);
    eff_transList.insert(TransitionMngr::PushB2T,         &TransitionMngr::Private::effectPushB2T);
}

TransitionMngr::TransType TransitionMngr::Private::getRandomEffect() const
{
    QList<TransitionMngr::TransType> effs = eff_transList.keys();
    effs.removeAt(effs.indexOf(TransitionMngr::None));

    int count = effs.count();
    int i     = qrand() % count;
    return effs[i];
}


int TransitionMngr::Private::effectRandom(bool /*aInit*/)
{
    return -1;
}

int TransitionMngr::Private::effectNone(bool)
{
    eff_curFrame = eff_outImage;
    return -1;
}

int TransitionMngr::Private::effectChessboard(bool aInit)
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

int TransitionMngr::Private::effectMeltdown(bool aInit)
{
    int i;

    if (aInit)
    {
        delete [] eff_intArray;
        eff_w        = eff_outSize.width();
        eff_h        = eff_outSize.height();
        eff_dx       = 4;
        eff_dy       = 16;
        eff_ix       = eff_w / eff_dx;
        eff_intArray = new int[eff_ix];

        for (i = eff_ix - 1 ; i >= 0 ; --i)
            eff_intArray[i] = 0;
    }

    eff_pdone = true;

    int y, x;
    QPainter bufferPainter(&eff_curFrame);

    for (i = 0, x = 0 ; i < eff_ix ; ++i, x += eff_dx)
    {
        y = eff_intArray[i];

        if (y >= eff_h)
            continue;

        eff_pdone = false;

        if ((qrand() & 15) < 6)
            continue;

        bufferPainter.drawImage(x, y + eff_dy, eff_curFrame, x, y, eff_dx, eff_h - y - eff_dy);
        bufferPainter.drawImage(x, y, eff_outImage, x, y, eff_dx, eff_dy);

        eff_intArray[i] += eff_dy;
    }

    bufferPainter.end();

    if (eff_pdone)
    {
        delete [] eff_intArray;
        eff_intArray = NULL;
        eff_curFrame = eff_outImage;
        return -1;
    }

    return 15;
}

int TransitionMngr::Private::effectSweep(bool aInit)
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

int TransitionMngr::Private::effectMosaic(bool aInit)
{
    int dim    = 10;         // Size of a cell (dim x dim)
    int margin = dim + (int)(dim / 4);

    if (aInit)
    {
        eff_i           = 30; // giri totaly
        eff_pixelMatrix = new bool*[eff_outSize.width()];

        for (int x = 0 ; x < eff_outSize.width() ; ++x)
        {
            eff_pixelMatrix[x] = new bool[eff_outSize.height()];

            for (int y = 0 ; y < eff_outSize.height() ; ++y)
            {
                eff_pixelMatrix[x][y] = false;
            }
        }
    }

    if (eff_i <= 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    int w = eff_outSize.width();
    int h = eff_outSize.height();

    QPainter bufferPainter(&eff_curFrame);

    for (int x = 0 ; x < w ; x += (qrand() % margin) + dim)
    {
        for (int y = 0 ; y < h ; y += (qrand() % margin) + dim)
        {
            if (eff_pixelMatrix[x][y] == true)
            {
                if (y != 0) y--;

                continue;
            }

            bufferPainter.fillRect(x, y, dim, dim, QBrush(eff_outImage));

            for (int i = 0 ; i < dim && (x + i) < w ; ++i)
            {
                for (int j = 0 ; j < dim && (y + j) < h ; ++j)
                {
                    eff_pixelMatrix[x+i][y+j] = true;
                }
            }
        }
    }

    bufferPainter.end();
    eff_i--;

    return 20;
}

int TransitionMngr::Private::effectCubism(bool aInit)
{
    if (aInit)
    {
        eff_alpha = M_PI * 2;
        eff_w     = eff_outSize.width();
        eff_h     = eff_outSize.height();
        eff_i     = 150;
    }

    if (eff_i <= 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    QPainterPath painterPath;
    QPainter bufferPainter(&eff_curFrame);

    eff_x   = qrand() % eff_w;
    eff_y   = qrand() % eff_h;
    int r  = (qrand() % 100) + 100;
    eff_px   = eff_x - r;
    eff_py   = eff_y - r;
    eff_psx  = r;
    eff_psy  = r;

    QMatrix matrix;
    matrix.rotate((qrand() % 20) - 10);
    QRect rect(eff_px, eff_py, eff_psx, eff_psy);
    bufferPainter.setMatrix(matrix);
    bufferPainter.fillRect(rect, QBrush(eff_outImage));
    bufferPainter.end();

    eff_i--;

    return 10;
}

int TransitionMngr::Private::effectGrowing(bool aInit)
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

int TransitionMngr::Private::effectHorizLines(bool aInit)
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

int TransitionMngr::Private::effectVertLines(bool aInit)
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

int TransitionMngr::Private::effectMultiCircleOut(bool aInit)
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

int TransitionMngr::Private::effectSpiralIn(bool aInit)
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

int TransitionMngr::Private::effectCircleOut(bool aInit)
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

int TransitionMngr::Private::effectBlobs(bool aInit)
{
    int r;

    if (aInit)
    {
        eff_alpha = M_PI * 2;
        eff_w     = eff_outSize.width();
        eff_h     = eff_outSize.height();
        eff_i     = 150;
    }

    if (eff_i <= 0)
    {
        eff_curFrame = eff_outImage;
        return -1;
    }

    eff_x   = qrand() % eff_w;
    eff_y   = qrand() % eff_h;
    r      = (qrand() % 200) + 50;
    eff_px   = eff_x - r;
    eff_py   = eff_y - r;
    eff_psx  = r;
    eff_psy  = r;

    QPainterPath painterPath;
    painterPath.addEllipse(eff_px, eff_py, eff_psx, eff_psy);
    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.fillPath(painterPath, QBrush(eff_outImage));
    bufferPainter.end();

    eff_i--;

    return 10;
}

int TransitionMngr::Private::effectFade(bool aInit)
{
    if (aInit)
    {
        eff_fd = 1.0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, 0, eff_outImage);
    bufferPainter.setOpacity(eff_fd);
    bufferPainter.drawImage(0, 0, eff_inImage);
    bufferPainter.setOpacity(1.0);
    bufferPainter.end();

    eff_fd = eff_fd - 0.1;

    if (eff_fd > 0.0)
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectSlideL2R(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0,     0, eff_outImage);
    bufferPainter.drawImage(eff_i, 0, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fx);

    if (eff_i <= eff_outSize.width())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectSlideR2L(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0,     0, eff_outImage);
    bufferPainter.drawImage(eff_i, 0, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fx);

    if (eff_i >= -eff_outSize.width())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectSlideT2B(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, 0,     eff_outImage);
    bufferPainter.drawImage(0, eff_i, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fy);

    if (eff_i <= eff_outSize.height())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectSlideB2T(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, 0,     eff_outImage);
    bufferPainter.drawImage(0, eff_i, eff_inImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fy);

    if (eff_i >= -eff_outSize.height())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectPushL2R(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(eff_i,                     0, eff_inImage);
    bufferPainter.drawImage(eff_i-eff_outSize.width(), 0, eff_outImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fx);

    if (eff_i <= eff_outSize.width())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectPushR2L(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(eff_i,                     0, eff_inImage);
    bufferPainter.drawImage(eff_i+eff_outSize.width(), 0, eff_outImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fx);

    if (eff_i >= -eff_outSize.width())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectPushT2B(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, eff_i,                      eff_inImage);
    bufferPainter.drawImage(0, eff_i-eff_outSize.height(), eff_outImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fy);

    if (eff_i <= eff_outSize.height())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::effectPushB2T(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, eff_i,                      eff_inImage);
    bufferPainter.drawImage(0, eff_i+eff_outSize.height(), eff_outImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fy);

    if (eff_i >= -eff_outSize.height())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

}  // namespace Digikam
