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
    transList.insert(TransitionMngr::None,            &TransitionMngr::Private::effectNone);
    transList.insert(TransitionMngr::ChessBoard,      &TransitionMngr::Private::effectChessboard);
    transList.insert(TransitionMngr::MeltDown,        &TransitionMngr::Private::effectMeltdown);
    transList.insert(TransitionMngr::Sweep,           &TransitionMngr::Private::effectSweep);
    transList.insert(TransitionMngr::Mosaic,          &TransitionMngr::Private::effectMosaic);
    transList.insert(TransitionMngr::Cubism,          &TransitionMngr::Private::effectCubism);
    transList.insert(TransitionMngr::Growing,         &TransitionMngr::Private::effectGrowing);
    transList.insert(TransitionMngr::HorizontalLines, &TransitionMngr::Private::effectHorizLines);
    transList.insert(TransitionMngr::VerticalLines,   &TransitionMngr::Private::effectVertLines);
    transList.insert(TransitionMngr::CircleOut,       &TransitionMngr::Private::effectCircleOut);
    transList.insert(TransitionMngr::MultiCircleOut,  &TransitionMngr::Private::effectMultiCircleOut);
    transList.insert(TransitionMngr::SpiralIn,        &TransitionMngr::Private::effectSpiralIn);
    transList.insert(TransitionMngr::Blobs,           &TransitionMngr::Private::effectBlobs);
}

TransitionMngr::TransType TransitionMngr::Private::getRandomEffect() const
{
    QList<TransitionMngr::TransType> effs = transList.keys();
    effs.removeAt(effs.indexOf(TransitionMngr::None));

    int count = effs.count();
    int i     = qrand() % count;
    return effs[i];
}

int TransitionMngr::Private::effectNone(bool)
{
    curFrame = outImage;
    return -1;
}

int TransitionMngr::Private::effectChessboard(bool aInit)
{
    if (aInit)
    {
        w    = outSize.width();
        h    = outSize.height();
        dx   = 8;                             // width of one tile
        dy   = 8;                             // height of one tile
        j    = (w + dx - 1) / dx;    // number of tiles
        x    = j * dx;                  // shrinking x-offset from screen border
        ix   = 0;                             // growing x-offset from screen border
        iy   = 0;                             // 0 or dy for growing tiling effect
        y    = (j & 1) ? 0 : dy;        // 0 or dy for shrinking tiling effect
        wait = 800 / j;                    // timeout between effects
    }

    if (ix >= w)
    {
        curFrame = outImage;
        return -1;
    }

    ix += dx;
    x  -= dx;
    iy  = iy ? 0 : dy;
    y   = y  ? 0 : dy;

    QPainter bufferPainter(&curFrame);
    QBrush brush = QBrush(inImage);

    for (int y = 0 ; y < w ; y += (dy << 1))
    {
        bufferPainter.fillRect(ix, y + iy, dx, dy, brush);
        bufferPainter.fillRect(x, y + y, dx, dy, brush);
    }

    return wait;
}

int TransitionMngr::Private::effectMeltdown(bool aInit)
{
    int i;

    if (aInit)
    {
        delete [] intArray;
        w        = outSize.width();
        h        = outSize.height();
        dx       = 4;
        dy       = 16;
        ix       = w / dx;
        intArray = new int[ix];

        for (i = ix - 1 ; i >= 0 ; --i)
            intArray[i] = 0;
    }

    pdone = true;

    int y, x;
    QPainter bufferPainter(&curFrame);

    for (i = 0, x = 0 ; i < ix ; ++i, x += dx)
    {
        y = intArray[i];

        if (y >= h)
            continue;

        pdone = false;

        if ((qrand() & 15) < 6)
            continue;

        bufferPainter.drawImage(x, y + dy, curFrame, x, y, dx, h - y - dy);
        bufferPainter.drawImage(x, y, inImage, x, y, dx, dy);

        intArray[i] += dy;
    }

    bufferPainter.end();

    if (pdone)
    {
        delete [] intArray;
        intArray = NULL;
        curFrame = outImage;
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
        subType = qrand() % 4;
        w       = outSize.width();
        h       = outSize.height();
        dx      = (subType == 1 ? 16 : -16);
        dy      = (subType == 3 ? 16 : -16);
        x       = (subType == 1 ? 0 : w);
        y       = (subType == 3 ? 0 : h);
    }

    if (subType == 0 || subType == 1)
    {
        // horizontal sweep
        if ((subType == 0 && x < -64) || (subType == 1 && x > w + 64))
        {
            curFrame = outImage;
            return -1;
        }

        int w;
        int intx;
        int i;

        for (w = 2, i = 4, intx = x ; i > 0 ; i--, w <<= 1, intx -= dx)
        {
            px  = intx;
            py  = 0;
            psx = w;
            psy = h;

            QPainter bufferPainter(&curFrame);
            bufferPainter.fillRect(px, py, psx, psy, QBrush(inImage));
            bufferPainter.end();
        }

        x += dx;
    }
    else
    {
        // vertical sweep
        if ((subType == 2 && y < -64) || (subType == 3 && y > h + 64))
        {
            curFrame = outImage;
            return -1;
        }

        int h;
        int inty;
        int i;

        for (h = 2, i = 4, inty = y ; i > 0 ; i--, h <<= 1, inty -= dy)
        {
            px  = 0;
            py  = inty;
            psx = w;
            psy = h;

            QPainter bufferPainter(&curFrame);
            bufferPainter.fillRect(px, py, psx, psy, QBrush(inImage));
            bufferPainter.end();
        }

        y += dy;
    }

    return 20;
}

