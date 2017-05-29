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
        VCD = 0,        // 352  x 240   (240p)
        SVCD,           // 480  x 576   (576p)
        DVD,            // 720  x 480   (480p)
        HDTV,           // 1280 x 720   (720p)
        BLUERAY,        // 1920 x 1080  (1080p)
        UHD4K,          // 3840 x 2160  (4k)
        UHD8K           // 7680 x 4320  (8k)
    };

public:

    explicit VidSlideSettings();
    ~VidSlideSettings();

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

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
