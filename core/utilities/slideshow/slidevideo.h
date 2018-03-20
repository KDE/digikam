/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-22
 * Description : Slideshow video viewer
 *
 * Copyright (C) 2014-2017 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDE_VIDEO_H
#define SLIDE_VIDEO_H

// Qt includes

#include <QWidget>
#include <QUrl>
#include <QEvent>

// QtAV includes

#include <QtAV/AVError.h>

namespace Digikam
{

class SlideVideo : public QWidget
{
    Q_OBJECT

public:

    explicit SlideVideo(QWidget* const parent);
    virtual ~SlideVideo();

    void showIndicator(bool);
    void setCurrentUrl(const QUrl& url);
    void pause(bool);
    void stop();

Q_SIGNALS:

    void signalVideoLoaded(bool);
    void signalVideoFinished();

    void signalVideoPosition(qint64);
    void signalVideoDuration(qint64);

private Q_SLOTS:

    void slotPlayerStateChanged(QtAV::MediaStatus);
    void slotHandlePlayerError(const QtAV::AVError&);

    void slotPositionChanged(qint64 position);
    void slotDurationChanged(qint64 duration);
    void slotPosition(int position);
    void slotSliderPressed();
    void slotSliderReleased();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SLIDE_VIDEO_H
