/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "itemmodel.h"

// Qt includes

#include <QHash>
#include <QItemSelection>

// Local includes

#include "digikam_debug.h"
#include "coredbchangesets.h"
#include "coredbfields.h"
#include "coredbwatch.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "abstractitemdragdrophandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemModel::Private
{
public:

    explicit Private()
    {
        preprocessor                = nullptr;
        keepFilePathCache           = false;
        sendRemovalSignals          = false;
        incrementalUpdater          = nullptr;
        refreshing                  = false;
        reAdding                    = false;
        incrementalRefreshRequested = false;
    }

    ItemInfoList                        infos;
    ItemInfo                            itemInfo;

    QList<QVariant>                     extraValues;
    QHash<qlonglong, int>               idHash;

    bool                                keepFilePathCache;
    QHash<QString, qlonglong>           filePathHash;

    bool                                sendRemovalSignals;

    QObject*                            preprocessor;
    bool                                refreshing;
    bool                                reAdding;
    bool                                incrementalRefreshRequested;

    DatabaseFields::Set                 watchFlags;

    class ItemModelIncrementalUpdater* incrementalUpdater;

    ItemInfoList                       pendingInfos;
    QList<QVariant>                     pendingExtraValues;

    inline bool isValid(const QModelIndex& index)
    {
        if (!index.isValid())
        {
            return false;
        }

        if (index.row() < 0 || index.row() >= infos.size())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid index" << index;
            return false;
        }

        return true;
    }
    inline bool extraValueValid(const QModelIndex& index)
    {
        // we assume isValid() being called before, no duplicate checks
        if (index.row() >= extraValues.size())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid index for extraData" << index;
            return false;
        }

        return true;
    }
};

typedef QPair<int, int> IntPair; // to make foreach macro happy
typedef QList<IntPair>  IntPairList;

class Q_DECL_HIDDEN ItemModelIncrementalUpdater
{
public:

    explicit ItemModelIncrementalUpdater(ItemModel::Private* d);

    void                  appendInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void                  aboutToBeRemovedInModel(const IntPairList& aboutToBeRemoved);
    QList<IntPair>        oldIndexes();

    static QList<IntPair> toContiguousPairs(const QList<int>& ids);

public:

    QHash<qlonglong, int> oldIds;
    QList<QVariant>       oldExtraValues;
    QList<ItemInfo>       newInfos;
    QList<QVariant>       newExtraValues;
    QList<IntPairList>    modelRemovals;
};

ItemModel::ItemModel(QObject* const parent)
    : QAbstractListModel(parent),
      d(new Private)
{
    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChange(ImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));
}

ItemModel::~ItemModel()
{
    delete d->incrementalUpdater;
    delete d;
}

// ------------ Access methods -------------

void ItemModel::setKeepsFilePathCache(bool keepCache)
{
    d->keepFilePathCache = keepCache;
}

bool ItemModel::keepsFilePathCache() const
{
    return d->keepFilePathCache;
}

bool ItemModel::isEmpty() const
{
    return d->infos.isEmpty();
}

void ItemModel::setWatchFlags(const DatabaseFields::Set& set)
{
    d->watchFlags = set;
}

ItemInfo ItemModel::imageInfo(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return ItemInfo();
    }

    return d->infos.at(index.row());
}

ItemInfo& ItemModel::imageInfoRef(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return d->itemInfo;
    }

    return d->infos[index.row()];
}

qlonglong ItemModel::imageId(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return 0;
    }

    return d->infos.at(index.row()).id();
}

QList<ItemInfo> ItemModel::imageInfos(const QList<QModelIndex>& indexes) const
{
    QList<ItemInfo> infos;

    foreach (const QModelIndex& index, indexes)
    {
        if (d->isValid(index))
        {
            infos << imageInfo(index);
        }
    }

    return infos;
}

