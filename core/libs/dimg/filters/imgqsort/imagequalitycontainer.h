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

#ifndef DIGIKAM_IMAGE_QUALITY_CONTAINER_H
#define DIGIKAM_IMAGE_QUALITY_CONTAINER_H

// Qt includes

#include <QDebug>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageQualityContainer
{
public:

    ImageQualityContainer();
    ImageQualityContainer(const ImageQualityContainer& other);
    ~ImageQualityContainer();

    ImageQualityContainer& operator=(const ImageQualityContainer& other);

public:

    void readFromConfig();
    void writeToConfig();

public:

    bool enableSorter;          /// Global quality dectection enabler/disabler.

    bool detectBlur;            /// Enable image blur detection.
    bool detectNoise;           /// Enable image noise detection.
    bool detectCompression;     /// Enable image compression detection.
    bool detectOverexposure;    /// Enable image over-exposure detection.
    bool lowQRejected;          /// Assign Rejected property to low quality.
    bool mediumQPending;        /// Assign Pending property to medium quality.
    bool highQAccepted;         /// Assign Accepted property to high quality.

    int  speed;                 /// Calculation speed.
    int  rejectedThreshold;     /// Item rejection threshold.
    int  pendingThreshold;      /// Item pending threshold.
    int  acceptedThreshold;     /// Item accepted threshold.
    int  blurWeight;            /// Item blur level.
    int  noiseWeight;           /// Item noise level.
    int  compressionWeight;     /// Item compression level.
};

//! qDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const ImageQualityContainer& s);

} // namespace Digikam

#endif // DIGIKAM_IMAGE_QUALITY_CONTAINER_H
