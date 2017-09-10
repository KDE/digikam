/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : a media server to export collections through DLNA.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QList>
#include <QUrl>
#include <QFile>
#include <QStandardPaths>

// Libhupnp includes

#include "hupnp_global.h"
#include "hdeviceinfo.h"
#include "hdevicehost.h"
#include "hdevicehost_configuration.h"
#include "hav_global.h"
#include "hrootdir.h"
#include "himageitem.h"
#include "hphoto.h"
#include "hcontainer.h"
#include "hav_devicemodel_creator.h"
#include "hmediaserver_deviceconfiguration.h"
#include "hfsys_datasource.h"
#include "hcontentdirectory_serviceconfiguration.h"

// Local includes

#include "digikam_debug.h"

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

namespace Digikam
{

class DMediaServer::Private
{
public:

    Private()
    {
        deviceHost = 0;
        datasource = 0;
    }

    HDeviceHost*           deviceHost;
    HFileSystemDataSource* datasource;
};

DMediaServer::DMediaServer(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    // Set Hupnp debug level on the console
    Herqq::Upnp::SetLoggingLevel(Herqq::Upnp::Debug);

    // Configure a data source
    HFileSystemDataSourceConfiguration datasourceConfig;

    // Here you could configure the data source in more detail if needed. For example,
    // you could add "root directories" to the configuration and the data source
    // would scan those directories for media content upon initialization.
    d->datasource = new HFileSystemDataSource(datasourceConfig);

    // Configure ContentDirectoryService by providing it access to the desired data source.
    HContentDirectoryServiceConfiguration cdsConfig;
    cdsConfig.setDataSource(d->datasource, false);

    // Configure MediaServer by giving it the ContentDirectoryService configuration.
    HMediaServerDeviceConfiguration mediaServerConfig;
    mediaServerConfig.setContentDirectoryConfiguration(cdsConfig);

    // Setup the "Device Model Cretor" that HUPnP will use to create
    // appropriate UPnP A/V device and service instances. Here you provide the
    // MediaServer configuration HUPnP will pass to the MediaServer device instance.
    HAvDeviceModelCreator creator;
    creator.setMediaServerConfiguration(mediaServerConfig);

    // Setup the HDeviceHost with desired configuration info.
    HDeviceConfiguration config;

    QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QString::fromLatin1("digikam/mediaserver/descriptions/herqq_mediaserver_description.xml"));

    config.setPathToDeviceDescription(filePath);

    qCDebug(DIGIKAM_GENERAL_LOG) << "filepath properly set : " << filePath;

    config.setCacheControlMaxAge(180);

    HDeviceHostConfiguration hostConfiguration;
    hostConfiguration.setDeviceModelCreator(creator);
    hostConfiguration.add(config);

    // Initialize the HDeviceHost.
    d->deviceHost = new HDeviceHost(this);

    if (!d->deviceHost->init(hostConfiguration))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Initialization failed. Description : "
                                     << d->deviceHost->errorDescription().toLocal8Bit();
    }
}

DMediaServer::~DMediaServer()
{
     delete d->datasource;
     delete d;
}

void DMediaServer::addImagesOnServer(const QList<QUrl>& imageUrlList)
{
    QList<HItem*> itemList;

    for (int i = 0 ; i < imageUrlList.size() ; i++)
    {
        itemList.append(new HItem(imageUrlList.at(i).fileName(),
                                  QLatin1String("0"),
                                  QString()));
        d->datasource->add(itemList.at(i), imageUrlList.at(i).path());
    }
}

void DMediaServer::addImagesOnServer(const QMap<QString, QList<QUrl>>& collectionMap)
{
    QList<HContainer*> containerList;
    QList<HItem*>      itemList;
    QList<QString>     keys        = collectionMap.uniqueKeys();
    QList<QUrl>        imageUrlList;
    int                currentSize = 0;

    for (int i = 0 ; i < keys.size() ; i++)
    {
        containerList.append(new HContainer(keys.at(i),
                                            QLatin1String("0")));
        d->datasource->add(containerList.at(i));

        imageUrlList.clear();
        imageUrlList = collectionMap.value(keys.at(i));
        currentSize  = itemList.size();

        for (int j = 0 ; j < imageUrlList.size() ; j++)
        {
            itemList.append(new HItem(imageUrlList.at(j).fileName(),
                                      containerList.at(i)->id(),
                                      QString()));
            d->datasource->add(itemList.at(j + currentSize), imageUrlList.at(j).toLocalFile());
            qCDebug(DIGIKAM_GENERAL_LOG) << "Add item to mediaserver : " << imageUrlList.at(j).toLocalFile();
        }
    }
}

} // namespace Digikam
