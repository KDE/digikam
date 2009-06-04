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

// KDE includes

#include <kdebug.h>
#include <kstringhandler.h>

// Local includes

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
    updateFilterTimer     = 0;
    needPrepare           = false;
    needPrepareComments   = false;
    needPrepareTags       = false;
    preparer              = 0;
    filterer              = 0;
    hasOneMatch           = false;
    hasOneMatchForText    = false;

    setupWorkers();
}

ImageFilterModelPrivate::~ImageFilterModelPrivate()
{
    // facilitate thread stopping
    ++version;
    preparer->shutDown();
    filterer->shutDown();
    delete preparer;
    delete filterer;
}

ImageFilterModelWorker::ImageFilterModelWorker(ImageFilterModelPrivate *d)
    : d(d) // do not install d as QObject parent, moveToThread wont work then
{
    thread = new Thread(this);
    moveToThread(thread);
    thread->start();
}

const int PrepareChunkSize = 100;
const int FilterChunkSize = 2000;

ImageFilterModel::ImageFilterModel(QObject *parent)
                : KCategorizedSortFilterProxyModel(parent),
                  d_ptr(new ImageFilterModelPrivate)
{
    d_ptr->init(this);
}

ImageFilterModel::ImageFilterModel(ImageFilterModelPrivate& dd, QObject *parent)
                : KCategorizedSortFilterProxyModel(parent),
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

ImageModel *ImageFilterModel::sourceModel() const
{
    Q_D(const ImageFilterModel);
    return d->imageModel;
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

        connect(d->imageModel, SIGNAL(imageChange(const ImageChangeset &, const QItemSelection &)),
                this, SLOT(slotImageChange(const ImageChangeset &)));

        connect(d->imageModel, SIGNAL(imageTagChange(const ImageTagChangeset &, const QItemSelection &)),
                this, SLOT(slotImageTagChange(const ImageTagChangeset &)));
    }

    setSourceModel(d->imageModel);
}

QVariant ImageFilterModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ImageFilterModel);
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
            return categoryIdentifier(d->imageModel->imageInfoRef(mapToSource(index)));
        case CategorizationModeRole:
            return d->sorter.categorizationMode;
        case SortOrderRole:
            return d->sorter.sortRole;
        //case CategoryCountRole:
          //  return categoryCount(d->imageModel->imageInfoRef(mapToSource(index)));
        case CategoryAlbumIdRole:
            return d->imageModel->imageInfoRef(mapToSource(index)).albumId();
        case CategoryFormatRole:
            return d->imageModel->imageInfoRef(mapToSource(index)).albumId();
        case ImageFilterModelPointerRole:
            return QVariant::fromValue(const_cast<ImageFilterModel*>(this));
    }

    return KCategorizedSortFilterProxyModel::data(index, role);
}

DatabaseFields::Set ImageFilterModel::suggestedWatchFlags() const
{
    DatabaseFields::Set watchFlags;
    watchFlags |= DatabaseFields::Name | DatabaseFields::FileSize | DatabaseFields::ModificationDate;
    watchFlags |= DatabaseFields::Rating | DatabaseFields::CreationDate | DatabaseFields::Orientation |
                  DatabaseFields::Width | DatabaseFields::Height;
    watchFlags |= DatabaseFields::Comment;
    return watchFlags;
}

// -------------- Convenience mappers --------------

QList<QModelIndex> ImageFilterModel::mapListToSource(const QList<QModelIndex>& indexes) const
{
    QList<QModelIndex> sourceIndexes;
    foreach (const QModelIndex& index, indexes)
        sourceIndexes << mapToSource(index);
    return sourceIndexes;
}

QList<QModelIndex> ImageFilterModel::mapListFromSource(const QList<QModelIndex>& sourceIndexes) const
{
    QList<QModelIndex> indexes;
    foreach (const QModelIndex& index, sourceIndexes)
        indexes << mapFromSource(index);
    return indexes;
}

ImageInfo ImageFilterModel::imageInfo(const QModelIndex& index) const
{
    Q_D(const ImageFilterModel);
    return d->imageModel->imageInfo(mapToSource(index));
}

qlonglong ImageFilterModel::imageId(const QModelIndex& index) const
{
    Q_D(const ImageFilterModel);
    return d->imageModel->imageId(mapToSource(index));
}

QList<ImageInfo> ImageFilterModel::imageInfos(const QList<QModelIndex>& indexes) const
{
    Q_D(const ImageFilterModel);
    QList<ImageInfo> infos;
    foreach (const QModelIndex& index, indexes)
        infos << d->imageModel->imageInfo(mapToSource(index));
    return infos;
}

QList<qlonglong> ImageFilterModel::imageIds(const QList<QModelIndex>& indexes) const
{
    Q_D(const ImageFilterModel);
    QList<qlonglong> ids;
    foreach (const QModelIndex& index, indexes)
        ids << d->imageModel->imageId(mapToSource(index));
    return ids;
}

