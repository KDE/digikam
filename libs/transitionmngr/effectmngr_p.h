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

// C++ includes

#include <cmath>

// Qt includes

#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QImage>

// Local includes

#include "effectmngr.h"
#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

class EffectMngr::Private
{
public:

    typedef int (EffectMngr::Private::*EffectMethod)(bool);

public:

    explicit Private()
    {
        eff_curEffect = EffectMngr::None;
        eff_isRunning = false;
        eff_step      = 0;
        eff_imgFrames = 125;

        registerEffects();
    }

    ~Private()
    {
    }

    QMap<EffectMngr::EffectType, EffectMethod>    eff_effectList;

    QImage                                        eff_image;
    QImage                                        eff_curFrame;
    QSize                                         eff_outSize;

    bool                                          eff_isRunning;
    EffectMngr::EffectType                        eff_curEffect;

    int                                           eff_step;
    int                                           eff_imgFrames;

public:

    void registerEffects();

    EffectMngr::EffectType getRandomEffect() const;

private:

    // Internal functions to render an effect frame.
    // The effect mouvement must be adjusted accordingly with amount of image frames to encode.
    // aInit is to true when effect is initialized (first call).
    // The integer value is a tempo in ms to wait between frames,
    // or -1 if the effect is completed.

    int effectNone(bool aInit);
    int effectRandom(bool aInit);
    int effectKenBurnsZoomIn(bool aInit);
    int effectKenBurnsZoomOut(bool aInit);
    int effectKenBurnsPanLR(bool aInit);
    int effectKenBurnsPanRL(bool aInit);
    int effectKenBurnsPanTB(bool aInit);
    int effectKenBurnsPanBT(bool aInit);

    void updateCurrentFrame(const QRectF& area);
};

}  // namespace Digikam
