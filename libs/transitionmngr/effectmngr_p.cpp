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

void EffectMngr::Private::registerEffects()
{
    eff_effectList.insert(EffectMngr::None,           &EffectMngr::Private::effectNone);
    eff_effectList.insert(EffectMngr::KenBurnsZoomIn, &EffectMngr::Private::effectKenBurnsZoomIn);
    eff_effectList.insert(EffectMngr::KenBurnsPanLR,  &EffectMngr::Private::effectKenBurnsPanLR);
    eff_effectList.insert(EffectMngr::KenBurnsPanRL,  &EffectMngr::Private::effectKenBurnsPanRL);
    eff_effectList.insert(EffectMngr::KenBurnsPanTB,  &EffectMngr::Private::effectKenBurnsPanTB);
    eff_effectList.insert(EffectMngr::KenBurnsPanBT,  &EffectMngr::Private::effectKenBurnsPanBT);
}

EffectMngr::EffectType EffectMngr::Private::getRandomEffect() const
{
    QList<EffectMngr::EffectType> effs = eff_effectList.keys();
    effs.removeAt(effs.indexOf(EffectMngr::None));

    int count = effs.count();
    int i     = qrand() % count;
    return effs[i];
}

void EffectMngr::Private::updateCurrentFrame(const QRect& area)
{
    QImage kbImg = eff_image.copy(area).scaled(eff_outSize,
                                               Qt::KeepAspectRatioByExpanding,
                                               Qt::SmoothTransformation);
    eff_curFrame = kbImg.convertToFormat(QImage::Format_ARGB32);
}

int EffectMngr::Private::effectRandom(bool /*aInit*/)
{
    return -1;
}

int EffectMngr::Private::effectNone(bool)
{
    eff_curFrame = eff_image;

    return -1;
}

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

    updateCurrentFrame(fRect.toAlignedRect());

    eff_step++;

    if (eff_step != eff_imgFrames)
        return 15;

    return -1;
}

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

    updateCurrentFrame(fRect.toAlignedRect());

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

    updateCurrentFrame(fRect.toAlignedRect());

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

    updateCurrentFrame(fRect.toAlignedRect());

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

    updateCurrentFrame(fRect.toAlignedRect());

    eff_step++;

    if (eff_step != eff_imgFrames)
        return 15;

    return -1;
}

}  // namespace Digikam
