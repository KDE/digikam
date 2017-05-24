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

#include "transitionmngr.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QMatrix>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

class TransitionMngr::Private
{

public:

    Private()
    {
        curEffect     = TransitionMngr::None;
        effectRunning = false;
        x             = 0;
        y             = 0;
        w             = 0;
        h             = 0;
        dx            = 0;
        dy            = 0;
        ix            = 0;
        iy            = 0;
        i             = 0;
        j             = 0;
        subType       = 0;
        x0            = 0;
        y0            = 0;
        x1            = 0;
        y1            = 0;
        wait          = 0;
        fx            = 0;
        fy            = 0;
        alpha         = 0;
        fd            = 0;
        intArray      = 0;
        pdone         = 0;
        pixelMatrix   = 0;
        px            = 0;
        py            = 0;
        psx           = 0;
        psy           = 0;
    }

    QMap<TransitionMngr::Effect, EffectMethod> effectList;

    QImage                                     inImage;
    QImage                                     outImage;
    QImage                                     curFrame;

    bool                                       effectRunning;
    TransitionMngr::Effect                     curEffect;

    // values for state of various effects:
    int                                        x;
    int                                        y;
    int                                        w;
    int                                        h;
    int                                        dx;
    int                                        dy;
    int                                        ix;
    int                                        iy;
    int                                        i;
    int                                        j;
    int                                        subType;
    int                                        x0;
    int                                        y0;
    int                                        x1;
    int                                        y1;
    int                                        wait;
    double                                     fx;
    double                                     fy;
    double                                     alpha;
    double                                     fd;
    int*                                       intArray;
    bool                                       pdone;
    bool**                                     pixelMatrix;

    QSize                                      outSize;

    //static
    QPolygon                                   pa;

    int                                        px;
    int                                        py;
    int                                        psx;
    int                                        psy;

    TransitionMngr::Effect getRandomEffect() const
    {
        QList<TransitionMngr::Effect> effs = effectList.keys();
        effs.removeAt(effs.indexOf(TransitionMngr::None));

        int count = effs.count();
        int i     = qrand() % count;
        return effs[i];
    }

};

TransitionMngr::TransitionMngr()
    : d(new Private)
{
    d->effectRunning = false;
    d->intArray      = 0;
    d->pa            = QPolygon(4);

    registerEffects();
}

TransitionMngr::~TransitionMngr()
{
    if (d->intArray)
        delete [] d->intArray;

    delete d;
}

void TransitionMngr::setOutputSize(const QSize& size)
{
    d->outSize  = size;
    d->curFrame = QImage(d->outSize, QImage::Format_ARGB32);
    d->curFrame.fill(Qt::black);
}

void TransitionMngr::setEffect(TransitionMngr::Effect eff)
{
    if (eff == Random)
        d->curEffect = d->getRandomEffect();
    else
        d->curEffect = eff;

    d->effectRunning = false;
}

void TransitionMngr::setInImage(const QImage& iimg)
{
    d->inImage = iimg;
}

void TransitionMngr::setOutImage(const QImage& oimg)
{
    d->outImage = oimg;
}

QImage TransitionMngr::currentframe(int& tmout)
{
    if (!d->effectRunning)
    {
        d->curFrame      = d->inImage;
        tmout            = (this->*d->effectList[d->curEffect])(true);
        d->effectRunning = true;
    }
    else
    {
        tmout = (this->*d->effectList[d->curEffect])(false);
    }

    if (tmout == -1)
    {
        d->effectRunning = false;
    }

    return d->curFrame;
}

void TransitionMngr::registerEffects()
{
    d->effectList.insert(None,            &TransitionMngr::effectNone);
    d->effectList.insert(ChessBoard,      &TransitionMngr::effectChessboard);
    d->effectList.insert(MeltDown,        &TransitionMngr::effectMeltdown);
    d->effectList.insert(Sweep,           &TransitionMngr::effectSweep);
    d->effectList.insert(Mosaic,          &TransitionMngr::effectMosaic);
    d->effectList.insert(Cubism,          &TransitionMngr::effectCubism);
    d->effectList.insert(Growing,         &TransitionMngr::effectGrowing);
    d->effectList.insert(HorizontalLines, &TransitionMngr::effectHorizLines);
    d->effectList.insert(VerticalLines,   &TransitionMngr::effectVertLines);
    d->effectList.insert(CircleOut,       &TransitionMngr::effectCircleOut);
    d->effectList.insert(MultiCircleOut,  &TransitionMngr::effectMultiCircleOut);
    d->effectList.insert(SpiralIn,        &TransitionMngr::effectSpiralIn);
    d->effectList.insert(Blobs,           &TransitionMngr::effectBlobs);
}

