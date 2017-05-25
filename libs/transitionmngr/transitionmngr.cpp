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

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

TransitionMngr::TransitionMngr()
    : d(new Private)
{
}

TransitionMngr::~TransitionMngr()
{
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
        tmout            = (this->d->*d->effectList[d->curEffect])(true);
        d->effectRunning = true;
    }
    else
    {
        tmout = (this->d->*d->effectList[d->curEffect])(false);
    }

    if (tmout == -1)
    {
        d->effectRunning = false;
    }

    return d->curFrame;
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

}  // namespace Digikam
