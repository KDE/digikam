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

#include "imagefiltermodel.h"
#include "imagefiltermodelpriv.h"
#include "imagefiltermodelthreads.h"

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "imageinfolist.h"
#include "imagemodel.h"

namespace Digikam
{

ImageSortFilterModel::ImageSortFilterModel(QObject* parent)
    : DCategorizedSortFilterProxyModel(parent), m_chainedModel(0)
{
}

void ImageSortFilterModel::setSourceImageModel(ImageModel* source)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceImageModel(source);
    }
    else
    {
        setDirectSourceImageModel(source);
    }
}

void ImageSortFilterModel::setSourceFilterModel(ImageSortFilterModel* source)
{
    if (source)
    {
        ImageModel* const model = sourceImageModel();

        if (model)
        {
            source->setSourceImageModel(model);
        }
    }

    m_chainedModel = source;
    setSourceModel(source);
}

void ImageSortFilterModel::setDirectSourceImageModel(ImageModel* model)
{
    setSourceModel(model);
}

void ImageSortFilterModel::setSourceModel(QAbstractItemModel* model)
{
    // made it protected, only setSourceImageModel is public
    DCategorizedSortFilterProxyModel::setSourceModel(model);
}

ImageModel* ImageSortFilterModel::sourceImageModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceImageModel();
    }

    return static_cast<ImageModel*>(sourceModel());
}

ImageSortFilterModel* ImageSortFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

ImageFilterModel* ImageSortFilterModel::imageFilterModel() const
{
    // reimplemented in ImageFilterModel
    if (m_chainedModel)
    {
        return m_chainedModel->imageFilterModel();
    }

    return 0;
}

QModelIndex ImageSortFilterModel::mapToSourceImageModel(const QModelIndex& index) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceImageModel(mapToSource(index));
    }

    return mapToSource(index);
}

QModelIndex ImageSortFilterModel::mapFromSourceImageModel(const QModelIndex& albummodel_index) const
{
    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceImageModel(albummodel_index));
    }

    return mapFromSource(albummodel_index);
}


QModelIndex ImageSortFilterModel::mapFromDirectSourceToSourceImageModel(const QModelIndex& sourceModel_index) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceImageModel(sourceModel_index);
    }
    return sourceModel_index;
}

// -------------- Convenience mappers -------------------------------------------------------------------

QList<QModelIndex> ImageSortFilterModel::mapListToSource(const QList<QModelIndex>& indexes) const
{
    QList<QModelIndex> sourceIndexes;
    foreach(const QModelIndex& index, indexes)
    {
        sourceIndexes << mapToSourceImageModel(index);
    }
    return sourceIndexes;
}

QList<QModelIndex> ImageSortFilterModel::mapListFromSource(const QList<QModelIndex>& sourceIndexes) const
{
    QList<QModelIndex> indexes;
    foreach(const QModelIndex& index, sourceIndexes)
    {
        indexes << mapFromSourceImageModel(index);
    }
    return indexes;
}

ImageInfo ImageSortFilterModel::imageInfo(const QModelIndex& index) const
{
    return sourceImageModel()->imageInfo(mapToSourceImageModel(index));
}

qlonglong ImageSortFilterModel::imageId(const QModelIndex& index) const
{
    return sourceImageModel()->imageId(mapToSourceImageModel(index));
}

QList<ImageInfo> ImageSortFilterModel::imageInfos(const QList<QModelIndex>& indexes) const
{
    QList<ImageInfo> infos;
    ImageModel* const model = sourceImageModel();

    foreach(const QModelIndex& index, indexes)
    {
        infos << model->imageInfo(mapToSourceImageModel(index));
    }

    return infos;
}

QList<qlonglong> ImageSortFilterModel::imageIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;
    ImageModel* const model = sourceImageModel();

    foreach(const QModelIndex& index, indexes)
    {
        ids << model->imageId(mapToSourceImageModel(index));
    }

    return ids;
}

QModelIndex ImageSortFilterModel::indexForPath(const QString& filePath) const
{
    return mapFromSourceImageModel(sourceImageModel()->indexForPath(filePath));
}

