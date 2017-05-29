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

#ifndef VIDSLIDE_SETTINGS_H
#define VIDSLIDE_SETTINGS_H

// Qt includes

#include <QString>
#include <QList>
#include <QUrl>
#include <QSize>
#include <QMap>

// Local includes

#include "dinfointerface.h"
#include "transitionmngr.h"

class KConfigGroup;

namespace Digikam
{

class VidSlideSettings
{
public:

    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

    // See https://en.wikipedia.org/wiki/List_of_common_resolutions#Digital_TV_standards
    enum VidType
    {
        RIM240 = 0,     // 240  x 136
        QVGA,           // 320  x 180
        VCD,            // 352  x 240
        HVGA,           // 480  x 270
        SVCD,           // 480  x 576
        VGA,            // 640  x 360
        DVD,            // 720  x 576
        WVGA,           // 800  x 450
        XVGA,           // 1024 x 576
        HDTV,           // 1280 x 720
        BLUERAY,        // 1920 x 1080
        UHD4K,          // 3840 x 2160
        UHD8K           // 7680 x 4320
    };

public:

    explicit VidSlideSettings();
    ~VidSlideSettings();

    QSize typeToSize() const;

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    static QMap<VidType, QString> typeNames();

public:

    bool                      openInPlayer;  // Open video stream in desktop player at end.
    Selection                 selMode;       // Items selection mode

    QList<QUrl>               inputImages;   // Selection::IMAGES
    DInfoInterface::DAlbumIDs inputAlbums;   // Selection::ALBUMS

    QList<QUrl>               inputAudio;    // Soundtracks streams.

    TransitionMngr::TransType transition;    // Transition type between images.

    int                       aframes;       // Amount of frames to encode in video stream by image.
                                             // ex: 120 frames = 5 s at 24 img/s.

    VidType                   outputType;    // Encoded video type.
    QUrl                      outputVideo;   // Encoded video stream.
};

} // namespace Digikam

#endif // VIDSLIDE_SETTINGS_H
