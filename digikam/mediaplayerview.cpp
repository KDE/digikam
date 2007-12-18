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

#include <QLabel>
#include <QString>
#include <QFrame>
#include <QGridLayout>

// KDE includes.

#include <kparts/componentfactory.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kservice.h>
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

    QFrame               *errorView;

    QFrame               *mediaPlayerView;

    QGridLayout          *grid;

    KParts::ReadOnlyPart *mediaPlayerPart;
};
    
MediaPlayerView::MediaPlayerView(QWidget *parent)
               : QStackedWidget(parent)
{
    d = new MediaPlayerViewPriv;
    
    setAttribute(Qt::WA_DeleteOnClose);

    // --------------------------------------------------------------------------

    d->errorView      = new QFrame(this);
    QLabel *errorMsg  = new QLabel(i18n("No media player available..."), d->errorView);
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

    d->mediaPlayerView->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->mediaPlayerView->setLineWidth(1);

    d->grid->setColumnStretch(0, 10),
    d->grid->setColumnStretch(2, 10),
    d->grid->setRowStretch(0, 10),
    d->grid->setMargin(KDialog::spacingHint());
    d->grid->setSpacing(KDialog::spacingHint());

    insertWidget(MediaPlayerViewPriv::PlayerView, d->mediaPlayerView);
    setPreviewMode(MediaPlayerViewPriv::PlayerView);

    // --------------------------------------------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));  
}

MediaPlayerView::~MediaPlayerView()
{
    if (d->mediaPlayerPart)
    {
        d->mediaPlayerPart->closeUrl();
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
            d->mediaPlayerPart->closeUrl();
            delete d->mediaPlayerPart;
            d->mediaPlayerPart = 0;
        }
        return;
    }

    KMimeType::Ptr mimePtr = KMimeType::findByUrl(url, 0, true, true);

    const KService::List services = KMimeTypeTrader::self()->query(mimePtr->name(),
                                    QString::fromLatin1("KParts/ReadOnlyPart"));

    DDebug() << "Search a KPart to preview " << url.fileName() << " (" << mimePtr->name() << ") " << endl;

    if (d->mediaPlayerPart)
    {
        d->mediaPlayerPart->closeUrl();
        delete d->mediaPlayerPart;
        d->mediaPlayerPart = 0;
    }

    QWidget *mediaPlayerWidget = 0;

    for( KService::List::ConstIterator it = services.begin() ; it != services.end() ; ++it ) 
    {
        // Ask for a part for this mime type
        KService::Ptr service = *it;

        if (!service.data()) 
        {
            DWarning() << "Couldn't find a KPart for media" << endl;
            continue;
        }

        QString library = service->library();
        if ( library.isNull() ) 
        {
            DWarning() << "The library returned from the service was null, "
                       << "indicating we could not play media." 
                       << endl;
            continue;
        }

        DDebug() << "Find KPart library " << library << endl;
        int error = 0;
        d->mediaPlayerPart = KParts::ComponentFactory::createPartInstanceFromService
                             <KParts::ReadOnlyPart>(service, d->mediaPlayerView, d->mediaPlayerView,
                                                    QStringList(), &error);
        if (!d->mediaPlayerPart) 
        {
            DWarning() << "Failed to instantiate KPart from library " << library 
                       << " error=" << error << endl;
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

    d->grid->addWidget(mediaPlayerWidget, 0, 0, 1, 3 );
    mediaPlayerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->mediaPlayerPart->openUrl(url);
    setPreviewMode(MediaPlayerViewPriv::PlayerView);
}

void MediaPlayerView::escapePreview()
{
    if (d->mediaPlayerPart)
    {
        d->mediaPlayerPart->closeUrl();
        delete d->mediaPlayerPart;
        d->mediaPlayerPart = 0;
    }
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

}  // NameSpace Digikam
