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
#include <QTime>

// KDE includes

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>

// Local includes

#include "maintenancesettings.h"
#include "newitemsfinder.h"
#include "thumbsgenerator.h"
#include "fingerprintsgenerator.h"
#include "duplicatesfinder.h"
#include "metadatasynchronizer.h"
#include "facedetector.h"
#include "knotificationwrapper.h"

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

    QTime               duration;

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

bool MaintenanceMngr::isRunning() const
{
    return d->running;
}

void MaintenanceMngr::setSettings(const MaintenanceSettings& settings)
{
    d->settings = settings;
    kDebug() << "settings.newItems            : " << d->settings.newItems;
    kDebug() << "settings.thumbnails          : " << d->settings.thumbnails;
    kDebug() << "settings.scanThumbs          : " << d->settings.scanThumbs;
    kDebug() << "settings.fingerPrints        : " << d->settings.fingerPrints;
    kDebug() << "settings.scanFingerPrints    : " << d->settings.scanFingerPrints;
    kDebug() << "settings.duplicates          : " << d->settings.duplicates;
    kDebug() << "settings.similarity          : " << d->settings.similarity;
    kDebug() << "settings.metadata            : " << d->settings.metadata;
    kDebug() << "settings.facedetection       : " << d->settings.faceDetection;
    kDebug() << "settings.faceScannedHandling : " << d->settings.faceSettings.alreadyScannedHandling;

    d->duration.start();
    slotStage1();
}

void MaintenanceMngr::slotStage1()
{
    if (d->settings.newItems)
    {
        NewItemsFinder* tool = new NewItemsFinder();
        tool->setNotificationEnabled(false);

        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage2()));

        connect(tool, SIGNAL(progressItemCanceled(const QString&)),
                this, SLOT(slotCancel()));
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
        tool->setNotificationEnabled(false);

        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage3()));

        connect(tool, SIGNAL(progressItemCanceled(const QString&)),
                this, SLOT(slotCancel()));
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
        tool->setNotificationEnabled(false);

        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage4()));

        connect(tool, SIGNAL(progressItemCanceled(const QString&)),
                this, SLOT(slotCancel()));
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

        connect(tool, SIGNAL(progressItemCanceled(const QString&)),
                this, SLOT(slotCancel()));
    }
    else
    {
        slotStage5();
    }
}

void MaintenanceMngr::slotStage5()
{
    if (d->settings.metadata)
    {
        MetadataSynchronizer* tool = new MetadataSynchronizer(MetadataSynchronizer::WriteFromDatabaseToFile);

        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotStage6()));

        connect(tool, SIGNAL(progressItemCanceled(const QString&)),
                this, SLOT(slotCancel()));
    }
    else
    {
        slotStage6();
    }
}

void MaintenanceMngr::slotStage6()
{
    if (d->settings.faceDetection)
    {
        FaceDetector* tool = new FaceDetector(d->settings.faceSettings);

        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotDone()));

        connect(tool, SIGNAL(progressItemCanceled(const QString&)),
                this, SLOT(slotCancel()));
    }
    else
    {
        slotDone();
    }
}

void MaintenanceMngr::slotDone()
{
    d->running   = false;
    QTime now, t = now.addMSecs(d->duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper("digiKam Maintenance", // not i18n
                         i18n("All operations are done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), i18n("digiKam Maintenance"));

    emit signalComplete();
}

void MaintenanceMngr::slotCancel()
{
    d->running = false;
    emit signalComplete();
}

}  // namespace Digikam
