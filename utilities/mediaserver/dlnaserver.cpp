/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : a media server to export collections through DLNA.
 *               Implementation inspired on Platinum File Media Server.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dlnaserver.h"

// Qt includes

#include <QImage>
#include <QString>
#include <QApplication>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DLNAMediaServer::DLNAMediaServer(const char*  friendly_name,
                                 bool         show_ip,
                                 const char*  uuid,
                                 NPT_UInt16   port,
                                 bool         port_rebind)
    : PLT_MediaServer(friendly_name,
                      show_ip,
                      uuid,
                      port,
                      port_rebind),
      DLNAMediaServerDelegate("/")
{
    SetDelegate(this);
}

void DLNAMediaServer::addAlbumsOnServer(const MediaServerMap& map)
{
    static_cast<DLNAMediaServerDelegate*>(GetDelegate())->addAlbumsOnServer(map);
}

DLNAMediaServer::~DLNAMediaServer()
{
}

NPT_Result DLNAMediaServer::SetupIcons()
{
    QString path;

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-digikam.png"));
    }
    else
    {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("showfoto/data/logo-showfoto.png"));
    }

    QImage icon;
    QImage img(path);

    if (!img.isNull())
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "Setup Media Server icons";

        icon = img.scaled(256, 256);
        AddIcon(PLT_DeviceIcon("image/png", icon.width(), icon.height(), icon.depth(), "/icon256x256.png"), icon.bits(), icon.byteCount(), true);
        icon = img.scaled(120, 120);
        AddIcon(PLT_DeviceIcon("image/png", icon.width(), icon.height(), icon.depth(), "/icon120x120.png"), icon.bits(), icon.byteCount(), true);
        icon = img.scaled(48, 48);
        AddIcon(PLT_DeviceIcon("image/png", icon.width(), icon.height(), icon.depth(), "/icon48x48.png"),   icon.bits(), icon.byteCount(), true);
        icon = img.scaled(32, 32);
        AddIcon(PLT_DeviceIcon("image/png", icon.width(), icon.height(), icon.depth(), "/icon32x32.png"),   icon.bits(), icon.byteCount(), true);
        icon = img.scaled(16, 16);
        AddIcon(PLT_DeviceIcon("image/png", icon.width(), icon.height(), icon.depth(), "/icon16x16.png"),   icon.bits(), icon.byteCount(), true);

        return NPT_SUCCESS;
    }

    return NPT_FAILURE;
}

} // namespace Digikam
