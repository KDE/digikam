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

int TransitionMngr::Private::transitionMeltdown(bool aInit)
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

        //bufferPainter.drawImage(x, y + eff_dy, eff_curFrame, x, y, eff_dx, eff_h - y - eff_dy);
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

int TransitionMngr::Private::transitionMosaic(bool aInit)
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

int TransitionMngr::Private::transitionCubism(bool aInit)
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

int TransitionMngr::Private::transitionBlobs(bool aInit)
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

}  // namespace Digikam
