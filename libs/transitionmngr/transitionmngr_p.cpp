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

void TransitionMngr::Private::registerEffects()
{
    eff_transList.insert(TransitionMngr::None,            &TransitionMngr::Private::effectNone);
    eff_transList.insert(TransitionMngr::ChessBoard,      &TransitionMngr::Private::effectChessboard);
    eff_transList.insert(TransitionMngr::MeltDown,        &TransitionMngr::Private::effectMeltdown);
    eff_transList.insert(TransitionMngr::Sweep,           &TransitionMngr::Private::effectSweep);
    eff_transList.insert(TransitionMngr::Mosaic,          &TransitionMngr::Private::effectMosaic);
    eff_transList.insert(TransitionMngr::Cubism,          &TransitionMngr::Private::effectCubism);
    eff_transList.insert(TransitionMngr::Growing,         &TransitionMngr::Private::effectGrowing);
    eff_transList.insert(TransitionMngr::HorizontalLines, &TransitionMngr::Private::effectHorizLines);
    eff_transList.insert(TransitionMngr::VerticalLines,   &TransitionMngr::Private::effectVertLines);
    eff_transList.insert(TransitionMngr::CircleOut,       &TransitionMngr::Private::effectCircleOut);
    eff_transList.insert(TransitionMngr::MultiCircleOut,  &TransitionMngr::Private::effectMultiCircleOut);
    eff_transList.insert(TransitionMngr::SpiralIn,        &TransitionMngr::Private::effectSpiralIn);
    eff_transList.insert(TransitionMngr::Blobs,           &TransitionMngr::Private::effectBlobs);
    eff_transList.insert(TransitionMngr::Fade,            &TransitionMngr::Private::effectFade);
    eff_transList.insert(TransitionMngr::SlideL2R,        &TransitionMngr::Private::effectSlideL2R);
    eff_transList.insert(TransitionMngr::SlideR2L,        &TransitionMngr::Private::effectSlideR2L);
    eff_transList.insert(TransitionMngr::SlideT2B,        &TransitionMngr::Private::effectSlideT2B);
    eff_transList.insert(TransitionMngr::SlideB2T,        &TransitionMngr::Private::effectSlideB2T);
    eff_transList.insert(TransitionMngr::PushL2R,         &TransitionMngr::Private::effectPushL2R);
    eff_transList.insert(TransitionMngr::PushR2L,         &TransitionMngr::Private::effectPushR2L);
    eff_transList.insert(TransitionMngr::PushT2B,         &TransitionMngr::Private::effectPushT2B);
    eff_transList.insert(TransitionMngr::PushB2T,         &TransitionMngr::Private::effectPushB2T);
    eff_transList.insert(TransitionMngr::SwapL2R,         &TransitionMngr::Private::effectSwapL2R);
    eff_transList.insert(TransitionMngr::SwapR2L,         &TransitionMngr::Private::effectSwapR2L);
    eff_transList.insert(TransitionMngr::SwapT2B,         &TransitionMngr::Private::effectSwapT2B);
    eff_transList.insert(TransitionMngr::SwapB2T,         &TransitionMngr::Private::effectSwapB2T);
    eff_transList.insert(TransitionMngr::BlurIn,          &TransitionMngr::Private::effectBlurIn);
    eff_transList.insert(TransitionMngr::BlurOut,         &TransitionMngr::Private::effectBlurOut);
}

TransitionMngr::TransType TransitionMngr::Private::getRandomEffect() const
{
    QList<TransitionMngr::TransType> effs = eff_transList.keys();
    effs.removeAt(effs.indexOf(TransitionMngr::None));

    int count = effs.count();
    int i     = qrand() % count;
    return effs[i];
}


int TransitionMngr::Private::effectRandom(bool /*aInit*/)
{
    return -1;
}

int TransitionMngr::Private::effectNone(bool)
{
    eff_curFrame = eff_outImage;
    return -1;
}

}  // namespace Digikam
