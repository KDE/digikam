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
 
#include <qvbox.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>

// KDE includes.

#include <klocale.h>
#include <kdialogbase.h>
#include <khtmlview.h>

// Local includes.

#include "themeengine.h"
#include "imagepreviewwidget.h"
#include "albumiconview.h"
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
        buttonsArea        = 0;
        previewItemWidget  = 0;
        previewAlbumWidget = 0;
        backButton         = 0;
        editButton         = 0;
        welcomePageView    = 0;
        mediaPlayerView    = 0;
    }

    QPushButton        *backButton;
    QPushButton        *editButton;

    QWidget            *buttonsArea;

    WelcomePageView    *welcomePageView;

    ImagePreviewWidget *previewItemWidget;

    AlbumIconView      *previewAlbumWidget;

    MediaPlayerView    *mediaPlayerView;
};

AlbumWidgetStack::AlbumWidgetStack(QWidget *parent)
                : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new AlbumWidgetStackPriv;

    // -- Album icon view -----------------------------------------------

    d->previewAlbumWidget = new AlbumIconView(this);

    // -- Picture preview -----------------------------------------------

    QVBox *previewArea   = new QVBox(this);
    d->previewItemWidget = new ImagePreviewWidget(previewArea);
    d->buttonsArea       = new QWidget(previewArea);
    QHBoxLayout *hlay    = new QHBoxLayout(d->buttonsArea);
    d->backButton        = new QPushButton(i18n("Back to Album"), d->buttonsArea);
    d->editButton        = new QPushButton(i18n("Edit..."), d->buttonsArea);
    previewArea->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    previewArea->setMargin(0);
    previewArea->setLineWidth(1);

    hlay->setMargin(KDialogBase::marginHint());
    hlay->addStretch(1);
    hlay->addWidget(d->backButton);
    hlay->addStretch(10);
    hlay->addWidget(d->editButton);
    hlay->addStretch(1);

    // -- Welcome page view --------------------------------------------

    d->welcomePageView = new WelcomePageView(this);

    // -- Media player view --------------------------------------------

    d->mediaPlayerView = new MediaPlayerView(this);

    // -- Stack widgets ------------------------------------------------

    addWidget(previewArea,                PreviewItemMode);
    addWidget(d->previewAlbumWidget,      PreviewAlbumMode);
    addWidget(d->welcomePageView->view(), WelcomePageMode);
    addWidget(d->mediaPlayerView,         MediaPlayerMode);

    setPreviewMode(PreviewAlbumMode);

    // -----------------------------------------------------------------

    connect(d->mediaPlayerView, SIGNAL( backToAlbumSignal() ),
            this, SIGNAL( backToAlbumSignal() ) );

    connect(d->backButton, SIGNAL( clicked() ),
            this, SIGNAL( backToAlbumSignal() ) );
             
    connect(d->editButton, SIGNAL( clicked() ),
            this, SIGNAL( editImageSignal() ) );          
             
    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));                

    connect(d->previewItemWidget, SIGNAL( previewStarted() ),
            this, SLOT( slotPreviewStarted() ) );          
    
    connect(d->previewItemWidget, SIGNAL( previewComplete() ),
            this, SLOT( slotPreviewComplete() ) );          
    
    connect(d->previewItemWidget, SIGNAL( previewFailed() ),
            this, SLOT( slotPreviewFailed() ) );    
}

AlbumWidgetStack::~AlbumWidgetStack()
{
    delete d;
}

void AlbumWidgetStack::slotThemeChanged()
{
    d->buttonsArea->setPaletteBackgroundColor(ThemeEngine::instance()->baseColor());
}

void AlbumWidgetStack::slotEscapePreview()
{
    if (previewMode() == MediaPlayerMode)
        d->mediaPlayerView->slotBackButtonClicked();
}

AlbumIconView* AlbumWidgetStack::albumIconView()
{
    return d->previewAlbumWidget;
}

ImagePreviewWidget* AlbumWidgetStack::imagePreviewWidget()
{
    return d->previewItemWidget;
}

void AlbumWidgetStack::setPreviewItem(const KURL& url)
{
    if (url.isEmpty())
        slotPreviewFailed();
    
    visibleWidget()->setFocus();

    if (previewMode() == MediaPlayerMode)
    {
        d->mediaPlayerView->setMediaPlayerFromUrl(url);
    }
    else
    {
        d->previewItemWidget->setImagePath(url.path());
    }
}

int AlbumWidgetStack::previewMode(void)
{
    return id(visibleWidget());
}

void AlbumWidgetStack::setPreviewMode(int mode)
{
    if (mode != PreviewAlbumMode && mode != PreviewItemMode && 
        mode != WelcomePageMode  && mode != MediaPlayerMode)
        return;

    raiseWidget(mode);
    visibleWidget()->setFocus();
}

void AlbumWidgetStack::slotPreviewStarted()
{
    d->backButton->setEnabled(false);
    d->editButton->setEnabled(false);
}

void AlbumWidgetStack::slotPreviewComplete()
{
    d->backButton->setEnabled(true);
    d->editButton->setEnabled(true);
}

void AlbumWidgetStack::slotPreviewFailed()
{
    d->backButton->setEnabled(true);
    d->editButton->setEnabled(false);
}

}  // namespace Digikam