int TransitionMngr::Private::effectMosaic(bool aInit)
{
    int dim    = 10;         // Size of a cell (dim x dim)
    int margin = dim + (int)(dim / 4);

    if (aInit)
    {
        i           = 30; // giri totaly
        pixelMatrix = new bool*[outSize.width()];

        for (int x = 0 ; x < outSize.width() ; ++x)
        {
            pixelMatrix[x] = new bool[outSize.height()];

            for (int y = 0 ; y < outSize.height() ; ++y)
            {
                pixelMatrix[x][y] = false;
            }
        }
    }

    if (i <= 0)
    {
        curFrame = outImage;
        return -1;
    }

    int w = outSize.width();
    int h = outSize.height();

    QPainter bufferPainter(&curFrame);

    for (int x = 0 ; x < w ; x += (qrand() % margin) + dim)
    {
        for (int y = 0 ; y < h ; y += (qrand() % margin) + dim)
        {
            if (pixelMatrix[x][y] == true)
            {
                if (y != 0) y--;

                continue;
            }

            bufferPainter.fillRect(x, y, dim, dim, QBrush(inImage));

            for (int i = 0 ; i < dim && (x + i) < w ; ++i)
            {
                for (int j = 0 ; j < dim && (y + j) < h ; ++j)
                {
                    pixelMatrix[x+i][y+j] = true;
                }
            }
        }
    }

    bufferPainter.end();
    i--;

    return 20;
}

int TransitionMngr::Private::effectCubism(bool aInit)
{
    if (aInit)
    {
        alpha = M_PI * 2;
        w     = outSize.width();
        h     = outSize.height();
        i     = 150;
    }

    if (i <= 0)
    {
        curFrame = outImage;
        return -1;
    }

    QPainterPath painterPath;
    QPainter bufferPainter(&curFrame);

    x   = qrand() % w;
    y   = qrand() % h;
    int r  = (qrand() % 100) + 100;
    px   = x - r;
    py   = y - r;
    psx  = r;
    psy  = r;

    QMatrix matrix;
    matrix.rotate((qrand() % 20) - 10);
    QRect rect(px, py, psx, psy);
    bufferPainter.setMatrix(matrix);
    bufferPainter.fillRect(rect, QBrush(inImage));
    bufferPainter.end();

    i--;

    return 10;
}

int TransitionMngr::Private::effectRandom(bool /*aInit*/)
{
    return -1;
}

int TransitionMngr::Private::effectGrowing(bool aInit)
{
    if (aInit)
    {
        w  = outSize.width();
        h  = outSize.height();
        x  = w >> 1;
        y  = h >> 1;
        i  = 0;
        fx = x / 100.0;
        fy = y / 100.0;
    }

    x = (w >> 1) - (int)(i * fx);
    y = (h >> 1) - (int)(i * fy);
    i++;

    if (x < 0 || y < 0)
    {
        curFrame = outImage;
        return -1;
    }

    px  = x;
    py  = y;
    psx = w - (x << 1);
    psy = h - (y << 1);

    QPainter bufferPainter(&curFrame);
    bufferPainter.fillRect(px, py, psx, psy, QBrush(inImage));
    bufferPainter.end();

    return 20;
}

