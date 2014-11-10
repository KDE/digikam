/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed Phonon media player.
 *
 * Copyright (C) 2006-2014 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mediaplayerview.moc"

// Qt includes

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QFrame>
#include <QGridLayout>
#include <QToolBar>
#include <QEvent>
#include <QMouseEvent>

// KDE includes

#include <kapplication.h>
#include <kaction.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <phonon/seekslider.h>
#include <phonon/videoplayer.h>
#include <phonon/videowidget.h>

// Local includes

#include "stackedview.h"
#include "thememanager.h"

namespace Digikam
{

MediaPlayerMouseClickFilter::MediaPlayerMouseClickFilter(QObject* const parent)
    : QObject(parent), m_parent(parent)
{
}

bool MediaPlayerMouseClickFilter::eventFilter(QObject* obj, QEvent* event)
{
    if ((KGlobalSettings::singleClick()  && event->type() == QEvent::MouseButtonRelease) ||
        (!KGlobalSettings::singleClick() && event->type() == QEvent::MouseButtonDblClick))
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
        grid(0),
        player(0),
        slider(0)
    {
    }

    QFrame*              errorView;
    QFrame*              playerView;

    QAction*             prevAction;
    QAction*             nextAction;

    QToolBar*            toolBar;

    QGridLayout*         grid;

    Phonon::VideoPlayer* player;
    Phonon::SeekSlider*  slider;

    KUrl                 currentItem;
};

MediaPlayerView::MediaPlayerView(QWidget* const parent)
    : QStackedWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->prevAction          = new QAction(SmallIcon("go-previous"),  i18nc("go to previous image", "Back"), this);
    d->nextAction          = new QAction(SmallIcon("go-next"),      i18nc("go to next image", "Forward"),  this);

    d->errorView           = new QFrame(this);
    QLabel* const errorMsg = new QLabel(i18n("An error has occurred with the media player...."), this);

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
    d->errorView->setLineWidth(1);

    QGridLayout* const grid = new QGridLayout;
    grid->addWidget(errorMsg, 1, 0, 1, 3 );
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(0, 10);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    d->errorView->setLayout(grid);

    insertWidget(Private::ErrorView, d->errorView);

    // --------------------------------------------------------------------------

    d->playerView = new QFrame(this);
    d->player     = new Phonon::VideoPlayer(Phonon::VideoCategory, this);
    d->slider     = new Phonon::SeekSlider(this);
    d->slider->setMediaObject(d->player->mediaObject());
    d->player->mediaObject()->setTickInterval(100);
    d->player->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->playerView->setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
    d->playerView->setLineWidth(1);

    d->grid = new QGridLayout;
    d->grid->addWidget(d->player->videoWidget(), 0, 0, 1, 3);
    d->grid->addWidget(d->slider,                1, 0, 1, 3);
    d->grid->setColumnStretch(0, 10);
    d->grid->setColumnStretch(2, 10);
    d->grid->setRowStretch(0, 10);
    d->grid->setMargin(KDialog::spacingHint());
    d->grid->setSpacing(KDialog::spacingHint());
    d->playerView->setLayout(d->grid);

    insertWidget(Private::PlayerView, d->playerView);

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);

    setPreviewMode(Private::PlayerView);

    d->errorView->installEventFilter(new MediaPlayerMouseClickFilter(this));
    d->player->videoWidget()->installEventFilter(new MediaPlayerMouseClickFilter(this));

    // --------------------------------------------------------------------------

    connect(d->player->mediaObject(), SIGNAL(finished()),
            this, SLOT(slotPlayerFinished()));

    connect(d->player->mediaObject(), SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(slotPlayerstateChanged(Phonon::State,Phonon::State)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(signalPrevItem()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(signalNextItem()));
}

MediaPlayerView::~MediaPlayerView()
{
    d->player->stop();
    delete d->player;
    delete d;
}

void MediaPlayerView::reload()
{
    d->player->stop();
    d->player->play(d->currentItem);
}

void MediaPlayerView::slotPlayerFinished()
{
    if (d->player->mediaObject()->errorType() == Phonon::FatalError)
    {
        setPreviewMode(Private::ErrorView);
    }
}

void MediaPlayerView::slotPlayerstateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    if (newState == Phonon::ErrorState)
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
    palette.setColor(d->errorView->backgroundRole(), kapp->palette().color(QPalette::Base));
    d->errorView->setPalette(palette);

    QPalette palette2;
    palette2.setColor(d->playerView->backgroundRole(), kapp->palette().color(QPalette::Base));
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
    d->toolBar->raise();
}

void MediaPlayerView::setCurrentItem(const KUrl& url, bool hasPrevious, bool hasNext)
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
        (d->player->isPlaying() || d->player->isPaused()))
    {
        return;
    }

    d->currentItem = url;

    d->player->play(d->currentItem);
    setPreviewMode(Private::PlayerView);
}

}  // namespace Digikam