QModelIndex ImageFilterModel::indexForPath(const QString& filePath) const
{
    Q_D(const ImageFilterModel);
    return mapFromSource(d->imageModel->indexForPath(filePath));
}

QModelIndex ImageFilterModel::indexForImageInfo(const ImageInfo& info) const
{
    Q_D(const ImageFilterModel);
    return mapFromSource(d->imageModel->indexForImageInfo(info));
}

QModelIndex ImageFilterModel::indexForImageId(qlonglong id) const
{
    Q_D(const ImageFilterModel);
    return mapFromSource(d->imageModel->indexForImageId(id));
}

QList<ImageInfo> ImageFilterModel::imageInfosSorted() const
{
    Q_D(const ImageFilterModel);
    QList<ImageInfo> infos;
    const int size = rowCount();
    for (int i=0; i<size; ++i)
    {
        infos << d->imageModel->imageInfo(mapToSource(index(i, 0)));
    }
    return infos;
}

// -------------- Filter settings --------------

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

void ImageFilterModel::setImageFilterSettings(const ImageFilterSettings& settings)
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

        d->hasOneMatch         = false;
        d->hasOneMatchForText  = false;
    }

    d->filterResults.clear();
    //d->categoryCountHashInt.clear();
    //d->categoryCountHashString.clear();
    if (d->imageModel)
        d->infosToProcess(d->imageModel->imageInfos(), false);
    emit filterSettingsChanged(settings);
}

void ImageFilterModel::slotUpdateFilter()
{
    Q_D(ImageFilterModel);

    {
        QMutexLocker lock(&d->mutex);
        // do not touch version, sentOut or sentOutForReAdd here.
        // filter did not change.
        d->hasOneMatch         = false;
        d->hasOneMatchForText  = false;
    }

    d->filterResults.clear();

    if (d->imageModel)
        d->infosToProcess(d->imageModel->imageInfos(), false);
}

ImageFilterSettings ImageFilterModel::imageFilterSettings() const
{
    Q_D(const ImageFilterModel);
    return d->filter;
}

ImageSortSettings ImageFilterModel::imageSortSettings() const
{
    Q_D(const ImageFilterModel);
    return d->sorter;
}

void ImageFilterModel::slotModelReset()
{
    Q_D(ImageFilterModel);
    {
        QMutexLocker lock(&d->mutex);
        // cause all packages on the way to be discarded
        d->version++;
        d->sentOut = 0;
        // discard all packages that are marked as send out for re-add
        d->lastDiscardVersion = d->version;
        d->sentOutForReAdd = 0;

        d->hasOneMatch = false;
        d->hasOneMatchForText = false;
    }
    d->filterResults.clear();
}

bool ImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const ImageFilterModel);
    if (source_parent.isValid())
        return false;

    qlonglong id = d->imageModel->imageId(source_row);
    QHash<qlonglong, bool>::const_iterator it = d->filterResults.constFind(id);
    if (it != d->filterResults.constEnd())
        return it.value();

    // usually done in thread and cache, unless source model changed
    return d->filter.matches(d->imageModel->imageInfo(source_row));
}

// -------------- Threaded preparation & filtering --------------

void ImageFilterModelPrivate::preprocessInfos(const QList<ImageInfo>& infos)
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

void ImageFilterModelPrivate::infosToProcess(const QList<ImageInfo>& infos, bool forReAdd)
{
    // prepare and filter in chunks
    const int size = infos.size();
    const int maxChunkSize = needPrepare ? PrepareChunkSize : FilterChunkSize;
    QList<ImageInfo>::const_iterator it = infos.constBegin();
    int index = 0;
    while (it != infos.constEnd())
    {
        QVector<ImageInfo> vector(qMin(maxChunkSize, size - index));
        QList<ImageInfo>::const_iterator end = it + vector.size();
        qCopy(it, end, vector.begin());
        it = end;
        index += vector.size();

        ++sentOut;
        if (forReAdd)
            ++sentOutForReAdd;

        if (needPrepare)
            emit packageToPrepare(ImageFilterModelTodoPackage(vector, version, forReAdd));
        else
            emit packageToFilter(ImageFilterModelTodoPackage(vector, version, forReAdd));
    }
}

void ImageFilterModelPrivate::packageFinished(const ImageFilterModelTodoPackage& package)
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
        filterResults.insert(it.key(), it.value());

    // re-add if necessary
    if (package.isForReAdd)
        emit reAddImageInfos(package.infos.toList());

    // decrement counters
    --sentOut;
    if (package.isForReAdd)
        --sentOutForReAdd;

    // If all packages have returned, filtered and readded,
    // and there is need to tell the filter result to the view, do that
    if (sentOut == 0 && sentOutForReAdd == 0 && version != lastFilteredVersion)
    {
        lastFilteredVersion = version;
        q->invalidate();
        emit q->filterMatches(hasOneMatch);
        emit q->filterMatchesForText(hasOneMatchForText);
    }
}

