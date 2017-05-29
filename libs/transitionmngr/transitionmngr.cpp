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

void TransitionMngr::setTransition(TransType trans)
{
    if (trans == Random)
        d->curTransition = d->getRandomEffect();
    else
        d->curTransition = trans;

    d->isRunning = false;
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
    if (!d->isRunning)
    {
        d->curFrame      = d->inImage;
        tmout            = (this->d->*d->transList[d->curTransition])(true);
        d->isRunning = true;
    }
    else
    {
        tmout = (this->d->*d->transList[d->curTransition])(false);
    }

    if (tmout == -1)
    {
        d->isRunning = false;
    }

    return d->curFrame;
}

QMap<TransitionMngr::TransType, QString> TransitionMngr::transitionNames()
{
    QMap<TransType, QString> trans;

    trans[None]            = i18nc("Transition Effect: No effect",        "None");
    trans[ChessBoard]      = i18nc("Transition Effect: Chess Board",      "Chess Board");
    trans[MeltDown]        = i18nc("Transition Effect: Melt Down",        "Melt Down");
    trans[Sweep]           = i18nc("Transition Effect: Sweep",            "Sweep");
    trans[Mosaic]          = i18nc("Transition Effect: Mosaic",           "Mosaic");
    trans[Cubism]          = i18nc("Transition Effect: Cubism",           "Cubism");
    trans[Growing]         = i18nc("Transition Effect: Growing",          "Growing");
    trans[HorizontalLines] = i18nc("Transition Effect: Horizontal Lines", "Horizontal Lines");
    trans[VerticalLines]   = i18nc("Transition Effect: Vertical Lines",   "Vertical Lines");
    trans[CircleOut]       = i18nc("Transition Effect: Circle Out",       "Circle Out");
    trans[MultiCircleOut]  = i18nc("Transition Effect: Multi-Circle Out", "Multi-Circle Out");
    trans[SpiralIn]        = i18nc("Transition Effect: Spiral In",        "Spiral In");
    trans[Blobs]           = i18nc("Transition Effect: Blobs",            "Blobs");
    trans[Random]          = i18nc("Transition Effect: Random effect",    "Random");

    return trans;
}

}  // namespace Digikam
