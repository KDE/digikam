/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-22
 * Description : Slideshow video viewer
 *
 * Copyright (C) 2014 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slidevideo.moc"

// Qt includes

#include <QWidget>
#include <QString>
#include <QGridLayout>

// KDE includes

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <phonon/seekslider.h>
#include <phonon/videoplayer.h>
#include <phonon/videowidget.h>

// Local includes


namespace Digikam
{

class SlideVideo::Private
{

public:

    Private() :
        player(0),
        slider(0)
    {
    }

    Phonon::VideoPlayer* player;
    Phonon::SeekSlider*  slider;

    KUrl                 url;
};

SlideVideo::SlideVideo(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setAutoFillBackground(true);

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    setPalette(palette);

    d->player = new Phonon::VideoPlayer(Phonon::VideoCategory, this);
    d->slider = new Phonon::SeekSlider(this);
    d->slider->setMediaObject(d->player->mediaObject());
    d->player->mediaObject()->setTickInterval(100);
    d->player->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->player->videoWidget(), 0, 0, 1, 3);
    grid->addWidget(d->slider,                1, 0, 1, 3);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(0, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------------------

    connect(d->player->mediaObject(), SIGNAL(finished()),
            this, SLOT(slotPlayerFinished()));

    connect(d->player->mediaObject(), SIGNAL(hasVideoChanged(bool)),
            this, SIGNAL(signalVideoLoaded(bool)));

    connect(d->player->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(slotPlayerstateChanged(Phonon::State, Phonon::State)));
}

SlideVideo::~SlideVideo()
{
    d->player->stop();
    delete d->player;
    delete d;
}

void SlideVideo::setCurrentUrl(const KUrl& url)
{
    d->url = url;
    d->player->play(d->url);
}

void SlideVideo::slotPlayerFinished()
{
    if (d->player->mediaObject()->errorType() == Phonon::NoError)
    {
        emit signalVideoFinished();
    }
    else
    {
        emit signalVideoLoaded(false);
    }
}

void SlideVideo::slotPlayerstateChanged(Phonon::State newState, Phonon::State oldState)
{
    if (oldState == Phonon::LoadingState && newState == Phonon::PlayingState)
    {
        emit signalVideoLoaded(true);
    }
    else if (newState == Phonon::ErrorState)
    {
        emit signalVideoLoaded(false);
    }
}

void SlideVideo::pause()
{
    d->player->pause();
}

void SlideVideo::play()
{
    d->player->play();
}

void SlideVideo::stop()
{
    d->player->stop();
}

}  // namespace Digikam
