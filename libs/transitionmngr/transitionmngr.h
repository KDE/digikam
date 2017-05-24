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

#ifndef TRANSITION_MNGR_H
#define TRANSITION_MNGR_H

// Qt includes

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TransitionMngr
{

public:

    enum Effect
    {
        None = 0,
        ChessBoard,
        MeltDown,
        Sweep,
        Mosaic,
        Cubism,
        Growing,
        HorizontalLines,
        VerticalLines,
        CircleOut,
        MultiCircleOut,
        SpiralIn,
        Blobs,
        Random,
    };

    typedef int (TransitionMngr::*EffectMethod)(bool);

public:

    explicit TransitionMngr();
    ~TransitionMngr();

    void setOutputSize(const QSize& size);
    void setEffect(Effect eff);
    void setInImage(const QImage& iimg);
    void setOutImage(const QImage& oimg);

    QImage currentframe(int& tmout);

    static QMap<Effect, QString> effectNames();

private:

    void    registerEffects();

    int     effectNone(bool);
    int     effectChessboard(bool doInit);
    int     effectMeltdown(bool doInit);
    int     effectSweep(bool doInit);
    int     effectMosaic(bool doInit);
    int     effectCubism(bool doInit);
    int     effectRandom(bool doInit);
    int     effectGrowing(bool doInit);
    int     effectHorizLines(bool doInit);
    int     effectVertLines(bool doInit);
    int     effectMultiCircleOut(bool doInit);
    int     effectSpiralIn(bool doInit);
    int     effectCircleOut(bool doInit);
    int     effectBlobs(bool doInit);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TRANSITION_MNGR_H
