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
    vStandard    = PAL;
    vbitRate     = VBR12;
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
    vStandard    = (VidStd)group.readEntry("VStandard",
                   (int)PAL);
    vbitRate     = (VidBitRate)group.readEntry("VBitRate",
                   (int)VBR12);
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
    group.writeEntry("VBitRate",     (int)vbitRate);
    group.writeEntry("ABitRate",     abitRate);
    group.writeEntry("VStandard",    (int)vStandard);
    group.writeEntry("OutputVideo",  outputVideo);
}

QSize VidSlideSettings::videoTypeSize() const
{
    QSize s;

    switch(outputType)
    {
        case QVGA:
            s = QSize(320, 180);
            break;

        case VCD:
            s = QSize(352, 240);
            break;

        case HVGA:
            s = QSize(480, 270);
            break;

        case SVCD:
            s = QSize(480, 576);
            break;

        case VGA:
            s = QSize(640, 360);
            break;

        case DVD:
            s = QSize(720, 480);
            break;

        case WVGA:
            s = QSize(800, 450);
            break;

        case XVGA:
            s = QSize(1024, 576);
            break;

        case HDTV:
            s = QSize(1280, 720);
            break;

        case UHD4K:
            s = QSize(3840, 2160);
            break;

        default: // BLUERAY
            s = QSize(1920, 1080);
            break;
    }

    return s;
}

int VidSlideSettings::videoBitRate() const
{
    int b;

    switch(vbitRate)
    {
        case VBR04:
            b = 400000;
            break;

        case VBR05:
            b = 500000;
            break;

        case VBR10:
            b = 1000000;
            break;

        case VBR15:
            b = 1500000;
            break;

        case VBR20:
            b = 2000000;
            break;

        case VBR25:
            b = 2500000;
            break;

        case VBR30:
            b = 3000000;
            break;

        case VBR40:
            b = 4000000;
            break;

        case VBR45:
            b = 4500000;
            break;

        case VBR50:
            b = 5000000;
            break;

        case VBR60:
            b = 6000000;
            break;

        case VBR80:
            b = 8000000;
            break;

        default: // VBR12
            b = 1200000;
            break;
    }

    return b;
}

qreal VidSlideSettings::videoFrameRate() const
{
    int fr;

    switch(vStandard)
    {
        case NTSC:
            fr = 29.97;
            break;

        default: // PAL
            fr = 25.0;
            break;
    }

    return fr;
}

QMap<VidSlideSettings::VidType, QString> VidSlideSettings::videoTypeNames()
{
    QMap<VidType, QString> types;

    types[QVGA]    = i18nc("Video Type: QVGA",    "QVGA - 320x180");
    types[VCD]     = i18nc("Video Type: VCD",     "VCD - 352x240");
    types[HVGA]    = i18nc("Video Type: HVGA",    "HVGA - 480x270");
    types[SVCD]    = i18nc("Video Type: SVCD",    "SVCD - 480x576");
    types[VGA]     = i18nc("Video Type: VGA",     "VGA - 640x360");
    types[DVD]     = i18nc("Video Type: DVD",     "DVD - 720x576");
    types[WVGA]    = i18nc("Video Type: WVGA",    "WVGA - 800x450");
    types[XVGA]    = i18nc("Video Type: XVGA",    "XVGA - 1024x576");
    types[HDTV]    = i18nc("Video Type: HDTV",    "HDTV - 1280x720");
    types[BLUERAY] = i18nc("Video Type: BLUERAY", "BLUERAY - 1920x1080");
    types[UHD4K]   = i18nc("Video Type: UHD4K",   "UHD4K - 3840x2160");

    return types;
}

QMap<VidSlideSettings::VidBitRate, QString> VidSlideSettings::videoBitRateNames()
{
    QMap<VidBitRate, QString> br;

    br[VBR04]  = i18nc("Video Bit Rate 400000",  "400k");
    br[VBR05]  = i18nc("Video Bit Rate 500000",  "500k");
    br[VBR10]  = i18nc("Video Bit Rate 1000000", "1000k");
    br[VBR12]  = i18nc("Video Bit Rate 1200000", "1200k");
    br[VBR15]  = i18nc("Video Bit Rate 1500000", "1500k");
    br[VBR20]  = i18nc("Video Bit Rate 2000000", "2000k");
    br[VBR25]  = i18nc("Video Bit Rate 2500000", "2500k");
    br[VBR30]  = i18nc("Video Bit Rate 3000000", "3000k");
    br[VBR40]  = i18nc("Video Bit Rate 4000000", "4000k");
    br[VBR45]  = i18nc("Video Bit Rate 4500000", "4500k");
    br[VBR50]  = i18nc("Video Bit Rate 5000000", "5000k");
    br[VBR60]  = i18nc("Video Bit Rate 6000000", "6000k");
    br[VBR80]  = i18nc("Video Bit Rate 8000000", "8000k");

    return br;
}

QMap<VidSlideSettings::VidStd, QString> VidSlideSettings::videoStdNames()
{
    QMap<VidStd, QString> std;

    std[PAL]  = i18nc("Video Standard PAL",  "PAL - 25 FPS");
    std[NTSC] = i18nc("Video Standard NTSC", "NTSC - 29.97 FPS");

    return std;
}

} // namespace Digikam
