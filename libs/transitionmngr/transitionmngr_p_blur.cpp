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

QRgb TransitionMngr::Private::convertFromPremult(const QRgb& p) const
{
    int alpha = qAlpha(p);

    return (!alpha ? 0
                   : qRgba(255 * qRed(p)   / alpha,
                           255 * qGreen(p) / alpha,
                           255 * qBlue(p)  / alpha,
                           alpha));
}

QImage TransitionMngr::Private::fastBlur(const QImage& img, int radius) const
{
    QRgb* p1 = 0;
    QRgb* p2 = 0;
    int*  as = 0;
    int*  rs = 0;
    int*  gs = 0;
    int*  bs = 0;
    int x, y, w, h, mx, my, mw, mh, mt, xx, yy;
    int a, r, g, b;

    if (radius < 1 || img.isNull() || img.width() < (radius << 1))
        return img;

    w = img.width();
    h = img.height();

    QImage buffer(w, h, img.hasAlphaChannel() ? QImage::Format_ARGB32
                                              : QImage::Format_RGB32);

    as = new int[w];
    rs = new int[w];
    gs = new int[w];
    bs = new int[w];

    for (y = 0 ; y < h ; y++)
    {
        my = y - radius;
        mh = (radius << 1) + 1;

        if (my < 0)
        {
            mh += my;
            my  = 0;
        }

        if ((my + mh) > h)
            mh = h - my;

        p1 = (QRgb*)buffer.scanLine(y);
        memset(as, 0, w*sizeof(int));
        memset(rs, 0, w*sizeof(int));
        memset(gs, 0, w*sizeof(int));
        memset(bs, 0, w*sizeof(int));

        QRgb pixel;

        for (yy = 0 ; yy < mh ; yy++)
        {
            p2 = (QRgb*)img.scanLine(yy + my);

            for (x = 0 ; x < w ; x++, p2++)
            {
                pixel  = convertFromPremult(*p2);
                as[x] += qAlpha(pixel);
                rs[x] += qRed(pixel);
                gs[x] += qGreen(pixel);
                bs[x] += qBlue(pixel);
            }
        }

        for (x = 0 ; x < w ; x++)
        {
            a  = 0;
            r  = 0;
            g  = 0;
            b  = 0;
            mx = x - radius;
            mw = (radius << 1) + 1;

            if (mx < 0)
            {
                mw += mx;
                mx  = 0;
            }

            if ((mx + mw) > w)
                mw = w - mx;

            mt = mw * mh;

            for (xx = mx ; xx < (mw + mx) ; xx++)
            {
                a += as[xx];
                r += rs[xx];
                g += gs[xx];
                b += bs[xx];
            }

            a     = a / mt;
            r     = r / mt;
            g     = g / mt;
            b     = b / mt;
            *p1++ = qRgba(r, g, b, a);
        }
    }

    delete [] as;
    delete [] rs;
    delete [] gs;
    delete [] bs;

    return buffer.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

int TransitionMngr::Private::transitionFade(bool aInit)
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

int TransitionMngr::Private::transitionBlurIn(bool aInit)
{
    if (aInit)
    {
        eff_fd = 25.0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, 0, fastBlur(eff_outImage, eff_fd));
    bufferPainter.end();

    eff_fd = eff_fd - 1.0;

    if (eff_fd >= 1.0)
        return 15;

    eff_curFrame = eff_outImage;

    return -1;

}

int TransitionMngr::Private::transitionBlurOut(bool aInit)
{
    if (aInit)
    {
        eff_fd = 1.0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, 0, fastBlur(eff_inImage, eff_fd));
    bufferPainter.end();

    eff_fd = eff_fd + 1.0;

    if (eff_fd <= 25.0)
        return 15;

    eff_curFrame = eff_outImage;

    return -1;

}

}  // namespace Digikam
