/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image quality Settings Container.
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagequalitysettings.h"

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kglobal.h>

namespace Digikam
{

ImageQualitySettings::ImageQualitySettings()
{
    enableSorter       = false;
    detectBlur         = true;
    detectNoise        = true;
    detectCompression  = true;
    lowQRejected       = true;
    mediumQPending     = true;
    highQAccepted      = true;
    speed              = 1;
}

ImageQualitySettings::~ImageQualitySettings()
{
}
    
void ImageQualitySettings::readFromConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Image Quality Settings");

    enableSorter      = group.readEntry("Enable Sorter",      false);    
    detectBlur        = group.readEntry("Detect Blur",        true);
    detectNoise       = group.readEntry("Detect Noise",       true);
    detectCompression = group.readEntry("Detect Compression", true);
    lowQRejected      = group.readEntry("LowQ Rejected",      true);
    mediumQPending    = group.readEntry("MediumQ Pending",    true);
    highQAccepted     = group.readEntry("HighQ Accepted",     true);
    speed             = group.readEntry("Speed",              1);
}

void ImageQualitySettings::writeToConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Image Quality Settings");

    group.writeEntry("Enable Sorter",      enableSorter);    
    group.writeEntry("Detect Blur",        detectBlur);
    group.writeEntry("Detect Noise",       detectNoise);
    group.writeEntry("Detect Compression", detectCompression);
    group.writeEntry("LowQ Rejected",      lowQRejected);
    group.writeEntry("MediumQ Pending",    mediumQPending);
    group.writeEntry("HighQ Accepted",     highQAccepted);
    group.writeEntry("Speed",              speed);
}

QDebug operator<<(QDebug dbg, const ImageQualitySettings& s)
{
    dbg.nospace() << endl;
    dbg.nospace() << "EnableSorter      : " << s.enableSorter << endl;
    dbg.nospace() << "DetectBlur        : " << s.detectBlur << endl;
    dbg.nospace() << "DetectNoise       : " << s.detectNoise << endl;
    dbg.nospace() << "DetectCompression : " << s.detectCompression << endl;
    dbg.nospace() << "LowQRejected      : " << s.lowQRejected << endl;
    dbg.nospace() << "MediumQPending    : " << s.mediumQPending << endl;
    dbg.nospace() << "HighQAccepted     : " << s.highQAccepted << endl;
    dbg.nospace() << "Speed             : " << s.speed << endl;
    
    return dbg.space();
}

}  // namespace Digikam
