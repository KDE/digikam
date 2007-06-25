/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed a KPart media player.
 * 
 * Copyright (C) 2006-2007 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <q3frame.h>
//Added by qt3to4:
#include <Q3GridLayout>

// KDE includes.

#include <kparts/componentfactory.h>
#include <kmimetype.h>
#include <kuserprofile.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "themeengine.h"
#include "mediaplayerview.h"
#include "mediaplayerview.moc"

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
        mediaPlayerPart   = 0;
        grid              = 0;
        errorView         = 0;
        mediaPlayerView   = 0;
    }

    Q3Frame               *errorView;

    Q3Frame               *mediaPlayerView;

    Q3GridLayout          *grid;

    KParts::ReadOnlyPart *mediaPlayerPart;
};
    
MediaPlayerView::MediaPlayerView(QWidget *parent)
               : Q3WidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new MediaPlayerViewPriv;

    // --------------------------------------------------------------------------

    d->errorView          = new Q3Frame(this);
    QLabel *errorMsg      = new QLabel(i18n("No media player available..."), d->errorView);
    Q3GridLayout *grid     = new Q3GridLayout(d->errorView, 2, 2, 
                                            KDialogBase::marginHint(), KDialogBase::spacingHint());

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(Q3Frame::GroupBoxPanel|Q3Frame::Plain);
    d->errorView->setMargin(0);
    d->errorView->setLineWidth(1);

    grid->addMultiCellWidget(errorMsg, 1, 1, 0, 2);
    grid->setColStretch(0, 10),
    grid->setColStretch(2, 10),
    grid->setRowStretch(0, 10),
    grid->setRowStretch(2, 10),

    addWidget(d->errorView, MediaPlayerViewPriv::ErrorView);

    // --------------------------------------------------------------------------

    d->mediaPlayerView    = new Q3Frame(this);
    d->grid               = new Q3GridLayout(d->mediaPlayerView, 2, 2, 
                                            KDialogBase::marginHint(), KDialogBase::spacingHint());

    d->mediaPlayerView->setFrameStyle(Q3Frame::GroupBoxPanel|Q3Frame::Plain);
    d->mediaPlayerView->setMargin(0);
    d->mediaPlayerView->setLineWidth(1);

    d->grid->setColStretch(0, 10),
    d->grid->setColStretch(2, 10),
    d->grid->setRowStretch(0, 10),

    addWidget(d->mediaPlayerView, MediaPlayerViewPriv::PlayerView);
    setPreviewMode(MediaPlayerViewPriv::PlayerView);

    // --------------------------------------------------------------------------

    connect(ThemeEngine::componentData(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));  
}

MediaPlayerView::~MediaPlayerView()
{
    if (d->mediaPlayerPart)
    {
        d->mediaPlayerPart->closeURL();
        delete d->mediaPlayerPart;
        d->mediaPlayerPart = 0;
    }

    delete d;
}

void MediaPlayerView::setMediaPlayerFromUrl(const KUrl& url)
{
    if (url.isEmpty())
    {
        if (d->mediaPlayerPart)
        {
            d->mediaPlayerPart->closeURL();
            delete d->mediaPlayerPart;
            d->mediaPlayerPart = 0;
        }
        return;
    }

    KMimeType::Ptr mimePtr = KMimeType::findByURL(url, 0, true, true);
    KServiceTypeProfile::OfferList services = KServiceTypeProfile::offers(mimePtr->name(),
                         QString::fromLatin1("KParts/ReadOnlyPart"));    

    if (d->mediaPlayerPart)
    {
        d->mediaPlayerPart->closeURL();
        delete d->mediaPlayerPart;
        d->mediaPlayerPart = 0;
    }

    QWidget *mediaPlayerWidget = 0;

    for( KServiceTypeProfile::OfferList::Iterator it = services.begin(); it != services.end(); ++it ) 
    {
        // Ask for a part for this mime type
        KService::Ptr service = (*it).service();
    
        if (!service.data()) 
        {
            DWarning() << "Couldn't find a KPart for video" << endl;
            continue;
        }
    
        QString library = service->library();
        if ( library.isNull() ) 
        {
            DWarning() << "The library returned from the service was null, "
                       << "indicating we could not display videos." 
                       << endl;
            continue;
        }

        d->mediaPlayerPart = KParts::ComponentFactory::createPartInstanceFromService
                             <KParts::ReadOnlyPart>(service, d->mediaPlayerView, 0, d->mediaPlayerView, 0);
        if (!d->mediaPlayerPart) 
        {
            DWarning() << "Failed to instantiate KPart from library " << library << endl;
            continue;
        }
    
        mediaPlayerWidget = d->mediaPlayerPart->widget(); 
        if ( !mediaPlayerWidget ) 
        {            
            DWarning() << "Failed to get KPart widget from library " << library << endl;
            continue;
        }
    
        break;
    }

    if (!mediaPlayerWidget)
    { 
        setPreviewMode(MediaPlayerViewPriv::ErrorView);
        return;
    }
    
    d->grid->addMultiCellWidget(mediaPlayerWidget, 0, 0, 0, 2);
    mediaPlayerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->mediaPlayerPart->openURL(url);
    setPreviewMode(MediaPlayerViewPriv::PlayerView);
}

void MediaPlayerView::escapePreview()
{
    if (d->mediaPlayerPart)
    {
        d->mediaPlayerPart->closeURL();
        delete d->mediaPlayerPart;
        d->mediaPlayerPart = 0;
    }
}

void MediaPlayerView::slotThemeChanged()
{
    d->errorView->setPaletteBackgroundColor(ThemeEngine::componentData().baseColor());
    d->mediaPlayerView->setPaletteBackgroundColor(ThemeEngine::componentData().baseColor());
}

int MediaPlayerView::previewMode(void)
{
    return id(visibleWidget());
}

void MediaPlayerView::setPreviewMode(int mode)
{
    if (mode != MediaPlayerViewPriv::ErrorView && mode != MediaPlayerViewPriv::PlayerView)
        return;

    raiseWidget(mode);
}

}  // NameSpace Digikam

