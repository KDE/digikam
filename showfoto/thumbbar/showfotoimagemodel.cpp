/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-05
 * Description : Qt model for Showfoto entries
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "showfotoimagemodel.h"

// Qt includes

#include <QHash>

// Local includes

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
    QHash<qlonglong, int>                     idHash;
    QHash<QString, qlonglong>                 fileUrlHash;

    bool                                      keepFileUrlCache;

    bool                                      refreshing;
    bool                                      reAdding;
    bool                                      incrementalRefreshRequested;

    bool                                      sendRemovalSignals;
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

QList<ShowfotoItemInfo> ShowfotoImageModel::showfotoItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<ShowfotoItemInfo> infos;

    foreach(const QModelIndex& index, indexes)
    {
        infos << showfotoItemInfo(index);
    }

    return infos;
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

QModelIndex ShowfotoImageModel::indexForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return indexForUrl(info.url);
}

QList<QModelIndex> ShowfotoImageModel::indexesForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return indexesForUrl(info.url);
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

QModelIndex ShowfotoImageModel::indexForUrl(const QUrl& fileUrl) const
{
        const int size = d->infos.size();

        for (int i = 0; i < size; i++)
        {
            if (d->infos.at(i).url == fileUrl)
            {
                return createIndex(i, 0);
            }
        }

    return QModelIndex();
}

QList<QModelIndex> ShowfotoImageModel::indexesForUrl(const QUrl& fileUrl) const
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

ShowfotoItemInfo ShowfotoImageModel::showfotoItemInfo(const QUrl& fileUrl) const
{
        foreach(const ShowfotoItemInfo& info, d->infos)
        {
            if (info.url == fileUrl)
            {
                return info;
            }
        }

    return ShowfotoItemInfo();
}

QList<ShowfotoItemInfo> ShowfotoImageModel::showfotoItemInfos(const QUrl& fileUrl) const
{
    QList<ShowfotoItemInfo> infos;


        foreach(const ShowfotoItemInfo& info, d->infos)
        {
            if (info.url == fileUrl)
            {
                infos << info;
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

        appendInfos(infos);

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
    d->fileUrlHash.clear();
    d->reAdding                    = false;
    d->refreshing                  = false;
    d->incrementalRefreshRequested = false;

    beginResetModel();
    showfotoItemInfosCleared();
    endResetModel();
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

bool ShowfotoImageModel::hasImage(const ShowfotoItemInfo& info) const
{
    return d->fileUrlHash.contains(info.url.toDisplayString());
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
    publiciseInfos(infos);
}

void ShowfotoImageModel::reAddingFinished()
{
    d->reAdding = false;
    //cleanSituationChecks();
}

void ShowfotoImageModel::slotFileDeleted(const QString& folder, const QString& file, bool status)
{
    Q_UNUSED(status)

    ShowfotoItemInfo info = showfotoItemInfo(QUrl::fromLocalFile(folder + file));
    //removeShowfotoItemInfo(info);
}

void ShowfotoImageModel::slotFileUploaded(const ShowfotoItemInfo& info)
{
    addShowfotoItemInfo(info);
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
            d->fileUrlHash[info.url.toDisplayString()] = id;
        }
    }

    endInsertRows();
    emit itemInfosAdded(infos);
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

    removeRowPairs(toContiguousPairs(indexesList));
}

void ShowfotoImageModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
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
            std::copy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());
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

QList<QPair<int, int> > ShowfotoImageModel::toContiguousPairs(const QList<int>& unsorted)
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
