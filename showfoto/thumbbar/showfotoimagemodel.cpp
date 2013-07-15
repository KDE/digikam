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

#include "showfotoimagemodel.moc"
#include "showfotoimagemodel.h"

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

void ShowfotoImageModel::reAddShowfotoItemInfos(ShowfotoItemInfoList& infos)
{
    qDebug() << "hey I got the list now";
    publiciseInfos(infos);
}


void ShowfotoImageModel::publiciseInfos(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    emit ShowfotoItemInfosAboutToBeAdded(infos);

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
    emit ShowfotoItemInfosAdded(infos);
}

// ------------  Drag and Drop Methods ------------------------

void ShowfotoImageModel::setSendRemovalSignals(bool send)
{
     d->sendRemovalSignals = send;
}

bool ShowfotoImageModel::isRefreshing() const
{
    return d->refreshing;
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

// --------------------------------------------------------------












} // namespace Digikam
