/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-20-12
 * Description : a view to embed a KPart media player.
 * 
 * Copyright 2006 Gilles Caulier
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
#include <qpushbutton.h>
#include <qlayout.h>

// KDE includes.

#include <kparts/componentfactory.h>
#include <kmimetype.h>
#include <kuserprofile.h>
#include <kdialogbase.h>
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

    MediaPlayerViewPriv()
    {
        mediaPlayerPart   = 0;
        mediaPlayerWidget = 0;
        grid              = 0;
        buttonsArea       = 0;
    }

    QWidget              *buttonsArea;
    QWidget              *mediaPlayerWidget;

    QGridLayout          *grid;

    KParts::ReadOnlyPart *mediaPlayerPart;
};
    
MediaPlayerView::MediaPlayerView(QWidget *parent)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new MediaPlayerViewPriv;
    d->grid = new QGridLayout(this, 1, 1, KDialogBase::marginHint(), KDialogBase::spacingHint());

    d->buttonsArea          = new QWidget(this);
    QHBoxLayout *hlay       = new QHBoxLayout(d->buttonsArea);
    QPushButton *backButton = new QPushButton(i18n("Back to Album"), d->buttonsArea);

    hlay->setMargin(KDialogBase::marginHint());
    hlay->addStretch(10);
    hlay->addWidget(backButton);
    hlay->addStretch(10);

    d->grid->addMultiCellWidget(d->buttonsArea, 1, 1, 0, 1);
    d->grid->setRowStretch(0, 10),

    connect(backButton, SIGNAL(clicked()),
            this, SLOT(slotBackButtonClicked()) );

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));  
}

MediaPlayerView::~MediaPlayerView()
{
    delete d;
}

void MediaPlayerView::setMediaPlayerFromUrl(const KURL& url)
{
    KMimeType::Ptr mimePtr = KMimeType::findByURL(url, 0, true, true);
    KServiceTypeProfile::OfferList services = KServiceTypeProfile::offers(mimePtr->name(),
                         QString::fromLatin1("KParts/ReadOnlyPart"));    

    if (d->mediaPlayerWidget)
    {
        delete d->mediaPlayerWidget;
        d->mediaPlayerWidget = 0;
    }

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
                             <KParts::ReadOnlyPart>(service, this, 0, this, 0);
        if (!d->mediaPlayerPart) 
        {
            DWarning() << "Failed to instantiate KPart from library " << library << endl;
            continue;
        }
    
        d->mediaPlayerWidget = d->mediaPlayerPart->widget();
        if ( !d->mediaPlayerWidget ) 
        {            
            DWarning() << "Failed to get KPart widget from library " << library << endl;
            continue;
        }
    
        break;
    }

    if (!d->mediaPlayerWidget)
    { 
        d->mediaPlayerWidget = new QLabel(this, "No media player available...");
        d->mediaPlayerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        d->grid->addMultiCellWidget(d->mediaPlayerWidget, 0, 0, 0, 1);
        return;
    }
    
    d->mediaPlayerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->grid->addMultiCellWidget(d->mediaPlayerWidget, 0, 0, 0, 1);
    d->mediaPlayerPart->openURL(url);
}

void MediaPlayerView::slotBackButtonClicked()
{
    if (d->mediaPlayerWidget)
        d->mediaPlayerPart->closeURL();

    emit backToAlbumSignal();
}

void MediaPlayerView::slotThemeChanged()
{
    setPaletteBackgroundColor(ThemeEngine::instance()->baseColor());
    d->buttonsArea->setPaletteBackgroundColor(ThemeEngine::instance()->baseColor());
}

}  // NameSpace Digikam

