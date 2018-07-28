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

#ifndef DIGIKAM_WS_AUTHENTICACTION_H
#define DIGIKAM_WS_AUTHENTICACTION_H

// Qt includes

#include <QtGlobal>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <QMap>
#include <QSettings>
#include <QStringList>
#include <QList>

// Local includes

#include "dinfointerface.h"
#include "wssettings.h"
#include "wsitem.h"

namespace Digikam
{
    
class WSAuthentication : public QObject
{
    Q_OBJECT
    
public:

    explicit WSAuthentication(QWidget* const parent, DInfoInterface* const iface=0);
    ~WSAuthentication();
    
    void createTalker(WSSettings::WebService ws,
                      const QString& serviceName=QString());
    
    void authenticate();
    void reauthenticate();
    bool authenticated() const;
    
    void listAlbums();
    
private:
    
    void parseTreeFromListAlbums(const QList <WSAlbum>& albumsList);

Q_SIGNALS:
    
    void signalOpenBrowser(const QUrl&);
    void signalCloseBrowser();
    void signalResponseTokenReceived(const QMap<QString, QString>&);
    void signalAuthenticationComplete(bool);
    
    void signalListAlbumsDone(const QMap<QString, AlbumSimplified>&, const QStringList&);
    
private Q_SLOTS:
    
    void slotListAlbumsDone(int errCode, const QString& errMsg, const QList<WSAlbum>& albumsList);
    
private:
    
    class Private;
    Private* const d;
    
};
    
} // namespace Digikam

#endif // DIGIKAM_WS_AUTHENTICACTION_H  


