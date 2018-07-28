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

// Qt includes
#include <QApplication>
#include <QMap>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

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
    
    QWidget*                        parent;
    DInfoInterface*                 iface;
    
    WSTalker*                       talker;
    
    WSSettings::WebService          ws;
    QString                         serviceName;
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
    d->serviceName = serviceName;
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "create " << serviceName << "talker";
    
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
    
    connect(d->talker, SIGNAL(signalListAlbumsDone(int, const QString&, const QList <WSAlbum>&)),
            this, SLOT(slotListAlbumsDone(int, const QString&, const QList <WSAlbum>&)));
}

void WSAuthentication::authenticate()
{
    d->talker->authenticate();
}

void WSAuthentication::reauthenticate()
{
    d->talker->reauthenticate();
}

bool WSAuthentication::authenticated() const
{
    return d->talker->linked();
}

void WSAuthentication::parseTreeFromListAlbums(const QList<WSAlbum>& albumsList)
{
    QMap<QString, AlbumSimplified>  albumTree;
    QStringList                     rootAlbums;
    
    foreach(const WSAlbum& album, albumsList)
    {
        if(albumTree.contains(album.id))
        {
            albumTree[album.id].title = album.title;
        }
        else
        {
            AlbumSimplified item(album.title);
            albumTree[album.id] = item;
        }
        
        if(album.isRoot)
        {
            rootAlbums << album.id;
        }
        else
        {
            if(albumTree.contains(album.parentID))
            {
                albumTree[album.parentID].childrenIDs << album.id;
            }
            else
            {
                AlbumSimplified parentAlbum;
                parentAlbum.childrenIDs << album.id;
                albumTree[album.parentID] = parentAlbum;
            }
        }
    }
    
    emit signalListAlbumsDone(albumTree, rootAlbums);
}

void WSAuthentication::listAlbums()
{
    d->talker->listAlbums();
}

void WSAuthentication::slotListAlbumsDone(int errCode, const QString& errMsg, const QList<WSAlbum>& albumsList)
{
    QString albumDebug = QString::fromLatin1("");
    foreach(const WSAlbum &album, albumsList)
    {
        albumDebug.append(QString::fromLatin1("%1: %2\n").arg(album.id).arg(album.title));
    }
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received albums (errCode = " << errCode << ", errMsg = " 
                                     << errMsg << "): " << albumDebug;
    
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), 
                              i18n(QString::fromLatin1("%1 Call Failed: %2\n").arg(d->serviceName).arg(errCode).toLatin1()),
                              errMsg);
        return;
    }
    
    parseTreeFromListAlbums(albumsList);
}

} // namespace Digikam




