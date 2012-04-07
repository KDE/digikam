/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : Duplicates items finder.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "duplicatesfinder.moc"

// Qt includes

#include <QTimer>

// KDE includes

#include <kio/job.h>
#include <kapplication.h>
#include <klocale.h>
#include <kicon.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imagelister.h"
#include "knotificationwrapper.h"

using namespace KIO;

namespace Digikam
{

class DuplicatesFinder::DuplicatesFinderPriv
{
public:

    DuplicatesFinderPriv() :
        similarity(90),
        job(0)
    {
    }

    int         similarity;
    QStringList albumsIdList;
    QStringList tagsIdList;
    Job*        job;
};

DuplicatesFinder::DuplicatesFinder(const QStringList& albumsIdList, const QStringList& tagsIdList, int similarity, ProgressItem* parent)
    : MaintenanceTool("DuplicatesFinder", parent),
      d(new DuplicatesFinderPriv)
{
    d->similarity   = similarity;
    d->albumsIdList = albumsIdList;
    d->tagsIdList   = tagsIdList;
}

DuplicatesFinder::DuplicatesFinder(int similarity, ProgressItem* parent)
    : MaintenanceTool("DuplicatesFinder", parent),
      d(new DuplicatesFinderPriv)
{
    d->similarity        = similarity;
    AlbumList palbumList = AlbumManager::instance()->allPAlbums();
    QStringList albumsIdList;
    foreach(Album* a, palbumList)
        d->albumsIdList << QString::number(a->id());
}

DuplicatesFinder::~DuplicatesFinder()
{
    delete d;
}

void DuplicatesFinder::slotStart()
{
    MaintenanceTool::slotStart();
    ProgressManager::addProgressItem(this);

    double thresh = d->similarity / 100.0;
    d->job        = ImageLister::startListJob(DatabaseUrl::searchUrl(-1));
    d->job->addMetaData("albumids",   d->albumsIdList.join(","));

    if (!d->tagsIdList.isEmpty())
        d->job->addMetaData("tagids", d->tagsIdList.join(","));

    d->job->addMetaData("duplicates", "normal");
    d->job->addMetaData("threshold",  QString::number(thresh));

    connect(d->job, SIGNAL(result(KJob*)),
            this, SLOT(slotDone()));

    connect(d->job, SIGNAL(totalAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong)));

    connect(d->job, SIGNAL(processedAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong)));

    setLabel(i18n("Find duplicates items"));
    setThumbnail(KIcon("tools-wizard").pixmap(22));
}

void DuplicatesFinder::slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong amount)
{
    setTotalItems(amount);
}

void DuplicatesFinder::slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong amount)
{
    setCompletedItems(amount);
    updateProgress();
}

void DuplicatesFinder::slotDone()
{
    d->job = NULL;
    MaintenanceTool::slotDone();
}

void DuplicatesFinder::slotCancel()
{
    if (d->job)
    {
        d->job->kill();
        d->job = NULL;
    }

    MaintenanceTool::slotCancel();
}

}  // namespace Digikam
