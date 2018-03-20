/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : Duplicates items finder.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "duplicatesfinder.h"

// Qt includes

#include <QTimer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "imagelister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "dbjobsthread.h"
#include "dbjobsmanager.h"
#include "applicationsettings.h"

namespace Digikam
{

class DuplicatesFinder::Private
{
public:

    Private() :
        minSimilarity(90),
        maxSimilarity(100),
        albumTagRelation(0),
        searchResultRestriction(0),
        isAlbumUpdate(false),
        job(0)
    {
    }

    int                   minSimilarity;
    int                   maxSimilarity;
    int                   albumTagRelation;
    int                   searchResultRestriction;
    bool                  isAlbumUpdate;
    QList<int>            albumsIdList;
    QList<qlonglong>      imageIdList;
    QList<int>            tagsIdList;
    SearchesDBJobsThread* job;
};

DuplicatesFinder::DuplicatesFinder(const QList<qlonglong>& imageIds, int minSimilarity, int maxSimilarity, int searchResultRestriction, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("DuplicatesFinder"), parent),
      d(new Private)
{
    d->minSimilarity            = minSimilarity;
    d->maxSimilarity            = maxSimilarity;

    d->isAlbumUpdate            = true;
    d->imageIdList              = imageIds;
    d->searchResultRestriction  = searchResultRestriction;
}

DuplicatesFinder::DuplicatesFinder(const AlbumList& albums, const AlbumList& tags, int albumTagRelation,int minSimilarity, int maxSimilarity, int searchResultRestriction, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("DuplicatesFinder"), parent),
      d(new Private)
{
    d->minSimilarity            = minSimilarity;
    d->maxSimilarity            = maxSimilarity;
    d->albumTagRelation         = albumTagRelation;
    d->searchResultRestriction  = searchResultRestriction;

    foreach(Album* const a, albums)
        d->albumsIdList << a->id();

    foreach(Album* const a, tags)
        d->tagsIdList << a->id();
}

DuplicatesFinder::DuplicatesFinder(const int minSimilarity, int maxSimilarity, int searchResultRestriction, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("DuplicatesFinder"), parent),
      d(new Private)
{
    d->minSimilarity            = minSimilarity;
    d->maxSimilarity            = maxSimilarity;
    d->searchResultRestriction  = searchResultRestriction;

    foreach(Album* const a, AlbumManager::instance()->allPAlbums())
        d->albumsIdList << a->id();
}

DuplicatesFinder::~DuplicatesFinder()
{
    delete d;
}

void DuplicatesFinder::slotStart()
{
    MaintenanceTool::slotStart();
    setLabel(i18n("Find duplicates items"));
    setThumbnail(QIcon::fromTheme(QLatin1String("tools-wizard")).pixmap(22));
    ProgressManager::addProgressItem(this);

    double minThresh = d->minSimilarity / 100.0;
    double maxThresh = d->maxSimilarity / 100.0;
    SearchesDBJobInfo jobInfo;
    jobInfo.setDuplicatesJob();
    jobInfo.setMinThreshold(minThresh);
    jobInfo.setMaxThreshold(maxThresh);
    jobInfo.setAlbumsIds(d->albumsIdList);
    jobInfo.setImageIds(d->imageIdList);
    jobInfo.setAlbumTagRelation(d->albumTagRelation);
    jobInfo.setSearchResultRestriction(d->searchResultRestriction);

    if (d->isAlbumUpdate)
        jobInfo.setAlbumUpdate();

    if (!d->tagsIdList.isEmpty())
        jobInfo.setTagsIds(d->tagsIdList);

    d->job = DBJobsManager::instance()->startSearchesJobThread(jobInfo);

    connect(d->job, SIGNAL(finished()),
            this, SLOT(slotDone()));

    connect(d->job, SIGNAL(totalSize(int)),
            this, SLOT(slotDuplicatesSearchTotalAmount(int)));

    connect(d->job, SIGNAL(processedSize(int)),
            this, SLOT(slotDuplicatesSearchProcessedAmount(int)));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SIGNAL(signalComplete()));
}

void DuplicatesFinder::slotDuplicatesSearchTotalAmount(int amount)
{
    setTotalItems(amount);
}

void DuplicatesFinder::slotDuplicatesSearchProcessedAmount(int amount)
{
    setCompletedItems(amount);
    updateProgress();
}

void DuplicatesFinder::slotDone()
{
    if (d->job && d->job->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list url: " << d->job->errorsList().first();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->job->errorsList().first(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    // save the min and max similarity in the configuration.
    ApplicationSettings::instance()->setDuplicatesSearchLastMinSimilarity(d->minSimilarity);
    ApplicationSettings::instance()->setDuplicatesSearchLastMaxSimilarity(d->maxSimilarity);
    ApplicationSettings::instance()->setDuplicatesAlbumTagRelation(d->albumTagRelation);
    ApplicationSettings::instance()->setDuplicatesSearchRestrictions(d->searchResultRestriction);

    d->job = 0;
    MaintenanceTool::slotDone();
}

void DuplicatesFinder::slotCancel()
{
    if (d->job)
    {
        d->job->cancel();
        d->job = 0;
    }

    MaintenanceTool::slotCancel();
}

}  // namespace Digikam
