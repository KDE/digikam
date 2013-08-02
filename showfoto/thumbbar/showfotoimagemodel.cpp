/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-5
 * Description : Qt model for Showfoto entries
 *
 * Copyright (C) 2012 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotoimagemodel.moc"

// Qt includes

#include <QHash>
#include <QDebug>
#include "showfoto.h"
#include "showfotoiteminfo.h"


namespace ShowFoto
{

class ShowfotoImageModel::Private
{
public:

    Private()
    {
        keepFileUrlCache            = false;
        refreshing                  = false;
        reAdding                    = false;
        incrementalRefreshRequested = false;
        sendRemovalSignals          = false;
        incrementalUpdater          = 0;
        Loader                      = 0;

    }

    inline bool isValid(const QModelIndex& index)
    {
        return (index.isValid()              &&
                (index.row() >= 0)           &&
                (index.row() < infos.size())
               );
    }

public:

    ShowfotoItemInfoList                      infos;
    ShowfotoItemLoader*                       Loader;
    QHash<qlonglong, int>                     idHash;
    QHash<QString, qlonglong>                 fileUrlHash;

    bool                                      keepFileUrlCache;

    bool                                      refreshing;
    bool                                      reAdding;
    bool                                      incrementalRefreshRequested;

    bool                                      sendRemovalSignals;

    class ShowfotoImageModelIncrementalUpdater* incrementalUpdater;
};

// ----------------------------------------------------------------------------------------------------

typedef QPair<int, int> IntPair;
typedef QList<IntPair>  IntPairList;

class ShowfotoImageModelIncrementalUpdater
{
public:

    explicit ShowfotoImageModelIncrementalUpdater(ShowfotoImageModel::Private* const d);

    void            appendInfos(const QList<ShowfotoItemInfo>& infos);
    void            aboutToBeRemovedInModel(const IntPairList& aboutToBeRemoved);
    QList<IntPair>  oldIndexes();

    static QList<IntPair> toContiguousPairs(const QList<int>& ids);

public:

    QHash<qlonglong, int>      oldIds;
    QList<ShowfotoItemInfo>    newInfos;
    QList<IntPairList>         modelRemovals;
};

// ----------------------------------------------------------------------------------------------------

ShowfotoImageModel::ShowfotoImageModel(QObject* const parent)
    : QAbstractListModel(parent),
      d(new Private)
{
}


ShowfotoImageModel::~ShowfotoImageModel()
{
    delete d;
}

void ShowfotoImageModel::setKeepsFileUrlCache(bool keepCache)
{
    d->keepFileUrlCache = keepCache;
}

bool ShowfotoImageModel::keepsFileUrlCache() const
{
    return d->keepFileUrlCache;
}

bool ShowfotoImageModel::isEmpty() const
{
    return d->infos.isEmpty();
}

ShowfotoItemInfo ShowfotoImageModel::showfotoItemInfo(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return ShowfotoItemInfo();
    }

    return d->infos.at(index.row());
}

ShowfotoItemInfo& ShowfotoImageModel::showfotoItemInfoRef(const QModelIndex& index) const
{
    return d->infos[index.row()];
}

qlonglong ShowfotoImageModel::showfotoItemId(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return -1;
    }

    return d->infos.at(index.row()).id;
}

QList<ShowfotoItemInfo> ShowfotoImageModel::showfotoItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<ShowfotoItemInfo> infos;

    foreach(const QModelIndex& index, indexes)
    {
        infos << showfotoItemInfo(index);
    }

    return infos;
}

QList<qlonglong> ShowfotoImageModel::showfotoItemIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;

    foreach(const QModelIndex& index, indexes)
    {
        ids << showfotoItemId(index);
    }

    return ids;
}

ShowfotoItemInfo ShowfotoImageModel::showfotoItemInfo(int row) const
{
    if (row >= d->infos.size())
    {
        return ShowfotoItemInfo();
    }

    return d->infos.at(row);
}

ShowfotoItemInfo& ShowfotoImageModel::showfotoItemInfoRef(int row) const
{
    return d->infos[row];
}

