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
#include "newitemsfinder.h"
#include "thumbsgenerator.h"
#include "fingerprintsgenerator.h"
#include "duplicatesfinder.h"

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

MaintenanceMngr::MaintenanceMngr(QObject* parent)
    : QObject(parent), d(new MaintenanceMngrPriv)
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

bool MaintenanceMngr::isRunning() const
{
    return d->running;
}

void MaintenanceMngr::stop()
{
}

void MaintenanceMngr::start()
{
    kDebug() << "settings.newItems         : " << d->settings.newItems;
    kDebug() << "settings.thumbnails       : " << d->settings.thumbnails;
    kDebug() << "settings.scanThumbs       : " << d->settings.scanThumbs;
    kDebug() << "settings.fingerPrints     : " << d->settings.fingerPrints;
    kDebug() << "settings.scanFingerPrints : " << d->settings.scanFingerPrints;
    kDebug() << "settings.duplicates       : " << d->settings.duplicates;
    kDebug() << "settings.similarity       : " << d->settings.similarity;
    slotStage1();
}

void MaintenanceMngr::slotStage1()
{
    if (d->settings.newItems)
    {
        NewItemsFinder* tool = new NewItemsFinder();
        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage2()));
    }
    else
    {
        slotStage2();
    }
}

void MaintenanceMngr::slotStage2()
{
    if (d->settings.thumbnails)
    {
        ThumbsGenerator* tool = new ThumbsGenerator(d->settings.scanThumbs);
        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage3()));
    }
    else
    {
        slotStage3();
    }
}

void MaintenanceMngr::slotStage3()
{
    if (d->settings.fingerPrints)
    {
        FingerPrintsGenerator* tool = new FingerPrintsGenerator(d->settings.scanFingerPrints);
        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage4()));
    }
    else
    {
        slotStage4();
    }
}

void MaintenanceMngr::slotStage4()
{
    if (d->settings.duplicates)
    {
        DuplicatesFinder* tool = new DuplicatesFinder(d->settings.similarity);
        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage5()));
    }
    else
    {
        slotStage5();
    }
}

void MaintenanceMngr::slotStage5()
{
}

}  // namespace Digikam
