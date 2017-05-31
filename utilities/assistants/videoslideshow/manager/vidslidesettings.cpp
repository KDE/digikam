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
#include <klocalizedstring.h>

namespace Digikam
{

VidSlideSettings::VidSlideSettings()
{
    openInPlayer = true;
    selMode      = IMAGES;
    outputType   = BLUERAY;
    aframes      = 125;
    frameRate    = 25.0;
    vbitRate     = 1024*1024;
    abitRate     = 64000;
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
                   125);
    frameRate    = group.readEntry("FrameRate",
                   25.0);
    vbitRate     = group.readEntry("FrameRate",
                   1024*1024);
    abitRate     = group.readEntry("AudioRate",
                   64000);
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
    group.writeEntry("VBitRate",     vbitRate);
    group.writeEntry("ABitRate",     abitRate);
    group.writeEntry("FramesRate",   frameRate);
    group.writeEntry("OutputVideo",  outputVideo);
}

QSize VidSlideSettings::typeToSize() const
{
    QSize s;

    switch(outputType)
    {
        case VidSlideSettings::RIM240:
            s = QSize(240, 136);
            break;

        case VidSlideSettings::QVGA:
            s = QSize(320, 180);
            break;

        case VidSlideSettings::VCD:
            s = QSize(352, 240);
            break;

        case VidSlideSettings::HVGA:
            s = QSize(480, 270);
            break;

        case VidSlideSettings::SVCD:
            s = QSize(480, 576);
            break;

        case VidSlideSettings::VGA:
            s = QSize(640, 360);
            break;

        case VidSlideSettings::DVD:
            s = QSize(720, 480);
            break;

        case VidSlideSettings::WVGA:
            s = QSize(800, 450);
            break;

        case VidSlideSettings::XVGA:
            s = QSize(1024, 576);
            break;

        case VidSlideSettings::HDTV:
            s = QSize(1280, 720);
            break;

        case VidSlideSettings::UHD4K:
            s = QSize(3840, 2160);
            break;

        case VidSlideSettings::UHD8K:
            s = QSize(7680, 4320);
            break;

        default: // VidSlideSettings::BLUERAY
            s = QSize(1920, 1080);

            break;
    }

    return s;
}


QMap<VidSlideSettings::VidType, QString> VidSlideSettings::typeNames()
{
    QMap<VidType, QString> types;

    types[RIM240]  = i18nc("Video Type: RIM240",  "RIM240 - 240x136 - 25 FPS");
    types[QVGA]    = i18nc("Video Type: QVGA",    "QVGA - 320x180 - 25 FPS");
    types[VCD]     = i18nc("Video Type: VCD",     "VCD - 352x240 - 25 FPS");
    types[HVGA]    = i18nc("Video Type: HVGA",    "HVGA - 480x270 - 25 FPS");
    types[SVCD]    = i18nc("Video Type: SVCD",    "SVCD - 480x576 - 25 FPS");
    types[VGA]     = i18nc("Video Type: VGA",     "VGA - 640x360 - 25 FPS");
    types[DVD]     = i18nc("Video Type: DVD",     "DVD - 720x576 - 25 FPS");
    types[WVGA]    = i18nc("Video Type: WVGA",    "WVGA - 800x450 - 25 FPS");
    types[XVGA]    = i18nc("Video Type: XVGA",    "XVGA - 1024x576 - 25 FPS");
    types[HDTV]    = i18nc("Video Type: HDTV",    "HDTV - 1280x720 - 25 FPS");
    types[BLUERAY] = i18nc("Video Type: BLUERAY", "BLUERAY - 1920x1080 - 25 FPS");
    types[UHD4K]   = i18nc("Video Type: UHD4K",   "UHD4K - 3840x2160 - 25 FPS");
    types[UHD8K]   = i18nc("Video Type: UHD8K",   "UHD8K - 7680x4320 - 25 FPS");

    return types;
}

} // namespace Digikam
