/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-22
 * Description : Qt item model for camera entries
 *
 * Copyright (C) 2009-2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importimagemodel.moc"

// Qt includes

#include <QHash>

namespace Digikam
{

class ImportImageModel::ImportImageModelPriv
{
public:

    ImportImageModelPriv()
    {
        keepFileUrlCache   = false;
        incrementalUpdater = 0;
    }

    CamItemInfoList                           infos;
    QHash<qlonglong, int>                     idHash;
    QHash<QString, qlonglong>                 fileUrlHash;

    bool                                      keepFileUrlCache;
    //bool                                    incremental

    class ImportImageModelIncrementalUpdater* incrementalUpdater;

    inline bool isValid(const QModelIndex& index)
    {
        return index.isValid() && index.row() >= 0 && index.row() < infos.size();
    }
};

// ----------------------------------------------------------------------------------------------------

typedef QPair<int, int> IntPair;
typedef QList<IntPair>  IntPairList;

class ImportImageModelIncrementalUpdater
{
public:

    ImportImageModelIncrementalUpdater(ImportImageModel::ImportImageModelPriv* d);

    void                  aboutToBeRemovedInModel(const IntPairList& aboutToBeRemoved);
    QList<IntPair>        oldIndexes();

    static QList<IntPair> toContiguousPairs(const QList<int>& ids);

public:

    QHash<qlonglong, int>    oldIds;
    QList<ImportImageModel>  newInfos;
    QList<IntPairList>       modelRemovals;
};

// ----------------------------------------------------------------------------------------------------

ImportImageModel::ImportImageModel(CameraController* const controller, QObject* const parent)
    : QAbstractListModel(parent),
      d(new ImportImageModelPriv)
{
    //TODO: Connections with the controller
}

ImportImageModel::~ImportImageModel()
{
    delete d;
}

bool ImportImageModel::isEmpty() const
{
    return d->infos.isEmpty();
}

CamItemInfo ImportImageModel::camItemInfo(const QModelIndex& index) const
{
    if(!d->isValid(index))
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
    if(!d->isValid(index))
    {
        return -1;
    }

    return d->infos.at(index.row()).id;
}

//FIXME:
QList<CamItemInfo> ImportImageModel::camItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<CamItemInfo> infos;
    foreach(const QModelIndex& index, indexes)
    {
        //FIXME:
        //infos << CamItemInfo(index);
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
    if(row >= d->infos.size())
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
    if(row < 0 || row >= d->infos.size())
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

    if(index != -1)
    {
        return createIndex(index, 0);
    }

    return QModelIndex();
}

QList<QModelIndex> ImportImageModel::indexesForCamItemId(qlonglong id) const
{
    QList<QModelIndex> indexes;

    QHash<qlonglong, int>::const_iterator it;
    for(it = d->idHash.constFind(id); it != d->idHash.constEnd() && it.key() == id; ++it)
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
    if(!index.isValid())
    {
        return CamItemInfo();
    }

    ImportImageModel* model = index.data(ImportImageModelPointerRole).value<ImportImageModel*>();
    int row                 = index.data(ImportImageModelInternalId).toInt();

    if(!model)
    {
        return CamItemInfo();
    }

    return model->camItemInfo(row);
}

// static method
qlonglong ImportImageModel::retrieveCamItemId(const QModelIndex& index)
{
    if(!index.isValid())
    {
        return -1;
    }

    ImportImageModel* model = index.data(ImportImageModelPointerRole).value<ImportImageModel*>();
    int               row   = index.data(ImportImageModelInternalId).toInt();

    if(!model)
    {
        return -1;
    }

    return model->camItemId(row);
}

QModelIndex ImportImageModel::indexForUrl(const KUrl &fileUrl) const
{
    if(d->keepFileUrlCache)
    {
        return indexForCamItemId(d->fileUrlHash.value(fileUrl.prettyUrl()));
    }
    else
    {
        const int size = d->infos.size();

        for(int i = 0; i < size; ++i)
        {
            if(d->infos.at(i).url() == fileUrl)
            {
                return createIndex(i, 0);
            }
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ImportImageModel::indexesForUrl(const KUrl& fileUrl) const
{
    if(d->keepFileUrlCache)
    {
        return indexesForCamItemId(d->fileUrlHash.value(fileUrl.prettyUrl()));
    }
    else
    {
        QList<QModelIndex> indexes;
        const int size = d->infos.size();

        for(int i = 0; i < size; ++i)
        {
            if(d->infos.at(i).url() == fileUrl)
            {
                indexes << createIndex(i, 0);
            }
        }

        return indexes;
    }
}

} // namespace Digikam
