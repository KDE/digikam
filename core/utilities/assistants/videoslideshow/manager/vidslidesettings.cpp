/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "vidslidesettings.h"

// Qt includes

#include <QStandardPaths>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

VidSlideSettings::VidSlideSettings()
{
    selMode      = IMAGES;
    imgFrames    = 125;
    vType        = BLUERAY;
    vStandard    = PAL;
    vbitRate     = VBR12;
    vCodec       = X264;
    vFormat      = MP4;
    vEffect      = EffectMngr::None;
    transition   = TransitionMngr::None;
    abitRate     = 64000;
    conflictRule = FileSaveConflictBox::OVERWRITE;
    outputDir    = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    outputPlayer = INTERNAL;
}

VidSlideSettings::~VidSlideSettings()
{
}

void VidSlideSettings::readSettings(KConfigGroup& group)
{
    selMode      = (Selection)group.readEntry("SelMode",
                   (int)IMAGES);
    imgFrames    = group.readEntry("ImgFrames",
                   125);
    vStandard    = (VidStd)group.readEntry("VStandard",
                   (int)PAL);
    vbitRate     = (VidBitRate)group.readEntry("VBitRate",
                   (int)VBR12);
    vCodec       = (VidCodec)group.readEntry("VCodec",
                   (int)X264);
    vType        = (VidType)group.readEntry("VType",
                   (int)BLUERAY);
    vFormat      = (VidFormat)group.readEntry("VFormat",
                   (int)MP4);
    vEffect      = (EffectMngr::EffectType)group.readEntry("VEffect",
                   (int)EffectMngr::None);
    abitRate     = group.readEntry("ABitRate",
                   64000);
    transition   = (TransitionMngr::TransType)group.readEntry("Transition",
                   (int)TransitionMngr::None);
    conflictRule = (FileSaveConflictBox::ConflictRule)group.readEntry("ConflictRule",
                   (int)FileSaveConflictBox::OVERWRITE);
    outputDir    = group.readEntry("OutputDir",
                   QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)));
    outputPlayer = (VidPlayer)group.readEntry("OutputPlayer",
                   (int)INTERNAL);
}

void VidSlideSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("SelMode",      (int)selMode);
    group.writeEntry("ImgFrames",    imgFrames);
    group.writeEntry("VStandard",    (int)vStandard);
    group.writeEntry("VBitRate",     (int)vbitRate);
    group.writeEntry("VCodec",       (int)vCodec);
    group.writeEntry("VType",        (int)vType);
    group.writeEntry("VFormat",      (int)vFormat);
    group.writeEntry("VEffect",      (int)vEffect);
    group.writeEntry("Transition",   (int)transition);
    group.writeEntry("ABitRate",     abitRate);
    group.writeEntry("ConflictRule", (int)conflictRule);
    group.writeEntry("OutputDir",    outputDir);
    group.writeEntry("OutputPlayer", (int)outputPlayer);
}