QMap<TransitionMngr::Effect, QString> TransitionMngr::effectNames()
{
    QMap<Effect, QString> effects;

    effects[None]            = i18nc("Transition Effect: No effect",        "None");
    effects[ChessBoard]      = i18nc("Transition Effect: Chess Board",      "Chess Board");
    effects[MeltDown]        = i18nc("Transition Effect: Melt Down",        "Melt Down");
    effects[Sweep]           = i18nc("Transition Effect: Sweep",            "Sweep");
    effects[Mosaic]          = i18nc("Transition Effect: Mosaic",           "Mosaic");
    effects[Cubism]          = i18nc("Transition Effect: Cubism",           "Cubism");
    effects[Growing]         = i18nc("Transition Effect: Growing",          "Growing");
    effects[HorizontalLines] = i18nc("Transition Effect: Horizontal Lines", "Horizontal Lines");
    effects[VerticalLines]   = i18nc("Transition Effect: Vertical Lines",   "Vertical Lines");
    effects[CircleOut]       = i18nc("Transition Effect: Circle Out",       "Circle Out");
    effects[MultiCircleOut]  = i18nc("Transition Effect: Multi-Circle Out", "Multi-Circle Out");
    effects[SpiralIn]        = i18nc("Transition Effect: Spiral In",        "Spiral In");
    effects[Blobs]           = i18nc("Transition Effect: Blobs",            "Blobs");
    effects[Random]          = i18nc("Transition Effect: Random effect",    "Random");

    return effects;
}

int TransitionMngr::effectNone(bool)
{
    d->curFrame = d->outImage;
    return -1;
}

int TransitionMngr::effectChessboard(bool aInit)
{
    if (aInit)
    {
        d->w    = d->outSize.width();
        d->h    = d->outSize.height();
        d->dx   = 8;                             // width of one tile
        d->dy   = 8;                             // height of one tile
        d->j    = (d->w + d->dx - 1) / d->dx;    // number of tiles
        d->x    = d->j * d->dx;                  // shrinking x-offset from screen border
        d->ix   = 0;                             // growing x-offset from screen border
        d->iy   = 0;                             // 0 or d->dy for growing tiling effect
        d->y    = (d->j & 1) ? 0 : d->dy;        // 0 or d->dy for shrinking tiling effect
        d->wait = 800 / d->j;                    // timeout between effects
    }

    if (d->ix >= d->w)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    d->ix += d->dx;
    d->x  -= d->dx;
    d->iy  = d->iy ? 0 : d->dy;
    d->y   = d->y  ? 0 : d->dy;

    QPainter bufferPainter(&d->curFrame);
    QBrush brush = QBrush(d->inImage);

    for (int y = 0 ; y < d->w ; y += (d->dy << 1))
    {
        bufferPainter.fillRect(d->ix, y + d->iy, d->dx, d->dy, brush);
        bufferPainter.fillRect(d->x, y + d->y, d->dx, d->dy, brush);
    }

    return d->wait;
}

int TransitionMngr::effectMeltdown(bool aInit)
{
    int i;

    if (aInit)
    {
        delete [] d->intArray;
        d->w        = d->outSize.width();
        d->h        = d->outSize.height();
        d->dx       = 4;
        d->dy       = 16;
        d->ix       = d->w / d->dx;
        d->intArray = new int[d->ix];

        for (i = d->ix - 1 ; i >= 0 ; --i)
            d->intArray[i] = 0;
    }

    d->pdone = true;

    int y, x;
    QPainter bufferPainter(&d->curFrame);

    for (i = 0, x = 0 ; i < d->ix ; ++i, x += d->dx)
    {
        y = d->intArray[i];

        if (y >= d->h)
            continue;

        d->pdone = false;

        if ((qrand() & 15) < 6)
            continue;

        bufferPainter.drawImage(x, y + d->dy, d->curFrame, x, y, d->dx, d->h - y - d->dy);
        bufferPainter.drawImage(x, y, d->inImage, x, y, d->dx, d->dy);

        d->intArray[i] += d->dy;
    }

    bufferPainter.end();

    if (d->pdone)
    {
        delete [] d->intArray;
        d->intArray = NULL;
        d->curFrame = d->outImage;
        return -1;
    }

    return 15;
}