void ImageFilterModelPrivate::packageDiscarded(const ImageFilterModelTodoPackage& package)
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
        foreach(const ImageInfo& info, package.infos)
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
        foreach(const ImageInfo& info, package.infos)
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
    bool hasOneMatch;
    bool hasOneMatchForText;
    {
        QMutexLocker lock(&d->mutex);
        localFilter = d->filterCopy;
        hasOneMatch = d->hasOneMatch;
        hasOneMatchForText = d->hasOneMatchForText;
    }

    // Actual filtering. The variants to spare checking hasOneMatch over and over again.
    if (hasOneMatch && hasOneMatchForText)
    {
        foreach (const ImageInfo& info, package.infos)
        {
            package.filterResults[info.id()] = localFilter.matches(info);
        }
    }
    else if (hasOneMatch)
    {
        bool matchForText;
        foreach (const ImageInfo& info, package.infos)
        {
            package.filterResults[info.id()] = localFilter.matches(info, &matchForText);
            if (matchForText)
                hasOneMatchForText = true;
        }
    }
    else
    {
        bool result, matchForText;
        foreach (const ImageInfo& info, package.infos)
        {
            result = localFilter.matches(info, &matchForText);
            package.filterResults[info.id()] = result;
            if (result)
                hasOneMatch = true;
            if (matchForText)
                hasOneMatchForText = true;
        }
    }
    
    if (checkVersion(package))
    {
        QMutexLocker lock(&d->mutex);
        d->hasOneMatch = hasOneMatchForText;
        d->hasOneMatchForText = hasOneMatchForText;
    }

    emit processed(package);
}

// -------------- Sorting and Categorization --------------

void ImageFilterModel::setImageSortSettings(const ImageSortSettings &sorter)
{
    Q_D(ImageFilterModel);
    d->sorter = sorter;
    setCategorizedModel(d->sorter.categorizationMode != ImageSortSettings::NoCategories);
    invalidate();
}

void ImageFilterModel::setCategorizationMode(ImageSortSettings::CategorizationMode mode)
{
    Q_D(ImageFilterModel);
    d->sorter.setCategorizationMode(mode);
    setImageSortSettings(d->sorter);
}

void ImageFilterModel::setSortRole(ImageSortSettings::SortRole role)
{
    Q_D(ImageFilterModel);
    d->sorter.setSortRole(role);
    setImageSortSettings(d->sorter);
}

void ImageFilterModel::setSortOrder(ImageSortSettings::SortOrder order)
{
    Q_D(ImageFilterModel);
    d->sorter.setSortOrder(order);
    setImageSortSettings(d->sorter);
}

int ImageFilterModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ImageFilterModel);
    if (!left.isValid() || !right.isValid())
        return -1;
    return compareInfosCategories(d->imageModel->imageInfoRef(left), d->imageModel->imageInfoRef(right));
}

bool ImageFilterModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ImageFilterModel);
    if (!left.isValid() || !right.isValid())
        return true;
    return infosLessThan(d->imageModel->imageInfoRef(left), d->imageModel->imageInfoRef(right));
}

int ImageFilterModel::compareInfosCategories(const ImageInfo& left, const ImageInfo& right) const
{
    // Note: reimplemented in ImageAlbumFilterModel
    Q_D(const ImageFilterModel);
    return d->sorter.compareCategories(left, right);
}

// Feel free to optimize. QString::number is 3x slower.
static inline QString fastNumberToString(int id)
{
    const int size = sizeof(int) * 2;
    char c[size+1];
    c[size] = '\0';
    char *p = c;
    int number = id;
    for (int i=0; i<size; i++)
    {
        *p = 'a' + (number & 0xF);
        number >>= 4;
        p++;
    }
    return QString::fromLatin1(c);
}

QString ImageFilterModel::categoryIdentifier(const ImageInfo& info) const
{
    Q_D(const ImageFilterModel);
    switch (d->sorter.categorizationMode)
    {
        case ImageSortSettings::NoCategories:
            return QString();
        case ImageSortSettings::OneCategory:
            return QString();
        case ImageSortSettings::CategoryByAlbum:
            return fastNumberToString(info.albumId());
        case ImageSortSettings::CategoryByFormat:
            return info.format();
        default:
            return QString();
    }
}

bool ImageFilterModel::infosLessThan(const ImageInfo& left, const ImageInfo& right) const
{
    Q_D(const ImageFilterModel);
    return d->sorter.lessThan(left, right);
}

// -------------- Watching changes --------------

void ImageFilterModel::slotImageTagChange(const ImageTagChangeset& changeset)
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

void ImageFilterModel::slotImageChange(const ImageChangeset& changeset)
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

} // namespace Digikam
