/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : fingerprints generator
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fingerprintsgenerator.moc"

// Qt includes

#include <QString>

// KDE includes

#include <klocale.h>
#include <kconfig.h>

// Local includes

#include "dimg.h"
#include "albumdb.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "maintenancethread.h"

namespace Digikam
{

class FingerPrintsGenerator::Private
{
public:

    Private() :
        rebuildAll(true),
        thread(0)
    {
    }

    bool               rebuildAll;

    QStringList        allPicturesPath;

    AlbumList          albumList;

    MaintenanceThread* thread;
};

FingerPrintsGenerator::FingerPrintsGenerator(const bool rebuildAll, const AlbumList& list, ProgressItem* const parent)
    : MaintenanceTool("FingerPrintsGenerator", parent),
      d(new Private)
{
    setLabel(i18n("Finger-prints"));
    ProgressManager::addProgressItem(this);

    d->albumList  = list;
    d->rebuildAll = rebuildAll;
    d->thread     = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvance(QImage)),
            this, SLOT(slotAdvance(QImage)));
}

FingerPrintsGenerator::~FingerPrintsGenerator()
{
    delete d;
}

void FingerPrintsGenerator::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void FingerPrintsGenerator::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void FingerPrintsGenerator::slotStart()
{
    MaintenanceTool::slotStart();

    if (d->albumList.isEmpty())
    {
        d->albumList = AlbumManager::instance()->allPAlbums();
    }

    QStringList dirty = DatabaseAccess().db()->getDirtyOrMissingFingerprintURLs();

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    for (AlbumList::ConstIterator it = d->albumList.constBegin();
         !canceled() && (it != d->albumList.constEnd()); ++it)
    {
        QStringList aPaths;

        if ((*it)->type() == Album::PHYSICAL)
        {
            aPaths = DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
        }
        else if ((*it)->type() == Album::TAG)
        {
            aPaths = DatabaseAccess().db()->getItemURLsInTag((*it)->id());
        }

        if (!d->rebuildAll)
        {
            foreach(const QString& path, aPaths)
            {
                if (dirty.contains(path))
                {
                    d->allPicturesPath += path;
                }
            }
        }
        else
        {
            d->allPicturesPath += aPaths;
        }
    }

    if (d->allPicturesPath.isEmpty())
    {
        slotDone();
        return;
    }

    setTotalItems(d->allPicturesPath.count());

    d->thread->generateFingerprints(d->allPicturesPath);
    d->thread->start();
}

void FingerPrintsGenerator::slotAdvance(const QImage& img)
{
    setThumbnail(QPixmap::fromImage(img));
    advance(1);
}

void FingerPrintsGenerator::slotDone()
{
    // Switch on scanned for finger-prints flag on digiKam config file.
    KGlobal::config()->group("General Settings").writeEntry("Finger Prints Generator First Run", true);

    MaintenanceTool::slotDone();
}

}  // namespace Digikam
