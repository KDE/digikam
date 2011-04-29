/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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

#include "imagemodel.moc"

// Qt includes

#include <QHash>
#include <QItemSelection>

// KDE includes

#include <kdebug.h>

// Local includes

#include "databasechangesets.h"
#include "databasefields.h"
#include "databasewatch.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "abstractitemdragdrophandler.h"

namespace Digikam
{

class ImageModel::ImageModelPriv
{
public:

    ImageModelPriv()
    {
        preprocessor                = 0;
        keepFilePathCache           = false;
        sendRemovalSignals          = false;
        incrementalUpdater          = 0;
        refreshing                  = false;
        reAdding                    = false;
        incrementalRefreshRequested = false;
    }

    ImageInfoList                       infos;
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

    class ImageModelIncrementalUpdater* incrementalUpdater;
};

class ImageModelIncrementalUpdater
{
public:

    ImageModelIncrementalUpdater(ImageModel::ImageModelPriv* d);

    void                          appendInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    QList<QPair<int,int> >        oldIndexes() const;

    static QList<QPair<int,int> > toContiguousPairs(const QList<int>& ids);

public:

    QHash<qlonglong, int> oldIds;
    QList<QVariant>       oldExtraValues;
    QList<ImageInfo>      newInfos;
    QList<QVariant>       newExtraValues;
};

ImageModel::ImageModel(QObject* parent)
    : QAbstractListModel(parent),
      d(new ImageModelPriv)
{
    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset&)),
            this, SLOT(slotImageChange(const ImageChangeset&)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(const ImageTagChangeset&)),
            this, SLOT(slotImageTagChange(const ImageTagChangeset&)));
}

ImageModel::~ImageModel()
{
    delete d->incrementalUpdater;
    delete d;
}

// ------------ Access methods -------------

void ImageModel::setKeepsFilePathCache(bool keepCache)
{
    d->keepFilePathCache = keepCache;
}

bool ImageModel::keepsFilePathCache() const
{
    return d->keepFilePathCache;
}

bool ImageModel::isEmpty() const
{
    return d->infos.isEmpty();
}

void ImageModel::setWatchFlags(const DatabaseFields::Set& set)
{
    d->watchFlags = set;
}

ImageInfo ImageModel::imageInfo(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return ImageInfo();
    }

    return d->infos[index.row()];
}

ImageInfo& ImageModel::imageInfoRef(const QModelIndex& index) const
{
    return d->infos[index.row()];
}

qlonglong ImageModel::imageId(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    return d->infos[index.row()].id();
}

QList<ImageInfo> ImageModel::imageInfos(const QList<QModelIndex>& indexes) const
{
    QList<ImageInfo> infos;
    foreach (const QModelIndex& index, indexes)
    {
        infos << imageInfo(index);
    }
    return infos;
}

QList<qlonglong> ImageModel::imageIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;
    foreach (const QModelIndex& index, indexes)
    {
        ids << imageId(index);
    }
    return ids;
}

ImageInfo ImageModel::imageInfo(int row) const
{
    if (row >= d->infos.size())
    {
        return ImageInfo();
    }

    return d->infos[row];
}

ImageInfo& ImageModel::imageInfoRef(int row) const
{
    return d->infos[row];
}

qlonglong ImageModel::imageId(int row) const
{
    if (row >= d->infos.size())
    {
        return -1;
    }

    return d->infos[row].id();
}

QModelIndex ImageModel::indexForImageInfo(const ImageInfo& info) const
{
    return indexForImageId(info.id());
}

QModelIndex ImageModel::indexForImageInfo(const ImageInfo& info, const QVariant& extraValue) const
{
    return indexForImageId(info.id(), extraValue);
}

QList<QModelIndex> ImageModel::indexesForImageInfo(const ImageInfo& info) const
{
    return indexesForImageId(info.id());
}

QModelIndex ImageModel::indexForImageId(qlonglong id) const
{
    int index = d->idHash.value(id, -1);

    if (index != -1)
    {
        return createIndex(index, 0);
    }

    return QModelIndex();
}

QModelIndex ImageModel::indexForImageId(qlonglong id, const QVariant& extraValue) const
{
    if (d->extraValues.isEmpty())
        return indexForImageId(id);

    QHash<qlonglong, int>::iterator it;
    for (it = d->idHash.find(id); it != d->idHash.end() && it.key() == id; ++it)
    {
        if (d->extraValues[it.value()] == extraValue)
            return createIndex(it.value(), 0);
    }
    return QModelIndex();
}

QList<QModelIndex> ImageModel::indexesForImageId(qlonglong id) const
{
    QList<QModelIndex> indexes;

    QHash<qlonglong, int>::iterator it;
    for (it = d->idHash.find(id); it != d->idHash.end() && it.key() == id; ++it)
    {
        indexes << createIndex(it.value(), 0);
    }

    return indexes;
}

