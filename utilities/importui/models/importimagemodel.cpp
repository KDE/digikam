/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-22
 * Description : Qt item model for camera entries
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importimagemodel.h"

// Qt includes

#include <QHash>

// Local includes

#include "digikam_debug.h"
#include "coredbdownloadhistory.h"
#include "cameracontroller.h"

namespace Digikam
{

class ImportImageModel::Private
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
        controller                  = 0;
    }

    inline bool isValid(const QModelIndex& index)
    {
        return (index.isValid()              &&
                (index.row() >= 0)           &&
                (index.row() < infos.size())
               );
    }

public:

    CameraController*                         controller;
    CamItemInfoList                           infos;
    QHash<qlonglong, int>                     idHash;
    QHash<QString, qlonglong>                 fileUrlHash;

    bool                                      keepFileUrlCache;

    bool                                      refreshing;
    bool                                      reAdding;
    bool                                      incrementalRefreshRequested;

    bool                                      sendRemovalSignals;

    class ImportImageModelIncrementalUpdater* incrementalUpdater;
};

// ----------------------------------------------------------------------------------------------------

typedef QPair<int, int> IntPair;
typedef QList<IntPair>  IntPairList;

class ImportImageModelIncrementalUpdater
{
public:

    explicit ImportImageModelIncrementalUpdater(ImportImageModel::Private* const d);

    void            appendInfos(const QList<CamItemInfo>& infos);
    void            aboutToBeRemovedInModel(const IntPairList& aboutToBeRemoved);
    QList<IntPair>  oldIndexes();

    static QList<IntPair> toContiguousPairs(const QList<int>& ids);

public:

    QHash<qlonglong, int> oldIds;
    QList<CamItemInfo>    newInfos;
    QList<IntPairList>    modelRemovals;
};

// ----------------------------------------------------------------------------------------------------

ImportImageModel::ImportImageModel(QObject* const parent)
    : QAbstractListModel(parent),
      d(new Private)
{
}

ImportImageModel::~ImportImageModel()
{
    delete d;
}

void ImportImageModel::setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl)
{
    d->controller = thumbsCtrl->cameraController();

    connect(d->controller, SIGNAL(signalFileList(CamItemInfoList)),
            SLOT(addCamItemInfos(CamItemInfoList)));

    connect(d->controller, SIGNAL(signalDeleted(QString,QString,bool)),
            SLOT(slotFileDeleted(QString,QString,bool)));

    connect(d->controller, SIGNAL(signalUploaded(CamItemInfo)),
            SLOT(slotFileUploaded(CamItemInfo)));
}

void ImportImageModel::setKeepsFileUrlCache(bool keepCache)
{
    d->keepFileUrlCache = keepCache;
}

bool ImportImageModel::keepsFileUrlCache() const
{
    return d->keepFileUrlCache;
}

bool ImportImageModel::isEmpty() const
{
    return d->infos.isEmpty();
}

CamItemInfo ImportImageModel::camItemInfo(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return CamItemInfo();
    }

    return d->infos.at(index.row());
}

CamItemInfo& ImportImageModel::camItemInfoRef(const QModelIndex& index) const
{
    return d->infos[index.row()];
}

qlonglong ImportImageModel::camItemId(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return -1;
    }

    return d->infos.at(index.row()).id;
}

QList<CamItemInfo> ImportImageModel::camItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<CamItemInfo> infos;

    foreach(const QModelIndex& index, indexes)
    {
        infos << camItemInfo(index);
    }

    return infos;
}

QList<qlonglong> ImportImageModel::camItemIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;

    foreach(const QModelIndex& index, indexes)
    {
        ids << camItemId(index);
    }

    return ids;
}

CamItemInfo ImportImageModel::camItemInfo(int row) const
{
    if (row >= d->infos.size())
    {
        return CamItemInfo();
    }

    return d->infos.at(row);
}

CamItemInfo& ImportImageModel::camItemInfoRef(int row) const
{
    return d->infos[row];
}

