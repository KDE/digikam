/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed Phonon media player.
 *
 * Copyright (C) 2006-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mediaplayerview.h"

// Qt includes

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QToolBar>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QAction>
#include <QVideoWidget>
#include <QSlider>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "stackedview.h"
#include "thememanager.h"
#include "digikam_debug.h"

namespace Digikam
{

MediaPlayerMouseClickFilter::MediaPlayerMouseClickFilter(QObject* const parent)
    : QObject(parent),
      m_parent(parent)
{
}

bool MediaPlayerMouseClickFilter::eventFilter(QObject* obj, QEvent* event)
{
    if ((qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick)  && event->type() == QEvent::MouseButtonRelease) ||
        (!qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick) && event->type() == QEvent::MouseButtonDblClick))
    {
        QMouseEvent* const mouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (mouseEvent && mouseEvent->button() == Qt::LeftButton)
        {
            if (m_parent)
            {
                MediaPlayerView* const mplayer = dynamic_cast<MediaPlayerView*>(m_parent);

                if (mplayer)
                {
                    mplayer->slotEscapePressed();
                }
            }

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}

// --------------------------------------------------------

class MediaPlayerView::Private
{

public:

    enum MediaPlayerViewMode
    {
        ErrorView=0,
        PlayerView
    };

public:

    Private() :
        errorView(0),
        playerView(0),
        prevAction(0),
        nextAction(0),
        toolBar(0),
        videoWidget(0),
        player(0), 
        slider(0)
    {
    }

    QFrame*              errorView;
    QFrame*              playerView;

    QAction*             prevAction;
    QAction*             nextAction;

    QToolBar*            toolBar;

    QVideoWidget*        videoWidget;
    QMediaPlayer*        player;
    QSlider*             slider;
    QUrl                 currentItem;
};

MediaPlayerView::MediaPlayerView(QWidget* const parent)
    : QStackedWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->prevAction          = new QAction(QIcon::fromTheme(QLatin1String("go-previous")), i18nc("go to previous image", "Back"), this);
    d->nextAction          = new QAction(QIcon::fromTheme(QLatin1String("go-next")),     i18nc("go to next image", "Forward"),  this);

    d->errorView           = new QFrame(this);
    QLabel* const errorMsg = new QLabel(i18n("An error has occurred with the media player...."), this);

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    d->errorView->setLineWidth(1);

    QGridLayout* const grid1 = new QGridLayout;
    grid1->addWidget(errorMsg, 1, 0, 1, 3);
    grid1->setColumnStretch(0, 10);
    grid1->setColumnStretch(2, 10);
    grid1->setRowStretch(0, 10);
    grid1->setRowStretch(2, 10);
    grid1->setContentsMargins(spacing, 0, spacing, spacing);
    grid1->setSpacing(spacing);
    d->errorView->setLayout(grid1);

    insertWidget(Private::ErrorView, d->errorView);

    // --------------------------------------------------------------------------

    d->playerView  = new QFrame(this);
    d->videoWidget = new QVideoWidget(this);
    d->player      = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    d->player->setVideoOutput(d->videoWidget);

    d->slider      = new QSlider(Qt::Horizontal, this);
    d->slider->setRange(0, 0);

    d->player->setNotifyInterval(250);
    d->videoWidget->setStyleSheet(QLatin1String("background-color:black;"));
    d->videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->playerView->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    d->playerView->setLineWidth(1);

    QGridLayout* const grid2 = new QGridLayout;
    grid2->addWidget(d->videoWidget, 0, 0, 1, 3);
    grid2->addWidget(d->slider,      1, 0, 1, 3);
    grid2->setColumnStretch(0, 10);
    grid2->setColumnStretch(2, 10);
    grid2->setRowStretch(0, 10);
    grid2->setContentsMargins(spacing, 0, spacing, spacing);
    grid2->setSpacing(spacing);
    d->playerView->setLayout(grid2);

    insertWidget(Private::PlayerView, d->playerView);

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->setAutoFillBackground(true);

    setPreviewMode(Private::PlayerView);

    d->errorView->installEventFilter(new MediaPlayerMouseClickFilter(this));
    d->videoWidget->installEventFilter(new MediaPlayerMouseClickFilter(this));

    // --------------------------------------------------------------------------

    connect(this, SIGNAL(signalFinished()),
            this, SLOT(slotPlayerFinished()));

    connect(d->player, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(slotPlayerStateChanged(QMediaPlayer::State)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(signalPrevItem()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(signalNextItem()));

    connect(d->slider, SIGNAL(sliderPressed()),
            this, SLOT(slotSliderPressed()));

    connect(d->slider, SIGNAL(sliderReleased()),
            this, SLOT(slotSliderReleased()));

    connect(d->slider, SIGNAL(sliderMoved(int)),
            this, SLOT(setPosition(int)));

    connect(d->player, SIGNAL(positionChanged(qint64)),
            this, SLOT(positionChanged(qint64)));

    connect(d->player, SIGNAL(durationChanged(qint64)),
            this, SLOT(durationChanged(qint64)));

    connect(d->player, SIGNAL(error(QMediaPlayer::Error)),
            this, SLOT(handlePlayerError()));
}

MediaPlayerView::~MediaPlayerView()
{
    d->player->stop();
    delete d->player;
    delete d->videoWidget;
    delete d->slider;
    delete d;
}

void MediaPlayerView::reload()
{
    d->player->stop();
    d->player->setMedia(d->currentItem);
    d->player->play();
}

void MediaPlayerView::slotPlayerFinished()
{
    if (d->player->state() == QMediaPlayer::StoppedState  &&
        d->player->error() != QMediaPlayer::NoError)
    {
        setPreviewMode(Private::ErrorView);
    }
}

void MediaPlayerView::slotPlayerStateChanged(QMediaPlayer::State newState)
{
    if (newState           == QMediaPlayer::StoppedState  &&
        d->player->error() != QMediaPlayer::NoError)
    {
        setPreviewMode(Private::ErrorView);
    }

    if (newState                 == QMediaPlayer::StoppedState &&
        d->player->mediaStatus() == QMediaPlayer::EndOfMedia)
    {
        emit signalFinished();
    }
}

void MediaPlayerView::escapePreview()
{
    d->player->stop();
}

void MediaPlayerView::slotThemeChanged()
{
    QPalette palette;
    palette.setColor(d->errorView->backgroundRole(), qApp->palette().color(QPalette::Base));
    d->errorView->setPalette(palette);

    QPalette palette2;
    palette2.setColor(d->playerView->backgroundRole(), qApp->palette().color(QPalette::Base));
    d->playerView->setPalette(palette2);
}

void MediaPlayerView::slotEscapePressed()
{
    escapePreview();
    emit signalEscapePreview();
}

int MediaPlayerView::previewMode()
{
    return indexOf(currentWidget());
}

void MediaPlayerView::setPreviewMode(int mode)
{
    if (mode != Private::ErrorView && mode != Private::PlayerView)
    {
        return;
    }

    setCurrentIndex(mode);

    // Workaround for no video frame in the QVideoWidget, possible Qt-5.6.0 bug?
    d->videoWidget->resize(0, 0);
    d->videoWidget->resize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    d->toolBar->adjustSize();
    d->toolBar->raise();
}

void MediaPlayerView::setCurrentItem(const QUrl& url, bool hasPrevious, bool hasNext)
{
    d->prevAction->setEnabled(hasPrevious);
    d->nextAction->setEnabled(hasNext);

    if (url.isEmpty())
    {
        d->currentItem = url;
        d->player->stop();
        return;
    }

    if (d->currentItem == url &&
        (d->player->state() == QMediaPlayer::PlayingState ||
         d->player->state() == QMediaPlayer::PausedState))
    {
        return;
    }

    d->currentItem = url;

    d->player->setMedia(d->currentItem);
    setPreviewMode(Private::PlayerView);
    d->player->play();
}

void MediaPlayerView::positionChanged(qint64 position)
{
    if (!d->slider->isSliderDown())
    {
        d->slider->setValue(position);
    }
}

void MediaPlayerView::durationChanged(qint64 duration)
{
    d->slider->setRange(0, duration);
}

void MediaPlayerView::setPosition(int position)
{
    if (d->player->isSeekable())
    {
        d->player->setPosition((qint64)position);
    }
}

void MediaPlayerView::slotSliderPressed()
{
    if (d->player->state() == QMediaPlayer::PlayingState ||
        d->player->mediaStatus() == QMediaPlayer::EndOfMedia)
    {
        d->player->pause();
    }
}

void MediaPlayerView::slotSliderReleased()
{
    if (d->player->mediaStatus() != QMediaPlayer::EndOfMedia)
    {
        d->player->play();
    }
}

void MediaPlayerView::handlePlayerError()
{
    setPreviewMode(Private::ErrorView);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Error: " << d->player->errorString();
}

}  // namespace Digikam
