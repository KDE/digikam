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

#ifndef TRANSITION_MNGR_H
#define TRANSITION_MNGR_H

// Qt includes

#include <QMap>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TransitionMngr
{
public:

    enum TransType
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
        Fade,
        SlideL2R,
        SlideR2L,
        SlideT2B,
        SlideB2T,
        PushL2R,
        PushR2L,
        PushT2B,
        PushB2T,
        SwapL2R,
        SwapR2L,
        SwapT2B,
        SwapB2T,
        BlurIn,
        BlurOut,
        Random
    };

public:

    explicit TransitionMngr();
    ~TransitionMngr();

    void setOutputSize(const QSize& size);
    void setTransition(TransType type);
    void setInImage(const QImage& iimg);
    void setOutImage(const QImage& oimg);

    QImage currentFrame(int& tmout);

    static QMap<TransType, QString> transitionNames();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TRANSITION_MNGR_H