int TransitionMngr::effectSweep(bool aInit)
{
    if (aInit)
    {
        // subtype: 0=sweep right to left, 1=sweep left to right
        //          2=sweep bottom to top, 3=sweep top to bottom
        d->subType = qrand() % 4;
        d->w       = d->outSize.width();
        d->h       = d->outSize.height();
        d->dx      = (d->subType == 1 ? 16 : -16);
        d->dy      = (d->subType == 3 ? 16 : -16);
        d->x       = (d->subType == 1 ? 0 : d->w);
        d->y       = (d->subType == 3 ? 0 : d->h);
    }

    if (d->subType == 0 || d->subType == 1)
    {
        // horizontal sweep
        if ((d->subType == 0 && d->x < -64) || (d->subType == 1 && d->x > d->w + 64))
        {
            d->curFrame = d->outImage;
            return -1;
        }

        int w;
        int x;
        int i;

        for (w = 2, i = 4, x = d->x ; i > 0 ; i--, w <<= 1, x -= d->dx)
        {
            d->px  = x;
            d->py  = 0;
            d->psx = w;
            d->psy = d->h;

            QPainter bufferPainter(&d->curFrame);
            bufferPainter.fillRect(d->px, d->py, d->psx, d->psy, QBrush(d->inImage));
            bufferPainter.end();
        }

        d->x += d->dx;
    }
    else
    {
        // vertical sweep
        if ((d->subType == 2 && d->y < -64) || (d->subType == 3 && d->y > d->h + 64))
        {
            d->curFrame = d->outImage;
            return -1;
        }

        int h;
        int y;
        int i;

        for (h = 2, i = 4, y = d->y ; i > 0 ; i--, h <<= 1, y -= d->dy)
        {
            d->px  = 0;
            d->py  = y;
            d->psx = d->w;
            d->psy = h;

            QPainter bufferPainter(&d->curFrame);
            bufferPainter.fillRect(d->px, d->py, d->psx, d->psy, QBrush(d->inImage));
            bufferPainter.end();
        }

        d->y += d->dy;
    }

    return 20;
}

int TransitionMngr::effectMosaic(bool aInit)
{
    int dim    = 10;         // Size of a cell (dim x dim)
    int margin = dim + (int)(dim / 4);

    if (aInit)
    {
        d->i           = 30; // giri totaly
        d->pixelMatrix = new bool*[d->outSize.width()];

        for (int x = 0 ; x < d->outSize.width() ; ++x)
        {
            d->pixelMatrix[x] = new bool[d->outSize.height()];

            for (int y = 0 ; y < d->outSize.height() ; ++y)
            {
                d->pixelMatrix[x][y] = false;
            }
        }
    }

    if (d->i <= 0)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    int w = d->outSize.width();
    int h = d->outSize.height();

    QPainter bufferPainter(&d->curFrame);

    for (int x = 0 ; x < w ; x += (qrand() % margin) + dim)
    {
        for (int y = 0 ; y < h ; y += (qrand() % margin) + dim)
        {
            if (d->pixelMatrix[x][y] == true)
            {
                if (y != 0) y--;

                continue;
            }

            bufferPainter.fillRect(x, y, dim, dim, QBrush(d->inImage));

            for (int i = 0 ; i < dim && (x + i) < w ; ++i)
            {
                for (int j = 0 ; j < dim && (y + j) < h ; ++j)
                {
                    d->pixelMatrix[x+i][y+j] = true;
                }
            }
        }
    }

    bufferPainter.end();
    d->i--;

    return 20;
}

