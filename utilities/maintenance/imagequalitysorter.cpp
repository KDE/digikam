/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : image quality sorter
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagequalitysorter.h"

// Qt includes

#include <QString>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dimg.h"
#include "coredb.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "tagscache.h"
#include "maintenancethread.h"

namespace Digikam
{

class ImageQualitySorter::Private
{
public:

    Private() :
        mode(ImageQualitySorter::NonAssignedItems),
        thread(0)
    {
    }

    QualityScanMode      mode;

    ImageQualitySettings quality;

    QStringList          allPicturesPath;

    AlbumList            albumList;

    MaintenanceThread*   thread;
};

ImageQualitySorter::ImageQualitySorter(QualityScanMode mode, const AlbumList& list,
                                       const ImageQualitySettings& quality, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("ImageQualitySorter"), parent),
      d(new Private)
{
    setLabel(i18n("Image Quality Sorter"));
    ProgressManager::addProgressItem(this);

    d->mode       = mode;
    d->albumList  = list;
    d->quality    = quality;
    d->thread     = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvance(QImage)),
            this, SLOT(slotAdvance(QImage)));
}

ImageQualitySorter::~ImageQualitySorter()
{
    delete d;
}

void ImageQualitySorter::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void ImageQualitySorter::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void ImageQualitySorter::slotStart()
{
    MaintenanceTool::slotStart();

    if (d->albumList.isEmpty())
    {
        d->albumList = AlbumManager::instance()->allPAlbums();
    }

    // Get all item in DB which do not have any Pick Label assigned.
    QStringList dirty = CoreDbAccess().db()->getItemsURLsWithTag(TagsCache::instance()->tagForPickLabel(NoPickLabel));

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    for (AlbumList::ConstIterator it = d->albumList.constBegin();
         !canceled() && (it != d->albumList.constEnd()); ++it)
    {
        QStringList aPaths;

        if ((*it)->type() == Album::PHYSICAL)
        {
            aPaths = CoreDbAccess().db()->getItemURLsInAlbum((*it)->id());
        }
        else if ((*it)->type() == Album::TAG)
        {
            aPaths = CoreDbAccess().db()->getItemURLsInTag((*it)->id());
        }

        if (d->mode == NonAssignedItems)
        {
            foreach(const QString& path, aPaths)
            {
                if (dirty.contains(path))
                {
                    d->allPicturesPath += path;
                }
            }
        }
        else  // AllItems
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

    d->thread->sortByImageQuality(d->allPicturesPath, d->quality);
    d->thread->start();
}

void ImageQualitySorter::slotAdvance(const QImage& img)
{
    setThumbnail(QIcon(QPixmap::fromImage(img)));
    advance(1);
}

}  // namespace Digikam