int TransitionMngr::Private::effectHorizLines(bool aInit)
{
    static int iyPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        w = outSize.width();
        h = outSize.height();
        i = 0;
    }

    if (iyPos[i] < 0)
        return -1;

    int until    = h;

    QPainter bufferPainter(&curFrame);
    QBrush brush = QBrush(outImage);

    for (int iPos = iyPos[i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(0, iPos, w, 1, brush);

    bufferPainter.end();

    i++;

    if (iyPos[i] >= 0)
        return 160;

    curFrame = outImage;

    return -1;
}

int TransitionMngr::Private::effectVertLines(bool aInit)
{
    static int ixPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        w = outSize.width();
        h = outSize.height();
        i = 0;
    }

    if (ixPos[i] < 0)
        return -1;

    int iPos;
    int until = w;

    QPainter bufferPainter(&curFrame);
    QBrush brush = QBrush(inImage);

    for (iPos = ixPos[i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(iPos, 0, 1, h, brush);

    bufferPainter.end();

    i++;

    if (ixPos[i] >= 0)
        return 160;

    curFrame = outImage;

    return -1;
}

int TransitionMngr::Private::effectMultiCircleOut(bool aInit)
{
    int x, y, i;
    double alpha;

    if (aInit)
    {
        w     = outSize.width();
        h     = outSize.height();
        x     = w;
        y     = h >> 1;
        pa.setPoint(0, w >> 1, h >> 1);
        pa.setPoint(3, w >> 1, h >> 1);
        fy    = sqrt((double)w * w + h * h) / 2;
        i     = qrand() % 15 + 2;
        fd    = M_PI * 2 / i;
        alpha = fd;
        wait  = 10 * i;
        fx    = M_PI / 32;  // divisor must be powers of 8
    }

    if (alpha < 0)
    {
        curFrame = outImage;
        return -1;
    }

    for (alpha = alpha, i = i ; i >= 0 ; i--, alpha += fd)
    {
        x    = (w >> 1) + (int)(fy * cos(-alpha));
        y    = (h >> 1) + (int)(fy * sin(-alpha));
        x = (w >> 1) + (int)(fy * cos(-alpha + fx));
        y = (h >> 1) + (int)(fy * sin(-alpha + fx));

        pa.setPoint(1, x, y);
        pa.setPoint(2, x, y);

        QPainterPath painterPath;
        painterPath.addPolygon(QPolygon(pa));

        QPainter bufferPainter(&curFrame);
        bufferPainter.fillPath(painterPath, QBrush(inImage));
        bufferPainter.end();
    }

    alpha -= fx;

    return wait;
}

int TransitionMngr::Private::effectSpiralIn(bool aInit)
{
    if (aInit)
    {
        w  = outSize.width();
        h  = outSize.height();
        ix = w / 8;
        iy = h / 8;
        x0 = 0;
        x1 = w - ix;
        y0 = iy;
        y1 = h - iy;
        dx = ix;
        dy = 0;
        i  = 0;
        j  = 16 * 16;
        x  = 0;
        y  = 0;
    }

    if (i == 0 && x0 >= x1)
    {
        curFrame = outImage;
        return -1;
    }

    if (i == 0 && x >= x1)      // switch to: down on right side
    {
        i   = 1;
        dx  = 0;
        dy  = iy;
        x1 -= ix;
    }
    else if (i == 1 && y >= y1) // switch to: right to left on bottom side
    {
        i   = 2;
        dx  = -ix;
        dy  = 0;
        y1 -= iy;
    }
    else if (i == 2 && x <= x0) // switch to: up on left side
    {
        i   = 3;
        dx  = 0;
        dy  = -iy;
        x0 += ix;
    }
    else if (i == 3 && y <= y0) // switch to: left to right on top side
    {
        i   = 0;
        dx  = ix;
        dy  = 0;
        y0 += iy;
    }

    px  = x;
    py  = y;
    psx = ix;
    psy = iy;

    QPainter bufferPainter(&curFrame);
    bufferPainter.fillRect(px, py, psx, psy, QBrush(inImage));
    bufferPainter.end();

    x += dx;
    y += dy;
    j--;

    return 8;
}

int TransitionMngr::Private::effectCircleOut(bool aInit)
{
    int x, y;

    if (aInit)
    {
        w     = outSize.width();
        h     = outSize.height();
        x     = w;
        y     = h >> 1;
        alpha = 2 * M_PI;
        pa.setPoint(0, w >> 1, h >> 1);
        pa.setPoint(3, w >> 1, h >> 1);
        fx    = M_PI / 16;                       // divisor must be powers of 8
        fy    = sqrt((double)w * w + h * h) / 2;
    }

    if (alpha < 0)
    {
        curFrame = outImage;
        return -1;
    }

    x         = x;
    y         = y;
    x      = (w >> 1) + (int)(fy * cos(alpha));
    y      = (h >> 1) + (int)(fy * sin(alpha));
    alpha -= fx;

    pa.setPoint(1, x, y);
    pa.setPoint(2, x, y);

    QPainterPath painterPath;
    painterPath.addPolygon(QPolygon(pa));
    QPainter bufferPainter(&curFrame);
    bufferPainter.fillPath(painterPath, QBrush(inImage));
    bufferPainter.end();

    return 20;
}

int TransitionMngr::Private::effectBlobs(bool aInit)
{
    int r;

    if (aInit)
    {
        alpha = M_PI * 2;
        w     = outSize.width();
        h     = outSize.height();
        i     = 150;
    }

    if (i <= 0)
    {
        curFrame = outImage;
        return -1;
    }

    x    = qrand() % w;
    y    = qrand() % h;
    r       = (qrand() % 200) + 50;
    px   = x - r;
    py   = y - r;
    psx  = r;
    psy  = r;

    QPainterPath painterPath;
    painterPath.addEllipse(px, py, psx, psy);
    QPainter bufferPainter(&curFrame);
    bufferPainter.fillPath(painterPath, QBrush(inImage));
    bufferPainter.end();

    i--;

    return 10;
}

}  // namespace Digikam
