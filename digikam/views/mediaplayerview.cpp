/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed Phonon media player.
 *
 * Copyright (C) 2006-2011 Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <phonon/seekslider.h>
#include <phonon/videoplayer.h>
#include <phonon/videowidget.h>

// Local includes

#include "stackedview.h"
#include "thememanager.h"

namespace Digikam
{

MediaPlayerMouseClickFilter::MediaPlayerMouseClickFilter(QObject* parent)
    : QObject(parent), m_parent(parent)
{
}

bool MediaPlayerMouseClickFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (mouseEvent && mouseEvent->button() == Qt::LeftButton)
        {
            if (m_parent)
            {
                MediaPlayerView* mplayer = dynamic_cast<MediaPlayerView*>(m_parent);

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

class MediaPlayerView::MediaPlayerViewPriv
{

public:

    enum MediaPlayerViewMode
    {
        ErrorView=0,
        PlayerView
    };

public:

    MediaPlayerViewPriv() :
        errorView(0),
        mediaPlayerView(0),
        back2AlbumAction(0),
        prevAction(0),
        nextAction(0),
        toolBar(0),
        grid(0),
        player(0),
        slider(0)
    {
    }

    QFrame*              errorView;
    QFrame*              mediaPlayerView;

    QAction*             back2AlbumAction;
    QAction*             prevAction;
    QAction*             nextAction;

    QToolBar*            toolBar;

    QGridLayout*         grid;

    ImageInfo            currentInfo;

    Phonon::VideoPlayer* player;
    Phonon::SeekSlider*  slider;
};

MediaPlayerView::MediaPlayerView(StackedView* parent)
    : QStackedWidget(parent), d(new MediaPlayerViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->back2AlbumAction = new QAction(SmallIcon("folder-image"), i18n("Back to Album"),                 this);
    d->prevAction       = new QAction(SmallIcon("go-previous"),  i18nc("go to previous image", "Back"), this);
    d->nextAction       = new QAction(SmallIcon("go-next"),      i18nc("go to next image", "Forward"),  this);

    d->errorView        = new QFrame(this);
    QLabel* errorMsg    = new QLabel(i18n("An error has occurred with the media player...."), this);

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->errorView->setLineWidth(1);

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(errorMsg, 1, 0, 1, 3 );
    grid->setColumnStretch(0, 10),
         grid->setColumnStretch(2, 10),
         grid->setRowStretch(0, 10),
         grid->setRowStretch(2, 10),
         grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    d->errorView->setLayout(grid);

    insertWidget(MediaPlayerViewPriv::ErrorView, d->errorView);

    // --------------------------------------------------------------------------

    d->mediaPlayerView = new QFrame(this);
    d->player          = new Phonon::VideoPlayer(Phonon::VideoCategory, this);
    d->slider          = new Phonon::SeekSlider(this);
    d->slider->setMediaObject(d->player->mediaObject());
    d->player->mediaObject()->setTickInterval(100);
    d->player->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->mediaPlayerView->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->mediaPlayerView->setLineWidth(1);

    d->grid = new QGridLayout;
    d->grid->addWidget(d->player->videoWidget(), 0, 0, 1, 3);
    d->grid->addWidget(d->slider,                1, 0, 1, 3);
    d->grid->setColumnStretch(0, 10),
      d->grid->setColumnStretch(2, 10),
      d->grid->setRowStretch(0, 10),
      d->grid->setMargin(KDialog::spacingHint());
    d->grid->setSpacing(KDialog::spacingHint());
    d->mediaPlayerView->setLayout(d->grid);

    insertWidget(MediaPlayerViewPriv::PlayerView, d->mediaPlayerView);

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->addAction(d->back2AlbumAction);

    setPreviewMode(MediaPlayerViewPriv::PlayerView);

    d->errorView->installEventFilter(new MediaPlayerMouseClickFilter(this));
    d->player->videoWidget()->installEventFilter(new MediaPlayerMouseClickFilter(this));

    // --------------------------------------------------------------------------

    connect(d->player->mediaObject(), SIGNAL(finished()),
            this, SLOT(slotPlayerFinished()));

    connect(d->player->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(slotPlayerstateChanged(Phonon::State, Phonon::State)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(signalPrevItem()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(signalNextItem()));

    connect(d->back2AlbumAction, SIGNAL(triggered()),
            parent, SIGNAL(signalBack2Album()));
}

MediaPlayerView::~MediaPlayerView()
{
    d->player->stop();
    delete d->player;
    delete d;
}

void MediaPlayerView::setImageInfo(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
    d->prevAction->setEnabled(!previous.isNull());
    d->nextAction->setEnabled(!next.isNull());

    KUrl url = info.fileUrl();

    if (info.isNull() || url.isEmpty())
    {
        d->currentInfo = info;
        d->player->stop();
        return;
    }

    if (d->currentInfo == info &&
        (d->player->isPlaying() || d->player->isPaused()))
    {
        return;
    }

    d->currentInfo = info;

    d->player->play(url);
    setPreviewMode(MediaPlayerViewPriv::PlayerView);
}

void MediaPlayerView::slotPlayerFinished()
{
    if (d->player->mediaObject()->errorType() == Phonon::FatalError)
    {
        setPreviewMode(MediaPlayerViewPriv::ErrorView);
    }
}

void MediaPlayerView::slotPlayerstateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    if (newState == Phonon::ErrorState)
    {
        setPreviewMode(MediaPlayerViewPriv::ErrorView);
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
    palette2.setColor(d->mediaPlayerView->backgroundRole(), kapp->palette().color(QPalette::Base));
    d->mediaPlayerView->setPalette(palette2);
}

void MediaPlayerView::slotEscapePressed()
{
    escapePreview();
    emit signalBack2Album();
}

int MediaPlayerView::previewMode()
{
    return indexOf(currentWidget());
}

void MediaPlayerView::setPreviewMode(int mode)
{
    if (mode != MediaPlayerViewPriv::ErrorView && mode != MediaPlayerViewPriv::PlayerView)
    {
        return;
    }

    setCurrentIndex(mode);
    d->toolBar->raise();
}

}  // namespace Digikam