QModelIndex ImageSortFilterModel::indexForImageInfo(const ImageInfo& info) const
{
    return mapFromSourceImageModel(sourceImageModel()->indexForImageInfo(info));
}

QModelIndex ImageSortFilterModel::indexForImageId(qlonglong id) const
{
    return mapFromSourceImageModel(sourceImageModel()->indexForImageId(id));
}

QList<ImageInfo> ImageSortFilterModel::imageInfosSorted() const
{
    QList<ImageInfo>  infos;
    const int         size  = rowCount();
    ImageModel* const model = sourceImageModel();

    for (int i=0; i<size; ++i)
    {
        infos << model->imageInfo(mapToSourceImageModel(index(i, 0)));
    }

    return infos;
}

// --------------------------------------------------------------------------------------------

ImageFilterModel::ImageFilterModel(QObject* parent)
    : ImageSortFilterModel(parent),
      d_ptr(new ImageFilterModelPrivate)
{
    d_ptr->init(this);
}

ImageFilterModel::ImageFilterModel(ImageFilterModelPrivate& dd, QObject* parent)
    : ImageSortFilterModel(parent),
      d_ptr(&dd)
{
    d_ptr->init(this);
}

ImageFilterModel::~ImageFilterModel()
{
    Q_D(ImageFilterModel);
    delete d;
}

void ImageFilterModel::setDirectSourceImageModel(ImageModel* sourceModel)
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

        connect(d->imageModel, SIGNAL(preprocess(QList<ImageInfo>,QList<QVariant>)),
                d, SLOT(preprocessInfos(QList<ImageInfo>,QList<QVariant>)));

        connect(d->imageModel, SIGNAL(processAdded(QList<ImageInfo>,QList<QVariant>)),
                d, SLOT(processAddedInfos(QList<ImageInfo>,QList<QVariant>)));

        connect(d, SIGNAL(reAddImageInfos(QList<ImageInfo>,QList<QVariant>)),
                d->imageModel, SLOT(reAddImageInfos(QList<ImageInfo>,QList<QVariant>)));

        connect(d, SIGNAL(reAddingFinished()),
                d->imageModel, SLOT(reAddingFinished()));

        connect(d->imageModel, SIGNAL(modelReset()),
                this, SLOT(slotModelReset()));

        connect(d->imageModel, SIGNAL(imageChange(ImageChangeset,QItemSelection)),
                this, SLOT(slotImageChange(ImageChangeset)));

        connect(d->imageModel, SIGNAL(imageTagChange(ImageTagChangeset,QItemSelection)),
                this, SLOT(slotImageTagChange(ImageTagChangeset)));
    }

    setSourceModel(d->imageModel);
}

QVariant ImageFilterModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ImageFilterModel);

    if (!index.isValid())
    {
        return QVariant();
    }

    switch (role)
    {
            // Attention: This breaks should there ever be another filter model between this and the ImageModel

        case DCategorizedSortFilterProxyModel::CategoryDisplayRole:
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
            return d->imageModel->imageInfoRef(mapToSource(index)).format();
        case GroupIsOpenRole:
            return d->groupFilter.isAllOpen() ||
                   d->groupFilter.isOpen(d->imageModel->imageInfoRef(mapToSource(index)).id());
        case ImageFilterModelPointerRole:
            return QVariant::fromValue(const_cast<ImageFilterModel*>(this));
    }

    return DCategorizedSortFilterProxyModel::data(index, role);
}

ImageFilterModel* ImageFilterModel::imageFilterModel() const
{
    return const_cast<ImageFilterModel*>(this);
}

DatabaseFields::Set ImageFilterModel::suggestedWatchFlags() const
{
    DatabaseFields::Set watchFlags;
    watchFlags |= DatabaseFields::Name   | DatabaseFields::FileSize     | DatabaseFields::ModificationDate;
    watchFlags |= DatabaseFields::Rating | DatabaseFields::CreationDate | DatabaseFields::Orientation |
                  DatabaseFields::Width  | DatabaseFields::Height;
    watchFlags |= DatabaseFields::Comment;
    watchFlags |= DatabaseFields::ImageRelations;
    return watchFlags;
}

// -------------- Filter settings --------------