qlonglong ShowfotoImageModel::showfotoItemId(int row) const
{
    if (row < 0 || (row >= d->infos.size()))
    {
        return -1;
    }

    return d->infos.at(row).id;
}

QModelIndex ShowfotoImageModel::indexForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return indexForShowfotoItemId(info.id);
}

QList<QModelIndex> ShowfotoImageModel::indexesForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return indexesForShowfotoItemId(info.id);
}

QModelIndex ShowfotoImageModel::indexForShowfotoItemId(qlonglong id) const
{
    int index = d->idHash.value(id, 0);

    if (index != -1)
    {
        return createIndex(index, 0);
    }

    return QModelIndex();
}

QList<QModelIndex> ShowfotoImageModel::indexesForShowfotoItemId(qlonglong id) const
{
    QList<QModelIndex> indexes;

    QHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id); it != d->idHash.constEnd() && it.key() == id; ++it)
    {
       indexes << createIndex(it.value(), 0);
    }

    return indexes;
}

int ShowfotoImageModel::numberOfIndexesForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return numberOfIndexesForShowfotoItemId(info.id);
}

int ShowfotoImageModel::numberOfIndexesForShowfotoItemId(qlonglong id) const
{
    int count = 0;
    QHash<qlonglong,int>::const_iterator it;

    for (it = d->idHash.constFind(id); it != d->idHash.constEnd() && it.key() == id; ++it)
    {
        ++count;
    }

    return count;
}

// static method
ShowfotoItemInfo ShowfotoImageModel::retrieveShowfotoItemInfo(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return ShowfotoItemInfo();
    }

    ShowfotoImageModel* const model = index.data(ShowfotoImageModelPointerRole).value<ShowfotoImageModel*>();
    int row                         = index.data(ShowfotoImageModelInternalId).toInt();

    if (!model)
    {
        return ShowfotoItemInfo();
    }

    return model->showfotoItemInfo(row);
}

// static method
qlonglong ShowfotoImageModel::retrieveShowfotoItemId(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return -1;
    }

    ShowfotoImageModel* const model = index.data(ShowfotoImageModelPointerRole).value<ShowfotoImageModel*>();
    int                       row   = index.data(ShowfotoImageModelInternalId).toInt();

    if (!model)
    {
        return -1;
    }

    return model->showfotoItemId(row);
}

QModelIndex ShowfotoImageModel::indexForUrl(const KUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        return indexForShowfotoItemId(d->fileUrlHash.value(fileUrl.prettyUrl()));
    }
    else
    {
        const int size = d->infos.size();

        for (int i = 0; i < size; i++)
        {
            if (d->infos.at(i).url == fileUrl)
            {
                return createIndex(i, 0);
            }
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ShowfotoImageModel::indexesForUrl(const KUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        return indexesForShowfotoItemId(d->fileUrlHash.value(fileUrl.prettyUrl()));
    }
    else
    {
        QList<QModelIndex> indexes;
        const int          size = d->infos.size();

        for (int i = 0; i < size; i++)
        {
            if (d->infos.at(i).url == fileUrl)
            {
                indexes << createIndex(i, 0);
            }
        }

        return indexes;
    }
}

ShowfotoItemInfo ShowfotoImageModel::showfotoItemInfo(const KUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        qlonglong id = d->fileUrlHash.value(fileUrl.prettyUrl());

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
        foreach(const ShowfotoItemInfo& info, d->infos)
        {
            if (info.url == fileUrl)
            {
                return info;
            }
        }
    }

    return ShowfotoItemInfo();
}

QList<ShowfotoItemInfo> ShowfotoImageModel::showfotoItemInfos(const KUrl& fileUrl) const
{
    QList<ShowfotoItemInfo> infos;

    if (d->keepFileUrlCache)
    {
        qlonglong id = d->fileUrlHash.value(fileUrl.prettyUrl());

        if (id)
        {
            foreach(int index, d->idHash.values(id))
            {
                infos << d->infos.at(index);
            }
        }
    }
    else
    {
        foreach(const ShowfotoItemInfo& info, d->infos)
        {
            if (info.url == fileUrl)
            {
                infos << info;
            }
        }
    }

    return infos;
}

