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
    d->eff_outSize  = size;
    d->eff_curFrame = QImage(d->eff_outSize, QImage::Format_ARGB32);
    d->eff_curFrame.fill(Qt::black);
}

void TransitionMngr::setTransition(TransType trans)
{
    if (trans == Random)
        d->eff_curTransition = d->getRandomEffect();
    else
        d->eff_curTransition = trans;

    d->eff_isRunning = false;
}

void TransitionMngr::setInImage(const QImage& iimg)
{
    d->eff_inImage = iimg;
}

void TransitionMngr::setOutImage(const QImage& oimg)
{
    d->eff_outImage = oimg;
}

QImage TransitionMngr::currentframe(int& tmout)
{
    if (!d->eff_isRunning)
    {
        d->eff_curFrame      = d->eff_inImage;
        tmout            = (this->d->*d->eff_transList[d->eff_curTransition])(true);
        d->eff_isRunning = true;
    }
    else
    {
        tmout = (this->d->*d->eff_transList[d->eff_curTransition])(false);
    }

    if (tmout == -1)
    {
        d->eff_isRunning = false;
    }

    return d->eff_curFrame;
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
    trans[Fade]            = i18nc("Transition Effect: Fade",             "Fade");
    trans[SlideL2R]        = i18nc("Transition Effect: SlideL2R",         "Slide Left to Right");
    trans[SlideR2L]        = i18nc("Transition Effect: SlideR2L",         "Slide Right to Left");
    trans[SlideT2B]        = i18nc("Transition Effect: SlideT2B",         "Slide Top to Bottom");
    trans[SlideB2T]        = i18nc("Transition Effect: SlideB2T",         "Slide Bottom to Top");
    trans[PushL2R]         = i18nc("Transition Effect: PushL2R",          "Push Left to Right");
    trans[PushR2L]         = i18nc("Transition Effect: PushR2L",          "Push Right to Left");
    trans[PushT2B]         = i18nc("Transition Effect: PushT2B",          "Push Top to Bottom");
    trans[PushB2T]         = i18nc("Transition Effect: PushB2T",          "Push Bottom to Top");
    trans[BlurIn]          = i18nc("Transition Effect: BlurIn",           "Blur In");
    trans[BlurOut]         = i18nc("Transition Effect: BlurOut",          "Blur Out");
    trans[Random]          = i18nc("Transition Effect: Random effect",    "Random");

    return trans;
}

}  // namespace Digikam
