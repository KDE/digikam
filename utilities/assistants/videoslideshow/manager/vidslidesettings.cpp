/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "vidslidesettings.h"

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

VidSlideSettings::VidSlideSettings()
{
    openInPlayer = true;
    selMode      = IMAGES;
    outputType   = BLUERAY;
    aframes      = 120;
    transition   = TransitionMngr::None;
    outputVideo  = QUrl::fromLocalFile(QLatin1String("./out.mp4"));
}

VidSlideSettings::~VidSlideSettings()
{
}

void VidSlideSettings::readSettings(KConfigGroup& group)
{
    openInPlayer = group.readEntry("OpenInPlayer",
                   true);
    selMode      = (Selection)group.readEntry("SelMode",
                   (int)IMAGES);
    outputType   = (VidType)group.readEntry("OutputType",
                   (int)BLUERAY);
    transition   = (TransitionMngr::TransType)group.readEntry("Transition",
                   (int)TransitionMngr::None);
    aframes      = group.readEntry("AFrames",
                   120);
    outputVideo  = group.readEntry("OutputVideo",
                   QUrl::fromLocalFile(QLatin1String("./out.mp4")));
}

void VidSlideSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("OpenInPlayer", openInPlayer);
    group.writeEntry("SelMode",      (int)selMode);
    group.writeEntry("OutputType",   (int)outputType);
    group.writeEntry("Transition",   (int)transition);
    group.writeEntry("AFrames",      aframes);
    group.writeEntry("OutputVideo",  outputVideo);
}

} // namespace Digikam
