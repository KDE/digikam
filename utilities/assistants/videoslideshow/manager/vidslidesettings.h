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

public:

    explicit VidSlideSettings();
    ~VidSlideSettings();

public:

    bool                      openInPlayer;
    Selection                 selMode;

    QList<QUrl>               inputImages;   // Selection::IMAGES
    DInfoInterface::DAlbumIDs inputAlbums;   // Selection::ALBUMS

    QList<QUrl>               inputAudio;    // Soundtracks stream

    // Amount of frames to encode in video stream by image.
    // ex: 120 frames = 5 s at 24 img/s.
    int                       aframes;
    QSize                     outputSize;
    QUrl                      outputVideo;   // Encoded video stream.
};

} // namespace Digikam

#endif // VIDSLIDE_SETTINGS_H
