/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2009-03-05
* Description : Qt item model for database entries
*
* Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagefiltermodel.h"
#include "imagefiltermodel.moc"
#include "imagefiltermodelpriv.h"
#include "imagefiltermodelpriv.moc"
#include "imagefiltermodelthreads.h"
#include "imagefiltermodelthreads.moc"

// Qt includes.

// KDE includes.

// Local includes.

#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databasewatch.h"
#include "imageinfolist.h"
#include "imagemodel.h"

namespace Digikam
{

ImageFilterModelPrivate::ImageFilterModelPrivate()
{
    imageModel            = 0;
    version               = 0;
    lastDiscardVersion    = 0;
    lastFilteredVersion   = 0;
    sentOut               = 0;
    sentOutForReAdd       = 0;
    updateFilterTimer           = 0;
    needPrepare           = false;
    needPrepareComments   = false;
    needPrepareTags       = false;
    preparer              = 0;
    filterer              = 0;

    setupWorkers();
}

ImageFilterModelPrivate::~ImageFilterModelPrivate()
{
    // facilitate thread stopping
    version++;
    preparer->shutDown();
    filterer->shutDown();
    delete preparer;
    delete filterer;
}

ImageFilterModelWorker::ImageFilterModelWorker(ImageFilterModelPrivate *d)
    : d(d)
{
    thread = new Thread(this);
    moveToThread(thread);
    thread->start();
}

const int PrepareChunkSize = 100;
const int FilterChunkSize = 2000;

ImageFilterModel::ImageFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      d_ptr(new ImageFilterModelPrivate)
{
    d_ptr->init(this);
}

ImageFilterModel::ImageFilterModel(ImageFilterModelPrivate &dd, QObject *parent)
    : QSortFilterProxyModel(parent),
      d_ptr(&dd)
{
    d_ptr->init(this);
}

void ImageFilterModelPrivate::init(ImageFilterModel *_q)
{
    q = _q;

    updateFilterTimer  = new QTimer(this);
    updateFilterTimer->setSingleShot(true);
    updateFilterTimer->setInterval(250);

    connect(updateFilterTimer, SIGNAL(timeout()),
            q, SLOT(slotUpdateFilter()));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset &)),
            q, SLOT(slotImageChange(const ImageChangeset &)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(const ImageTagChangeset &)),
            q, SLOT(slotImageTagChange(const ImageTagChangeset &)));

    // inter-thread redirection
    qRegisterMetaType<ImageFilterModelTodoPackage>("ImageFilterModelTodoPackage");
}

ImageFilterModel::~ImageFilterModel()
{
    Q_D(ImageFilterModel);
    delete d;
}

void ImageFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    // made it protected, only setSourceImageModel is public
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void ImageFilterModel::setSourceImageModel(ImageModel *sourceModel)
{
    Q_D(ImageFilterModel);

    if (d->imageModel)
    {
        d->imageModel->unsetPreprocessor(d);
        disconnect(d->imageModel, SIGNAL(modelReset()),
                   this, SLOT(slotModelReset()));
        slotModelReset();
    }

    d->imageModel = sourceModel;

    if (d->imageModel)
    {
        d->imageModel->setPreprocessor(d);

        connect(d->imageModel, SIGNAL(preprocess(const QList<ImageInfo> &)),
                d, SLOT(preprocessInfos(const QList<ImageInfo> &)));

        connect(d, SIGNAL(reAddImageInfos(const QList<ImageInfo> &)),
                d->imageModel, SLOT(reAddImageInfos(const QList<ImageInfo> &)));

        connect(d->imageModel, SIGNAL(modelReset()),
                this, SLOT(slotModelReset()));
    }

    setSourceModel(d->imageModel);
}

