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
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// QtAV includes

#include <QtAV/AVPlayer.h>
#include <QtAVWidgets/WidgetRenderer.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"

using namespace QtAV;

namespace Digikam
{
/*
class SlidePlayerThread : public QThread
{
public:

    explicit SlidePlayerThread(AVPlayer* const player)
      : QThread(0),
        m_player(player)
    {
        m_player->moveToThread(this);
    }

    virtual ~SlidePlayerThread()
    {
    }

private:

    virtual void run()
    {
        exec();
    }

private:

    AVPlayer* m_player;
};
*/
// --------------------------------------------------------

class SlideVideo::Private
{

public:

    Private() :
        videoWidget(0),
        player(0),
        //thread(0),
        slider(0),
        tlabel(0),
        indicator(0)
    {
    }

    WidgetRenderer*      videoWidget;
    AVPlayer*            player;
    //SlidePlayerThread*   thread;
    QSlider*             slider;
    QLabel*              tlabel;
    DHBox*               indicator;
};

SlideVideo::SlideVideo(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setAutoFillBackground(true);

    d->videoWidget    = new WidgetRenderer(this);
    d->videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->videoWidget->setOutAspectRatioMode(VideoRenderer::VideoAspectRatio);

    d->player         = new AVPlayer(this);
    d->player->setRenderer(d->videoWidget);
    //d->thread         = new SlidePlayerThread(d->player);

    d->indicator      = new DHBox(this);
    d->slider         = new QSlider(Qt::Horizontal, d->indicator);
    d->slider->setRange(0, 0);
    d->slider->setAutoFillBackground(true);
    d->tlabel         = new QLabel(d->indicator);
    d->tlabel->setText(QString::fromLatin1("00:00:00 / 00:00:00"));
    d->tlabel->setAutoFillBackground(true);
    d->indicator->setStretchFactor(d->slider, 10);
    d->indicator->setAutoFillBackground(true);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->videoWidget, 0, 0, 2, 1);
    grid->addWidget(d->indicator,   0, 0, 1, 1); // Widget will be over player to not change layout when visibility is changed.
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 100);
    grid->setContentsMargins(QMargins(0, 0, 0, 0));

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
    //d->thread->quit();
    //d->thread->wait();
    //delete d->thread;
    //delete d->player;
    delete d;
}

void SlideVideo::showIndicator(bool b)
{
    d->indicator->setVisible(b);
}

void SlideVideo::setCurrentUrl(const QUrl& url)
{
    d->player->stop();
    d->player->setFile(url.toLocalFile());
    //d->thread->start();
    d->player->play();
    showIndicator(false);
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
        case InvalidMedia:
            emit signalVideoLoaded(false);
            break;
        default:
            break;
    }
}

void SlideVideo::pause(bool b)
{
    d->player->pause(b);
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

    d->tlabel->setText(QString::fromLatin1("%1 / %2")
                       .arg(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")))
                       .arg(QTime(0, 0, 0).addMSecs(d->slider->maximum()).toString(QString::fromLatin1("HH:mm:ss"))));

    emit signalVideoPosition(position);
}

void SlideVideo::slotDurationChanged(qint64 duration)
{
    qint64 max = qMax((qint64)1, duration);
    d->slider->setRange(0, max);

    emit signalVideoDuration(duration);
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
    if (!d->player->isPlaying())
    {
        d->player->play();
        return;
    }

    d->player->pause(true);
}

void SlideVideo::slotSliderReleased()
{
    if (d->player->isPaused())
    {
        d->player->pause(false);
    }
}

void SlideVideo::slotHandlePlayerError(const QtAV::AVError& err)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Error: " << err.string();
}

} // namespace Digikam