QSize VidSlideSettings::videoSize() const
{
    QSize s;

    switch(vType)
    {
        case QVGA:
            s = QSize(320, 180);
            break;

        case VCD1:
            s = QSize(352, 240);
            break;

        case VCD2:
            s = QSize(352, 288);
            break;

        case HVGA:
            s = QSize(480, 270);
            break;

        case SVCD1:
            s = QSize(480, 480);
            break;

        case SVCD2:
            s = QSize(480, 576);
            break;

        case VGA:
            s = QSize(640, 360);
            break;

        case DVD1:
            s = QSize(720, 480);
            break;

        case DVD2:
            s = QSize(720, 576);
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
    qreal fr;

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

QString VidSlideSettings::videoCodec() const
{
    QString cod;

    switch(vCodec)
    {
        case MJPEG:
            cod = QLatin1String("mjpeg");
            break;

        case MPEG2:
            cod = QLatin1String("mpeg2video");
            break;

        case MPEG4:
            cod = QLatin1String("mpeg4");
            break;

        case WEBMVP8:
            cod = QLatin1String("libvpx");
            break;

        case FLASH:
            cod = QLatin1String("flv");
            break;

        case THEORA:
            cod = QLatin1String("libtheora");
            break;

        case WMV7:
            cod = QLatin1String("wm1");
            break;

        case WMV8:
            cod = QLatin1String("wm2");
            break;

        case WMV9:
            cod = QLatin1String("wm3");
            break;

        default: // X264
            cod = QLatin1String("libx264");
            break;
    }

    return cod;
}

QString VidSlideSettings::videoFormat() const
{
    QString frm;

    switch(vFormat)
    {
        case AVI:
            frm = QLatin1String("avi");
            break;

        case MKV:
            frm = QLatin1String("mkv");
            break;

        case MPG:
            frm = QLatin1String("mpg");
            break;

        default: // MP4
            frm = QLatin1String("mp4");
            break;
    }

    return frm;
}

QMap<VidSlideSettings::VidType, QString> VidSlideSettings::videoTypeNames()
{
    QMap<VidType, QString> types;

    types[QVGA]    = i18nc("Video Type: QVGA",    "QVGA - 320x180 - 16:9");
    types[VCD1]    = i18nc("Video Type: VCD",     "VCD - 352x240");
    types[VCD2]    = i18nc("Video Type: VCD",     "VCD - 352x288");
    types[HVGA]    = i18nc("Video Type: HVGA",    "HVGA - 480x270 - 16:9");
    types[SVCD1]   = i18nc("Video Type: SVCD",    "SVCD - 480x480");
    types[SVCD2]   = i18nc("Video Type: SVCD",    "SVCD - 480x576");
    types[VGA]     = i18nc("Video Type: VGA",     "VGA - 640x360 - 16:9");
    types[DVD1]    = i18nc("Video Type: DVD",     "DVD - 720x480");
    types[DVD2]    = i18nc("Video Type: DVD",     "DVD - 720x576");
    types[WVGA]    = i18nc("Video Type: WVGA",    "WVGA - 800x450 - 16:9");
    types[XVGA]    = i18nc("Video Type: XVGA",    "XVGA - 1024x576 - 16:9");
    types[HDTV]    = i18nc("Video Type: HDTV",    "HDTV - 1280x720 - 16:9");
    types[BLUERAY] = i18nc("Video Type: BLUERAY", "BLUERAY - 1920x1080 - 16:9");
    types[UHD4K]   = i18nc("Video Type: UHD4K",   "UHD4K - 3840x2160 - 16:9");

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

QMap<VidSlideSettings::VidCodec, QString> VidSlideSettings::videoCodecNames()
{
    QMap<VidCodec, QString> codecs;

    // NOTE: Some video codecs are currently disabled due to QtAV incompatibility
    //       with bits rate, frames rate, or video sizes. This need some investiguations.

    codecs[X264]    = i18nc("Video Codec X264",    "High Quality H.264 AVC/MPEG-4 AVC");
    codecs[MPEG4]   = i18nc("Video Codec MPEG4",   "DivX/XVid/MPEG-4");
    codecs[MPEG2]   = i18nc("Video Codec MPEG2",   "MPEG-2 Video");
//    codecs[MJPEG]   = i18nc("Video Codec MJPEG",   "Motion JPEG");
//    codecs[WEBMVP8] = i18nc("Video Codec WEBMVP8", "WebM-VP8");
//    codecs[THEORA]  = i18nc("Video Codec THEORA",  "Theora-VP3");
//    codecs[FLASH]   = i18nc("Video Codec FLASH",   "Flash Video/Sorenson H.263");
//    codecs[WMV7]    = i18nc("Video Codec WMV7",    "Window Media Video 7");
//    codecs[WMV8]    = i18nc("Video Codec WMV8",    "Window Media Video 8");
//    codecs[WMV9]    = i18nc("Video Codec WMV9",    "Window Media Video 9");

    return codecs;
}

QMap<VidSlideSettings::VidFormat, QString> VidSlideSettings::videoFormatNames()
{
    QMap<VidFormat, QString> frm;

    frm[AVI] = i18nc("Video Standard AVI", "AVI - Audio Video Interleave");
    frm[MKV] = i18nc("Video Standard MKV", "MKV - Matroska");
    frm[MP4] = i18nc("Video Standard MP4", "MP4 - MPEG-4");
    frm[MPG] = i18nc("Video Standard MPG", "MPG - MPEG-2");

    return frm;
}

QMap<VidSlideSettings::VidPlayer, QString> VidSlideSettings::videoPlayerNames()
{
    QMap<VidPlayer, QString> pla;

    pla[NOPLAYER] = i18nc("Video Effect NOPLAYER",   "None");
    pla[INTERNAL] = i18nc("Video Standard INTERNAL", "Internal");
    pla[DESKTOP]  = i18nc("Video Standard DESKTOP",  "Default from Desktop");

    return pla;
}

} // namespace Digikam
