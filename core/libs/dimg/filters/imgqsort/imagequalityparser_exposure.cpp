/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser - exposure detection
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#include "imagequalityparser_p.h"

namespace Digikam
{

double ImageQualityParser::exposureAmount() const
{
    double exposureLevel = 0.0;
    int overCount        = 0;

    ExposureSettingsContainer expo;
    expo.underExposureIndicator = false;
    expo.overExposureIndicator  = true;
    expo.exposureIndicatorMode  = false;
    expo.underExposurePercent   = 5.0;
    expo.overExposurePercent    = 5.0;
    expo.underExposureColor     = Qt::black;
    expo.overExposureColor      = Qt::white;

    QImage mask = d->image.pureColorMask(&expo);

    for (int x = 0 ; d->running && (x < mask.width()) ; ++x)
    {
        for (int y = 0 ; d->running && (y < mask.height()) ; ++y)
        {
            if (mask.pixelColor(x, y) == Qt::white)
            {
                ++overCount;
            }
        }
    }

    if (d->running)
    {
        exposureLevel = (double)(overCount) / (mask.width() * mask.height());
    }

    return exposureLevel;
}

} // namespace Digikam