QList<qlonglong> ItemModel::imageIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;

    foreach (const QModelIndex& index, indexes)
    {
        if (d->isValid(index))
        {
            ids << imageId(index);
        }
    }

    return ids;
}

ItemInfo ItemModel::imageInfo(int row) const
{
    if (row < 0 || row >= d->infos.size())
    {
        return ItemInfo();
    }

    return d->infos.at(row);
}

ItemInfo& ItemModel::imageInfoRef(int row) const
{
    if (row < 0 || row >= d->infos.size())
    {
        return d->itemInfo;
    }

    return d->infos[row];
}

qlonglong ItemModel::imageId(int row) const
{
    if (row < 0 || row >= d->infos.size())
    {
        return -1;
    }

    return d->infos.at(row).id();
}

QModelIndex ItemModel::indexForItemInfo(const ItemInfo& info) const
{
    return indexForImageId(info.id());
}

QModelIndex ItemModel::indexForItemInfo(const ItemInfo& info, const QVariant& extraValue) const
{
    return indexForImageId(info.id(), extraValue);
}

QList<QModelIndex> ItemModel::indexesForItemInfo(const ItemInfo& info) const
{
    return indexesForImageId(info.id());
}

QModelIndex ItemModel::indexForImageId(qlonglong id) const
{
    int index = d->idHash.value(id, -1);

    if (index != -1)
    {
        return createIndex(index, 0);
    }

    return QModelIndex();
}

QModelIndex ItemModel::indexForImageId(qlonglong id, const QVariant& extraValue) const
{
    if (d->extraValues.isEmpty())
        return indexForImageId(id);

    QHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
        if (d->extraValues.at(it.value()) == extraValue)
            return createIndex(it.value(), 0);
    }

    return QModelIndex();
}

QList<QModelIndex> ItemModel::indexesForImageId(qlonglong id) const
{
    QList<QModelIndex> indexes;
    QHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
        indexes << createIndex(it.value(), 0);
    }

    return indexes;
}

int ItemModel::numberOfIndexesForItemInfo(const ItemInfo& info) const
{
    return numberOfIndexesForImageId(info.id());
}

int ItemModel::numberOfIndexesForImageId(qlonglong id) const
{
    if (d->extraValues.isEmpty())
    {
        return 0;
    }

    int count = 0;
    QHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
        ++count;
    }

    return count;
}

// static method
ItemInfo ItemModel::retrieveItemInfo(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return ItemInfo();
    }

    ItemModel* const model = index.data(ItemModelPointerRole).value<ItemModel*>();
    int row                 = index.data(ItemModelInternalId).toInt();

    if (!model)
    {
        return ItemInfo();
    }

    return model->imageInfo(row);
}

// static method
qlonglong ItemModel::retrieveImageId(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return 0;
    }

    ItemModel* const model = index.data(ItemModelPointerRole).value<ItemModel*>();
    int row                 = index.data(ItemModelInternalId).toInt();

    if (!model)
    {
        return 0;
    }

    return model->imageId(row);
}

QModelIndex ItemModel::indexForPath(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        return indexForImageId(d->filePathHash.value(filePath));
    }
    else
    {
        const int size = d->infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (d->infos.at(i).filePath() == filePath)
            {
                return createIndex(i, 0);
            }
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ItemModel::indexesForPath(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        return indexesForImageId(d->filePathHash.value(filePath));
    }
    else
    {
        QList<QModelIndex> indexes;
        const int size = d->infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (d->infos.at(i).filePath() == filePath)
            {
                indexes << createIndex(i, 0);
            }
        }

        return indexes;
    }
}

ItemInfo ItemModel::imageInfo(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        qlonglong id = d->filePathHash.value(filePath);

        if (id)
        {
            int index = d->idHash.value(id, -1);

            if (index != -1)
            {
                return d->infos.at(index);
            }
        }
    }
    else
    {
        foreach (const ItemInfo& info, d->infos)
        {
            if (info.filePath() == filePath)
            {
                return info;
            }
        }
    }

    return ItemInfo();
}