void ImageFilterModel::setDayFilter(const QList<QDateTime>& days)
{
    Q_D(ImageFilterModel);
    d->filter.setDayFilter(days);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setTagFilter(const QList<int>& includedTags, const QList<int>& excludedTags,
                                    ImageFilterSettings::MatchingCondition matchingCond,
                                    bool showUnTagged, const QList<int>& clTagIds, const QList<int>& plTagIds)
{
    Q_D(ImageFilterModel);
    d->filter.setTagFilter(includedTags, excludedTags, matchingCond, showUnTagged, clTagIds, plTagIds);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setRatingFilter(int rating, ImageFilterSettings::RatingCondition ratingCond, bool isUnratedExcluded)
{
    Q_D(ImageFilterModel);
    d->filter.setRatingFilter(rating, ratingCond, isUnratedExcluded);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setUrlWhitelist(const QList<QUrl> urlList, const QString& id)
{
    Q_D(ImageFilterModel);
    d->filter.setUrlWhitelist(urlList, id);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setIdWhitelist(const QList<qlonglong>& idList, const QString& id)
{
    Q_D(ImageFilterModel);
    d->filter.setIdWhitelist(idList, id);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setMimeTypeFilter(int mimeTypeFilter)
{
    Q_D(ImageFilterModel);
    d->filter.setMimeTypeFilter(mimeTypeFilter);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setGeolocationFilter(const ImageFilterSettings::GeolocationCondition& condition)
{
    Q_D(ImageFilterModel);
    d->filter.setGeolocationFilter(condition);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setTextFilter(const SearchTextFilterSettings& settings)
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
        d->filter              = settings;
        d->filterCopy          = settings;
        d->versionFilterCopy   = d->versionFilter;
        d->groupFilterCopy     = d->groupFilter;

        d->needPrepareComments = settings.isFilteringByText();
        d->needPrepareTags     = settings.isFilteringByTags();
        d->needPrepareGroups   = true;
        d->needPrepare         = d->needPrepareComments || d->needPrepareTags || d->needPrepareGroups;

        d->hasOneMatch         = false;
        d->hasOneMatchForText  = false;
    }

    d->filterResults.clear();

    //d->categoryCountHashInt.clear();
    //d->categoryCountHashString.clear();
    if (d->imageModel)
    {
        d->infosToProcess(d->imageModel->imageInfos());
    }

    emit filterSettingsChanged(settings);
}

void ImageFilterModel::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    Q_D(ImageFilterModel);
    d->versionFilter.setVersionManagerSettings(settings);
    setVersionImageFilterSettings(d->versionFilter);
}

void ImageFilterModel::setExceptionList(const QList<qlonglong>& idList, const QString& id)
{
    Q_D(ImageFilterModel);
    d->versionFilter.setExceptionList(idList, id);
    setVersionImageFilterSettings(d->versionFilter);
}

void ImageFilterModel::setVersionImageFilterSettings(const VersionImageFilterSettings& settings)
{
    Q_D(ImageFilterModel);
    d->versionFilter = settings;
    slotUpdateFilter();
}

bool ImageFilterModel::isGroupOpen(qlonglong group) const
{
    Q_D(const ImageFilterModel);
    return d->groupFilter.isOpen(group);
}

bool ImageFilterModel::isAllGroupsOpen() const
{
    Q_D(const ImageFilterModel);
    return d->groupFilter.isAllOpen();
}

void ImageFilterModel::setGroupOpen(qlonglong group, bool open)
{
    Q_D(ImageFilterModel);
    d->groupFilter.setOpen(group, open);
    setGroupImageFilterSettings(d->groupFilter);
}

void ImageFilterModel::toggleGroupOpen(qlonglong group)
{
    setGroupOpen(group, !isGroupOpen(group));
}

void ImageFilterModel::setAllGroupsOpen(bool open)
{
    Q_D(ImageFilterModel);
    d->groupFilter.setAllOpen(open);
    setGroupImageFilterSettings(d->groupFilter);
}

void ImageFilterModel::setGroupImageFilterSettings(const GroupImageFilterSettings& settings)
{
    Q_D(ImageFilterModel);
    d->groupFilter = settings;
    slotUpdateFilter();
}

void ImageFilterModel::slotUpdateFilter()
{
    Q_D(ImageFilterModel);
    setImageFilterSettings(d->filter);
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

VersionImageFilterSettings ImageFilterModel::versionImageFilterSettings() const
{
    Q_D(const ImageFilterModel);
    return d->versionFilter;
}

GroupImageFilterSettings ImageFilterModel::groupImageFilterSettings() const
{
    Q_D(const ImageFilterModel);
    return d->groupFilter;
}

void ImageFilterModel::slotModelReset()
{
    Q_D(ImageFilterModel);
    {
        QMutexLocker lock(&d->mutex);
        // discard all packages on the way that are marked as send out for re-add
        d->lastDiscardVersion = d->version;
        d->sentOutForReAdd    = 0;
        // discard all packages on the way
        d->version++;
        d->sentOut            = 0;

        d->hasOneMatch        = false;
        d->hasOneMatchForText = false;
    }
    d->filterResults.clear();
}

bool ImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const ImageFilterModel);

    if (source_parent.isValid())
    {
        return false;
    }

    qlonglong id                              = d->imageModel->imageId(source_row);
    QHash<qlonglong, bool>::const_iterator it = d->filterResults.constFind(id);

    if (it != d->filterResults.constEnd())
    {
        return it.value();
    }

    // usually done in thread and cache, unless source model changed
    ImageInfo info = d->imageModel->imageInfo(source_row);
    bool match     = d->filter.matches(info);
    match          = match ? d->versionFilter.matches(info) : false;

    return match ? d->groupFilter.matches(info) : false;
}

void ImageFilterModel::setSendImageInfoSignals(bool sendSignals)
{
    if (sendSignals)
    {
        connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
    else
    {
        disconnect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        disconnect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
}

void ImageFilterModel::slotRowsInserted(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ImageInfo> infos;

    for (int i=start; i<=end; ++i)
    {
        infos << imageInfo(index(i, 0));
    }

    emit imageInfosAdded(infos);
}

void ImageFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ImageInfo> infos;

    for (int i=start; i<=end; ++i)
    {
        infos << imageInfo(index(i, 0));
    }

    emit imageInfosAboutToBeRemoved(infos);
}

// -------------- Threaded preparation & filtering --------------

void ImageFilterModel::addPrepareHook(ImageFilterModelPrepareHook* hook)
{
    Q_D(ImageFilterModel);
    QMutexLocker lock(&d->mutex);
    d->prepareHooks << hook;
}

void ImageFilterModel::removePrepareHook(ImageFilterModelPrepareHook* hook)
{
    Q_D(ImageFilterModel);
    QMutexLocker lock(&d->mutex);
    d->prepareHooks.removeAll(hook);
}

void ImageFilterModelPreparer::process(ImageFilterModelTodoPackage package)
{
    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    // get thread-local copy
    bool needPrepareTags, needPrepareComments, needPrepareGroups;
    QList<ImageFilterModelPrepareHook*> prepareHooks;
    {
        QMutexLocker lock(&d->mutex);
        needPrepareTags     = d->needPrepareTags;
        needPrepareComments = d->needPrepareComments;
        needPrepareGroups   = d->needPrepareGroups;
        prepareHooks        = d->prepareHooks;
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

    // The downside of QVector: At some point, we may need a QList for an API.
    // Nonetheless, QList and ImageInfo is fast. We could as well
    // reimplement ImageInfoList to ImageInfoVector (internally with templates?)
    ImageInfoList infoList;

    if (needPrepareTags || needPrepareGroups)
    {
        infoList = package.infos.toList();
    }

    if (needPrepareTags)
    {
        infoList.loadTagIds();
    }

    if (needPrepareGroups)
    {
        infoList.loadGroupImageIds();
    }

    foreach(ImageFilterModelPrepareHook* hook, prepareHooks)
    {
        hook->prepare(package.infos);
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
    ImageFilterSettings        localFilter;
    VersionImageFilterSettings localVersionFilter;
    GroupImageFilterSettings   localGroupFilter;
    bool                       hasOneMatch;
    bool                       hasOneMatchForText;
    {
        QMutexLocker lock(&d->mutex);
        localFilter        = d->filterCopy;
        localVersionFilter = d->versionFilterCopy;
        localGroupFilter   = d->groupFilterCopy;
        hasOneMatch        = d->hasOneMatch;
        hasOneMatchForText = d->hasOneMatchForText;
    }

    // Actual filtering. The variants to spare checking hasOneMatch over and over again.
    if (hasOneMatch && hasOneMatchForText)
    {
        foreach(const ImageInfo& info, package.infos)
        {
            package.filterResults[info.id()] = localFilter.matches(info)        &&
                                               localVersionFilter.matches(info) &&
                                               localGroupFilter.matches(info);
        }
    }
    else if (hasOneMatch)
    {
        bool matchForText;

        foreach(const ImageInfo& info, package.infos)
        {
            package.filterResults[info.id()] = localFilter.matches(info, &matchForText) &&
                                               localVersionFilter.matches(info)         &&
                                               localGroupFilter.matches(info);

            if (matchForText)
            {
                hasOneMatchForText = true;
            }
        }
    }
    else
    {
        bool result, matchForText;

        foreach(const ImageInfo& info, package.infos)
        {
            result                           = localFilter.matches(info, &matchForText) &&
                                               localVersionFilter.matches(info)         &&
                                               localGroupFilter.matches(info);
            package.filterResults[info.id()] = result;

            if (result)
            {
                hasOneMatch = true;
            }

            if (matchForText)
            {
                hasOneMatchForText = true;
            }
        }
    }

    if (checkVersion(package))
    {
        QMutexLocker lock(&d->mutex);
        d->hasOneMatch        = hasOneMatch;
        d->hasOneMatchForText = hasOneMatchForText;
    }

    emit processed(package);
}

// -------------- Sorting and Categorization -------------------------------------------------------

void ImageFilterModel::setImageSortSettings(const ImageSortSettings& sorter)
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

void ImageFilterModel::setCategorizationSortOrder(ImageSortSettings::SortOrder order)
{
    Q_D(ImageFilterModel);
    d->sorter.setCategorizationSortOrder(order);
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

void ImageFilterModel::setStringTypeNatural(bool natural)
{
    Q_D(ImageFilterModel);
    d->sorter.setStringTypeNatural(natural);
    setImageSortSettings(d->sorter);
}

int ImageFilterModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ImageFilterModel);

    if (!d->sorter.isCategorized())
    {
        return 0;
    }

    if (!left.isValid() || !right.isValid())
    {
        return -1;
    }

    const ImageInfo& leftInfo  = d->imageModel->imageInfoRef(left);
    const ImageInfo& rightInfo = d->imageModel->imageInfoRef(right);

    // Check grouping
    qlonglong leftGroupImageId = leftInfo.groupImageId();
    qlonglong rightGroupImageId = rightInfo.groupImageId();

    return compareInfosCategories(leftGroupImageId  == -1 ? leftInfo  : ImageInfo(leftGroupImageId),
                                  rightGroupImageId == -1 ? rightInfo : ImageInfo(rightGroupImageId));
}

bool ImageFilterModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ImageFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return true;
    }

    if (left == right)
    {
        return false;
    }

    const ImageInfo& leftInfo  = d->imageModel->imageInfoRef(left);
    const ImageInfo& rightInfo = d->imageModel->imageInfoRef(right);

    if (leftInfo == rightInfo)
    {
        return d->sorter.lessThan(left.data(ImageModel::ExtraDataRole), right.data(ImageModel::ExtraDataRole));
    }

    // Check grouping
    qlonglong leftGroupImageId = leftInfo.groupImageId();
    qlonglong rightGroupImageId = rightInfo.groupImageId();

    // Either no grouping (-1), or same group image, or same image
    if (leftGroupImageId == rightGroupImageId)
    {
        return infosLessThan(leftInfo, rightInfo);
    }

   // We have grouping to handle

    // Is one grouped on the other? Sort behind leader.
    if (leftGroupImageId == rightInfo.id())
    {
        return false;
    }
    if (rightGroupImageId == leftInfo.id())
    {
        return true;
    }

    // Use the group leader for sorting
    return infosLessThan(leftGroupImageId  == -1 ? leftInfo  : ImageInfo(leftGroupImageId),
                         rightGroupImageId == -1 ? rightInfo : ImageInfo(rightGroupImageId));
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
    c[size]    = '\0';
    char* p    = c;
    int number = id;

    for (int i=0; i<size; ++i)
    {
        *p = 'a' + (number & 0xF);
        number >>= 4;
        ++p;
    }

    return QString::fromLatin1(c);
}

QString ImageFilterModel::categoryIdentifier(const ImageInfo& i) const
{
    Q_D(const ImageFilterModel);

    if (!d->sorter.isCategorized())
    {
        return QString();
    }

    qlonglong groupedImageId = i.groupImageId();
    ImageInfo info = groupedImageId == -1 ? i : ImageInfo(groupedImageId);

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

// -------------- Watching changes -----------------------------------------------------------------

void ImageFilterModel::slotImageTagChange(const ImageTagChangeset& changeset)
{
    Q_D(ImageFilterModel);

    if (!d->imageModel || d->imageModel->isEmpty())
    {
        return;
    }

    // already scheduled to re-filter?
    if (d->updateFilterTimer->isActive())
    {
        return;
    }

    // do we filter at all?
    if (!d->versionFilter.isFilteringByTags() &&
        !d->filter.isFilteringByTags()        &&
        !d->filter.isFilteringByText())
    {
        return;
    }

    // is one of our images affected?
    foreach(const qlonglong& id, changeset.ids())
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
    {
        return;
    }

    // already scheduled to re-filter?
    if (d->updateFilterTimer->isActive())
    {
        return;
    }

    // is one of the values affected that we filter or sort by?
    DatabaseFields::Set set = changeset.changes();
    bool sortAffected       = (set & d->sorter.watchFlags());
    bool filterAffected     = (set & d->filter.watchFlags()) || (set & d->groupFilter.watchFlags());

    if (!sortAffected && !filterAffected)
    {
        return;
    }

    // is one of our images affected?
    bool imageAffected = false;

    foreach(const qlonglong& id, changeset.ids())
    {
        // if one matching image id is found, trigger a refresh
        if (d->imageModel->hasImage(id))
        {
            imageAffected = true;
            break;
        }
    }

    if (!imageAffected)
    {
        return;
    }

    if (filterAffected)
    {
        d->updateFilterTimer->start();
    }
    else
    {
        invalidate();    // just resort, reuse filter results
    }
}

// -------------------------------------------------------------------------------------------------------

NoDuplicatesImageFilterModel::NoDuplicatesImageFilterModel(QObject* parent)
    : ImageSortFilterModel(parent)
{
}

bool NoDuplicatesImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if (index.data(ImageModel::ExtraDataDuplicateCount).toInt() <= 1)
    {
        return true;
    }

    QModelIndex previousIndex = sourceModel()->index(source_row - 1, 0, source_parent);

    if (!previousIndex.isValid())
    {
        return true;
    }

    if (sourceImageModel()->imageId(mapFromDirectSourceToSourceImageModel(index)) == sourceImageModel()->imageId(mapFromDirectSourceToSourceImageModel(previousIndex)))
    {
        return false;
    }
    return true;
}

/*
void NoDuplicatesImageFilterModel::setSourceModel(QAbstractItemModel* model)
{
    if (sourceModel())
    {
    }

    ImageSortFilterModel::setSourceModel(model);

    if (sourceModel())
    {
        connect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
}

void NoDuplicatesImageFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int begin, int end)
{
    bool needInvalidate = false;

    for (int i = begin; i<=end; ++i)
    {
        QModelIndex index = sourceModel()->index(i, 0, parent);

        // filtered out by us?
        if (!mapFromSource(index).isValid())
        {
            continue;
        }

        QModelIndex sourceIndex = mapFromDirectSourceToSourceImageModel(index);
        qlonglong id = sourceImageModel()->imageId(sourceIndex);

        if (sourceImageModel()->numberOfIndexesForImageId(id) > 1)
        {
            needInvalidate = true;
        }
    }
}*/

} // namespace Digikam
