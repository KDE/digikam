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

#include <QApplication>
#include <QStandardPaths>
#include <QBuffer>
#include <QIODevice>

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

    QByteArray icon;
    QImage     img(path);
    QString    uri;
    int        depth;
    int        size;

    if (!img.isNull())
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "Setup Media Server icons";

        size = 256;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 120;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 48;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 32;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 16;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);

        return NPT_SUCCESS;
    }

    return NPT_FAILURE;
}

QByteArray DLNAMediaServer::iconData(const QImage& img, int size, QString& uri, int& depth) const
{
    QByteArray ba;
    QBuffer    buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    QImage icon = img.scaled(size, size);
    icon.save(&buffer, "PNG");
    buffer.close();

    uri         = QString::fromLatin1("/icon%1x%2.png").arg(size).arg(size);
    depth       = icon.depth();

    return ba;
}

} // namespace Digikam