int TransitionMngr::effectCubism(bool aInit)
{
    if (aInit)
    {
        d->alpha = M_PI * 2;
        d->w     = d->outSize.width();
        d->h     = d->outSize.height();
        d->i     = 150;
    }

    if (d->i <= 0)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    QPainterPath painterPath;
    QPainter bufferPainter(&d->curFrame);

    d->x   = qrand() % d->w;
    d->y   = qrand() % d->h;
    int r  = (qrand() % 100) + 100;
    d->px   = d->x - r;
    d->py   = d->y - r;
    d->psx  = r;
    d->psy  = r;

    QMatrix matrix;
    matrix.rotate((qrand() % 20) - 10);
    QRect rect(d->px, d->py, d->psx, d->psy);
    bufferPainter.setMatrix(matrix);
    bufferPainter.fillRect(rect, QBrush(d->inImage));
    bufferPainter.end();

    d->i--;

    return 10;
}

int TransitionMngr::effectRandom(bool /*aInit*/)
{
    return -1;
}

int TransitionMngr::effectGrowing(bool aInit)
{
    if (aInit)
    {
        d->w  = d->outSize.width();
        d->h  = d->outSize.height();
        d->x  = d->w >> 1;
        d->y  = d->h >> 1;
        d->i  = 0;
        d->fx = d->x / 100.0;
        d->fy = d->y / 100.0;
    }

    d->x = (d->w >> 1) - (int)(d->i * d->fx);
    d->y = (d->h >> 1) - (int)(d->i * d->fy);
    d->i++;

    if (d->x < 0 || d->y < 0)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    d->px  = d->x;
    d->py  = d->y;
    d->psx = d->w - (d->x << 1);
    d->psy = d->h - (d->y << 1);

    QPainter bufferPainter(&d->curFrame);
    bufferPainter.fillRect(d->px, d->py, d->psx, d->psy, QBrush(d->inImage));
    bufferPainter.end();

    return 20;
}