QList<ItemInfo> ItemModel::imageInfos(const QString& filePath) const
{
    QList<ItemInfo> infos;

    if (d->keepFilePathCache)
    {
        qlonglong id = d->filePathHash.value(filePath);

        if (id)
        {
            foreach (int index, d->idHash.values(id))
            {
                infos << d->infos.at(index);
            }
        }
    }
    else
    {
        foreach (const ItemInfo& info, d->infos)
        {
            if (info.filePath() == filePath)
            {
                infos << info;
            }
        }
    }

    return infos;
}

void ItemModel::addItemInfo(const ItemInfo& info)
{
    addItemInfos(QList<ItemInfo>() << info, QList<QVariant>());
}

void ItemModel::addItemInfos(const QList<ItemInfo>& infos)
{
    addItemInfos(infos, QList<QVariant>());
}

void ItemModel::addItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->appendInfos(infos, extraValues);
    }
    else
    {
        appendInfos(infos, extraValues);
    }
}

void ItemModel::addItemInfoSynchronously(const ItemInfo& info)
{
    addItemInfosSynchronously(QList<ItemInfo>() << info, QList<QVariant>());
}

void ItemModel::addItemInfosSynchronously(const QList<ItemInfo>& infos)
{
    addItemInfos(infos, QList<QVariant>());
}

void ItemModel::addItemInfosSynchronously(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos, extraValues);
    emit processAdded(infos, extraValues);
}

void ItemModel::ensureHasItemInfo(const ItemInfo& info)
{
    ensureHasItemInfos(QList<ItemInfo>() << info, QList<QVariant>());
}

void ItemModel::ensureHasItemInfos(const QList<ItemInfo>& infos)
{
    ensureHasItemInfos(infos, QList<QVariant>());
}

void ItemModel::ensureHasItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        if (!d->pendingExtraValues.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ExtraValue / No Extra Value mismatch. Ignoring added infos.";
            return;
        }
    }
    else
    {
        if (d->pendingInfos.size() != d->pendingExtraValues.size())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ExtraValue / No Extra Value mismatch. Ignoring added infos.";
            return;
        }
    }

    d->pendingInfos << infos;
    d->pendingExtraValues << extraValues;
    cleanSituationChecks();
}

void ItemModel::clearItemInfos()
{
    beginResetModel();

    d->infos.clear();
    d->extraValues.clear();
    d->idHash.clear();
    d->filePathHash.clear();

    delete d->incrementalUpdater;

    d->incrementalUpdater          = nullptr;
    d->pendingInfos.clear();
    d->pendingExtraValues.clear();
    d->refreshing                  = false;
    d->reAdding                    = false;
    d->incrementalRefreshRequested = false;

    imageInfosCleared();
    endResetModel();
}

void ItemModel::setItemInfos(const QList<ItemInfo>& infos)
{
    clearItemInfos();
    addItemInfos(infos);
}

QList<ItemInfo> ItemModel::imageInfos() const
{
    return d->infos;
}

QList<qlonglong> ItemModel::imageIds() const
{
    return d->idHash.keys();
}

bool ItemModel::hasImage(qlonglong id) const
{
    return d->idHash.contains(id);
}

bool ItemModel::hasImage(const ItemInfo& info) const
{
    return d->idHash.contains(info.id());
}

bool ItemModel::hasImage(const ItemInfo& info, const QVariant& extraValue) const
{
    return hasImage(info.id(), extraValue);
}

bool ItemModel::hasImage(qlonglong id, const QVariant& extraValue) const
{
    if (d->extraValues.isEmpty())
        return hasImage(id);

    QHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
        if (d->extraValues.at(it.value()) == extraValue)
            return true;
    }

    return false;;
}

