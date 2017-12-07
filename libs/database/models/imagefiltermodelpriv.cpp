/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C)      2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C)      2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "imagefiltermodelpriv.h"

// Local includes

#include "digikam_debug.h"
#include "imagefiltermodelthreads.h"

namespace Digikam
{

ImageFilterModel::ImageFilterModelPrivate::ImageFilterModelPrivate()
{
    imageModel            = 0;
    version               = 0;
    lastDiscardVersion    = 0;
    sentOut               = 0;
    sentOutForReAdd       = 0;
    updateFilterTimer     = 0;
    needPrepare           = false;
    needPrepareComments   = false;
    needPrepareTags       = false;
    needPrepareGroups     = false;
    preparer              = 0;
    filterer              = 0;
    hasOneMatch           = false;
    hasOneMatchForText    = false;

    setupWorkers();
}

ImageFilterModel::ImageFilterModelPrivate::~ImageFilterModelPrivate()
{
    // facilitate thread stopping
    ++version;
    preparer->deactivate();
    filterer->deactivate();
    delete preparer;
    delete filterer;
}

void ImageFilterModel::ImageFilterModelPrivate::init(ImageFilterModel* _q)
{
    q = _q;

    updateFilterTimer = new QTimer(this);
    updateFilterTimer->setSingleShot(true);
    updateFilterTimer->setInterval(250);

    connect(updateFilterTimer, SIGNAL(timeout()),
            q, SLOT(slotUpdateFilter()));

    // inter-thread redirection
    qRegisterMetaType<ImageFilterModelTodoPackage>("ImageFilterModelTodoPackage");
}

void ImageFilterModel::ImageFilterModelPrivate::preprocessInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    infosToProcess(infos, extraValues, true);
}

void ImageFilterModel::ImageFilterModelPrivate::processAddedInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    // These have already been added, we just process them afterwards
    infosToProcess(infos, extraValues, false);
}

void ImageFilterModel::ImageFilterModelPrivate::setupWorkers()
{
    preparer = new ImageFilterModelPreparer(this);
    filterer = new ImageFilterModelFilterer(this);

    // A package in constructed in infosToProcess.
    // Normal flow is infosToProcess -> preparer::process -> filterer::process -> packageFinished.
    // If no preparation is needed, the first step is skipped.
    // If filter version changes, both will discard old package and send them to packageDiscarded.

    connect(this, SIGNAL(packageToPrepare(ImageFilterModelTodoPackage)),
            preparer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(this, SIGNAL(packageToFilter(ImageFilterModelTodoPackage)),
            filterer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(preparer, SIGNAL(processed(ImageFilterModelTodoPackage)),
            filterer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(filterer, SIGNAL(processed(ImageFilterModelTodoPackage)),
            this, SLOT(packageFinished(ImageFilterModelTodoPackage)));

    connect(preparer, SIGNAL(discarded(ImageFilterModelTodoPackage)),
            this, SLOT(packageDiscarded(ImageFilterModelTodoPackage)));

    connect(filterer, SIGNAL(discarded(ImageFilterModelTodoPackage)),
            this, SLOT(packageDiscarded(ImageFilterModelTodoPackage)));
}

void ImageFilterModel::ImageFilterModelPrivate::infosToProcess(const QList<ImageInfo>& infos)
{
    infosToProcess(infos, QList<QVariant>(), false);
}

void ImageFilterModel::ImageFilterModelPrivate::infosToProcess(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues, bool forReAdd)
{
    if (infos.isEmpty())
    {
        return;
    }

    filterer->schedule();

    if (needPrepare)
    {
        preparer->schedule();
    }

    Q_ASSERT(extraValues.isEmpty() || infos.size() == extraValues.size());

    // prepare and filter in chunks
    const int size                      = infos.size();
    const int maxChunkSize              = needPrepare ? PrepareChunkSize : FilterChunkSize;
    const bool hasExtraValues           = !extraValues.isEmpty();
    QList<ImageInfo>::const_iterator it = infos.constBegin(), end;
    QList<QVariant>::const_iterator xit = extraValues.constBegin(), xend;
    int index                           = 0;
    QVector<ImageInfo>  infoVector;
    QVector<QVariant> extraValueVector;

    while (it != infos.constEnd())
    {
        const int chunkSize = qMin(maxChunkSize, size - index);
        infoVector.resize(chunkSize);
        end = it + chunkSize;
        std::copy(it, end, infoVector.begin());

        if (hasExtraValues)
        {
            extraValueVector.resize(chunkSize);
            xend = xit + chunkSize;
            std::copy(xit, xend, extraValueVector.begin());
            xit = xend;
        }

        it    = end;
        index += chunkSize;

        ++sentOut;

        if (forReAdd)
        {
            ++sentOutForReAdd;
        }

        if (needPrepare)
        {
            emit packageToPrepare(ImageFilterModelTodoPackage(infoVector, extraValueVector, version, forReAdd));
        }
        else
        {
            emit packageToFilter(ImageFilterModelTodoPackage(infoVector, extraValueVector, version, forReAdd));
        }
    }
}

void ImageFilterModel::ImageFilterModelPrivate::packageFinished(const ImageFilterModelTodoPackage& package)
{
    // check if it got discarded on the journey
    if (package.version != version)
    {
        packageDiscarded(package);
        return;
    }

    // incorporate result
    QHash<qlonglong, bool>::const_iterator it = package.filterResults.constBegin();

    for (; it != package.filterResults.constEnd(); ++it)
    {
        filterResults.insert(it.key(), it.value());
    }

    // re-add if necessary
    if (package.isForReAdd)
    {
        emit reAddImageInfos(package.infos.toList(), package.extraValues.toList());

        if (sentOutForReAdd == 1) // last package
        {
            emit reAddingFinished();
        }
    }

    // decrement counters
    --sentOut;

    if (package.isForReAdd)
    {
        --sentOutForReAdd;
    }

    // If all packages have returned, filtered and readded, and no more are expected,
    // and there is need to tell the filter result to the view, do that
    if (sentOut == 0 && sentOutForReAdd == 0 && !imageModel->isRefreshing())
    {
        q->invalidate(); // use invalidate, not invalidateFilter only. Sorting may have changed as well.
        emit (q->filterMatches(hasOneMatch));
        emit (q->filterMatchesForText(hasOneMatchForText));
        filterer->deactivate();
        preparer->deactivate();
    }
}

void ImageFilterModel::ImageFilterModelPrivate::packageDiscarded(const ImageFilterModelTodoPackage& package)
{
    // Either, the model was reset, or the filter changed
    // In the former case throw all away, in the latter case, recycle
    if (package.version > lastDiscardVersion)
    {
        // Recycle packages: Send again with current version
        // Do not increment sentOut or sentOutForReAdd here: it was not decremented!

        if (needPrepare)
        {
            emit packageToPrepare(ImageFilterModelTodoPackage(package.infos, package.extraValues, version, package.isForReAdd));
        }
        else
        {
            emit packageToFilter(ImageFilterModelTodoPackage(package.infos, package.extraValues, version, package.isForReAdd));
        }
    }
}

} // namespace Digikam
