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

// Local includes

#include "transitionmngr.h"
#include "digikam_config.h"

namespace Digikam
{

class TransitionMngr::Private
{
public:

    typedef int (TransitionMngr::Private::*EffectMethod)(bool);

public:

    explicit Private()
    {
        curTransition        = TransitionMngr::None;
        isRunning    = false;
        x                = 0;
        y                = 0;
        w                = 0;
        h                = 0;
        dx               = 0;
        dy               = 0;
        ix               = 0;
        iy               = 0;
        i                = 0;
        j                = 0;
        subType          = 0;
        x0               = 0;
        y0               = 0;
        x1               = 0;
        y1               = 0;
        wait             = 0;
        fx               = 0;
        fy               = 0;
        alpha            = 0;
        fd               = 0;
        intArray         = 0;
        pdone            = 0;
        pixelMatrix      = 0;
        px               = 0;
        py               = 0;
        psx              = 0;
        psy              = 0;
        pa               = QPolygon(4);

        registerEffects();
    }

    ~Private()
    {
        if (intArray)
            delete [] intArray;
    }

    QMap<TransitionMngr::TransType, EffectMethod> transList;

    QImage                                        inImage;
    QImage                                        outImage;
    QImage                                        curFrame;

    bool                                          isRunning;
    TransitionMngr::TransType                     curTransition;

    // values for state of various effects:
    int                                           x;
    int                                           y;
    int                                           w;
    int                                           h;
    int                                           dx;
    int                                           dy;
    int                                           ix;
    int                                           iy;
    int                                           i;
    int                                           j;
    int                                           subType;
    int                                           x0;
    int                                           y0;
    int                                           x1;
    int                                           y1;
    int                                           wait;
    double                                        fx;
    double                                        fy;
    double                                        alpha;
    double                                        fd;
    int*                                          intArray;
    bool                                          pdone;
    bool**                                        pixelMatrix;

    QSize                                         outSize;

    //static
    QPolygon                                      pa;

    int                                           px;
    int                                           py;
    int                                           psx;
    int                                           psy;

public:

    void registerEffects();

    TransitionMngr::TransType getRandomEffect() const;

    int effectNone(bool);
    int effectChessboard(bool doInit);
    int effectMeltdown(bool doInit);
    int effectSweep(bool doInit);
    int effectMosaic(bool doInit);
    int effectCubism(bool doInit);
    int effectRandom(bool doInit);
    int effectGrowing(bool doInit);
    int effectHorizLines(bool doInit);
    int effectVertLines(bool doInit);
    int effectMultiCircleOut(bool doInit);
    int effectSpiralIn(bool doInit);
    int effectCircleOut(bool doInit);
    int effectBlobs(bool doInit);
};

}  // namespace Digikam
