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

namespace Digikam
{

void TransitionMngr::Private::registerTransitions()
{
    eff_transList.insert(TransitionMngr::None,            &TransitionMngr::Private::transitionNone);
    eff_transList.insert(TransitionMngr::ChessBoard,      &TransitionMngr::Private::transitionChessboard);
    eff_transList.insert(TransitionMngr::MeltDown,        &TransitionMngr::Private::transitionMeltdown);
    eff_transList.insert(TransitionMngr::Sweep,           &TransitionMngr::Private::transitionSweep);
    eff_transList.insert(TransitionMngr::Mosaic,          &TransitionMngr::Private::transitionMosaic);
    eff_transList.insert(TransitionMngr::Cubism,          &TransitionMngr::Private::transitionCubism);
    eff_transList.insert(TransitionMngr::Growing,         &TransitionMngr::Private::transitionGrowing);
    eff_transList.insert(TransitionMngr::HorizontalLines, &TransitionMngr::Private::transitionHorizLines);
    eff_transList.insert(TransitionMngr::VerticalLines,   &TransitionMngr::Private::transitionVertLines);
    eff_transList.insert(TransitionMngr::CircleOut,       &TransitionMngr::Private::transitionCircleOut);
    eff_transList.insert(TransitionMngr::MultiCircleOut,  &TransitionMngr::Private::transitionMultiCircleOut);
    eff_transList.insert(TransitionMngr::SpiralIn,        &TransitionMngr::Private::transitionSpiralIn);
    eff_transList.insert(TransitionMngr::Blobs,           &TransitionMngr::Private::transitionBlobs);
    eff_transList.insert(TransitionMngr::Fade,            &TransitionMngr::Private::transitionFade);
    eff_transList.insert(TransitionMngr::SlideL2R,        &TransitionMngr::Private::transitionSlideL2R);
    eff_transList.insert(TransitionMngr::SlideR2L,        &TransitionMngr::Private::transitionSlideR2L);
    eff_transList.insert(TransitionMngr::SlideT2B,        &TransitionMngr::Private::transitionSlideT2B);
    eff_transList.insert(TransitionMngr::SlideB2T,        &TransitionMngr::Private::transitionSlideB2T);
    eff_transList.insert(TransitionMngr::PushL2R,         &TransitionMngr::Private::transitionPushL2R);
    eff_transList.insert(TransitionMngr::PushR2L,         &TransitionMngr::Private::transitionPushR2L);
    eff_transList.insert(TransitionMngr::PushT2B,         &TransitionMngr::Private::transitionPushT2B);
    eff_transList.insert(TransitionMngr::PushB2T,         &TransitionMngr::Private::transitionPushB2T);
    eff_transList.insert(TransitionMngr::SwapL2R,         &TransitionMngr::Private::transitionSwapL2R);
    eff_transList.insert(TransitionMngr::SwapR2L,         &TransitionMngr::Private::transitionSwapR2L);
    eff_transList.insert(TransitionMngr::SwapT2B,         &TransitionMngr::Private::transitionSwapT2B);
    eff_transList.insert(TransitionMngr::SwapB2T,         &TransitionMngr::Private::transitionSwapB2T);
    eff_transList.insert(TransitionMngr::BlurIn,          &TransitionMngr::Private::transitionBlurIn);
    eff_transList.insert(TransitionMngr::BlurOut,         &TransitionMngr::Private::transitionBlurOut);
}

TransitionMngr::TransType TransitionMngr::Private::getRandomTransition() const
{
    QList<TransitionMngr::TransType> effs = eff_transList.keys();
    effs.removeAt(effs.indexOf(TransitionMngr::None));

    int count = effs.count();
    int i     = qrand() % count;

    return effs[i];
}

int TransitionMngr::Private::transitionRandom(bool /*aInit*/)
{
    return -1;
}

int TransitionMngr::Private::transitionNone(bool)
{
    eff_curFrame = eff_outImage;

    return -1;
}

}  // namespace Digikam
