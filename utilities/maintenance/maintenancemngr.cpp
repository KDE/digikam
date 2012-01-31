/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "maintenancemngr.moc"

// Qt includes

#include <QString>

// KDE includes

#include <kurl.h>
#include <kdebug.h>

// Local includes

#include "maintenancesettings.h"

namespace Digikam
{

class MaintenanceMngr::MaintenanceMngrPriv
{
public:

    MaintenanceMngrPriv()
    {
        running = false;        
    }

    bool                running;

    MaintenanceSettings settings;
};

MaintenanceMngr::MaintenanceMngr()
    : d(new MaintenanceMngrPriv)
{
}

MaintenanceMngr::~MaintenanceMngr()
{
    delete d;
}

void MaintenanceMngr::setSettings(const MaintenanceSettings& settings)
{
    d->settings = settings;
}

void MaintenanceMngr::start()
{
}

void MaintenanceMngr::stop()
{
}

bool MaintenanceMngr::isRunning() const
{
    return d->running;
}

}  // namespace Digikam
