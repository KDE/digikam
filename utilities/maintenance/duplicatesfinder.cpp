/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : Duplicates items finder.
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

#include "duplicatesfinder.moc"

// Qt includes

#include <QTime>

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
        duration.start();
        job = ImageLister::startListJob(DatabaseUrl::searchUrl(-1));
    }

    int   similarity;
    QTime duration;
    Job*  job;
};

DuplicatesFinder::DuplicatesFinder(const QStringList& albumsIdList, const QStringList& tagsIdList, int similarity)
    : ProgressItem(0, "DuplicatesFinder", QString(), QString(), true, true),
      d(new DuplicatesFinderPriv)
{
    d->similarity = similarity;
    d->job->addMetaData("albumids",   albumsIdList.join(","));
    d->job->addMetaData("tagids",     tagsIdList.join(","));
    init();
}

DuplicatesFinder::DuplicatesFinder(int similarity)
    : ProgressItem(0, "DuplicatesFinder", QString(), QString(), true, true),
      d(new DuplicatesFinderPriv)
{
    d->similarity        = similarity;
    AlbumList palbumList = AlbumManager::instance()->allPAlbums();
    QStringList albumsIdList;
    foreach(Album* a, palbumList)
        albumsIdList << QString::number(a->id());

    d->job->addMetaData("albumids",   albumsIdList.join(","));
    init();
}

DuplicatesFinder::~DuplicatesFinder()
{
    delete d;
}

void DuplicatesFinder::init()
{
    ProgressManager::addProgressItem(this);

    double thresh = d->similarity / 100.0;
    d->job->addMetaData("duplicates", "normal");
    d->job->addMetaData("threshold",  QString::number(thresh));

    connect(d->job, SIGNAL(result(KJob*)),
            this, SLOT(slotDuplicatesSearchResult()));

    connect(d->job, SIGNAL(totalAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong)));

    connect(d->job, SIGNAL(processedAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong)));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

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

void DuplicatesFinder::slotDuplicatesSearchResult()
{
    QTime now, t = now.addMSecs(d->duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper(id(),
                         i18n("Find duplicates is done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), label());
    d->job = NULL;

    emit signalComplete();

    setComplete();
}

void DuplicatesFinder::slotCancel()
{
    if (d->job)
    {
        d->job->kill();
        d->job = NULL;
    }

    setComplete();
}

}  // namespace Digikam