qlonglong ImportImageModel::camItemId(int row) const
{
    if (row < 0 || (row >= d->infos.size()))
    {
        return -1;
    }

    return d->infos.at(row).id;
}

QModelIndex ImportImageModel::indexForCamItemInfo(const CamItemInfo& info) const
{
    return indexForCamItemId(info.id);
}

QList<QModelIndex> ImportImageModel::indexesForCamItemInfo(const CamItemInfo& info) const
{
    return indexesForCamItemId(info.id);
}

QModelIndex ImportImageModel::indexForCamItemId(qlonglong id) const
{
    int index = d->idHash.value(id, 0);

    if (index != -1)
    {
        return createIndex(index, 0);
    }

    return QModelIndex();
}

QList<QModelIndex> ImportImageModel::indexesForCamItemId(qlonglong id) const
{
    QList<QModelIndex> indexes;

    QHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id); it != d->idHash.constEnd() && it.key() == id; ++it)
    {
       indexes << createIndex(it.value(), 0);
    }

    return indexes;
}

int ImportImageModel::numberOfIndexesForCamItemInfo(const CamItemInfo& info) const
{
    return numberOfIndexesForCamItemId(info.id);
}

int ImportImageModel::numberOfIndexesForCamItemId(qlonglong id) const
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
CamItemInfo ImportImageModel::retrieveCamItemInfo(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return CamItemInfo();
    }

    ImportImageModel* const model = index.data(ImportImageModelPointerRole).value<ImportImageModel*>();
    int row                       = index.data(ImportImageModelInternalId).toInt();

    if (!model)
    {
        return CamItemInfo();
    }

    return model->camItemInfo(row);
}

// static method
qlonglong ImportImageModel::retrieveCamItemId(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return -1;
    }

    ImportImageModel* const model = index.data(ImportImageModelPointerRole).value<ImportImageModel*>();
    int                     row   = index.data(ImportImageModelInternalId).toInt();

    if (!model)
    {
        return -1;
    }

    return model->camItemId(row);
}

QModelIndex ImportImageModel::indexForUrl(const QUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        return indexForCamItemId(d->fileUrlHash.value(fileUrl.toLocalFile()));
    }
    else
    {
        const int size = d->infos.size();

        for (int i = 0; i < size; i++)
        {
            if (d->infos.at(i).url() == fileUrl)
            {
                return createIndex(i, 0);
            }
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ImportImageModel::indexesForUrl(const QUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        return indexesForCamItemId(d->fileUrlHash.value(fileUrl.toLocalFile()));
    }
    else
    {
        QList<QModelIndex> indexes;
        const int          size = d->infos.size();

        for (int i = 0; i < size; i++)
        {
            if (d->infos.at(i).url() == fileUrl)
            {
                indexes << createIndex(i, 0);
            }
        }

        return indexes;
    }
}

CamItemInfo ImportImageModel::camItemInfo(const QUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        qlonglong id = d->fileUrlHash.value(fileUrl.toLocalFile());

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
        foreach(const CamItemInfo& info, d->infos)
        {
            if (info.url() == fileUrl)
            {
                return info;
            }
        }
    }

    return CamItemInfo();
}

QList<CamItemInfo> ImportImageModel::camItemInfos(const QUrl& fileUrl) const
{
    QList<CamItemInfo> infos;

    if (d->keepFileUrlCache)
    {
        qlonglong id = d->fileUrlHash.value(fileUrl.toLocalFile());

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
        foreach(const CamItemInfo& info, d->infos)
        {
            if (info.url() == fileUrl)
            {
                infos << info;
            }
        }
    }

    return infos;
}

void ImportImageModel::addCamItemInfo(const CamItemInfo& info)
{
    addCamItemInfos(QList<CamItemInfo>() << info);
}

void ImportImageModel::addCamItemInfos(const CamItemInfoList& infos)
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