void ImageFilterModel::setDayFilter(const QList<QDateTime>& days)
{
    Q_D(ImageFilterModel);
    d->filter.setDayFilter(days);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setTagFilter(const QList<int>& tags, ImageFilterSettings::MatchingCondition matchingCond,
                               bool showUnTagged)
{
    Q_D(ImageFilterModel);
    d->filter.setTagFilter(tags, matchingCond, showUnTagged);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setRatingFilter(int rating, ImageFilterSettings::RatingCondition ratingCond)
{
    Q_D(ImageFilterModel);
    d->filter.setRatingFilter(rating, ratingCond);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setMimeTypeFilter(int mimeTypeFilter)
{
    Q_D(ImageFilterModel);
    d->filter.setMimeTypeFilter(mimeTypeFilter);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setTextFilter(const SearchTextSettings& settings)
{
    Q_D(ImageFilterModel);
    d->filter.setTextFilter(settings);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setImageFilterSettings(const ImageFilterSettings &settings)
{
    Q_D(ImageFilterModel);

    {
        QMutexLocker lock(&d->mutex);
        d->version++;
        d->sentOut = 0;
        // do not touch sentOutForReAdd here
        d->filter = settings;
        d->filterCopy = settings;

        d->needPrepareComments = settings.isFilteringByText();
        d->needPrepareTags     = settings.isFilteringByTags();
        d->needPrepare         = d->needPrepareComments || d->needPrepareTags;
    }

    d->filterResults.clear();
    if (d->imageModel)
        d->infosToProcess(d->imageModel->imageInfos(), false);
}

void ImageFilterModel::slotUpdateFilter()
{
    Q_D(ImageFilterModel);

    d->filterResults.clear();
    if (d->imageModel)
        d->infosToProcess(d->imageModel->imageInfos(), false);
}

ImageFilterSettings ImageFilterModel::imageFilterSettings() const
{
    Q_D(const ImageFilterModel);
    return d->filter;
}

void ImageFilterModel::slotModelReset()
{
    Q_D(ImageFilterModel);
    d->filterResults.clear();
    d->sentOut = 0;
    // discard all packages that are marked as send out for re-add
    d->lastDiscardVersion = d->version;
    d->sentOutForReAdd = 0;
    // cause all packages on the way to be discarded
    d->version++;
}

bool ImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_D(const ImageFilterModel);
    if (source_parent.isValid())
        return false;

    qlonglong id = d->imageModel->imageId(source_row);
    QHash<qlonglong, bool>::const_iterator it = d->filterResults.find(id);
    if (it != d->filterResults.end())
        return it.value();

    // usually done in thread and cache, unless source model changed
    return d->filter.matches(d->imageModel->imageInfo(source_row));
}

void ImageFilterModelPrivate::preprocessInfos(const QList<ImageInfo> &infos)
{
    infosToProcess(infos, true);
}

void ImageFilterModelPrivate::setupWorkers()
{
    preparer = new ImageFilterModelPreparer(this);
    filterer = new ImageFilterModelFilterer(this);

    // A package in constructed in infosToProcess.
    // Normal flow is infosToProcess -> preparer::process -> filterer::process -> packageFinished.
    // If no preparation is needed, the first step is skipped.
    // If filter version changes, both will discard old package and send them to packageDiscarded.

    connect(this, SIGNAL(packageToPrepare(const ImageFilterModelTodoPackage &)),
            preparer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(this, SIGNAL(packageToFilter(const ImageFilterModelTodoPackage &)),
            filterer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(preparer, SIGNAL(processed(const ImageFilterModelTodoPackage &)),
            filterer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(filterer, SIGNAL(processed(const ImageFilterModelTodoPackage &)),
            this, SLOT(packageFinished(const ImageFilterModelTodoPackage &)));

    connect(preparer, SIGNAL(discarded(const ImageFilterModelTodoPackage &)),
            this, SLOT(packageDiscarded(const ImageFilterModelTodoPackage &)));

    connect(filterer, SIGNAL(discarded(const ImageFilterModelTodoPackage &)),
            this, SLOT(packageDiscarded(const ImageFilterModelTodoPackage &)));
}

void ImageFilterModelPrivate::infosToProcess(const QList<ImageInfo> &infos, bool forReAdd)
{
    // prepare and filter in chunks
    const int size = infos.size();
    const int maxChunkSize = needPrepare ? PrepareChunkSize : FilterChunkSize;
    QList<ImageInfo>::const_iterator it = infos.begin();
    int index = 0;
    while (it != infos.end())
    {
        QVector<ImageInfo> vector(qMin(maxChunkSize, size - index));
        QList<ImageInfo>::const_iterator end = it + vector.size();
        qCopy(it, end, vector.begin());
        it = end;
        index += vector.size();

        sentOut++;
        if (forReAdd)
            sentOutForReAdd++;

        if (needPrepare)
            emit packageToPrepare(ImageFilterModelTodoPackage(vector, version, forReAdd));
        else
            emit packageToFilter(ImageFilterModelTodoPackage(vector, version, forReAdd));
    }
}

void ImageFilterModelPrivate::packageFinished(const ImageFilterModelTodoPackage &package)
{
    // check if it got discarded on the journey
    if (package.version != version)
    {
        packageDiscarded(package);
        return;
    }

    // incorporate result
    QHash<qlonglong, bool>::const_iterator it = package.filterResults.begin();
    for (; it != package.filterResults.end(); it++)
        filterResults.insert(it.key(), it.value());

    // re-add if necessary
    if (package.isForReAdd)
        emit reAddImageInfos(package.infos.toList());

    // decrement counters
    sentOut--;
    if (package.isForReAdd)
        sentOutForReAdd--;

    // If all packages have returned, filtered and readded,
    // and there is need to tell the filter result to the view, do that
    if (sentOut == 0 && sentOutForReAdd == 0 && version != lastFilteredVersion)
    {
        lastFilteredVersion = version;
        q->invalidate();
    }
}

void ImageFilterModelPrivate::packageDiscarded(const ImageFilterModelTodoPackage &package)
{
    // Either, the model was reset, or the filter changed
    // In the former case throw all away, in the latter case, recycle
    if (package.version > lastDiscardVersion)
    {
        // Do not increment sentOut or sentOutForReAdd here: it was not decremented!

        if (needPrepare)
            emit packageToPrepare(ImageFilterModelTodoPackage(package.infos, version, package.isForReAdd));
        else
            emit packageToFilter(ImageFilterModelTodoPackage(package.infos, version, package.isForReAdd));
    }
}

void ImageFilterModelPreparer::process(ImageFilterModelTodoPackage package)
{
    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    // get thread-local copy
    bool needPrepareTags, needPrepareComments;
    {
        QMutexLocker lock(&d->mutex);
        needPrepareTags = d->needPrepareTags;
        needPrepareComments = d->needPrepareComments;
    }

    //TODO: Make efficient!!
    if (needPrepareComments)
    {
        foreach(const ImageInfo &info, package.infos)
        {
            info.comment();
        }
    }

    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    //TODO: Make efficient!!
    if (needPrepareTags)
    {
        foreach(const ImageInfo &info, package.infos)
        {
            info.tagIds();
        }
    }

    emit processed(package);
}

void ImageFilterModelFilterer::process(ImageFilterModelTodoPackage package)
{
    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    // get thread-local copy
    ImageFilterSettings localFilter;
    {
        QMutexLocker lock(&d->mutex);
        localFilter = d->filterCopy;
    }

    foreach (const ImageInfo &info, package.infos)
    {
        package.filterResults[info.id()] = localFilter.matches(info);
    }

    emit processed(package);
}

void ImageFilterModel::slotImageTagChange(const ImageTagChangeset &changeset)
{
    Q_D(ImageFilterModel);

    if (!d->imageModel || d->imageModel->isEmpty())
        return;

    // already scheduled to re-filter?
    if (d->updateFilterTimer->isActive())
        return;

    // do we filter at all?
    if (!d->filter.isFilteringByTags() && !d->filter.isFilteringByText())
        return;

    // is one of our images affected?
    foreach(qlonglong id, changeset.ids())
    {
        // if one matching image id is found, trigger a refresh
        if (d->imageModel->hasImage(id))
        {
            d->updateFilterTimer->start();
            return;
        }
    }
}

void ImageFilterModel::slotImageChange(const ImageChangeset &changeset)
{
    Q_D(ImageFilterModel);

    if (!d->imageModel || d->imageModel->isEmpty())
        return;

    // already scheduled to re-filter?
    if (d->updateFilterTimer->isActive())
        return;

    // do we filter at all?
    if (!d->filter.isFiltering())
        return;

    // is one of the values affected that we filter by?
    DatabaseFields::Set set = changeset.changes();
    if (!(set & DatabaseFields::CreationDate) && !(set & DatabaseFields::Rating)
        && !(set & DatabaseFields::Category) && !(set & DatabaseFields::Format)
        && !(set & DatabaseFields::Name) && !(set & DatabaseFields::Comment))
        return;

    // is one of our images affected?
    foreach(qlonglong id, changeset.ids())
    {
        // if one matching image id is found, trigger a refresh
        if (d->imageModel->hasImage(id))
        {
            d->updateFilterTimer->start();
            return;
        }
    }
}

}
