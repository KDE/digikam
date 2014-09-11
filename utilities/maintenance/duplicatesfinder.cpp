/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : Duplicates items finder.
 *
 * Copyright (C) 2012-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "duplicatesfinder.moc"

// Qt includes

#include <QTimer>

// KDE includes

#include <kio/job.h>
#include <kapplication.h>
#include <klocale.h>
#include <kicon.h>
#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "imagelister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"

using namespace KIO;

namespace Digikam
{

class DuplicatesFinder::Private
{
public:

    Private() :
        similarity(90),
        job(0)
    {
    }

    int         similarity;
    QStringList albumsIdList;
    QStringList tagsIdList;
    Job*        job;
};

DuplicatesFinder::DuplicatesFinder(const AlbumList& albums, const AlbumList& tags, int similarity, ProgressItem* const parent)
    : MaintenanceTool("DuplicatesFinder", parent),
      d(new Private)
{
    d->similarity   = similarity;

    foreach(Album* const a, albums)
        d->albumsIdList << QString::number(a->id());

    foreach(Album* const a, tags)
        d->tagsIdList << QString::number(a->id());
}

DuplicatesFinder::DuplicatesFinder(const int similarity, ProgressItem* const parent)
    : MaintenanceTool("DuplicatesFinder", parent),
      d(new Private)
{
    d->similarity = similarity;

    foreach(Album* const a, AlbumManager::instance()->allPAlbums())
        d->albumsIdList << QString::number(a->id());
}

DuplicatesFinder::~DuplicatesFinder()
{
    delete d;
}

void DuplicatesFinder::slotStart()
{
    MaintenanceTool::slotStart();
    setLabel(i18n("Find duplicates items"));
    setThumbnail(KIcon("tools-wizard").pixmap(22));
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

    connect(d->job, SIGNAL(totalAmount(KJob*,KJob::Unit,qulonglong)),
            this, SLOT(slotDuplicatesSearchTotalAmount(KJob*,KJob::Unit,qulonglong)));

    connect(d->job, SIGNAL(processedAmount(KJob*,KJob::Unit,qulonglong)),
            this, SLOT(slotDuplicatesSearchProcessedAmount(KJob*,KJob::Unit,qulonglong)));
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
    if (d->job->error())
    {
        kWarning() << "Failed to list url: " << d->job->errorString();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->job->errorString(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    d->job = 0;
    MaintenanceTool::slotDone();
}

void DuplicatesFinder::slotCancel()
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    MaintenanceTool::slotCancel();
}

}  // namespace Digikam
