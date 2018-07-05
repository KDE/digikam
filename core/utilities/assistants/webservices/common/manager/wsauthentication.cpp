/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-03
 * Description : Web Service settings container.
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

namespace Digikam
{

class WSAuthentication::Private
{
public:
    
    explicit Private()
      : parent(0),
        iface(0),
        dbtalker(0),
        fbtalker(0),
        fltalker(0),
        gptalker(0),
        gdtalker(0),
        igtalker(0),
        smtalker(0)
    {
    }
    
    QWidget*                    parent;
    DInfoInterface*             iface;
    
    DBTalker*                   dbtalker;
    FbTalker*                   fbtalker;
    FlickrTalker*               fltalker;
    GPTalker*                   gptalker;
    GDTalker*                   gdtalker;
    ImgurTalker*                igtalker;
    SmugTalker*                 smtalker;
    
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

void WSAuthentication::createTalker(WSSettings::WebService ws, const QString& serviceName)
{
    d->ws = ws;
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "createTalker";
    
    switch(ws)
    {
        case WSSettings::WebService::FLICKR:
            d->fltalker = new FlickrTalker(d->parent, serviceName, d->iface);
            break;
        case WSSettings::WebService::DROPBOX:
            d->dbtalker = new DBTalker(d->parent);
            break;
        case WSSettings::WebService::IMGUR:
            d->igtalker = new ImgurTalker(d->parent);
            break;
        case WSSettings::WebService::FACEBOOK:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "create FbTalker";
            d->fbtalker = new FbTalker(d->parent);
            connect(this, SIGNAL(signalResponseTokenReceived(const QMap<QString, QString>&)),
                    d->fbtalker, SLOT(slotResponseTokenReceived(const QMap<QString, QString>&)));
            connect(d->fbtalker, SIGNAL(signalOpenBrowser(const QUrl&)),
                    this, SIGNAL(signalOpenBrowser(const QUrl&)));
            connect(d->fbtalker, SIGNAL(signalCloseBrowser()),
                    this, SIGNAL(signalCloseBrowser()));
            break;
        case WSSettings::WebService::SMUGMUG:
            d->smtalker = new SmugTalker(d->iface, d->parent);
            break;
        case WSSettings::WebService::GDRIVE:
            d->gdtalker = new GDTalker(d->parent);
            break;
    }
}

void WSAuthentication::reauthenticate()
{
    switch(d->ws)
    {
        case WSSettings::WebService::FLICKR:
            //d->fltalker->reauthenticate();
            break;
        case WSSettings::WebService::DROPBOX:
            //d->dbtalker->reauthenticate();
            break;
        case WSSettings::WebService::IMGUR:
            //d->igtalker->reauthenticate();
            break;
        case WSSettings::WebService::FACEBOOK:
            d->fbtalker->reauthenticate();
            break;
        case WSSettings::WebService::SMUGMUG:
            //d->smtalker->reauthenticate();
            break;
        case WSSettings::WebService::GDRIVE:
            //d->gdtalker->reauthenticate();
            break;
    }
}



} // namespace Digikam




