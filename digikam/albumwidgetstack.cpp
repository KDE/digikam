/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qfileinfo.h>

// KDE includes.

#include <khtmlview.h>

// Local includes.

#include "albumsettings.h"
#include "albumiconview.h"
#include "imagepreviewview.h"
#include "welcomepageview.h"
#include "mediaplayerview.h"
#include "albumwidgetstack.h"
#include "albumwidgetstack.moc"

namespace Digikam
{

class AlbumWidgetStackPriv
{

public:

    AlbumWidgetStackPriv()
    {
        albumIconView    = 0;
        imagePreviewView = 0;
        welcomePageView  = 0;
        mediaPlayerView  = 0;
    }

    AlbumIconView    *albumIconView;

    ImagePreviewView *imagePreviewView;

    WelcomePageView  *welcomePageView;

    MediaPlayerView  *mediaPlayerView;
};

AlbumWidgetStack::AlbumWidgetStack(QWidget *parent)
                : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new AlbumWidgetStackPriv;

    d->albumIconView    = new AlbumIconView(this);
    d->imagePreviewView = new ImagePreviewView(this);
    d->welcomePageView  = new WelcomePageView(this);
    d->mediaPlayerView  = new MediaPlayerView(this);

    addWidget(d->albumIconView,           PreviewAlbumMode);
    addWidget(d->imagePreviewView,        PreviewImageMode);
    addWidget(d->welcomePageView->view(), WelcomePageMode);
    addWidget(d->mediaPlayerView,         MediaPlayerMode);

    setPreviewMode(PreviewAlbumMode);

    // -----------------------------------------------------------------

    connect(d->mediaPlayerView, SIGNAL( backToAlbumSignal() ),
            this, SIGNAL( backToAlbumSignal() ) );

    connect(d->imagePreviewView, SIGNAL( backToAlbumSignal() ),
            this, SIGNAL( backToAlbumSignal() ) );

    connect(d->imagePreviewView, SIGNAL( editImageSignal() ),
            this, SIGNAL( editImageSignal() ) );
}

AlbumWidgetStack::~AlbumWidgetStack()
{
    delete d;
}

void AlbumWidgetStack::slotEscapePreview()
{
    if (previewMode() == MediaPlayerMode)
        d->mediaPlayerView->slotBackButtonClicked();
}

AlbumIconView* AlbumWidgetStack::albumIconView()
{
    return d->albumIconView;
}

ImagePreviewWidget* AlbumWidgetStack::imagePreviewWidget()
{
    return d->imagePreviewView->imagePreviewWidget();
}

void AlbumWidgetStack::setPreviewItem(const KURL& url)
{
    if (url.isEmpty())
    {
        if (previewMode() == MediaPlayerMode)
            d->mediaPlayerView->setMediaPlayerFromUrl(KURL());
        else if (previewMode() == PreviewImageMode)
            d->imagePreviewView->slotPreviewFailed();
    }    
    else
    {
        AlbumSettings *settings      = AlbumSettings::instance();
        QString currentFileExtension = QFileInfo(url.path()).extension(false);
        QString mediaplayerfilter    = settings->getMovieFileFilter().lower() +
                                       settings->getMovieFileFilter().upper() +
                                       settings->getAudioFileFilter().lower() +
                                       settings->getAudioFileFilter().upper();
        if (mediaplayerfilter.contains(currentFileExtension) )
        {
            setPreviewMode(AlbumWidgetStack::MediaPlayerMode);
            d->mediaPlayerView->setMediaPlayerFromUrl(url);
        }
        else
        {
            // Stop media player if running...
            if (previewMode() == MediaPlayerMode)
                setPreviewItem();

            setPreviewMode(AlbumWidgetStack::PreviewImageMode);
            imagePreviewWidget()->setImagePath(url.path());
        }
    }

    if (visibleWidget())
        visibleWidget()->setFocus();
}

int AlbumWidgetStack::previewMode(void)
{
    return id(visibleWidget());
}

void AlbumWidgetStack::setPreviewMode(int mode)
{
    if (mode != PreviewAlbumMode && mode != PreviewImageMode && 
        mode != WelcomePageMode  && mode != MediaPlayerMode)
        return;

    if (mode == PreviewAlbumMode || mode == WelcomePageMode)
        setPreviewItem();

    raiseWidget(mode);
    visibleWidget()->setFocus();
}

}  // namespace Digikam
