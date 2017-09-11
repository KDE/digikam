/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media server manager
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dmediaservermngr.h"

// Qt includes

#include <QList>
#include <QMap>
#include <QStringList>
#include <QUrl>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dmediaserver.h"

namespace Digikam
{

class DMediaServerMngrCreator
{
public:

    DMediaServerMngr object;
};

Q_GLOBAL_STATIC(DMediaServerMngrCreator, creator)

// ---------------------------------------------------------------------------------------------

class DMediaServerMngr::Private
{
public:

    Private()
    {
        server = 0;
    }

    DMediaServer*              server;
    QMap<QString, QList<QUrl>> collectionMap;
};

DMediaServerMngr* DMediaServerMngr::instance()
{
    return &creator->object;
}

DMediaServerMngr::DMediaServerMngr()
    : d(new Private)
{
}

DMediaServerMngr::~DMediaServerMngr()
{
    cleanUp();
    delete d;
}

void DMediaServerMngr::cleanUp()
{
    slotTurnOff();
}

void DMediaServerMngr::slotTurnOff()
{
    delete d->server;
    d->server = 0;
}

void DMediaServerMngr::checkLoadAtStartup()
{
    // Load mediaserver at startup ?

    KSharedConfig::Ptr config    = KSharedConfig::openConfig();
    KConfigGroup dlnaConfigGroup = config->group(QLatin1String("DLNA Settings"));
    bool startServerOnStartup    = dlnaConfigGroup.readEntry(QLatin1String("Start Server On Startup"), false);

    if (startServerOnStartup)
    {
        // TODO: restore old configuration.
        slotTurnOn();
    }
}

void DMediaServerMngr::slotTurnOn()
{
    startMediaServer();
}

void DMediaServerMngr::setCollectionMap(const QMap<QString, QList<QUrl>>& collectionMap)
{
    d->collectionMap = collectionMap;
}

void DMediaServerMngr::startMediaServer()
{
    if (!d->server)
    {
        d->server = new DMediaServer();
    }

    d->server->addImagesOnServer(d->collectionMap);
}

} // namespace Digikam
