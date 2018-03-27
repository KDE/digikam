/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : video frame effects manager.
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

#ifndef EFFECT_MNGR_H
#define EFFECT_MNGR_H

// Qt includes

#include <QMap>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT EffectMngr
{
public:

    // See KEn Burns effect description: https://en.wikipedia.org/wiki/Ken_Burns_effect
    enum EffectType
    {
        None = 0,        // Static camera
        KenBurnsZoomIn,
        KenBurnsZoomOut,
        KenBurnsPanLR,
        KenBurnsPanRL,
        KenBurnsPanTB,
        KenBurnsPanBT,
        Random
    };

public:

    explicit EffectMngr();
    ~EffectMngr();

    void setOutputSize(const QSize& size);
    void setEffect(EffectType eff);
    void setImage(const QImage& img);
    void setFrames(int ifrms);

    QImage currentFrame(int& tmout);

    static QMap<EffectType, QString> effectNames();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // EFFECT_MNGR_H