QList<ItemInfo> ItemModel::uniqueItemInfos() const
{
    if (d->extraValues.isEmpty())
    {
        return d->infos;
    }

    QList<ItemInfo> uniqueInfos;
    const int size = d->infos.size();

    for (int i = 0 ; i < size ; ++i)
    {
        const ItemInfo& info = d->infos.at(i);

        if (d->idHash.value(info.id()) == i)
        {
            uniqueInfos << info;
        }
    }

    return uniqueInfos;
}

void ItemModel::emitDataChangedForAll()
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QModelIndex first = createIndex(0, 0);
    QModelIndex last  = createIndex(d->infos.size() - 1, 0);
    emit dataChanged(first, last);
}

void ItemModel::emitDataChangedForSelection(const QItemSelection& selection)
{
    if (!selection.isEmpty())
    {
        foreach (const QItemSelectionRange& range, selection)
        {
            emit dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

void ItemModel::ensureHasGroupedImages(const ItemInfo& groupLeader)
{
    ensureHasItemInfos(groupLeader.groupedImages());
}

// ------------ Preprocessing -------------

void ItemModel::setPreprocessor(QObject* const preprocessor)
{
    unsetPreprocessor(d->preprocessor);
    d->preprocessor = preprocessor;
}

void ItemModel::unsetPreprocessor(QObject* const preprocessor)
{
    if (preprocessor && d->preprocessor == preprocessor)
    {
        disconnect(this, SIGNAL(preprocess(QList<ItemInfo>,QList<QVariant>)), nullptr, nullptr);
        disconnect(d->preprocessor, nullptr, this, SLOT(reAddItemInfos(QList<ItemInfo>,QList<QVariant>)));
        disconnect(d->preprocessor, nullptr, this, SLOT(reAddingFinished()));
    }
}

void ItemModel::appendInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (d->preprocessor)
    {
        d->reAdding = true;
        emit preprocess(infos, extraValues);
    }
    else
    {
        publiciseInfos(infos, extraValues);
    }
}

void ItemModel::appendInfosChecked(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    // This method does deduplication. It is private because in context of readding or refreshing it is of no use.

    if (extraValues.isEmpty())
    {
        QList<ItemInfo> checkedInfos;

        foreach (const ItemInfo& info, infos)
        {
            if (!hasImage(info))
            {
                checkedInfos << info;
            }
        }

        appendInfos(checkedInfos, QList<QVariant>());
    }
    else
    {
        QList<ItemInfo> checkedInfos;
        QList<QVariant>  checkedExtraValues;
        const int size = infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (!hasImage(infos[i], extraValues[i]))
            {
                checkedInfos << infos[i];
                checkedExtraValues << extraValues[i];
            }
        }

        appendInfos(checkedInfos, checkedExtraValues);
    }
}

void ItemModel::reAddItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    // addItemInfos -> appendInfos -> preprocessor -> reAddItemInfos
    publiciseInfos(infos, extraValues);
}

void ItemModel::reAddingFinished()
{
    d->reAdding = false;
    cleanSituationChecks();
}

void ItemModel::startRefresh()
{
    d->refreshing = true;
}

void ItemModel::finishRefresh()
{
    d->refreshing = false;
    cleanSituationChecks();
}

bool ItemModel::isRefreshing() const
{
    return d->refreshing;
}

void ItemModel::cleanSituationChecks()
{
    // For starting an incremental refresh we want a clear situation:
    // Any remaining batches from non-incremental refreshing subclasses have been received in appendInfos(),
    // any batches sent to preprocessor for re-adding have been re-added.
    if (d->refreshing || d->reAdding)
    {
        return;
    }

    if (!d->pendingInfos.isEmpty())
    {
        appendInfosChecked(d->pendingInfos, d->pendingExtraValues);
        d->pendingInfos.clear();
        d->pendingExtraValues.clear();
        cleanSituationChecks();
        return;
    }

    if (d->incrementalRefreshRequested)
    {
        d->incrementalRefreshRequested = false;
        emit readyForIncrementalRefresh();
    }
    else
    {
        emit allRefreshingFinished();
    }
}

