/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-03
 * Description : Web Service authentication container.
 *
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */ 

#include "wsauthentication.h"

// Local includes

#include "digikam_debug.h"
#include "wstalker.h"
#include "dbtalker.h"
#include "fbtalker.h"
#include "flickrtalker.h"
#include "gptalker.h"
#include "gdtalker.h"
#include "imgurtalker.h"
#include "smugtalker.h"

namespace Digikam
{

class WSAuthentication::Private
{
public:
    
    explicit Private()
      : parent(0),
        iface(0),
        talker(0)
    {
    }
    
    QWidget*                    parent;
    DInfoInterface*             iface;
    
    WSTalker*                   talker;
    
    WSSettings::WebService      ws;
};
    
WSAuthentication::WSAuthentication(QWidget* const parent, DInfoInterface* const iface)
  : d(new Private())
{
    d->parent       = parent;
    d->iface        = iface;
}

WSAuthentication::~WSAuthentication()
{
    delete d;
}

void WSAuthentication::createTalker(WSSettings::WebService ws,
                                    const QString& serviceName)
{
    d->ws = ws;
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "createTalker";
    
    switch(ws)
    {
        case WSSettings::WebService::FLICKR:
            //d->talker = new FlickrTalker(d->parent, serviceName, d->iface);
            break;
        case WSSettings::WebService::DROPBOX:
            //d->talker = new DBTalker(d->parent);
            break;
        case WSSettings::WebService::IMGUR:
            //d->talker = new ImgurTalker(d->parent);
            break;
        case WSSettings::WebService::FACEBOOK:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "create FbTalker";
            d->talker = new FbTalker(d->parent);
            break;
        case WSSettings::WebService::SMUGMUG:
            //d->talker = new SmugTalker(d->iface, d->parent);
            break;
        case WSSettings::WebService::GDRIVE:
            //d->talker = new GDTalker(d->parent);
            break;
        case WSSettings::WebService::GPHOTO:
            //d->talker = new GPTalker(d->parent);
            break;
    }
    
    connect(this, SIGNAL(signalResponseTokenReceived(const QMap<QString, QString>&)),
            d->talker, SLOT(slotResponseTokenReceived(const QMap<QString, QString>&)));
    connect(d->talker, SIGNAL(signalOpenBrowser(const QUrl&)),
            this, SIGNAL(signalOpenBrowser(const QUrl&)));
    connect(d->talker, SIGNAL(signalCloseBrowser()),
            this, SIGNAL(signalCloseBrowser()));
    connect(d->talker, SIGNAL(signalAuthenticationComplete(bool)),
            this, SIGNAL(signalAuthenticationComplete(bool)));
}

void WSAuthentication::authenticate()
{
    d->talker->authenticate();
}

bool WSAuthentication::authenticated() const
{
    return d->talker->linked();
}

} // namespace Digikam




