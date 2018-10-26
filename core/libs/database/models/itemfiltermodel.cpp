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
 * Copyright (C)      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "itemfiltermodel_p.h"
#include "itemfiltermodelthreads.h"

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "iteminfolist.h"
#include "itemmodel.h"

namespace Digikam
{

ImageSortFilterModel::ImageSortFilterModel(QObject* const parent)
    : DCategorizedSortFilterProxyModel(parent),
      m_chainedModel(0)
{
}

void ImageSortFilterModel::setSourceItemModel(ItemModel* const source)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceItemModel(source);
    }
    else
    {
        setDirectSourceItemModel(source);
    }
}

void ImageSortFilterModel::setSourceFilterModel(ImageSortFilterModel* const source)
{
    if (source)
    {
        ItemModel* const model = sourceItemModel();

        if (model)
        {
            source->setSourceItemModel(model);
        }
    }

    m_chainedModel = source;
    setSourceModel(source);
}

void ImageSortFilterModel::setDirectSourceItemModel(ItemModel* const model)
{
    setSourceModel(model);
}

void ImageSortFilterModel::setSourceModel(QAbstractItemModel* const model)
{
    // made it protected, only setSourceItemModel is public
    DCategorizedSortFilterProxyModel::setSourceModel(model);
}

ItemModel* ImageSortFilterModel::sourceItemModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceItemModel();
    }

    return static_cast<ItemModel*>(sourceModel());
}

ImageSortFilterModel* ImageSortFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

ItemFilterModel* ImageSortFilterModel::imageFilterModel() const
{
    // reimplemented in ItemFilterModel
    if (m_chainedModel)
    {
        return m_chainedModel->imageFilterModel();
    }

    return 0;
}

QModelIndex ImageSortFilterModel::mapToSourceItemModel(const QModelIndex& index) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceItemModel(mapToSource(index));
    }

    return mapToSource(index);
}

QModelIndex ImageSortFilterModel::mapFromSourceItemModel(const QModelIndex& albummodel_index) const
{
    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceItemModel(albummodel_index));
    }

    return mapFromSource(albummodel_index);
}


QModelIndex ImageSortFilterModel::mapFromDirectSourceToSourceItemModel(const QModelIndex& sourceModel_index) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceItemModel(sourceModel_index);
    }

    return sourceModel_index;
}

// -------------- Convenience mappers -------------------------------------------------------------------

QList<QModelIndex> ImageSortFilterModel::mapListToSource(const QList<QModelIndex>& indexes) const
{
    QList<QModelIndex> sourceIndexes;

    foreach (const QModelIndex& index, indexes)
    {
        sourceIndexes << mapToSourceItemModel(index);
    }

    return sourceIndexes;
}

QList<QModelIndex> ImageSortFilterModel::mapListFromSource(const QList<QModelIndex>& sourceIndexes) const
{
    QList<QModelIndex> indexes;

    foreach (const QModelIndex& index, sourceIndexes)
    {
        indexes << mapFromSourceItemModel(index);
    }

    return indexes;
}

ItemInfo ImageSortFilterModel::imageInfo(const QModelIndex& index) const
{
    return sourceItemModel()->imageInfo(mapToSourceItemModel(index));
}

qlonglong ImageSortFilterModel::imageId(const QModelIndex& index) const
{
    return sourceItemModel()->imageId(mapToSourceItemModel(index));
}

QList<ItemInfo> ImageSortFilterModel::imageInfos(const QList<QModelIndex>& indexes) const
{
    QList<ItemInfo> infos;
    ItemModel* const model = sourceItemModel();

    foreach (const QModelIndex& index, indexes)
    {
        infos << model->imageInfo(mapToSourceItemModel(index));
    }

    return infos;
}

QList<qlonglong> ImageSortFilterModel::imageIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;
    ItemModel* const model = sourceItemModel();

    foreach (const QModelIndex& index, indexes)
    {
        ids << model->imageId(mapToSourceItemModel(index));
    }

    return ids;
}

QModelIndex ImageSortFilterModel::indexForPath(const QString& filePath) const
{
    return mapFromSourceItemModel(sourceItemModel()->indexForPath(filePath));
}

