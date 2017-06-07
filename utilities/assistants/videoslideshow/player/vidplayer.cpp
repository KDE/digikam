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
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "vidplayer.h"

// Qt includes

#include <QPushButton>
#include <QSlider>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QLabel>
#include <QTime>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// QtAv includes

#include <QtAV/VideoOutput.h>
#include <QtAV/AVPlayer.h>

using namespace QtAV;

namespace Digikam
{

class VidPlayer::Private
{
public:

    Private()
    {
        videoOut = 0;
        player   = 0;
        slider   = 0;
        playBtn  = 0;
        pauseBtn = 0;
        slider   = 0;
        timeLbl  = 0;
        unit     = 1000;
    }

public:

    QString      file;
    VideoOutput* videoOut;
    AVPlayer*    player;
    QSlider*     slider;
    QPushButton* openBtn;
    QPushButton* playBtn;
    QPushButton* pauseBtn;
    QLabel*      timeLbl;
    int          unit;
};

VidPlayer::VidPlayer(const QString& file, QWidget* const parent)
    : QDialog(parent),
      d(new Private)

{
    setModal(false);
    setWindowTitle(file);

    d->file     = file;
    d->player   = new AVPlayer(this);
    d->videoOut = new VideoOutput(this);

    if (!d->videoOut->widget())
    {
        QMessageBox::warning(this, i18n("Error"), i18n("Can not create video renderer"));
        return;
    }

    d->player->setRenderer(d->videoOut);

    d->slider   = new QSlider(this);
    d->slider->setOrientation(Qt::Horizontal);
    d->timeLbl  = new QLabel(this);
    d->timeLbl->setText(QString::fromLatin1("00:00:00"));
    d->playBtn  = new QPushButton(this);
    d->playBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
    d->pauseBtn  = new QPushButton(this);
    d->pauseBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));

    // ----------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(d->videoOut->widget(), 0, 0, 1, 4);
    grid->addWidget(d->slider,             1, 0, 1, 1);
    grid->addWidget(d->timeLbl,            1, 1, 1, 1);
    grid->addWidget(d->playBtn,            1, 2, 1, 1);
    grid->addWidget(d->pauseBtn,           1, 3, 1, 1);
    grid->setColumnStretch(0, 10);
    setLayout(grid);

    // ----------------------

    connect(d->slider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotSeekBySlider(int)));

    connect(d->slider, SIGNAL(sliderPressed()),
            this, SLOT(slotSeekBySlider()));

    connect(d->player, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotUpdateSlider(qint64)));

    connect(d->player, SIGNAL(started()),
            this, SLOT(slotUpdateSlider()));

    connect(d->player, SIGNAL(notifyIntervalChanged()),
            this, SLOT(slotUpdateSliderUnit()));

    connect(d->playBtn, SIGNAL(clicked()),
            this, SLOT(slotPlay()));

    connect(d->pauseBtn, SIGNAL(clicked()),
            this, SLOT(slotPause()));

    QTimer::singleShot(500, this, SLOT(slotOpenMedia()));
}

VidPlayer::~VidPlayer()
{
    delete d;
}

void VidPlayer::slotOpenMedia()
{
    d->player->play(d->file);
}

void VidPlayer::slotSeekBySlider(int value)
{
    if (!d->player->isPlaying())
        return;

    d->player->seek(qint64(value * d->unit));
}

void VidPlayer::slotSeekBySlider()
{
    slotSeekBySlider(d->slider->value());
}

void VidPlayer::slotPlay()
{
    if (!d->player->isPlaying())
    {
        d->player->play();
        return;
    }
}

void VidPlayer::slotPause()
{
    d->player->pause(!d->player->isPaused());
}

void VidPlayer::slotUpdateSlider(qint64 value)
{
    d->slider->setRange(0, int(d->player->duration() / d->unit));
    d->slider->setValue(int(value / d->unit));
    d->timeLbl->setText(QTime(0, 0, 0).addMSecs(qint64(value))
                        .toString(QString::fromLatin1("HH:mm:ss")));
}

void VidPlayer::slotUpdateSlider()
{
    slotUpdateSlider(d->player->position());
}

void VidPlayer::slotUpdateSliderUnit()
{
    d->unit = d->player->notifyInterval();
    slotUpdateSlider();
}

} // namespace Digikam
