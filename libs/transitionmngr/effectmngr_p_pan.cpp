/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : video frame effects manager.
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

#include "effectmngr_p.h"

namespace Digikam
{

int EffectMngr::Private::effectKenBurnsPanLR(bool aInit)
{
    if (aInit)
    {
        eff_step = 0;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom to 80 percents and pan from left to right.

    double nx = eff_step * ((eff_image.width() - eff_image.width() * 0.8) / eff_imgFrames);
    double ny = (eff_image.height() - eff_image.height() * 0.8) / 2.0;
    double nh = eff_image.height() * 0.8;
    double nw = eff_image.width()  * 0.8;
    fRect.setTopLeft(QPointF(nx, ny));
    fRect.setSize(QSize(nw, nh));

    updateCurrentFrame(fRect);

    eff_step++;

    if (eff_step != eff_imgFrames)
        return 15;

    return -1;
}

int EffectMngr::Private::effectKenBurnsPanRL(bool aInit)
{
    if (aInit)
    {
        eff_step = 0;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom to 80 percents and pan from right to left.

    double nx = eff_step * ((eff_image.width() - eff_image.width() * 0.8) / eff_imgFrames);
    double ny = (eff_image.height() - eff_image.height() * 0.8) / 2.0;
    double nh = eff_image.height() * 0.8;
    double nw = eff_image.width()  * 0.8;
    fRect.setTopLeft(QPointF(eff_image.width() - nw - nx, ny));
    fRect.setSize(QSize(nw, nh));

    updateCurrentFrame(fRect);

    eff_step++;

    if (eff_step != eff_imgFrames)
        return 15;

    return -1;
}

int EffectMngr::Private::effectKenBurnsPanTB(bool aInit)
{
    if (aInit)
    {
        eff_step = 0;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom to 80 percents and pan from top to bottom.

    double nx = (eff_image.width() - eff_image.width() * 0.8) / 2.0;
    double ny = eff_step * ((eff_image.height() - eff_image.height() * 0.8) / eff_imgFrames);
    double nh = eff_image.height() * 0.8;
    double nw = eff_image.width()  * 0.8;
    fRect.setTopLeft(QPointF(nx, ny));
    fRect.setSize(QSize(nw, nh));

    updateCurrentFrame(fRect);

    eff_step++;

    if (eff_step != eff_imgFrames)
        return 15;

    return -1;
}

int EffectMngr::Private::effectKenBurnsPanBT(bool aInit)
{
    if (aInit)
    {
        eff_step = 0;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom to 80 percents and pan from bottom to top.

    double nx = (eff_image.width() - eff_image.width() * 0.8) / 2.0;
    double ny = eff_step * ((eff_image.height() - eff_image.height() * 0.8) / eff_imgFrames);
    double nh = eff_image.height() * 0.8;
    double nw = eff_image.width()  * 0.8;
    fRect.setTopLeft(QPointF(nx, eff_image.height() - nh - ny));
    fRect.setSize(QSize(nw, nh));

    updateCurrentFrame(fRect);

    eff_step++;

    if (eff_step != eff_imgFrames)
        return 15;

    return -1;
}

}  // namespace Digikam