QModelIndex ImageSortFilterModel::indexForItemInfo(const ItemInfo& info) const
{
    return mapFromSourceItemModel(sourceItemModel()->indexForItemInfo(info));
}

QModelIndex ImageSortFilterModel::indexForImageId(qlonglong id) const
{
    return mapFromSourceItemModel(sourceItemModel()->indexForImageId(id));
}

QList<ItemInfo> ImageSortFilterModel::imageInfosSorted() const
{
    QList<ItemInfo>  infos;
    const int         size  = rowCount();
    ItemModel* const model = sourceItemModel();

    for (int i = 0 ; i < size ; ++i)
    {
        infos << model->imageInfo(mapToSourceItemModel(index(i, 0)));
    }

    return infos;
}

// --------------------------------------------------------------------------------------------

ItemFilterModel::ItemFilterModel(QObject* const parent)
    : ImageSortFilterModel(parent),
      d_ptr(new ItemFilterModelPrivate)
{
    d_ptr->init(this);
}

ItemFilterModel::ItemFilterModel(ItemFilterModelPrivate& dd, QObject* const parent)
    : ImageSortFilterModel(parent),
      d_ptr(&dd)
{
    d_ptr->init(this);
}

ItemFilterModel::~ItemFilterModel()
{
    Q_D(ItemFilterModel);
    delete d;
}

void ItemFilterModel::setDirectSourceItemModel(ItemModel* const sourceModel)
{
    Q_D(ItemFilterModel);

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

        connect(d->imageModel, SIGNAL(preprocess(QList<ItemInfo>,QList<QVariant>)),
                d, SLOT(preprocessInfos(QList<ItemInfo>,QList<QVariant>)));

        connect(d->imageModel, SIGNAL(processAdded(QList<ItemInfo>,QList<QVariant>)),
                d, SLOT(processAddedInfos(QList<ItemInfo>,QList<QVariant>)));

        connect(d, SIGNAL(reAddItemInfos(QList<ItemInfo>,QList<QVariant>)),
                d->imageModel, SLOT(reAddItemInfos(QList<ItemInfo>,QList<QVariant>)));

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

QVariant ItemFilterModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ItemFilterModel);

    if (!index.isValid())
    {
        return QVariant();
    }

    switch (role)
    {
            // Attention: This breaks should there ever be another filter model between this and the ItemModel

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
        case CategoryDateRole:
            return d->imageModel->imageInfoRef(mapToSource(index)).dateTime();
        case GroupIsOpenRole:
            return d->groupFilter.isAllOpen() ||
                   d->groupFilter.isOpen(d->imageModel->imageInfoRef(mapToSource(index)).id());
        case ItemFilterModelPointerRole:
            return QVariant::fromValue(const_cast<ItemFilterModel*>(this));
    }

    return DCategorizedSortFilterProxyModel::data(index, role);
}

ItemFilterModel* ItemFilterModel::imageFilterModel() const
{
    return const_cast<ItemFilterModel*>(this);
}

DatabaseFields::Set ItemFilterModel::suggestedWatchFlags() const
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

