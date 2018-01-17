/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed QtAv media player.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QApplication>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QLabel>
#include <QFrame>
#include <QEvent>
#include <QStyle>

// QtAV includes

#include <QtAVWidgets/WidgetRenderer.h>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "thememanager.h"

using namespace QtAV;

namespace Digikam
{

class MediaPlayerMouseClickFilter : public QObject
{
public:

    explicit MediaPlayerMouseClickFilter(QObject* const parent)
        : QObject(parent),
          m_parent(parent)
    {
    }

protected:

    bool eventFilter(QObject* obj, QEvent* event)
    {
        if ((qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick)  && event->type() == QEvent::MouseButtonRelease)   ||
            (!qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick) && event->type() == QEvent::MouseButtonDblClick))
        {
            QMouseEvent* const mouseEvent = dynamic_cast<QMouseEvent*>(event);

            if (mouseEvent && (mouseEvent->button() == Qt::LeftButton ||
                               mouseEvent->button() == Qt::RightButton))
            {
                if (m_parent)
                {
                    MediaPlayerView* const mplayer = dynamic_cast<MediaPlayerView*>(m_parent);

                    if (mplayer)
                    {
                        if (mouseEvent->button() == Qt::LeftButton)
                        {
                            mplayer->slotEscapePressed();
                        }
                        else
                        {
                            mplayer->slotRotateVideo();
                        }

                        return true;
                    }
                }
            }
        }

        return QObject::eventFilter(obj, event);
    }

private:

    QObject* m_parent;
};

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
        playAction(0),
        toolBar(0),
        videoWidget(0),
        player(0),
        slider(0),
        tlabel(0)
    {
    }

    QFrame*              errorView;
    QFrame*              playerView;

    QAction*             prevAction;
    QAction*             nextAction;
    QAction*             playAction;

    QToolBar*            toolBar;

    WidgetRenderer*      videoWidget;
    AVPlayer*            player;
    QSlider*             slider;
    QLabel*              tlabel;
    QUrl                 currentItem;
};

MediaPlayerView::MediaPlayerView(QWidget* const parent)
    : QStackedWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QMargins margins(spacing, 0, spacing, spacing);

    d->prevAction          = new QAction(QIcon::fromTheme(QLatin1String("go-previous")),          i18nc("go to previous image", "Back"),   this);
    d->nextAction          = new QAction(QIcon::fromTheme(QLatin1String("go-next")),              i18nc("go to next image", "Forward"),    this);
    d->playAction          = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18nc("pause/play video", "Pause/Play"), this);

    d->errorView           = new QFrame(this);
    QLabel* const errorMsg = new QLabel(i18n("An error has occurred with the media player..."), this);

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    d->errorView->setLineWidth(1);

    QVBoxLayout* const vbox1 = new QVBoxLayout(d->errorView);
    vbox1->addWidget(errorMsg, 10);
    vbox1->setContentsMargins(margins);
    vbox1->setSpacing(spacing);

    insertWidget(Private::ErrorView, d->errorView);

    // --------------------------------------------------------------------------

    d->playerView  = new QFrame(this);
    d->videoWidget = new WidgetRenderer(this);
    d->player      = new AVPlayer(this);

    DHBox* const hbox = new DHBox(this);
    d->slider         = new QSlider(Qt::Horizontal, hbox);
    d->slider->setRange(0, 0);
    d->tlabel         = new QLabel(hbox);
    d->tlabel->setText(QString::fromLatin1("00:00:00 / 00:00:00"));
    hbox->setStretchFactor(d->slider, 10);

    d->videoWidget->setOutAspectRatioMode(VideoRenderer::VideoAspectRatio);
    d->player->setRenderer(d->videoWidget);

    d->playerView->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    d->playerView->setLineWidth(1);

    QVBoxLayout* const vbox2 = new QVBoxLayout(d->playerView);
    vbox2->addWidget(d->videoWidget, 10);
    vbox2->addWidget(hbox,           0);
    vbox2->setContentsMargins(margins);
    vbox2->setSpacing(spacing);

    insertWidget(Private::PlayerView, d->playerView);

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->addAction(d->playAction);

    setPreviewMode(Private::PlayerView);

    d->errorView->installEventFilter(new MediaPlayerMouseClickFilter(this));
    d->videoWidget->installEventFilter(new MediaPlayerMouseClickFilter(this));

    // --------------------------------------------------------------------------

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(signalPrevItem()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(signalNextItem()));

    connect(d->playAction, SIGNAL(triggered()),
            this, SLOT(slotPausePlay()));

    connect(d->slider, SIGNAL(sliderPressed()),
            this, SLOT(slotPausePlay()));

    connect(d->slider, SIGNAL(sliderReleased()),
            this, SLOT(slotPausePlay()));

    connect(d->slider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotPosition(int)));

    connect(d->player, SIGNAL(stateChanged(QtAV::AVPlayer::State)),
            this, SLOT(slotPlayerStateChanged(QtAV::AVPlayer::State)));

    connect(d->player, SIGNAL(mediaStatusChanged(QtAV::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QtAV::MediaStatus)));

    connect(d->player, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotPositionChanged(qint64)));

    connect(d->player, SIGNAL(durationChanged(qint64)),
            this, SLOT(slotDurationChanged(qint64)));

    connect(d->player, SIGNAL(error(QtAV::AVError)),
            this, SLOT(slotHandlePlayerError(QtAV::AVError)));

    qCDebug(DIGIKAM_GENERAL_LOG) << "AudioOutput backends:" << d->player->audio()->backendsAvailable();
}

