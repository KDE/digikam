/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : Qt Multimedia based video thumbnailer
 *
 * Copyright (C) 2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef VIDEOTHUMBNAILER_P_H
#define VIDEOTHUMBNAILER_P_H

#include "videothumbnailer.h"

// Qt includes

#include <QObject>
#include <QString>
#include <QTimer>

// QtAV includes

#include <QtAV/QtAV.h>
#include <QtAV/AVError.h>
#include <QtAV/AVPlayer.h>
#include <QtAV/VideoFrameExtractor.h>

using namespace QtAV;

namespace Digikam
{

class VideoThumbnailer::Private : public QObject
{
    Q_OBJECT

public:

    Private(VideoThumbnailer* const p);

public Q_SLOTS:

    void slotMediaStatusChanged(QtAV::MediaStatus);
    void slotFrameExtracted(const QtAV::VideoFrame& frame);
    void slotHandlePlayerError();
    void slotExtractedTimout();

public:

    bool                 createStrip;
    int                  thumbSize;
    quint64              thumbJob;
    bool                 ready;
    QString              file;

    AVPlayer*            player;
    VideoFrameExtractor* extractor;

private:

    qint64            position;
    qint64            duration;
    QImage            strip;
    QTimer*           timer;
    VideoThumbnailer* dd;
};

}  // namespace Digikam

#endif /* VIDEOTHUMBNAILER_P_H */
