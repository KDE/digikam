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

int TransitionMngr::Private::transitionPushL2R(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(eff_i,                     0, eff_inImage);
    bufferPainter.drawImage(eff_i-eff_outSize.width(), 0, eff_outImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fx);

    if (eff_i <= eff_outSize.width())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionPushR2L(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(eff_i,                     0, eff_inImage);
    bufferPainter.drawImage(eff_i+eff_outSize.width(), 0, eff_outImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fx);

    if (eff_i >= -eff_outSize.width())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionPushT2B(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, eff_i,                      eff_inImage);
    bufferPainter.drawImage(0, eff_i-eff_outSize.height(), eff_outImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fy);

    if (eff_i <= eff_outSize.height())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionPushB2T(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, eff_i,                      eff_inImage);
    bufferPainter.drawImage(0, eff_i+eff_outSize.height(), eff_outImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fy);

    if (eff_i >= -eff_outSize.height())
        return 15;

    eff_curFrame = eff_outImage;

    return -1;
}

}  // namespace Digikam