int TransitionMngr::effectHorizLines(bool aInit)
{
    static int iyPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        d->w = d->outSize.width();
        d->h = d->outSize.height();
        d->i = 0;
    }

    if (iyPos[d->i] < 0)
        return -1;

    int until    = d->h;

    QPainter bufferPainter(&d->curFrame);
    QBrush brush = QBrush(d->outImage);

    for (int iPos = iyPos[d->i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(0, iPos, d->w, 1, brush);

    bufferPainter.end();

    d->i++;

    if (iyPos[d->i] >= 0)
        return 160;

    d->curFrame = d->outImage;

    return -1;
}

int TransitionMngr::effectVertLines(bool aInit)
{
    static int ixPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        d->w = d->outSize.width();
        d->h = d->outSize.height();
        d->i = 0;
    }

    if (ixPos[d->i] < 0)
        return -1;

    int iPos;
    int until = d->w;

    QPainter bufferPainter(&d->curFrame);
    QBrush brush = QBrush(d->inImage);

    for (iPos = ixPos[d->i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(iPos, 0, 1, d->h, brush);

    bufferPainter.end();

    d->i++;

    if (ixPos[d->i] >= 0)
        return 160;

    d->curFrame = d->outImage;

    return -1;
}

int TransitionMngr::effectMultiCircleOut(bool aInit)
{
    int x, y, i;
    double alpha;

    if (aInit)
    {
        d->w     = d->outSize.width();
        d->h     = d->outSize.height();
        d->x     = d->w;
        d->y     = d->h >> 1;
        d->pa.setPoint(0, d->w >> 1, d->h >> 1);
        d->pa.setPoint(3, d->w >> 1, d->h >> 1);
        d->fy    = sqrt((double)d->w * d->w + d->h * d->h) / 2;
        d->i     = qrand() % 15 + 2;
        d->fd    = M_PI * 2 / d->i;
        d->alpha = d->fd;
        d->wait  = 10 * d->i;
        d->fx    = M_PI / 32;  // divisor must be powers of 8
    }

    if (d->alpha < 0)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    for (alpha = d->alpha, i = d->i ; i >= 0 ; i--, alpha += d->fd)
    {
        x    = (d->w >> 1) + (int)(d->fy * cos(-alpha));
        y    = (d->h >> 1) + (int)(d->fy * sin(-alpha));
        d->x = (d->w >> 1) + (int)(d->fy * cos(-alpha + d->fx));
        d->y = (d->h >> 1) + (int)(d->fy * sin(-alpha + d->fx));

        d->pa.setPoint(1, x, y);
        d->pa.setPoint(2, d->x, d->y);

        QPainterPath painterPath;
        painterPath.addPolygon(QPolygon(d->pa));

        QPainter bufferPainter(&d->curFrame);
        bufferPainter.fillPath(painterPath, QBrush(d->inImage));
        bufferPainter.end();
    }

    d->alpha -= d->fx;

    return d->wait;
}

int TransitionMngr::effectSpiralIn(bool aInit)
{
    if (aInit)
    {
        d->w  = d->outSize.width();
        d->h  = d->outSize.height();
        d->ix = d->w / 8;
        d->iy = d->h / 8;
        d->x0 = 0;
        d->x1 = d->w - d->ix;
        d->y0 = d->iy;
        d->y1 = d->h - d->iy;
        d->dx = d->ix;
        d->dy = 0;
        d->i  = 0;
        d->j  = 16 * 16;
        d->x  = 0;
        d->y  = 0;
    }

    if (d->i == 0 && d->x0 >= d->x1)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    if (d->i == 0 && d->x >= d->x1)      // switch to: down on right side
    {
        d->i   = 1;
        d->dx  = 0;
        d->dy  = d->iy;
        d->x1 -= d->ix;
    }
    else if (d->i == 1 && d->y >= d->y1) // switch to: right to left on bottom side
    {
        d->i   = 2;
        d->dx  = -d->ix;
        d->dy  = 0;
        d->y1 -= d->iy;
    }
    else if (d->i == 2 && d->x <= d->x0) // switch to: up on left side
    {
        d->i   = 3;
        d->dx  = 0;
        d->dy  = -d->iy;
        d->x0 += d->ix;
    }
    else if (d->i == 3 && d->y <= d->y0) // switch to: left to right on top side
    {
        d->i   = 0;
        d->dx  = d->ix;
        d->dy  = 0;
        d->y0 += d->iy;
    }

    d->px  = d->x;
    d->py  = d->y;
    d->psx = d->ix;
    d->psy = d->iy;

    QPainter bufferPainter(&d->curFrame);
    bufferPainter.fillRect(d->px, d->py, d->psx, d->psy, QBrush(d->inImage));
    bufferPainter.end();

    d->x += d->dx;
    d->y += d->dy;
    d->j--;

    return 8;
}

int TransitionMngr::effectCircleOut(bool aInit)
{
    int x, y;

    if (aInit)
    {
        d->w     = d->outSize.width();
        d->h     = d->outSize.height();
        d->x     = d->w;
        d->y     = d->h >> 1;
        d->alpha = 2 * M_PI;
        d->pa.setPoint(0, d->w >> 1, d->h >> 1);
        d->pa.setPoint(3, d->w >> 1, d->h >> 1);
        d->fx    = M_PI / 16;                       // divisor must be powers of 8
        d->fy    = sqrt((double)d->w * d->w + d->h * d->h) / 2;
    }

    if (d->alpha < 0)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    x         = d->x;
    y         = d->y;
    d->x      = (d->w >> 1) + (int)(d->fy * cos(d->alpha));
    d->y      = (d->h >> 1) + (int)(d->fy * sin(d->alpha));
    d->alpha -= d->fx;

    d->pa.setPoint(1, x, y);
    d->pa.setPoint(2, d->x, d->y);

    QPainterPath painterPath;
    painterPath.addPolygon(QPolygon(d->pa));
    QPainter bufferPainter(&d->curFrame);
    bufferPainter.fillPath(painterPath, QBrush(d->inImage));
    bufferPainter.end();

    return 20;
}

int TransitionMngr::effectBlobs(bool aInit)
{
    int r;

    if (aInit)
    {
        d->alpha = M_PI * 2;
        d->w     = d->outSize.width();
        d->h     = d->outSize.height();
        d->i     = 150;
    }

    if (d->i <= 0)
    {
        d->curFrame = d->outImage;
        return -1;
    }

    d->x   = qrand() % d->w;
    d->y   = qrand() % d->h;
    r      = (qrand() % 200) + 50;
    d->px   = d->x - r;
    d->py   = d->y - r;
    d->psx  = r;
    d->psy  = r;

    QPainterPath painterPath;
    painterPath.addEllipse(d->px, d->py, d->psx, d->psy);
    QPainter bufferPainter(&d->curFrame);
    bufferPainter.fillPath(painterPath, QBrush(d->inImage));
    bufferPainter.end();

    d->i--;

    return 10;
}

}  // namespace Digikam