void ItemModel::publiciseInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    Q_ASSERT(infos.size() == extraValues.size() || (extraValues.isEmpty() && d->extraValues.isEmpty()));

    emit imageInfosAboutToBeAdded(infos);
    const int firstNewIndex = d->infos.size();
    const int lastNewIndex  = d->infos.size() + infos.size() - 1;
    beginInsertRows(QModelIndex(), firstNewIndex, lastNewIndex);
    d->infos << infos;
    d->extraValues << extraValues;

    for (int i = firstNewIndex ; i <= lastNewIndex ; ++i)
    {
        const ItemInfo& info = d->infos.at(i);
        qlonglong id          = info.id();
        d->idHash.insertMulti(id, i);

        if (d->keepFilePathCache)
        {
            d->filePathHash[info.filePath()] = id;
        }
    }

    endInsertRows();
    emit imageInfosAdded(infos);
}

void ItemModel::requestIncrementalRefresh()
{
    if (d->reAdding)
    {
        d->incrementalRefreshRequested = true;
    }
    else
    {
        emit readyForIncrementalRefresh();
    }
}

bool ItemModel::hasIncrementalRefreshPending() const
{
    return d->incrementalRefreshRequested;
}

void ItemModel::startIncrementalRefresh()
{
    delete d->incrementalUpdater;

    d->incrementalUpdater = new ItemModelIncrementalUpdater(d);
}

void ItemModel::finishIncrementalRefresh()
{
    if (!d->incrementalUpdater)
    {
        return;
    }

    // remove old entries
    QList<QPair<int, int> > pairs = d->incrementalUpdater->oldIndexes();
    removeRowPairs(pairs);

    // add new indexes
    appendInfos(d->incrementalUpdater->newInfos, d->incrementalUpdater->newExtraValues);

    delete d->incrementalUpdater;
    d->incrementalUpdater = nullptr;
}

void ItemModel::removeIndex(const QModelIndex& index)
{
    removeIndexes(QList<QModelIndex>() << index);
}

void ItemModel::removeIndexes(const QList<QModelIndex>& indexes)
{
    QList<int> listIndexes;

    foreach (const QModelIndex& index, indexes)
    {
        if (d->isValid(index))
        {
            listIndexes << index.row();
        }
    }

    if (listIndexes.isEmpty())
    {
        return;
    }

    removeRowPairsWithCheck(ItemModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ItemModel::removeItemInfo(const ItemInfo& info)
{
    removeItemInfos(QList<ItemInfo>() << info);
}

void ItemModel::removeItemInfos(const QList<ItemInfo>& infos)
{
    QList<int> listIndexes;

    foreach (const ItemInfo& info, infos)
    {
        QModelIndex index = indexForImageId(info.id());

        if (index.isValid())
        {
            listIndexes << index.row();
        }
    }
    removeRowPairsWithCheck(ItemModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ItemModel::removeItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        removeItemInfos(infos);
        return;
    }

    QList<int> listIndexes;

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        QModelIndex index = indexForImageId(infos.at(i).id(), extraValues.at(i));

        if (index.isValid())
        {
            listIndexes << index.row();
        }
    }

    removeRowPairsWithCheck(ItemModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ItemModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
}

template <class List, typename T>
static bool pairsContain(const List& list, T value)
{
    typename List::const_iterator middle;
    typename List::const_iterator begin = list.begin();
    typename List::const_iterator end   = list.end();
    int n                               = int(end - begin);
    int half;

    while (n > 0)
    {
        half   = n >> 1;
        middle = begin + half;

        if (middle->first <= value && middle->second >= value)
        {
            return true;
        }
        else if (middle->second < value)
        {
            begin = middle + 1;
            n     -= half + 1;
        }
        else
        {
            n = half;
        }
    }

    return false;
}

void ItemModel::removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove)
{
    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->aboutToBeRemovedInModel(toRemove);
    }

    removeRowPairs(toRemove);
}