void ShowfotoImageModel::addShowfotoItemInfo(const ShowfotoItemInfo& info)
{
    addShowfotoItemInfos(QList<ShowfotoItemInfo>() << info);
}

void ShowfotoImageModel::addShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->appendInfos(infos);
    }
    else
    {
        appendInfos(infos);
    }
}

void ShowfotoImageModel::addShowfotoItemInfoSynchronously(const ShowfotoItemInfo& info)
{
    addShowfotoItemInfosSynchronously(QList<ShowfotoItemInfo>() << info);
}

void ShowfotoImageModel::addShowfotoItemInfosSynchronously(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);
    emit processAdded(infos);
}

void ShowfotoImageModel::clearShowfotoItemInfos()
{
    d->infos.clear();
    d->idHash.clear();
    d->fileUrlHash.clear();

    delete d->incrementalUpdater;

    d->incrementalUpdater          = 0;
    d->reAdding                    = false;
    d->refreshing                  = false;
    d->incrementalRefreshRequested = false;

    reset();
    showfotoItemInfosCleared();
}

void ShowfotoImageModel::setShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos)
{
    clearShowfotoItemInfos();
    addShowfotoItemInfos(infos);
}

QList<ShowfotoItemInfo> ShowfotoImageModel::showfotoItemInfos() const
{
    return d->infos;
}

QList<qlonglong> ShowfotoImageModel::showfotoItemIds() const
{
    return d->idHash.keys();
}

QList<ShowfotoItemInfo> ShowfotoImageModel::uniqueShowfotoItemInfos() const
{
    QList<ShowfotoItemInfo> uniqueInfos;
    const int          size = d->infos.size();

    for (int i = 0; i < size; i++)
    {
        const ShowfotoItemInfo& info = d->infos.at(i);

        if (d->idHash.value(info.id) == i)
        {
            uniqueInfos << info;
        }
    }

    return uniqueInfos;
}

bool ShowfotoImageModel::hasImage(qlonglong id) const
{
    return d->idHash.contains(id);
}

bool ShowfotoImageModel::hasImage(const ShowfotoItemInfo& info) const
{
    return d->fileUrlHash.contains(info.url.prettyUrl());
}

void ShowfotoImageModel::emitDataChangedForAll()
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QModelIndex first = createIndex(0, 0);
    QModelIndex last  = createIndex(d->infos.size() - 1, 0);
    emit dataChanged(first, last);
}