void ImportImageModel::addCamItemInfoSynchronously(const CamItemInfo& info)
{
    addCamItemInfosSynchronously(QList<CamItemInfo>() << info);
}

void ImportImageModel::addCamItemInfosSynchronously(const CamItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);
    emit processAdded(infos);
}

void ImportImageModel::clearCamItemInfos()
{
    d->infos.clear();
    d->idHash.clear();
    d->fileUrlHash.clear();

    delete d->incrementalUpdater;

    d->incrementalUpdater          = 0;
    d->reAdding                    = false;
    d->refreshing                  = false;
    d->incrementalRefreshRequested = false;

    beginResetModel();
    camItemInfosCleared();
    endResetModel();
}

// TODO unused
void ImportImageModel::setCamItemInfos(const CamItemInfoList& infos)
{
    clearCamItemInfos();
    addCamItemInfos(infos);
}

QList<CamItemInfo> ImportImageModel::camItemInfos() const
{
    return d->infos;
}

QList<qlonglong> ImportImageModel::camItemIds() const
{
    return d->idHash.keys();
}

QList<CamItemInfo> ImportImageModel::uniqueCamItemInfos() const
{
    QList<CamItemInfo> uniqueInfos;
    const int          size = d->infos.size();

    for (int i = 0; i < size; i++)
    {
        const CamItemInfo& info = d->infos.at(i);

        if (d->idHash.value(info.id) == i)
        {
            uniqueInfos << info;
        }
    }

    return uniqueInfos;
}

bool ImportImageModel::hasImage(qlonglong id) const
{
    return d->idHash.contains(id);
}

bool ImportImageModel::hasImage(const CamItemInfo& info) const
{
    return d->fileUrlHash.contains(info.url().toLocalFile());
}

void ImportImageModel::emitDataChangedForAll()
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QModelIndex first = createIndex(0, 0);
    QModelIndex last  = createIndex(d->infos.size() - 1, 0);
    emit dataChanged(first, last);
}

void ImportImageModel::emitDataChangedForSelections(const QItemSelection& selection)
{
    if (!selection.isEmpty())
    {
        foreach(const QItemSelectionRange& range, selection)
        {
            emit dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

void ImportImageModel::appendInfos(const CamItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);
}

void ImportImageModel::reAddCamItemInfos(const CamItemInfoList& infos)
{
    publiciseInfos(infos);
}

void ImportImageModel::reAddingFinished()
{
    d->reAdding = false;
    cleanSituationChecks();
}

void ImportImageModel::slotFileDeleted(const QString& folder, const QString& file, bool status)
{
    Q_UNUSED(status)

    QUrl url = QUrl::fromLocalFile(folder);
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + (file));
    CamItemInfo info = camItemInfo(url);
    removeCamItemInfo(info);
}

void ImportImageModel::slotFileUploaded(const CamItemInfo& info)
{
    addCamItemInfo(info);
}

void ImportImageModel::startRefresh()
{
    d->refreshing = true;
}

void ImportImageModel::finishRefresh()
{
    d->refreshing = false;
    cleanSituationChecks();
}

bool ImportImageModel::isRefreshing() const
{
    return d->refreshing;
}

void ImportImageModel::cleanSituationChecks()
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

void ImportImageModel::publiciseInfos(const CamItemInfoList& infos)
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
        CamItemInfo& info = d->infos[i];

        // TODO move this to a separate thread, see CameraHistoryUpdater
        // TODO this is ugly, using different enums to point the similar status..
        // TODO can we/do we want to differentiate at all between whether the status is unknown and not downloaded?
        CoreDbDownloadHistory::Status status = CoreDbDownloadHistory::status(QString::fromUtf8(d->controller->cameraMD5ID()), info.name, info.size, info.ctime);
        info.downloaded  = status;
        // TODO is this safe? if so, is there a need to store this inside idHash separately?
        info.id = i;

        qlonglong id            = info.id;
        d->idHash.insertMulti(id, i);

        if (d->keepFileUrlCache)
        {
            d->fileUrlHash[info.url().toLocalFile()] = id;
        }
    }

    endInsertRows();
    emit processAdded(infos);
    emit itemInfosAdded(infos);
}

