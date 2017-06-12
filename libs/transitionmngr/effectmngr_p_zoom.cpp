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

int EffectMngr::Private::effectKenBurnsZoomIn(bool aInit)
{
    if (aInit)
    {
        eff_step = 0;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom in on the center of image from 100 to 80 percents.
    double nx    = eff_step * ((eff_image.width() - eff_image.width() * 0.8) / eff_imgFrames);
    double ny    = nx / ((double)eff_image.width() / (double)eff_image.height());
    fRect.setTopLeft(QPointF(nx, ny));
    fRect.setBottomRight(QPointF((double)eff_image.width()-nx, (double)eff_image.height()-ny));

    updateCurrentFrame(fRect);

    eff_step++;

    if (eff_step != eff_imgFrames)
        return 15;

    return -1;
}

int EffectMngr::Private::effectKenBurnsZoomOut(bool aInit)
{
    if (aInit)
    {
        eff_step = eff_imgFrames;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom out on the center of image from 80 to 100 percents.
    double nx    = eff_step * ((eff_image.width() - eff_image.width() * 0.8) / eff_imgFrames);
    double ny    = nx / ((double)eff_image.width() / (double)eff_image.height());
    fRect.setTopLeft(QPointF(nx, ny));
    fRect.setBottomRight(QPointF((double)eff_image.width()-nx, (double)eff_image.height()-ny));

    updateCurrentFrame(fRect);

    eff_step--;

    if (eff_step != 0)
        return 15;

    return -1;
}

}  // namespace Digikam
