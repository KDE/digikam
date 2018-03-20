/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media server manager
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DMEDIA_SERVER_MNGR_H
#define DMEDIA_SERVER_MNGR_H

// Qt includes

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dmediaserver.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMediaServerMngr : public QObject
{
    Q_OBJECT

public:

    /// Setup the list of items to share with the DLNA server into a single album.
    void setItemsList(const QString& aname, const QList<QUrl>& urls);

    /// Return a flat list of items shared
    QList<QUrl> itemsList() const;

    /// Setup the list of albums to share with DLNA server.
    void setCollectionMap(const MediaServerMap&);

    /// Return the current album map shared.
    MediaServerMap collectionMap() const;

    /// Start the DLNA server and share the contents. Return true is all is on-line.
    bool startMediaServer();

    /// Stop the DLNA server and clean-up.
    void cleanUp();

    /// Low level methods to save and load from xml data file.
    bool save();
    bool load();

    /// Wrapper to check if server configuration must be saved and restored between application sessions.
    bool loadAtStartup();
    void saveAtShutdown();

    /// Return true if server is running in background.
    bool isRunning()    const;

    /// Return some stats about total albums and total items shared on the network.
    int  albumsShared() const;
    int  itemsShared()  const;

    /// Config properties methods.

    QString configGroupName()                 const;
    QString configStartServerOnStartupEntry() const;

    /// Send a notification message if MediaServer have been started or not.
    void mediaServerNotification(bool started);

public:

    /// This manager is a singleton. Use this method to control the DLNA server instance.
    static DMediaServerMngr* instance();

private:

    explicit DMediaServerMngr();
    ~DMediaServerMngr();

private:

    friend class DMediaServerMngrCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DMEDIA_SERVER_MNGR_H