void ImportImageModel::requestIncrementalRefresh()
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

bool ImportImageModel::hasIncrementalRefreshPending() const
{
    return d->incrementalRefreshRequested;
}

void ImportImageModel::startIncrementalRefresh()
{
    delete d->incrementalUpdater;

    d->incrementalUpdater = new ImportImageModelIncrementalUpdater(d);
}

void ImportImageModel::finishIncrementalRefresh()
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

    while(n > 0)
    {
        int half   = n >> 1;
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

void ImportImageModel::removeIndex(const QModelIndex& index)
{
    removeIndexs(QList<QModelIndex>() << index);
}

void ImportImageModel::removeIndexs(const QList<QModelIndex>& indexes)
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

    removeRowPairsWithCheck(ImportImageModelIncrementalUpdater::toContiguousPairs(indexesList));
}

void ImportImageModel::removeCamItemInfo(const CamItemInfo& info)
{
    removeCamItemInfos(QList<CamItemInfo>() << info);
}

void ImportImageModel::removeCamItemInfos(const QList<CamItemInfo>& infos)
{
    QList<int> indexesList;

    foreach(const CamItemInfo& info, infos)
    {
        QModelIndex index = indexForCamItemId(info.id);

        if (index.isValid())
        {
            indexesList << index.row();
        }
    }

    removeRowPairsWithCheck(ImportImageModelIncrementalUpdater::toContiguousPairs(indexesList));
}

void ImportImageModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
}

void ImportImageModel::removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove)
{
    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->aboutToBeRemovedInModel(toRemove);
    }

    removeRowPairs(toRemove);
}

void ImportImageModel::removeRowPairs(const QList<QPair<int, int> >& toRemove)
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

    foreach(const IntPair& pair, toRemove)
    {
        const int begin = pair.first  - offset;
        const int end   = pair.second - offset;
        removedRows     = end - begin + 1;

        // when removing from the list, all subsequent indexes are affected
        offset += removedRows;

        QList<CamItemInfo> removedInfos;

        if (d->sendRemovalSignals)
        {
            std::copy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());
            emit itemInfosAboutToBeRemoved(removedInfos);
        }

        itemInfosAboutToBeRemoved(begin, end);
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

// ------------ ImportImageModelIncrementalUpdater ------------

ImportImageModelIncrementalUpdater::ImportImageModelIncrementalUpdater(ImportImageModel::Private* const d)
{
    oldIds = d->idHash;
}

void ImportImageModelIncrementalUpdater::aboutToBeRemovedInModel(const IntPairList& toRemove)
{
    modelRemovals << toRemove;
}

void ImportImageModelIncrementalUpdater::appendInfos(const QList<CamItemInfo>& infos)
{
    for (int i = 0; i < infos.size(); i++)
    {
        const CamItemInfo& info = infos.at(i);
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

QList<QPair<int, int> > ImportImageModelIncrementalUpdater::toContiguousPairs(const QList<int>& unsorted)
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

QList<QPair<int, int> > ImportImageModelIncrementalUpdater::oldIndexes()
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

QVariant ImportImageModel::data(const QModelIndex& index, int role) const
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

        case ImportImageModelPointerRole:
            return QVariant::fromValue(const_cast<ImportImageModel*>(this));
            break;

        case ImportImageModelInternalId:
            return index.row();
            break;
    }

    return QVariant();
}

QVariant ImportImageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

int ImportImageModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->infos.size();
}

Qt::ItemFlags ImportImageModel::flags(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return 0;
    }

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    f |= dragDropFlags(index);

    return f;
}

QModelIndex ImportImageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0 || row < 0 || parent.isValid() || row >= d->infos.size())
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

} // namespace Digikam