void ItemFilterModel::setDayFilter(const QList<QDateTime>& days)
{
    Q_D(ItemFilterModel);
    d->filter.setDayFilter(days);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setTagFilter(const QList<int>& includedTags, const QList<int>& excludedTags,
                                    ItemFilterSettings::MatchingCondition matchingCond,
                                    bool showUnTagged, const QList<int>& clTagIds, const QList<int>& plTagIds)
{
    Q_D(ItemFilterModel);
    d->filter.setTagFilter(includedTags, excludedTags, matchingCond, showUnTagged, clTagIds, plTagIds);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setRatingFilter(int rating, ItemFilterSettings::RatingCondition ratingCond, bool isUnratedExcluded)
{
    Q_D(ItemFilterModel);
    d->filter.setRatingFilter(rating, ratingCond, isUnratedExcluded);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setUrlWhitelist(const QList<QUrl> urlList, const QString& id)
{
    Q_D(ItemFilterModel);
    d->filter.setUrlWhitelist(urlList, id);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setIdWhitelist(const QList<qlonglong>& idList, const QString& id)
{
    Q_D(ItemFilterModel);
    d->filter.setIdWhitelist(idList, id);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setMimeTypeFilter(int mimeTypeFilter)
{
    Q_D(ItemFilterModel);
    d->filter.setMimeTypeFilter(mimeTypeFilter);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setGeolocationFilter(const ItemFilterSettings::GeolocationCondition& condition)
{
    Q_D(ItemFilterModel);
    d->filter.setGeolocationFilter(condition);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setTextFilter(const SearchTextFilterSettings& settings)
{
    Q_D(ItemFilterModel);
    d->filter.setTextFilter(settings);
    setItemFilterSettings(d->filter);
}

void ItemFilterModel::setItemFilterSettings(const ItemFilterSettings& settings)
{
    Q_D(ItemFilterModel);

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

void ItemFilterModel::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    Q_D(ItemFilterModel);
    d->versionFilter.setVersionManagerSettings(settings);
    setVersionItemFilterSettings(d->versionFilter);
}

void ItemFilterModel::setExceptionList(const QList<qlonglong>& idList, const QString& id)
{
    Q_D(ItemFilterModel);
    d->versionFilter.setExceptionList(idList, id);
    setVersionItemFilterSettings(d->versionFilter);
}

void ItemFilterModel::setVersionItemFilterSettings(const VersionItemFilterSettings& settings)
{
    Q_D(ItemFilterModel);
    d->versionFilter = settings;
    slotUpdateFilter();
}

bool ItemFilterModel::isGroupOpen(qlonglong group) const
{
    Q_D(const ItemFilterModel);
    return d->groupFilter.isOpen(group);
}

bool ItemFilterModel::isAllGroupsOpen() const
{
    Q_D(const ItemFilterModel);
    return d->groupFilter.isAllOpen();
}

void ItemFilterModel::setGroupOpen(qlonglong group, bool open)
{
    Q_D(ItemFilterModel);
    d->groupFilter.setOpen(group, open);
    setGroupItemFilterSettings(d->groupFilter);
}

void ItemFilterModel::toggleGroupOpen(qlonglong group)
{
    setGroupOpen(group, !isGroupOpen(group));
}

void ItemFilterModel::setAllGroupsOpen(bool open)
{
    Q_D(ItemFilterModel);
    d->groupFilter.setAllOpen(open);
    setGroupItemFilterSettings(d->groupFilter);
}

void ItemFilterModel::setGroupItemFilterSettings(const GroupItemFilterSettings& settings)
{
    Q_D(ItemFilterModel);
    d->groupFilter = settings;
    slotUpdateFilter();
}

void ItemFilterModel::slotUpdateFilter()
{
    Q_D(ItemFilterModel);
    setItemFilterSettings(d->filter);
}

ItemFilterSettings ItemFilterModel::imageFilterSettings() const
{
    Q_D(const ItemFilterModel);
    return d->filter;
}

ItemSortSettings ItemFilterModel::imageSortSettings() const
{
    Q_D(const ItemFilterModel);
    return d->sorter;
}

VersionItemFilterSettings ItemFilterModel::versionItemFilterSettings() const
{
    Q_D(const ItemFilterModel);
    return d->versionFilter;
}

GroupItemFilterSettings ItemFilterModel::groupItemFilterSettings() const
{
    Q_D(const ItemFilterModel);
    return d->groupFilter;
}

void ItemFilterModel::slotModelReset()
{
    Q_D(ItemFilterModel);
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

bool ItemFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const ItemFilterModel);

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
    ItemInfo info = d->imageModel->imageInfo(source_row);
    bool match     = d->filter.matches(info);
    match          = match ? d->versionFilter.matches(info) : false;

    return match ? d->groupFilter.matches(info) : false;
}

void ItemFilterModel::setSendItemInfoSignals(bool sendSignals)
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

void ItemFilterModel::slotRowsInserted(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ItemInfo> infos;

    for (int i = start ; i <= end ; ++i)
    {
        infos << imageInfo(index(i, 0));
    }

    emit imageInfosAdded(infos);
}

void ItemFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ItemInfo> infos;

    for (int i = start ; i <= end; ++i)
    {
        infos << imageInfo(index(i, 0));
    }

    emit imageInfosAboutToBeRemoved(infos);
}

// -------------- Threaded preparation & filtering --------------

void ItemFilterModel::addPrepareHook(ItemFilterModelPrepareHook* const hook)
{
    Q_D(ItemFilterModel);
    QMutexLocker lock(&d->mutex);
    d->prepareHooks << hook;
}

void ItemFilterModel::removePrepareHook(ItemFilterModelPrepareHook* const hook)
{
    Q_D(ItemFilterModel);
    QMutexLocker lock(&d->mutex);
    d->prepareHooks.removeAll(hook);
}

void ItemFilterModelPreparer::process(ItemFilterModelTodoPackage package)
{
    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    // get thread-local copy
    bool needPrepareTags, needPrepareComments, needPrepareGroups;
    QList<ItemFilterModelPrepareHook*> prepareHooks;

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
        foreach (const ItemInfo& info, package.infos)
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
    // Nonetheless, QList and ItemInfo is fast. We could as well
    // reimplement ItemInfoList to ItemInfoVector (internally with templates?)
    ItemInfoList infoList;

    if (needPrepareTags || needPrepareGroups)
    {
        infoList = ItemInfoList(package.infos.toList());
    }

    if (needPrepareTags)
    {
        infoList.loadTagIds();
    }

    if (needPrepareGroups)
    {
        infoList.loadGroupImageIds();
    }

    foreach (ItemFilterModelPrepareHook* const hook, prepareHooks)
    {
        hook->prepare(package.infos);
    }

    emit processed(package);
}

void ItemFilterModelFilterer::process(ItemFilterModelTodoPackage package)
{
    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    // get thread-local copy
    ItemFilterSettings        localFilter;
    VersionItemFilterSettings localVersionFilter;
    GroupItemFilterSettings   localGroupFilter;
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
        foreach (const ItemInfo& info, package.infos)
        {
            package.filterResults[info.id()] = localFilter.matches(info)        &&
                                               localVersionFilter.matches(info) &&
                                               localGroupFilter.matches(info);
        }
    }
    else if (hasOneMatch)
    {
        bool matchForText;

        foreach (const ItemInfo& info, package.infos)
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

        foreach (const ItemInfo& info, package.infos)
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

void ItemFilterModel::setItemSortSettings(const ItemSortSettings& sorter)
{
    Q_D(ItemFilterModel);
    d->sorter = sorter;
    setCategorizedModel(d->sorter.categorizationMode != ItemSortSettings::NoCategories);
    invalidate();
}

void ItemFilterModel::setCategorizationMode(ItemSortSettings::CategorizationMode mode)
{
    Q_D(ItemFilterModel);
    d->sorter.setCategorizationMode(mode);
    setItemSortSettings(d->sorter);
}

void ItemFilterModel::setCategorizationSortOrder(ItemSortSettings::SortOrder order)
{
    Q_D(ItemFilterModel);
    d->sorter.setCategorizationSortOrder(order);
    setItemSortSettings(d->sorter);
}

void ItemFilterModel::setSortRole(ItemSortSettings::SortRole role)
{
    Q_D(ItemFilterModel);
    d->sorter.setSortRole(role);
    setItemSortSettings(d->sorter);
}

void ItemFilterModel::setSortOrder(ItemSortSettings::SortOrder order)
{
    Q_D(ItemFilterModel);
    d->sorter.setSortOrder(order);
    setItemSortSettings(d->sorter);
}

void ItemFilterModel::setStringTypeNatural(bool natural)
{
    Q_D(ItemFilterModel);
    d->sorter.setStringTypeNatural(natural);
    setItemSortSettings(d->sorter);
}

int ItemFilterModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ItemFilterModel);

    if (!d->sorter.isCategorized())
    {
        return 0;
    }

    if (!left.isValid() || !right.isValid())
    {
        return -1;
    }

    const ItemInfo& leftInfo  = d->imageModel->imageInfoRef(left);
    const ItemInfo& rightInfo = d->imageModel->imageInfoRef(right);

    // Check grouping
    qlonglong leftGroupImageId  = leftInfo.groupImageId();
    qlonglong rightGroupImageId = rightInfo.groupImageId();

    return compareInfosCategories(leftGroupImageId  == -1 ? leftInfo  : ItemInfo(leftGroupImageId),
                                  rightGroupImageId == -1 ? rightInfo : ItemInfo(rightGroupImageId));
}

bool ItemFilterModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ItemFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return true;
    }

    if (left == right)
    {
        return false;
    }

    const ItemInfo& leftInfo  = d->imageModel->imageInfoRef(left);
    const ItemInfo& rightInfo = d->imageModel->imageInfoRef(right);

    if (leftInfo == rightInfo)
    {
        return d->sorter.lessThan(left.data(ItemModel::ExtraDataRole), right.data(ItemModel::ExtraDataRole));
    }

    // Check grouping
    qlonglong leftGroupImageId  = leftInfo.groupImageId();
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
    return infosLessThan(leftGroupImageId  == -1 ? leftInfo  : ItemInfo(leftGroupImageId),
                         rightGroupImageId == -1 ? rightInfo : ItemInfo(rightGroupImageId));
}

int ItemFilterModel::compareInfosCategories(const ItemInfo& left, const ItemInfo& right) const
{
    // Note: reimplemented in ImageAlbumFilterModel
    Q_D(const ItemFilterModel);
    return d->sorter.compareCategories(left, right);
}

// Feel free to optimize. QString::number is 3x slower.
static inline QString fastNumberToString(int id)
{
    const int size = sizeof(int) * 2;
    int number     = id;
    char c[size + 1];
    c[size] = '\0';

    for (int i = 0 ; i < size ; ++i)
    {
        c[i] = 'a' + (number & 0xF);
        number >>= 4;
    }

    return QLatin1String(c);
}

QString ItemFilterModel::categoryIdentifier(const ItemInfo& i) const
{
    Q_D(const ItemFilterModel);

    if (!d->sorter.isCategorized())
    {
        return QString();
    }

    qlonglong groupedImageId = i.groupImageId();
    ItemInfo info = groupedImageId == -1 ? i : ItemInfo(groupedImageId);

    switch (d->sorter.categorizationMode)
    {
        case ItemSortSettings::NoCategories:
            return QString();
        case ItemSortSettings::OneCategory:
            return QString();
        case ItemSortSettings::CategoryByAlbum:
            return fastNumberToString(info.albumId());
        case ItemSortSettings::CategoryByFormat:
            return info.format();
        case ItemSortSettings::CategoryByMonth:
            return info.dateTime().date().toString(QLatin1String("MMyyyy"));
        default:
            return QString();
    }
}

bool ItemFilterModel::infosLessThan(const ItemInfo& left, const ItemInfo& right) const
{
    Q_D(const ItemFilterModel);
    return d->sorter.lessThan(left, right);
}

// -------------- Watching changes -----------------------------------------------------------------

void ItemFilterModel::slotImageTagChange(const ImageTagChangeset& changeset)
{
    Q_D(ItemFilterModel);

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
    foreach (const qlonglong& id, changeset.ids())
    {
        // if one matching image id is found, trigger a refresh
        if (d->imageModel->hasImage(id))
        {
            d->updateFilterTimer->start();
            return;
        }
    }
}

void ItemFilterModel::slotImageChange(const ImageChangeset& changeset)
{
    Q_D(ItemFilterModel);

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

    foreach (const qlonglong& id, changeset.ids())
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

NoDuplicatesItemFilterModel::NoDuplicatesItemFilterModel(QObject* const parent)
    : ImageSortFilterModel(parent)
{
}

bool NoDuplicatesItemFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if (index.data(ItemModel::ExtraDataDuplicateCount).toInt() <= 1)
    {
        return true;
    }

    QModelIndex previousIndex = sourceModel()->index(source_row - 1, 0, source_parent);

    if (!previousIndex.isValid())
    {
        return true;
    }

    if (sourceItemModel()->imageId(mapFromDirectSourceToSourceItemModel(index)) ==
        sourceItemModel()->imageId(mapFromDirectSourceToSourceItemModel(previousIndex)))
    {
        return false;
    }

    return true;
}

/*
void NoDuplicatesItemFilterModel::setSourceModel(QAbstractItemModel* model)
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

void NoDuplicatesItemFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int begin, int end)
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

        QModelIndex sourceIndex = mapFromDirectSourceToSourceItemModel(index);
        qlonglong id = sourceItemModel()->imageId(sourceIndex);

        if (sourceItemModel()->numberOfIndexesForImageId(id) > 1)
        {
            needInvalidate = true;
        }
    }
}*/

} // namespace Digikam
