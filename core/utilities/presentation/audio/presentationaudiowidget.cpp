/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-14
 * Description : a presentation tool.
 *
 * Copyright (C) 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "presentationaudiowidget.h"

// Qt includes

#include <QTime>
#include <QUrl>
#include <QKeyEvent>
#include <QIcon>

// QtAV includes

#include <QtAV/AVError.h>
#include <QtAV/AVPlayer.h>

// Local includes

#include "presentationcontainer.h"
#include "digikam_debug.h"

using namespace QtAV;

namespace Digikam
{

class PresentationAudioWidget::Private
{

public:

    Private()
    {
        sharedData  = 0;
        currIndex   = 0;
        mediaObject = 0;
        canHide     = true;
        isZeroTime  = false;
        playingNext = false;
    }

    PresentationContainer* sharedData;
    QList<QUrl>            urlList;
    int                    currIndex;
    bool                   canHide;
    bool                   isZeroTime;
    bool                   playingNext;

    AVPlayer*              mediaObject;
};

PresentationAudioWidget::PresentationAudioWidget(QWidget* const parent, const QList<QUrl>& urls, PresentationContainer* const sharedData)
    : QWidget(parent),
      d(new Private)
{
    setupUi(this);

    d->sharedData = sharedData;

    m_soundLabel->setPixmap(QIcon::fromTheme(QLatin1String("speaker")).pixmap(64, 64));

    m_prevButton->setText(QLatin1String(""));
    m_nextButton->setText(QLatin1String(""));
    m_playButton->setText(QLatin1String(""));
    m_stopButton->setText(QLatin1String(""));

    m_prevButton->setIcon(QIcon::fromTheme(QLatin1String("media-skip-backward")));
    m_nextButton->setIcon(QIcon::fromTheme(QLatin1String("media-skip-forward")));
    m_playButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
    m_stopButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-stop")));

    connect(m_prevButton, SIGNAL(clicked()),
            this, SLOT(slotPrev()));

    connect(m_nextButton, SIGNAL(clicked()),
            this, SLOT(slotNext()));

    connect(m_playButton, SIGNAL(clicked()),
            this, SLOT(slotPlay()));

    connect(m_stopButton, SIGNAL(clicked()),
            this, SLOT(slotStop()));

    if (urls.empty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Tracks list is empty...";
        setEnabled(false);
        return;
    }

    // Waiting for files to be enqueued.
    m_playButton->setEnabled(false);
    m_prevButton->setEnabled(false);

    d->mediaObject = new AVPlayer(this);

    connect(d->mediaObject, SIGNAL(mediaStatusChanged(QtAV::MediaStatus)),
            this, SLOT(slotMediaStateChanged(QtAV::MediaStatus)));

    connect(d->mediaObject, SIGNAL(stateChanged(QtAV::AVPlayer::State)),
            this, SLOT(slotPlayerStateChanged(QtAV::AVPlayer::State)));

    connect(d->mediaObject, SIGNAL(error(QtAV::AVError)),
            this, SLOT(slotPlayerError(QtAV::AVError)));

    connect(d->mediaObject, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotTimeUpdaterTimeout()));

    connect(m_volumeWidget, SIGNAL(valueChanged(int)),
            this, SLOT(slotSetVolume(int)));

    enqueue(urls);

    setZeroTime();
}

PresentationAudioWidget::~PresentationAudioWidget()
{
    if (!d->urlList.empty())
    {
        d->mediaObject->stop();
    }

    delete d;
}

void PresentationAudioWidget::slotSetVolume(int v)
{
    if (d->mediaObject->audio())
    {
        d->mediaObject->audio()->setVolume(v / 100.0);
    }
}

bool PresentationAudioWidget::canHide() const
{
    return d->canHide;
}

bool PresentationAudioWidget::isPaused() const
{
    return d->mediaObject->isPaused();
}

void PresentationAudioWidget::checkSkip()
{
    m_prevButton->setEnabled(true);
    m_nextButton->setEnabled(true);

    if (!d->sharedData->soundtrackLoop)
    {
        if (d->currIndex == 0)
            m_prevButton->setEnabled(false);

        if (d->currIndex == d->urlList.count() - 1)
            m_nextButton->setEnabled(false);
    }
}

void PresentationAudioWidget::setZeroTime()
{
    QTime zeroTime(0, 0, 0);
    m_elapsedTimeLabel->setText(zeroTime.toString(QLatin1String("H:mm:ss")));
    m_totalTimeLabel->setText(zeroTime.toString(QLatin1String("H:mm:ss")));
    d->isZeroTime = true;
}

void PresentationAudioWidget::enqueue(const QList<QUrl>& urls)
{
    d->urlList   = urls;
    d->currIndex = 0;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Tracks : " << d->urlList;

    if (d->urlList.isEmpty())
        return;

    m_playButton->setEnabled(true);
}