int ImageModel::numberOfIndexesForImageInfo(const ImageInfo& info) const
{
    return numberOfIndexesForImageId(info.id());
}

int ImageModel::numberOfIndexesForImageId(qlonglong id) const
{
    if (d->extraValues.isEmpty())
    {
        return 0;
    }

    int count = 0;
    QHash<qlonglong,int>::iterator it;
    for (it = d->idHash.find(id); it != d->idHash.end() && it.key() == id; ++it)
    {
        ++count;
    }

    return count;
}

// static method
ImageInfo ImageModel::retrieveImageInfo(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return ImageInfo();
    }

    ImageModel* model = index.data(ImageModelPointerRole).value<ImageModel*>();
    int row = index.data(ImageModelInternalId).toInt();
    return model->imageInfo(row);
}

// static method
qlonglong ImageModel::retrieveImageId(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return 0;
    }

    ImageModel* model = index.data(ImageModelPointerRole).value<ImageModel*>();
    int row = index.data(ImageModelInternalId).toInt();
    return model->imageId(row);
}

QModelIndex ImageModel::indexForPath(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        return indexForImageId(d->filePathHash.value(filePath));
    }
    else
    {
        const int size = d->infos.size();

        for (int i=0; i<size; ++i)
        {
            if (d->infos[i].filePath() == filePath)
            {
                return createIndex(i, 0);
            }
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ImageModel::indexesForPath(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        return indexesForImageId(d->filePathHash.value(filePath));
    }
    else
    {
        QList<QModelIndex> indexes;
        const int size = d->infos.size();

        for (int i=0; i<size; ++i)
            if (d->infos[i].filePath() == filePath)
            {
                indexes << createIndex(i, 0);
            }

        return indexes;
    }
}

ImageInfo ImageModel::imageInfo(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        qlonglong id = d->filePathHash.value(filePath);

        if (id)
        {
            int index = d->idHash.value(id, -1);

            if (index != -1)
            {
                return d->infos[index];
            }
        }
    }
    else
    {
        foreach (const ImageInfo& info, d->infos)
        {
            if (info.filePath() == filePath)
            {
                return info;
            }
        }
    }

    return ImageInfo();
}

QList<ImageInfo> ImageModel::imageInfos(const QString& filePath) const
{
    QList<ImageInfo> infos;

    if (d->keepFilePathCache)
    {
        qlonglong id = d->filePathHash.value(filePath);

        if (id)
        {
            foreach (int index, d->idHash.values(id))
            {
                infos << d->infos[index];
            }
        }
    }
    else
    {
        foreach (const ImageInfo& info, d->infos)
        {
            if (info.filePath() == filePath)
            {
                infos << info;
            }
        }
    }

    return infos;
}

void ImageModel::addImageInfo(const ImageInfo& info)
{
    addImageInfos(QList<ImageInfo>() << info, QList<QVariant>());
}

void ImageModel::addImageInfos(const QList<ImageInfo>& infos)
{
    addImageInfos(infos, QList<QVariant>());
}

void ImageModel::addImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
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

void ImageModel::addImageInfoSynchronously(const ImageInfo& info)
{
    addImageInfosSynchronously(QList<ImageInfo>() << info, QList<QVariant>());
}

void ImageModel::addImageInfosSynchronously(const QList<ImageInfo>& infos)
{
    addImageInfos(infos, QList<QVariant>());
}

void ImageModel::addImageInfosSynchronously(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos, extraValues);
    emit processAdded(infos, extraValues);
}

void ImageModel::clearImageInfos()
{
    d->infos.clear();
    d->extraValues.clear();
    d->idHash.clear();
    d->filePathHash.clear();
    delete d->incrementalUpdater;
    d->incrementalUpdater = 0;
    d->refreshing = false;
    d->reAdding = false;
    d->incrementalRefreshRequested = false;
    reset();
    imageInfosCleared();
}

void ImageModel::setImageInfos(const QList<ImageInfo>& infos)
{
    clearImageInfos();
    addImageInfos(infos);
}

QList<ImageInfo> ImageModel::imageInfos() const
{
    return d->infos;
}

QList<qlonglong> ImageModel::imageIds() const
{
    return d->idHash.keys();
}

bool ImageModel::hasImage(qlonglong id) const
{
    return d->idHash.contains(id);
}

bool ImageModel::hasImage(const ImageInfo& info) const
{
    return d->idHash.contains(info.id());
}

QList<ImageInfo> ImageModel::uniqueImageInfos() const
{
    if (d->extraValues.isEmpty())
    {
        return d->infos;
    }

    QList<ImageInfo> uniqueInfos;
    const int size = d->infos.size();
    for (int i=0; i<size; ++i)
    {
        const ImageInfo& info = d->infos[i];
        if (d->idHash.value(info.id()) == i)
        {
            uniqueInfos << info;
        }
    }
    return uniqueInfos;
}

