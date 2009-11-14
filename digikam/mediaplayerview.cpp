/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed Phonon media player.
 *
 * Copyright (C) 2006-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

// #include "mediaplayerview.h"
#include "mediaplayerview.moc"

// Qt includes

#include <QLabel>
#include <QString>
#include <QFrame>
#include <QGridLayout>

// KDE includes


#include <kdialog.h>
#include <klocale.h>
#include <phonon/seekslider.h>
#include <phonon/videoplayer.h>
#include <phonon/videowidget.h>

// Local includes

#include "themeengine.h"

namespace Digikam
{

class MediaPlayerViewPriv
{

public:

    enum MediaPlayerViewMode
    {
        ErrorView=0,
        PlayerView
    };

public:

    MediaPlayerViewPriv()
    {
        player          = 0;
        grid            = 0;
        errorView       = 0;
        mediaPlayerView = 0;
        slider          = 0;
    }

    QFrame*              errorView;
    QFrame*              mediaPlayerView;

    QGridLayout*         grid;

    Phonon::VideoPlayer* player;
    Phonon::SeekSlider*  slider;
};

MediaPlayerView::MediaPlayerView(QWidget *parent)
               : QStackedWidget(parent), d(new MediaPlayerViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->errorView      = new QFrame(this);
    QLabel *errorMsg  = new QLabel(i18n("An error has occurred with the media player...."), d->errorView);
    QGridLayout *grid = new QGridLayout(d->errorView);

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->errorView->setLineWidth(1);

    grid->addWidget(errorMsg, 1, 0, 1, 3 );
    grid->setColumnStretch(0, 10),
    grid->setColumnStretch(2, 10),
    grid->setRowStretch(0, 10),
    grid->setRowStretch(2, 10),
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    insertWidget(MediaPlayerViewPriv::ErrorView, d->errorView);

    // --------------------------------------------------------------------------

    d->mediaPlayerView = new QFrame(this);
    d->grid            = new QGridLayout(d->mediaPlayerView);
    d->player          = new Phonon::VideoPlayer(Phonon::VideoCategory, d->mediaPlayerView);
    d->slider          = new Phonon::SeekSlider(d->mediaPlayerView);
    d->slider->setMediaObject(d->player->mediaObject());
    d->player->mediaObject()->setTickInterval(100);
    d->player->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->mediaPlayerView->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->mediaPlayerView->setLineWidth(1);

    d->grid->addWidget(d->player->videoWidget(),    0, 0, 1, 3);
    d->grid->addWidget(d->slider,                   1, 0, 1, 3);
    d->grid->setColumnStretch(0, 10),
    d->grid->setColumnStretch(2, 10),
    d->grid->setRowStretch(0, 10),
    d->grid->setMargin(KDialog::spacingHint());
    d->grid->setSpacing(KDialog::spacingHint());

    insertWidget(MediaPlayerViewPriv::PlayerView, d->mediaPlayerView);
    setPreviewMode(MediaPlayerViewPriv::PlayerView);

    // --------------------------------------------------------------------------

    connect(d->player->mediaObject(), SIGNAL(finished()),
            this, SLOT(slotPlayerFinished()));

    connect(d->player->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(slotPlayerstateChanged(Phonon::State, Phonon::State)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

MediaPlayerView::~MediaPlayerView()
{
    d->player->stop();
    delete d->player;
    delete d;
}

void MediaPlayerView::setMediaPlayerFromUrl(const KUrl& url)
{
    if (url.isEmpty())
    {
        d->player->stop();
        return;
    }

    d->player->play(url);
    setPreviewMode(MediaPlayerViewPriv::PlayerView);
}

void MediaPlayerView::slotPlayerFinished()
{
    if (d->player->mediaObject()->errorType() == Phonon::FatalError)
        setPreviewMode(MediaPlayerViewPriv::ErrorView);
}

void MediaPlayerView::slotPlayerstateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    if (newState == Phonon::ErrorState)
        setPreviewMode(MediaPlayerViewPriv::ErrorView);
}

void MediaPlayerView::escapePreview()
{
    d->player->stop();
}

void MediaPlayerView::slotThemeChanged()
{
    QPalette palette;
    palette.setColor(d->errorView->backgroundRole(), ThemeEngine::instance()->baseColor());
    d->errorView->setPalette(palette);

    QPalette palette2;
    palette2.setColor(d->mediaPlayerView->backgroundRole(), ThemeEngine::instance()->baseColor());
    d->mediaPlayerView->setPalette(palette2);
}

int MediaPlayerView::previewMode()
{
    return indexOf(currentWidget());
}

void MediaPlayerView::setPreviewMode(int mode)
{
    if (mode != MediaPlayerViewPriv::ErrorView && mode != MediaPlayerViewPriv::PlayerView)
        return;

    setCurrentIndex(mode);
}

}  // namespace Digikam