MediaPlayerView::~MediaPlayerView()
{
    d->player->stop();
    delete d;
}

void MediaPlayerView::reload()
{
    d->player->stop();
    d->player->setFile(d->currentItem.toLocalFile());
    d->player->play();
}

void MediaPlayerView::slotPlayerStateChanged(QtAV::AVPlayer::State state)
{
    if (state == QtAV::AVPlayer::PlayingState)
    {
        d->playAction->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
    }
    else if (state == QtAV::AVPlayer::PausedState ||
             state == QtAV::AVPlayer::StoppedState)
    {
        d->playAction->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
    }
}

void MediaPlayerView::slotMediaStatusChanged(QtAV::MediaStatus status)
{
    if (status == QtAV::InvalidMedia)
    {
        setPreviewMode(Private::ErrorView);
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

void MediaPlayerView::slotRotateVideo()
{
    if (d->player->isPlaying())
    {
        int orientation = 0;

        switch (d->videoWidget->orientation())
        {
            case 0:
                orientation = 90;
                break;
            case 90:
                orientation = 180;
                break;
            case 180:
                orientation = 270;
                break;
            default:
                orientation = 0;
        }

        d->videoWidget->setOrientation(orientation);
    }
}

void MediaPlayerView::slotPausePlay()
{
    if (!d->player->isPlaying())
    {
        d->player->play();
        return;
    }

    d->player->pause(!d->player->isPaused());
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

    if (d->currentItem == url)
    {
        return;
    }

    d->currentItem = url;

    d->player->stop();
    setPreviewMode(Private::PlayerView);
    d->player->setFile(d->currentItem.toLocalFile());
    d->player->play();
}

void MediaPlayerView::slotPositionChanged(qint64 position)
{
    if (!d->slider->isSliderDown())
    {
        d->slider->setValue(position);
    }

    d->tlabel->setText(QString::fromLatin1("%1 / %2")
                       .arg(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")))
                       .arg(QTime(0, 0, 0).addMSecs(d->slider->maximum()).toString(QString::fromLatin1("HH:mm:ss"))));
}

void MediaPlayerView::slotDurationChanged(qint64 duration)
{
    qint64 max = qMax((qint64)1, duration);
    d->slider->setRange(0, max);
}

void MediaPlayerView::slotPosition(int position)
{
    if (d->player->isSeekable())
    {
        d->player->setPosition((qint64)position);
    }
}

void MediaPlayerView::slotHandlePlayerError(const QtAV::AVError& err)
{
    setPreviewMode(Private::ErrorView);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Error: " << err.string();
}

} // namespace Digikam
