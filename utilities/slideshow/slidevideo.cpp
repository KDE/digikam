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

#include "slidevideo.h"

// Qt includes

#include <QWidget>
#include <QString>
#include <QGridLayout>
#include <QApplication>
#include <QSlider>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// QtAV includes

#include <QtAV/AVError.h>
#include <QtAV/AVPlayer.h>
#include <QtAVWidgets/WidgetRenderer.h>

// Local includes

#include "digikam_debug.h"

using namespace QtAV;

namespace Digikam
{

class SlideVideo::Private
{

public:

    Private() :
        videoWidget(0),
        player(0),
        slider(0)
    {
    }

    WidgetRenderer*      videoWidget;
    AVPlayer*            player;
    QSlider*             slider;
};

SlideVideo::SlideVideo(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setAutoFillBackground(true);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QMargins margins(spacing, 0, spacing, spacing);

    d->videoWidget = new WidgetRenderer(this);
    d->player      = new AVPlayer(this);

    d->slider      = new QSlider(Qt::Horizontal, this);
    d->slider->setRange(0, 0);

    d->player->addVideoRenderer(d->videoWidget);
    d->player->setNotifyInterval(250);

    d->videoWidget->setOutAspectRatioMode(VideoRenderer::VideoAspectRatio);
    d->videoWidget->setStyleSheet(QLatin1String("background-color:black;"));
    d->videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* const vbox2 = new QVBoxLayout(this);
    vbox2->addWidget(d->videoWidget, 10);
    vbox2->addWidget(d->slider,       0);
    vbox2->setContentsMargins(margins);
    vbox2->setSpacing(spacing);

    // --------------------------------------------------------------------------

    connect(d->slider, SIGNAL(sliderPressed()),
            this, SLOT(slotSliderPressed()));

    connect(d->slider, SIGNAL(sliderReleased()),
            this, SLOT(slotSliderReleased()));

    connect(d->slider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotPosition(int)));

    connect(d->player, SIGNAL(mediaStatusChanged(QtAV::MediaStatus)),
            this, SLOT(slotPlayerStateChanged(QtAV::MediaStatus)));

    connect(d->player, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotPositionChanged(qint64)));

    connect(d->player, SIGNAL(durationChanged(qint64)),
            this, SLOT(slotDurationChanged(qint64)));

    connect(d->player, SIGNAL(error(QtAV::AVError)),
            this, SLOT(slotHandlePlayerError(QtAV::AVError)));

    // --------------------------------------------------------------------------

    layout()->activate();
    resize(sizeHint());
    show();
}

SlideVideo::~SlideVideo()
{
    d->player->stop();
    delete d->player;
    delete d->videoWidget;
    delete d->slider;
    delete d;
}

void SlideVideo::setCurrentUrl(const QUrl& url)
{
    d->player->setFile(url.toLocalFile());
    d->player->play();
}

void SlideVideo::slotPlayerStateChanged(QtAV::MediaStatus newState)
{
    switch (newState)
    {
        case EndOfMedia:
            emit signalVideoFinished();
            break;
        case LoadedMedia:
            emit signalVideoLoaded(true);
            break;
        case UnknownMediaStatus:
        case NoMedia:
        case StalledMedia:
        case InvalidMedia:
            emit signalVideoLoaded(false);
            break;
        default:
            break;
    }
}

void SlideVideo::pause(bool b)
{
    if (b && d->player->state() == AVPlayer::PlayingState)
    {
        d->player->pause();
    }
    else
    {
        d->player->togglePause();
    }
}

void SlideVideo::stop()
{
    d->player->stop();
}

void SlideVideo::slotPositionChanged(qint64 position)
{
    if (!d->slider->isSliderDown())
    {
        d->slider->setValue(position);
    }
}

void SlideVideo::slotDurationChanged(qint64 duration)
{
    qint64 max = qMax((qint64)1, duration);
    d->slider->setRange(0, max);
}

void SlideVideo::slotPosition(int position)
{
    if (d->player->isSeekable())
    {
        d->player->setPosition((qint64)position);
    }
}

void SlideVideo::slotSliderPressed()
{
    if (d->player->state() == AVPlayer::PlayingState)
    {
        d->player->pause();
    }
    else if (d->player->state() == AVPlayer::StoppedState)
    {
        d->player->play();
    }
}

void SlideVideo::slotSliderReleased()
{
    if (d->player->state() == AVPlayer::PausedState)
    {
        d->player->togglePause();
    }
}

void SlideVideo::slotHandlePlayerError(const QtAV::AVError& err)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Error: " << err.string();
}

}  // namespace Digikam
