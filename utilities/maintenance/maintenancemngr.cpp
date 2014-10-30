/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager
 *
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "config-digikam.h"
#include "maintenancesettings.h"
#include "newitemsfinder.h"
#include "thumbsgenerator.h"
#include "fingerprintsgenerator.h"
#include "duplicatesfinder.h"
#include "imagequalitysorter.h"
#include "metadatasynchronizer.h"
#include "dnotificationwrapper.h"
#include "progressmanager.h"

#ifdef HAVE_KFACE
#include "facedetector.h"
#endif /* HAVE_KFACE */

namespace Digikam
{

class MaintenanceMngr::Private
{
public:

    Private()
    {
        running               = false;
        newItemsFinder        = 0;
        thumbsGenerator       = 0;
        fingerPrintsGenerator = 0;
        duplicatesFinder      = 0;
        metadataSynchronizer  = 0;
        imageQualitySorter    = 0;

#ifdef HAVE_KFACE
        faceDetector          = 0;
#endif /* HAVE_KFACE */
    }

    bool                   running;

    QTime                  duration;

    MaintenanceSettings    settings;

    NewItemsFinder*        newItemsFinder;
    ThumbsGenerator*       thumbsGenerator;
    FingerPrintsGenerator* fingerPrintsGenerator;
    DuplicatesFinder*      duplicatesFinder;
    MetadataSynchronizer*  metadataSynchronizer;
    ImageQualitySorter*    imageQualitySorter;

#ifdef HAVE_KFACE
    FaceDetector*          faceDetector;
#endif /* HAVE_KFACE */
};

MaintenanceMngr::MaintenanceMngr(QObject* const parent)
    : QObject(parent), d(new Private)
{
    connect(ProgressManager::instance(), SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotToolCompleted(ProgressItem*)));

    connect(ProgressManager::instance(), SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotToolCanceled(ProgressItem*)));
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
    kDebug() << d->settings;

    d->duration.start();
    stage1();
}

void MaintenanceMngr::slotToolCompleted(ProgressItem* tool)
{
    // At each stage, relevant tool instance is set to zero to prevent redondant call to this slot
    // from ProgressManager. This will disable multiple triggering in this method.
    // There is no memory leak. Each tool instance are delete later by ProgressManager.

    if (tool == dynamic_cast<ProgressItem*>(d->newItemsFinder))
    {
        d->newItemsFinder = 0;
        stage2();
    }
    else if (tool == dynamic_cast<ProgressItem*>(d->thumbsGenerator))
    {
        d->thumbsGenerator = 0;
        stage3();
    }
    else if (tool == dynamic_cast<ProgressItem*>(d->fingerPrintsGenerator))
    {
        d->fingerPrintsGenerator = 0;
        stage4();
    }
    else if (tool == dynamic_cast<ProgressItem*>(d->duplicatesFinder))
    {
        d->duplicatesFinder = 0;
        stage5();
    }
#ifdef HAVE_KFACE
    else if (tool == dynamic_cast<ProgressItem*>(d->faceDetector))
    {
        d->faceDetector = 0;
        stage6();
    }
#endif /* HAVE_KFACE */
   else if (tool == dynamic_cast<ProgressItem*>(d->imageQualitySorter))
    {
        d->imageQualitySorter = 0;
        stage7();
    }
    else if (tool == dynamic_cast<ProgressItem*>(d->metadataSynchronizer))
    {
        d->metadataSynchronizer = 0;
        done();
    }
}

void MaintenanceMngr::slotToolCanceled(ProgressItem* tool)
{
    if (tool == dynamic_cast<ProgressItem*>(d->newItemsFinder)        ||
        tool == dynamic_cast<ProgressItem*>(d->thumbsGenerator)       ||
        tool == dynamic_cast<ProgressItem*>(d->fingerPrintsGenerator) ||
        tool == dynamic_cast<ProgressItem*>(d->duplicatesFinder)      ||
#ifdef HAVE_KFACE
        tool == dynamic_cast<ProgressItem*>(d->faceDetector)          ||
#endif /* HAVE_KFACE */
        tool == dynamic_cast<ProgressItem*>(d->imageQualitySorter)    ||
        tool == dynamic_cast<ProgressItem*>(d->metadataSynchronizer))
    {
        cancel();
    }
}