void ItemModel::removeRowPairs(const QList<QPair<int, int> >& toRemove)
{
    if (toRemove.isEmpty())
    {
        return;
    }

    // Remove old indexes
    // Keep in mind that when calling beginRemoveRows all structures announced to be removed
    // must still be valid, and this includes our hashes as well, which limits what we can optimize

    int removedRows = 0, offset = 0;
    typedef QPair<int, int> IntPair; // to make foreach macro happy

    foreach (const IntPair& pair, toRemove)
    {
        const int begin = pair.first - offset;
        const int end   = pair.second - offset; // inclusive
        removedRows     = end - begin + 1;

        // when removing from the list, all subsequent indexes are affected
        offset += removedRows;

        QList<ItemInfo> removedInfos;

        if (d->sendRemovalSignals)
        {
            std::copy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());
            emit imageInfosAboutToBeRemoved(removedInfos);
        }

        imageInfosAboutToBeRemoved(begin, end);
        beginRemoveRows(QModelIndex(), begin, end);

        // update idHash - which points to indexes of d->infos, and these change now!
        QHash<qlonglong, int>::iterator it;

        for (it = d->idHash.begin() ; it != d->idHash.end() ; )
        {
            if (it.value() >= begin)
            {
                if (it.value() > end)
                {
                    // after the removed interval: adjust index
                    it.value() -= removedRows;
                }
                else
                {
                    // in the removed interval
                    it = d->idHash.erase(it);
                    continue;
                }
            }

            ++it;
        }

        // remove from list
        d->infos.erase(d->infos.begin() + begin, d->infos.begin() + (end + 1));

        if (!d->extraValues.isEmpty())
        {
            d->extraValues.erase(d->extraValues.begin() + begin, d->extraValues.begin() + (end + 1));
        }

        endRemoveRows();

        if (d->sendRemovalSignals)
        {
            emit imageInfosRemoved(removedInfos);
        }
    }

    // tidy up: remove old indexes from file path hash now
    if (d->keepFilePathCache)
    {
        QHash<QString, qlonglong>::iterator it;

        for (it = d->filePathHash.begin() ; it != d->filePathHash.end() ; )
        {
            if (pairsContain(toRemove, it.value()))
            {
                it = d->filePathHash.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

ItemModelIncrementalUpdater::ItemModelIncrementalUpdater(ItemModel::Private* d)
{
    oldIds         = d->idHash;
    oldExtraValues = d->extraValues;
}

void ItemModelIncrementalUpdater::appendInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        foreach (const ItemInfo& info, infos)
        {
            QHash<qlonglong, int>::iterator it = oldIds.find(info.id());

            if (it != oldIds.end())
            {
                oldIds.erase(it);
            }
            else
            {
                newInfos << info;
            }
        }
    }
    else
    {
        for (int i = 0 ; i < infos.size() ; ++i)
        {
            const ItemInfo& info = infos.at(i);
            bool found            = false;
            QHash<qlonglong, int>::iterator it;

            for (it = oldIds.find(info.id()) ; it != oldIds.end() && it.key() == info.id() ; ++it)
            {
                // first check is for bug #262596. Not sure if needed.
                if (it.value() < oldExtraValues.size() && extraValues.at(i) == oldExtraValues.at(it.value()))
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                oldIds.erase(it);
                // do not erase from oldExtraValues - oldIds is a hash id -> index.
            }
            else
            {
                newInfos << info;
                newExtraValues << extraValues.at(i);
            }
        }
    }
}

void ItemModelIncrementalUpdater::aboutToBeRemovedInModel(const IntPairList& toRemove)
{
    modelRemovals << toRemove;
}

QList<QPair<int, int> > ItemModelIncrementalUpdater::oldIndexes()
{
    // first, apply all changes to indexes by direct removal in model
    // while the updater was active
    foreach (const IntPairList& list, modelRemovals)
    {
        int removedRows = 0, offset = 0;

        foreach (const IntPair& pair, list)
        {
            const int begin = pair.first - offset;
            const int end   = pair.second - offset; // inclusive
            removedRows     = end - begin + 1;

            // when removing from the list, all subsequent indexes are affected
            offset += removedRows;

            // update idHash - which points to indexes of d->infos, and these change now!
            QHash<qlonglong, int>::iterator it;

            for (it = oldIds.begin() ; it != oldIds.end() ; )
            {
                if (it.value() >= begin)
                {
                    if (it.value() > end)
                    {
                        // after the removed interval: adjust index
                        it.value() -= removedRows;
                    }
                    else
                    {
                        // in the removed interval
                        it = oldIds.erase(it);
                        continue;
                    }
                }

                ++it;
            }
        }
    }

    modelRemovals.clear();

    return toContiguousPairs(oldIds.values());
}

QList<QPair<int, int> > ItemModelIncrementalUpdater::toContiguousPairs(const QList<int>& unsorted)
{
    // Take the given indices and return them as contiguous pairs [begin, end]

    QList<QPair<int, int> > pairs;

    if (unsorted.isEmpty())
    {
        return pairs;
    }

    QList<int> indices(unsorted);
    std::sort(indices.begin(), indices.end());

    QPair<int, int> pair(indices.first(), indices.first());

    for (int i = 1 ; i < indices.size() ; ++i)
    {
        const int &index = indices.at(i);

        if (index == pair.second + 1)
        {
            pair.second = index;
            continue;
        }

        pairs << pair; // insert last pair
        pair.first  = index;
        pair.second = index;
    }

    pairs << pair;

    return pairs;
}

// ------------ QAbstractItemModel implementation -------------

QVariant ItemModel::data(const QModelIndex& index, int role) const
{
    if (!d->isValid(index))
    {
        return QVariant();
    }

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return d->infos.at(index.row()).name();

        case ItemModelPointerRole:
            return QVariant::fromValue(const_cast<ItemModel*>(this));

        case ItemModelInternalId:
            return index.row();

        case CreationDateRole:
            return d->infos.at(index.row()).dateTime();

        case ExtraDataRole:

            if (d->extraValueValid(index))
            {
                return d->extraValues.at(index.row());
            }
            else
            {
                return QVariant();
            }

        case ExtraDataDuplicateCount:
        {
            qlonglong id = d->infos.at(index.row()).id();
            return numberOfIndexesForImageId(id);
        }
    }

    return QVariant();
}

QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

int ItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->infos.size();
}

Qt::ItemFlags ItemModel::flags(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return nullptr;
    }

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    f |= dragDropFlags(index);

    return f;
}

QModelIndex ItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0 || row < 0 || parent.isValid() || row >= d->infos.size())
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

// ------------ Database watch -------------

void ItemModel::slotImageChange(const ImageChangeset& changeset)
{
    if (d->infos.isEmpty())
    {
        return;
    }

    if (d->watchFlags & changeset.changes())
    {
        QItemSelection items;

        foreach (const qlonglong& id, changeset.ids())
        {
            QModelIndex index = indexForImageId(id);

            if (index.isValid())
            {
                items.select(index, index);
            }
        }

        if (!items.isEmpty())
        {
            emitDataChangedForSelection(items);
            emit imageChange(changeset, items);
        }
    }
}

void ItemModel::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QItemSelection items;

    foreach (const qlonglong& id, changeset.ids())
    {
        QModelIndex index = indexForImageId(id);

        if (index.isValid())
        {
            items.select(index, index);
        }
    }

    if (!items.isEmpty())
    {
        emitDataChangedForSelection(items);
        emit imageTagChange(changeset, items);
    }
}

} // namespace Digikam
