/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : a media server to export collections through DLNA.
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

#include "dmediaserver.h"

// Qt includes

#include <QApplication>
#include <QList>
#include <QUrl>
#include <QFile>
#include <QStandardPaths>

// Platinum includes

#include "PltDeviceHost.h"
#include "Platinum.h"

// Local includes

#include "dlnaserver.h"
#include "digikam_debug.h"
#include "digikam_version.h"
#include "daboutdata.h"

void NPT_Console::Output(const char* msg)
{
    qCDebug(DIGIKAM_MEDIASRV_LOG) << msg;
}

void UPnPLogger(const NPT_LogRecord* record)
{
    QString msg = QLatin1String("Platinum [")             +
                  QString::fromUtf8(record->m_LoggerName) +
                  QLatin1String("] : ")                   +
                  QString::fromUtf8(record->m_Message);

    switch (record->m_Level)
    {
        case NPT_LOG_LEVEL_FATAL:
            qCDebug(DIGIKAM_MEDIASRV_LOG_FATAL) << msg;
            break;
        case NPT_LOG_LEVEL_SEVERE:
            qCDebug(DIGIKAM_MEDIASRV_LOG_SEVERE) << msg;
            break;
        case NPT_LOG_LEVEL_WARNING:
            qCDebug(DIGIKAM_MEDIASRV_LOG_WARN) << msg;
            break;
        case NPT_LOG_LEVEL_INFO:
            qCDebug(DIGIKAM_MEDIASRV_LOG_INFO) << msg;
            break;
        case NPT_LOG_LEVEL_FINE:
            qCDebug(DIGIKAM_MEDIASRV_LOG) << msg;
            break;
        default: // NPT_LOG_LEVEL_DEBUG:
            qCDebug(DIGIKAM_MEDIASRV_LOG_DEBUG) << msg;
            break;
    }
}

namespace Digikam
{

class CDeviceHostReferenceHolder
{
public:

    PLT_DeviceHostReference m_device;
};

class DMediaServer::Private
{
public:

    Private()
      : upnp(0),
        logHandler(NULL),
        serverHolder(new CDeviceHostReferenceHolder())
    {
        NPT_LogManager::GetDefault().Configure("plist:.level=INFO;.handlers=CustomHandler;");
        NPT_LogHandler::Create("digiKam", "CustomHandler", logHandler);
        logHandler->SetCustomHandlerFunction(&UPnPLogger);
    }

    PLT_UPnP*                   upnp;
    NPT_LogHandler*             logHandler;
    CDeviceHostReferenceHolder* serverHolder;
};

DMediaServer::DMediaServer(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    d->upnp = new PLT_UPnP();
    d->upnp->Start();
}

bool DMediaServer::init(int port)
{
    QString devDesc = i18n("%1 Media Server", qApp->applicationName());

    DLNAMediaServer* const device = new DLNAMediaServer(
                                    devDesc.toUtf8().data(),
                                    false,
                                    NULL,
                                    port);

    device->m_ModelName        = "digiKam";
    device->m_ModelNumber      = digikam_version;
    device->m_ModelDescription = DAboutData::digiKamSlogan().toUtf8().data();
    device->m_ModelURL         = DAboutData::webProjectUrl().toString().toUtf8().data();
    device->m_Manufacturer     = "digiKam.org";
    device->m_ManufacturerURL  = DAboutData::webProjectUrl().toString().toUtf8().data();
    device->SetDelegate(device);

    d->serverHolder->m_device  = device;

    NPT_Result res = d->upnp->AddDevice(d->serverHolder->m_device);

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "Upnp device created:" << res;

    return true;
}

DMediaServer::~DMediaServer()
{
    d->upnp->Stop();
    d->upnp->RemoveDevice(d->serverHolder->m_device);

    delete d->upnp;
    delete d->logHandler;
    delete d->serverHolder;
    delete d;
}

void DMediaServer::addAlbumsOnServer(const MediaServerMap& map)
{
    static_cast<DLNAMediaServer*>(d->serverHolder->m_device.AsPointer())->addAlbumsOnServer(map);
}

} // namespace Digikam