void MaintenanceMngr::stage1()
{
    kDebug() << "stage1";

    if (d->settings.newItems)
    {
        if (d->settings.wholeAlbums)
        {
            d->newItemsFinder = new NewItemsFinder();
        }
        else
        {
            QStringList paths;

            foreach(Album* const a, d->settings.albums)
            {
                PAlbum* const pa = dynamic_cast<PAlbum*>(a);
                if (pa)
                    paths << pa->folderPath();
            }

            d->newItemsFinder = new NewItemsFinder(NewItemsFinder::ScheduleCollectionScan, paths);
        }

        d->newItemsFinder->setNotificationEnabled(false);
        d->newItemsFinder->start();
    }
    else
    {
        stage2();
    }
}

void MaintenanceMngr::stage2()
{
    kDebug() << "stage2";

    if (d->settings.thumbnails)
    {
        bool rebuildAll = (d->settings.scanThumbs == false);
        AlbumList list;
        list << d->settings.albums;
        list << d->settings.tags;

        d->thumbsGenerator = new ThumbsGenerator(rebuildAll, list);
        d->thumbsGenerator->setNotificationEnabled(false);
        d->thumbsGenerator->setUseMultiCoreCPU(d->settings.useMutiCoreCPU);
        d->thumbsGenerator->start();
    }
    else
    {
        stage3();
    }
}

void MaintenanceMngr::stage3()
{
    kDebug() << "stage3";

    if (d->settings.fingerPrints)
    {
        bool rebuildAll = (d->settings.scanFingerPrints == false);
        AlbumList list;
        list << d->settings.albums;
        list << d->settings.tags;

        d->fingerPrintsGenerator = new FingerPrintsGenerator(rebuildAll, list);
        d->fingerPrintsGenerator->setNotificationEnabled(false);
        d->fingerPrintsGenerator->setUseMultiCoreCPU(d->settings.useMutiCoreCPU);
        d->fingerPrintsGenerator->start();
    }
    else
    {
        stage4();
    }
}

void MaintenanceMngr::stage4()
{
    kDebug() << "stage4";

    if (d->settings.duplicates)
    {
        d->duplicatesFinder = new DuplicatesFinder(d->settings.albums, d->settings.tags, d->settings.similarity);
        d->duplicatesFinder->setNotificationEnabled(false);
        d->duplicatesFinder->start();
    }
    else
    {
        stage5();
    }
}

void MaintenanceMngr::stage5()
{
    kDebug() << "stage5";

#ifdef HAVE_KFACE
    if (d->settings.faceManagement)
    {
        // NOTE : Use multi-core CPU option is passed through FaceScanSettings
        d->settings.faceSettings.useFullCpu = d->settings.useMutiCoreCPU;
        d->faceDetector                     = new FaceDetector(d->settings.faceSettings);
        d->faceDetector->setNotificationEnabled(false);
        d->faceDetector->start();
    }
    else
#endif /* HAVE_KFACE */
    {
        stage6();
    }
}

void MaintenanceMngr::stage6()
{
    kDebug() << "stage6";

    if (d->settings.qualitySort && d->settings.quality.enableSorter)
    {
        AlbumList list;
        list << d->settings.albums;
        list << d->settings.tags;

        d->imageQualitySorter = new ImageQualitySorter((ImageQualitySorter::QualityScanMode)d->settings.qualityScanMode, list, d->settings.quality);
        d->imageQualitySorter->setNotificationEnabled(false);
        d->imageQualitySorter->setUseMultiCoreCPU(d->settings.useMutiCoreCPU);
        d->imageQualitySorter->start();
    }
    else
    {
        stage7();
    }
}


void MaintenanceMngr::stage7()
{
    kDebug() << "stage7";

    if (d->settings.metadataSync)
    {
        AlbumList list;
        list << d->settings.albums;
        list << d->settings.tags;
        d->metadataSynchronizer = new MetadataSynchronizer(list, MetadataSynchronizer::SyncDirection(d->settings.syncDirection));
        d->metadataSynchronizer->setNotificationEnabled(false);
        d->metadataSynchronizer->setUseMultiCoreCPU(d->settings.useMutiCoreCPU);
        d->metadataSynchronizer->start();
    }
    else
    {
        done();
    }
}

void MaintenanceMngr::done()
{
    d->running   = false;
    QTime now, t = now.addMSecs(d->duration.elapsed());

    // Pop-up a message to bring user when all is done.
    DNotificationWrapper("digiKam Maintenance", // not i18n
                         i18n("All operations are done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), i18n("digiKam Maintenance"));

    emit signalComplete();
}

void MaintenanceMngr::cancel()
{
    d->running = false;
    emit signalComplete();
}

}  // namespace Digikam
