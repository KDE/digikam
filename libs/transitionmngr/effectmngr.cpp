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

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

EffectMngr::EffectMngr()
    : d(new Private)
{
}

EffectMngr::~EffectMngr()
{
    delete d;
}

void EffectMngr::setOutputSize(const QSize& size)
{
    d->eff_outSize  = size;
    d->eff_curFrame = QImage(d->eff_outSize, QImage::Format_ARGB32);
    d->eff_curFrame.fill(Qt::black);
}

void EffectMngr::setEffect(EffectType type)
{
    if (type == Random)
        d->eff_curEffect = d->getRandomEffect();
    else
        d->eff_curEffect = type;

    d->eff_isRunning = false;
}

void EffectMngr::setImage(const QImage& img)
{
    d->eff_image = img;
}

void EffectMngr::setFrames(int ifrms)
{
    d->eff_imgFrames = ifrms;
}

QImage EffectMngr::currentFrame(int& tmout)
{
    if (!d->eff_isRunning)
    {
        d->eff_curFrame  = d->eff_image;
        tmout            = (this->d->*d->eff_effectList[d->eff_curEffect])(true);
        d->eff_isRunning = true;
    }
    else
    {
        tmout = (this->d->*d->eff_effectList[d->eff_curEffect])(false);
    }

    if (tmout == -1)
    {
        d->eff_isRunning = false;
    }

    return d->eff_curFrame;
}

QMap<EffectMngr::EffectType, QString> EffectMngr::effectNames()
{
    QMap<EffectType, QString> eff;

    eff[None]            = i18nc("Transition Effect: No Effect",        "None");
    eff[KenBurnsZoomIn]  = i18nc("Transition Effect: KenBurnsZoomIn",   "Ken Burns - Camera Zoom In");
    eff[Random]          = i18nc("Transition Effect: Random effect",    "Random");

    return eff;
}

}  // namespace Digikam