void PresentationAudioWidget::setPaused(bool val)
{
    if (val == isPaused())
        return;

    slotPlay();
}

void PresentationAudioWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case(Qt::Key_Space):
        {
            m_playButton->animateClick();
            break;
        }

        case(Qt::Key_A):
        {
            if (m_prevButton->isEnabled())
                m_prevButton->animateClick();

            break;
        }

        case(Qt::Key_S):
        {
            if (m_nextButton->isEnabled())
                m_nextButton->animateClick();

            break;
        }

        case(Qt::Key_Escape):
        {
            if (m_stopButton->isEnabled())
                m_stopButton->animateClick();

            break;
        }

        default:
            break;
    }

    event->accept();
}

void PresentationAudioWidget::slotPlay()
{
    if (!d->mediaObject->isPlaying() || d->mediaObject->isPaused())
    {
        if (!d->mediaObject->isPlaying())
        {
            d->mediaObject->setFile(d->urlList[d->currIndex].toLocalFile());
            d->mediaObject->play();
            setZeroTime();
        }
        else
        {
            d->mediaObject->pause(false);
        }

        d->canHide = true;
        emit signalPlay();
    }
    else
    {
        d->mediaObject->pause();
        d->canHide = false;
        emit signalPause();
    }
}

void PresentationAudioWidget::slotStop()
{
    d->playingNext = false;
    d->mediaObject->stop();
    d->currIndex  = 0;
    setZeroTime();
    checkSkip();
}

void PresentationAudioWidget::slotPrev()
{
    d->currIndex--;

    if (d->currIndex < 0)
    {
        if (d->sharedData->soundtrackLoop)
        {
            d->currIndex = d->urlList.count() - 1;
        }
        else
        {
            d->currIndex = 0;
            return;
        }
    }

    d->playingNext = false;
    d->mediaObject->stop();
    slotPlay();
}

void PresentationAudioWidget::slotNext()
{
    d->currIndex++;

    if (d->currIndex >= d->urlList.count())
    {
        if (d->sharedData->soundtrackLoop)
        {
            d->currIndex = 0;
        }
        else
        {
            d->currIndex = d->urlList.count() - 1;
            return;
        }
    }

    d->playingNext = false;
    d->mediaObject->stop();
    slotPlay();
}

void PresentationAudioWidget::slotTimeUpdaterTimeout()
{
    if (d->mediaObject->mediaStatus() == QtAV::InvalidMedia)
    {
        slotError();
        return;
    }

    qint64 current = d->mediaObject->position();
    int hours      = (int)(current  / (qint64)(60 * 60 * 1000));
    int mins       = (int)((current / (qint64)(60 * 1000)) - (qint64)(hours * 60));
    int secs       = (int)((current / (qint64)1000) - (qint64)(hours * 60 + mins * 60));
    QTime elapsedTime(hours, mins, secs);

    if (d->isZeroTime && d->mediaObject->duration() > 0)
    {
        d->isZeroTime = false;
        qint64 total  = d->mediaObject->duration();
        hours         = (int)(total  / (qint64)(60 * 60 * 1000));
        mins          = (int)((total / (qint64)(60 * 1000)) - (qint64)(hours * 60));
        secs          = (int)((total / (qint64)1000) - (qint64)(hours * 60 + mins * 60));
        QTime totalTime(hours, mins, secs);
        m_totalTimeLabel->setText(totalTime.toString(QLatin1String("H:mm:ss")));
    }

    m_elapsedTimeLabel->setText(elapsedTime.toString(QLatin1String("H:mm:ss")));
}

void PresentationAudioWidget::slotMediaStateChanged(QtAV::MediaStatus status)
{
    if (d->playingNext && (status == QtAV::EndOfMedia))
    {
        slotNext();
    }
}

void PresentationAudioWidget::slotPlayerError(const QtAV::AVError& err)
{
    if (err.error() != AVError::NoError)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "An error as occured while playing (" << err.string() << ")";
        slotError();
    }
}

void PresentationAudioWidget::slotPlayerStateChanged(QtAV::AVPlayer::State state)
{
    switch (state)
    {
        case QtAV::AVPlayer::PausedState:
        case QtAV::AVPlayer::StoppedState:
            m_playButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
            checkSkip();
            break;

        case QtAV::AVPlayer::PlayingState:
            m_playButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
            d->playingNext = true;
            checkSkip();
            break;

        default:
            break;
    }
}

void PresentationAudioWidget::slotError()
{
    /* TODO :
     * Display error on slideshow.
     * A QWidget pop-up could help
     */

    slotNext();
}

} // namespace Digikam