void ShowfotoImageModel::emitDataChangedForSelections(const QItemSelection& selection)
{
    if (!selection.isEmpty())
    {
        foreach(const QItemSelectionRange& range, selection)
        {
            emit dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

void ShowfotoImageModel::appendInfos(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);
}



void ShowfotoImageModel::reAddShowfotoItemInfos(ShowfotoItemInfoList& infos)
{
    qDebug() << "hey I got the list now";
    publiciseInfos(infos);
}

void ShowfotoImageModel::reAddingFinished()
{
    d->reAdding = false;
    cleanSituationChecks();
}

void ShowfotoImageModel::slotFileDeleted(const QString& folder, const QString& file, bool status)
{
    Q_UNUSED(status)

    ShowfotoItemInfo info = showfotoItemInfo(KUrl::fromLocalFile(folder + file));
    removeShowfotoItemInfo(info);
}

void ShowfotoImageModel::slotFileUploaded(const ShowfotoItemInfo& info)
{
    addShowfotoItemInfo(info);
}

void ShowfotoImageModel::startRefresh()
{
    d->refreshing = true;
}

void ShowfotoImageModel::finishRefresh()
{
    d->refreshing = false;
    cleanSituationChecks();
}

bool ShowfotoImageModel::isRefreshing() const
{
    return d->refreshing;
}

void ShowfotoImageModel::cleanSituationChecks()
{
    // For starting an incremental refresh we want a clear situation:
    // Any remaining batches from non-incremental refreshing subclasses have been received in appendInfos(),
    // any batches sent to preprocessor for re-adding have been re-added.
    if (d->refreshing || d->reAdding)
    {
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



void ShowfotoImageModel::publiciseInfos(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    emit itemInfosAboutToBeAdded(infos);

    const int firstNewIndex = d->infos.size();
    const int lastNewIndex  = d->infos.size() + infos.size() -1;
    beginInsertRows(QModelIndex(), firstNewIndex, lastNewIndex);
    d->infos << infos;

    for (int i = firstNewIndex; i <= lastNewIndex; ++i)
    {
        const ShowfotoItemInfo& info = d->infos.at(i);
        qlonglong id                 = info.id;
        d->idHash.insertMulti(id, i);

        if (d->keepFileUrlCache)
        {
            d->fileUrlHash[info.url.prettyUrl()] = id;
        }
    }
    endInsertRows();
    emit itemInfosAdded(infos);
}

void ShowfotoImageModel::requestIncrementalRefresh()
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

bool ShowfotoImageModel::hasIncrementalRefreshPending() const
{
    return d->incrementalRefreshRequested;
}

void ShowfotoImageModel::startIncrementalRefresh()
{
    delete d->incrementalUpdater;

    d->incrementalUpdater = new ShowfotoImageModelIncrementalUpdater(d);
}

void ShowfotoImageModel::finishIncrementalRefresh()
{
    if (!d->incrementalUpdater)
    {
        return;
    }

    // remove old entries
    QList<QPair<int, int> > pairs = d->incrementalUpdater->oldIndexes();
    removeRowPairs(pairs);

    // add new indexes
    appendInfos(d->incrementalUpdater->newInfos);

    delete d->incrementalUpdater;
    d->incrementalUpdater = 0;
}

template <class List, typename T>
static bool pairsContain(const List& list, T value)
{
    typename List::const_iterator middle;
    typename List::const_iterator begin = list.begin();
    typename List::const_iterator end   = list.end();
    int n                               = int(end - begin);
    int half;

    while(n > 0)
    {
        half   = n >> 1;
        middle = begin + half;

        if ((middle->first <= value) && (middle->second >= value))
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

void ShowfotoImageModel::removeIndex(const QModelIndex& index)
{
    removeIndexs(QList<QModelIndex>() << index);
}

void ShowfotoImageModel::removeIndexs(const QList<QModelIndex>& indexes)
{
    QList<int> indexesList;

    foreach(const QModelIndex& index, indexes)
    {
        if (d->isValid(index))
        {
            indexesList << index.row();
        }
    }

    if (indexesList.isEmpty())
    {
        return;
    }

    removeRowPairsWithCheck(ShowfotoImageModelIncrementalUpdater::toContiguousPairs(indexesList));
}

void ShowfotoImageModel::removeShowfotoItemInfo(const ShowfotoItemInfo& info)
{
    removeShowfotoItemInfos(QList<ShowfotoItemInfo>() << info);
}

void ShowfotoImageModel::removeShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos)
{
    QList<int> indexesList;

    foreach(const ShowfotoItemInfo& info, infos)
    {
        QModelIndex index = indexForShowfotoItemId(info.id);

        if (index.isValid())
        {
            indexesList << index.row();
        }
    }

    removeRowPairsWithCheck(ShowfotoImageModelIncrementalUpdater::toContiguousPairs(indexesList));
}

void ShowfotoImageModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
}

void ShowfotoImageModel::removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove)
{
    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->aboutToBeRemovedInModel(toRemove);
    }

    removeRowPairs(toRemove);
}

void ShowfotoImageModel::removeRowPairs(const QList<QPair<int, int> >& toRemove)
{
    if (toRemove.isEmpty())
    {
        return;
    }

    // Remove old indexes
    // Keep in mind that when calling beginRemoveRows all structures announced to be removed
    // must still be valid, and this includes our hashes as well, which limits what we can optimize

    int                     removedRows = 0;
    int                     offset      = 0;
    typedef QPair<int, int> IntPair;

    foreach(const IntPair& pair, toRemove)
    {
        const int begin = pair.first  - offset;
        const int end   = pair.second - offset;
        removedRows     = end - begin + 1;

        // when removing from the list, all subsequent indexes are affected
        offset += removedRows;

        QList<ShowfotoItemInfo> removedInfos;

        if (d->sendRemovalSignals)
        {
            qCopy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());
            emit itemInfosAboutToBeRemoved(removedInfos);
        }

        showfotoItemInfosAboutToBeRemoved(begin, end);
        beginRemoveRows(QModelIndex(), begin, end);

        // update idHash - which points to indexes of d->infos
        QHash<qlonglong, int>::iterator it;

        for (it = d->idHash.begin(); it != d->idHash.end(); )
        {
            if (it.value() >= begin)
            {
                if (it.value() > end)
                {
                    // after the removed interval, adjust index
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

        endRemoveRows();

        if (d->sendRemovalSignals)
        {
            emit itemInfosRemoved(removedInfos);
        }
    }

    // tidy up: remove old indexes from file path hash now
    if (d->keepFileUrlCache)
    {
        QHash<QString, qlonglong>::iterator it;

        for (it = d->fileUrlHash.begin(); it!= d->fileUrlHash.end(); )
        {
            if (pairsContain(toRemove, it.value()))
            {
                it = d->fileUrlHash.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

// ------------ ShowfotoImageModelIncrementalUpdater ------------

ShowfotoImageModelIncrementalUpdater::ShowfotoImageModelIncrementalUpdater(ShowfotoImageModel::Private* const d)
{
    oldIds = d->idHash;
}

void ShowfotoImageModelIncrementalUpdater::aboutToBeRemovedInModel(const IntPairList& toRemove)
{
    modelRemovals << toRemove;
}

void ShowfotoImageModelIncrementalUpdater::appendInfos(const QList<ShowfotoItemInfo>& infos)
{
    for (int i = 0; i < infos.size(); i++)
    {
        const ShowfotoItemInfo& info = infos.at(i);
        bool found              = false;
        QHash<qlonglong, int>::iterator it;

        for (it = oldIds.find(info.id) ; it != oldIds.end() ; ++it)
        {
            if (it.key() == info.id)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            oldIds.erase(it);
        }
        else
        {
            newInfos << info;
        }
    }
}

QList<QPair<int, int> > ShowfotoImageModelIncrementalUpdater::toContiguousPairs(const QList<int>& unsorted)
{
    // Take the given indices and return them as contiguous pairs [begin, end]

    QList<QPair<int, int> > pairs;

    if (unsorted.isEmpty())
    {
        return pairs;
    }

    QList<int> indices(unsorted);
    qSort(indices);

    QPair<int, int> pair(indices.first(), indices.first());

    for (int i=1; i < indices.size(); i++)
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

QList<QPair<int, int> > ShowfotoImageModelIncrementalUpdater::oldIndexes()
{
    // first, apply all changes to indexes by direct removal in model
    // while the updater was active
    foreach(const IntPairList& list, modelRemovals)
    {
        int removedRows = 0;
        int offset      = 0;

        foreach(const IntPair& pair, list)
        {
            const int begin = pair.first  - offset;
            const int end   = pair.second - offset; // inclusive
            removedRows     = end - begin + 1;

            // when removing from the list, all subsequent indexes are affected
            offset += removedRows;

            // update idHash - which points to indexes of d->infos, and these change now!
            QHash<qlonglong, int>::iterator it;

            for (it = oldIds.begin(); it != oldIds.end(); )
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

// ------------ QAbstractItemModel implementation -------------


int ShowfotoImageModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->infos.size();
}


Qt::ItemFlags ShowfotoImageModel::flags(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return 0;
    }

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    f |= dragDropFlags(index);

    return f;
}

QModelIndex ShowfotoImageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0 || row < 0 || parent.isValid() || row >= d->infos.size())
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

QVariant ShowfotoImageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

QVariant ShowfotoImageModel::data(const QModelIndex& index, int role) const
{
    if (!d->isValid(index))
    {
        return QVariant();
    }

    switch(role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return d->infos.at(index.row()).name;
            break;

        case ShowfotoImageModelPointerRole:
            return QVariant::fromValue(const_cast<ShowfotoImageModel*>(this));
            break;

        case ShowfotoImageModelInternalId:
            return index.row();
            break;
    }

    return QVariant();
}

} // namespace Digikam
