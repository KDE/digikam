/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image quality Settings Container.
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMAGEQUALITYSETTINGS_H
#define IMAGEQUALITYSETTINGS_H

// Qt includes

#include <QDebug>

namespace Digikam
{

class ImageQualitySettings
{
public:

    ImageQualitySettings();
    virtual ~ImageQualitySettings();

public:

    void readFromConfig();
    void writeToConfig();

public:

    bool enableSorter;

    bool detectBlur;
    bool detectNoise;
    bool detectCompression;
    bool detectOverexposure;
    bool lowQRejected;
    bool mediumQPending;
    bool highQAccepted;

    int  speed;
    int  rejectedThreshold;
    int  pendingThreshold;
    int  acceptedThreshold ;
    int  blurWeight;
    int  noiseWeight;
    int  compressionWeight;
};

//! qDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const ImageQualitySettings& s);

}  // namespace Digikam

#endif  // IMAGEQUALITYSETTINGS_H
