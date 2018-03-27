/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : images transition manager.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QTime>

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
    qsrand(static_cast<quint64>(QTime::currentTime().msecsSinceStartOfDay()));
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

void TransitionMngr::setTransition(TransType type)
{
    if (type == Random)
        d->eff_curTransition = d->getRandomTransition();
    else
        d->eff_curTransition = type;

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

QImage TransitionMngr::currentFrame(int& tmout)
{
    if (!d->eff_isRunning)
    {
        d->eff_curFrame  = d->eff_inImage;
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

    trans[None]            = i18nc("Transition: No Transition",    "None");
    trans[ChessBoard]      = i18nc("Transition: Chess Board",      "Chess Board");
    trans[MeltDown]        = i18nc("Transition: Melt Down",        "Melt Down");
    trans[Sweep]           = i18nc("Transition: Sweep",            "Sweep");
    trans[Mosaic]          = i18nc("Transition: Mosaic",           "Mosaic");
    trans[Cubism]          = i18nc("Transition: Cubism",           "Cubism");
    trans[Growing]         = i18nc("Transition: Growing",          "Growing");
    trans[HorizontalLines] = i18nc("Transition: Horizontal Lines", "Horizontal Lines");
    trans[VerticalLines]   = i18nc("Transition: Vertical Lines",   "Vertical Lines");
    trans[CircleOut]       = i18nc("Transition: Circle Out",       "Circle Out");
    trans[MultiCircleOut]  = i18nc("Transition: Multi-Circle Out", "Multi-Circle Out");
    trans[SpiralIn]        = i18nc("Transition: Spiral In",        "Spiral In");
    trans[Blobs]           = i18nc("Transition: Blobs",            "Blobs");
    trans[Fade]            = i18nc("Transition: Fade",             "Fade");
    trans[SlideL2R]        = i18nc("Transition: SlideL2R",         "Slide Left to Right");
    trans[SlideR2L]        = i18nc("Transition: SlideR2L",         "Slide Right to Left");
    trans[SlideT2B]        = i18nc("Transition: SlideT2B",         "Slide Top to Bottom");
    trans[SlideB2T]        = i18nc("Transition: SlideB2T",         "Slide Bottom to Top");
    trans[PushL2R]         = i18nc("Transition: PushL2R",          "Push Left to Right");
    trans[PushR2L]         = i18nc("Transition: PushR2L",          "Push Right to Left");
    trans[PushT2B]         = i18nc("Transition: PushT2B",          "Push Top to Bottom");
    trans[PushB2T]         = i18nc("Transition: PushB2T",          "Push Bottom to Top");
    trans[SwapL2R]         = i18nc("Transition: SwapL2R",          "Swap Left to Right");
    trans[SwapR2L]         = i18nc("Transition: SwapR2L",          "Swap Right to Left");
    trans[SwapT2B]         = i18nc("Transition: SwapT2B",          "Swap Top to Bottom");
    trans[SwapB2T]         = i18nc("Transition: SwapB2T",          "Swap Bottom to Top");
    trans[BlurIn]          = i18nc("Transition: BlurIn",           "Blur In");
    trans[BlurOut]         = i18nc("Transition: BlurOut",          "Blur Out");
    trans[Random]          = i18nc("Transition: Random Effect",    "Random");

    return trans;
}

}  // namespace Digikam