void ImageModel::emitDataChangedForAll()
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QModelIndex first = createIndex(0, 0);
    QModelIndex last = createIndex(d->infos.size() - 1, 0);
    emit dataChanged(first, last);
}

void ImageModel::emitDataChangedForSelection(const QItemSelection& selection)
{
    if (!selection.isEmpty())
    {
        foreach (const QItemSelectionRange& range, selection)
        {
            emit dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

// ------------ Preprocessing -------------

void ImageModel::setPreprocessor(QObject* preprocessor)
{
    unsetPreprocessor(d->preprocessor);
    d->preprocessor = preprocessor;
}

void ImageModel::unsetPreprocessor(QObject* preprocessor)
{
    if (preprocessor && d->preprocessor == preprocessor)
    {
        disconnect(this, SIGNAL(preprocess(const QList<ImageInfo> &, const QList<QVariant> &)), 0, 0);
        disconnect(d->preprocessor, 0, this, SLOT(reAddImageInfos(const QList<ImageInfo> &, const QList<QVariant> &)));
        disconnect(d->preprocessor, 0, this, SLOT(reAddingFinished()));
    }
}

void ImageModel::appendInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
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

void ImageModel::reAddImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    // addImageInfos -> appendInfos -> preprocessor -> reAddImageInfos
    publiciseInfos(infos, extraValues);
}

void ImageModel::reAddingFinished()
{
    d->reAdding = false;
    cleanSituationChecks();
}

void ImageModel::startRefresh()
{
    d->refreshing = true;
}

void ImageModel::finishRefresh()
{
    d->refreshing = false;
    cleanSituationChecks();
}

bool ImageModel::isRefreshing() const
{
    return d->refreshing;
}

void ImageModel::cleanSituationChecks()
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

void ImageModel::publiciseInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    emit imageInfosAboutToBeAdded(infos);
    const int firstNewIndex = d->infos.size();
    const int lastNewIndex  = d->infos.size() + infos.size() - 1;
    beginInsertRows(QModelIndex(), firstNewIndex, lastNewIndex);
    d->infos << infos;
    d->extraValues << extraValues;

    for (int i=firstNewIndex; i<=lastNewIndex; ++i)
    {
        const ImageInfo& info = d->infos[i];
        qlonglong id = info.id();
        d->idHash.insertMulti(id, i);

        if (d->keepFilePathCache)
        {
            d->filePathHash[info.filePath()] = id;
        }
    }

    endInsertRows();
    emit imageInfosAdded(infos);
}

void ImageModel::requestIncrementalRefresh()
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

bool ImageModel::hasIncrementalRefreshPending() const
{
    return d->incrementalRefreshRequested;
}

void ImageModel::startIncrementalRefresh()
{
    delete d->incrementalUpdater;

    d->incrementalUpdater = new ImageModelIncrementalUpdater(d);
}

void ImageModel::finishIncrementalRefresh()
{
    if (!d->incrementalUpdater)
    {
        return;
    }

    // remove old entries
    QList<QPair<int,int> > pairs = d->incrementalUpdater->oldIndexes();
    removeRowPairs(pairs);

    // add new indexes
    appendInfos(d->incrementalUpdater->newInfos, d->incrementalUpdater->newExtraValues);

    delete d->incrementalUpdater;
    d->incrementalUpdater = 0;
}

void ImageModel::removeIndex(const QModelIndex& index)
{
    removeIndexes(QList<QModelIndex>() << index);
}

void ImageModel::removeIndexes(const QList<QModelIndex>& indexes)
{
    QList<int> listIndexes;
    foreach (const QModelIndex& index, indexes)
    {
        if (index.isValid())
        {
            listIndexes << index.row();
        }
    }
    removeRowPairs(ImageModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ImageModel::removeImageInfo(const ImageInfo& info)
{
    removeImageInfos(QList<ImageInfo>() << info);
}

void ImageModel::removeImageInfos(const QList<ImageInfo>& infos)
{
    QList<int> listIndexes;
    foreach (const ImageInfo& info, infos)
    {
        QModelIndex index = indexForImageId(info.id());
        if (index.isValid())
            listIndexes << index.row();
    }
    removeRowPairs(ImageModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ImageModel::removeImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        removeImageInfos(infos);
        return;
    }

    QList<int> listIndexes;

    for (int i=0; i<infos.size(); i++)
    {
        QModelIndex index = indexForImageId(infos[i].id(), extraValues[i]);
        if (index.isValid())
            listIndexes << index.row();
    }

    removeRowPairs(ImageModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ImageModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
}

template <class List, typename T>
static bool pairsContain(const List& list, T value)
{
    typename List::const_iterator begin = list.begin();
    typename List::const_iterator end   = list.end();
    typename List::const_iterator middle;
    int n = int(end - begin);
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
            n -= half + 1;
        }
        else
        {
            n = half;
        }
    }

    return false;
}

void ImageModel::removeRowPairs(const QList<QPair<int,int> >& toRemove)
{
    if (toRemove.isEmpty())
    {
        return;
    }
    // Remove old indexes
    // Keep in mind that when calling beginRemoveRows all structures announced to be removed
    // must still be valid, and this includes our hashes as well, which limits what we can optimize

    int removedRows = 0, offset = 0;
    typedef QPair<int,int> IntPair; // to make foreach macro happy
    foreach (const IntPair& pair, toRemove)
    {
        const int begin = pair.first - offset;
        const int end   = pair.second - offset; // inclusive
        removedRows = end - begin + 1;
        // when removing from the list, all subsequent indexes are affected
        offset += removedRows;

        QList<ImageInfo> removedInfos;
        if (d->sendRemovalSignals)
        {
            qCopy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());
            emit imageInfosAboutToBeRemoved(removedInfos);
        }
        beginRemoveRows(QModelIndex(), begin, end);

        // update idHash - which points to indexes of d->infos, and these change now!
        QHash<qlonglong, int>::iterator it;

        for (it = d->idHash.begin(); it != d->idHash.end(); )
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

        for (it = d->filePathHash.begin(); it != d->filePathHash.end(); )
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

ImageModelIncrementalUpdater::ImageModelIncrementalUpdater(ImageModel::ImageModelPriv* d)
{
    oldIds         = d->idHash;
    oldExtraValues = d->extraValues;
}

void ImageModelIncrementalUpdater::appendInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        foreach (const ImageInfo& info, infos)
        {
            QHash<qlonglong,int>::iterator it = oldIds.find(info.id());

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
        for (int i=0; i<infos.size(); i++)
        {
            const ImageInfo& info = infos[i];
            bool found            = false;
            QHash<qlonglong,int>::iterator it;

            for (it = oldIds.find(info.id()); it != oldIds.end() && it.key() == info.id(); ++it)
            {
                // first check is for bug #262596. Not sure if needed.
                if (it.value() < oldExtraValues.size() && extraValues[i] == oldExtraValues[it.value()])
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                oldIds.erase(it);
                // dont erase from oldExtraValues - oldIds is a hash id -> index.
            }
            else
            {
                newInfos << info;
                newExtraValues << extraValues[i];
            }
        }
    }
}

QList<QPair<int,int> > ImageModelIncrementalUpdater::oldIndexes() const
{
    return toContiguousPairs(oldIds.values());
}

QList<QPair<int,int> > ImageModelIncrementalUpdater::toContiguousPairs(const QList<int>& unsorted)
{
    // Take the given indices and return them as contiguous pairs [begin, end]

    QList<QPair<int,int> > pairs;

    if (unsorted.isEmpty())
    {
        return pairs;
    }

    QList<int> indices(unsorted);
    qSort(indices);

    QPair<int,int> pair(indices.first(), indices.first());

    for (int i=1; i<indices.size(); ++i)
    {
        int index = indices[i];

        if (index == pair.second + 1)
        {
            pair.second = index;
            continue;
        }

        pairs << pair; // insert last pair
        pair.first = index;
        pair.second  = index;
    }

    pairs << pair;

    return pairs;
}

// ------------ QAbstractItemModel implementation -------------

QVariant ImageModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= d->infos.size())
    {
        return QVariant();
    }

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return d->infos[index.row()].name();
        case ImageModelPointerRole:
            return QVariant::fromValue(const_cast<ImageModel*>(this));
        case ImageModelInternalId:
            return index.row();
        case CreationDateRole:
            return d->infos[index.row()].dateTime();
        case ExtraDataRole:

            if (!d->extraValues.isEmpty())
            {
                return d->extraValues[index.row()];
            }
            else
            {
                return QVariant();
            }

        case ExtraDataDuplicateCount:
        {
            qlonglong id = d->infos[index.row()].id();
            return numberOfIndexesForImageId(id);
        }
    }

    return QVariant();
}

QVariant ImageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

int ImageModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->infos.size();
}

Qt::ItemFlags ImageModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    f |= dragDropFlags(index);

    return f;
}

QModelIndex ImageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0 || row < 0 || parent.isValid() || row >= d->infos.size())
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

// ------------ Database watch -------------

void ImageModel::slotImageChange(const ImageChangeset& changeset)
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

void ImageModel::slotImageTagChange(const ImageTagChangeset& changeset)
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
