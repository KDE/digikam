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

#include "imagemodel.h"
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
#include "imagemodeldragdrophandler.h"

namespace Digikam
{

class ImageModelPriv
{
public:

    ImageModelPriv()
    {
        preprocessor      = 0;
        keepFilePathCache = false;
        itemDrag          = true;
        itemDrop          = true;
        dragDropHandler   = 0;
    }

    ImageInfoList         infos;
    QHash<qlonglong, int> idHash;

    bool                  itemDrag;
    bool                  itemDrop;
    ImageModelDragDropHandler
                         *dragDropHandler;

    bool                  keepFilePathCache;
    QHash<QString, int>   filePathHash;

    QObject              *preprocessor;

    DatabaseFields::Set   watchFlags;
};

ImageModel::ImageModel(QObject *parent)
          : QAbstractListModel(parent),
            d(new ImageModelPriv)
{
    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset &)),
            this, SLOT(slotImageChange(const ImageChangeset &)));
}

ImageModel::~ImageModel()
{
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

void ImageModel::setWatchFlags(const DatabaseFields::Set &set)
{
    d->watchFlags = set;
}

ImageInfo ImageModel::imageInfo(const QModelIndex &index) const
{
    if (!index.isValid())
        return ImageInfo();
    return d->infos[index.row()];
}

ImageInfo &ImageModel::imageInfoRef(const QModelIndex &index) const
{
    return d->infos[index.row()];
}

qlonglong ImageModel::imageId(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return d->infos[index.row()].id();
}

QList<ImageInfo> ImageModel::imageInfos(const QList<QModelIndex> &indexes) const
{
    QList<ImageInfo> infos;
    foreach (const QModelIndex &index, indexes)
        infos << imageInfo(index);
    return infos;
}

QList<qlonglong> ImageModel::imageIds(const QList<QModelIndex> &indexes) const
{
    QList<qlonglong> ids;
    foreach (const QModelIndex &index, indexes)
        ids << imageId(index);
    return ids;
}

ImageInfo ImageModel::imageInfo(int row) const
{
    if (row >= d->infos.size())
        return ImageInfo();
    return d->infos[row];
}

ImageInfo &ImageModel::imageInfoRef(int row) const
{
    return d->infos[row];
}

qlonglong ImageModel::imageId(int row) const
{
    if (row >= d->infos.size())
        return -1;
    return d->infos[row].id();
}

QModelIndex ImageModel::indexForImageInfo(const ImageInfo &info) const
{
    return indexForImageId(info.id());
}

QModelIndex ImageModel::indexForImageId(qlonglong id) const
{
    QHash<qlonglong, int>::iterator it = d->idHash.find(id);
    if (it != d->idHash.end())
        return createIndex(it.value(), 0);
    return QModelIndex();
}

// static method
ImageInfo ImageModel::retrieveImageInfo(const QModelIndex &index)
{
    if (!index.isValid())
        return ImageInfo();

    ImageModel *model = index.data(ImageModelPointerRole).value<ImageModel*>();
    int row = index.data(ImageModelInternalId).toInt();
    return model->imageInfo(row);
}

// static method
qlonglong ImageModel::retrieveImageId(const QModelIndex &index)
{
    if (!index.isValid())
        return 0;

    ImageModel *model = index.data(ImageModelPointerRole).value<ImageModel*>();
    int row = index.data(ImageModelInternalId).toInt();
    return model->imageId(row);
}

QModelIndex ImageModel::indexForPath(const QString &filePath) const
{
    if (d->keepFilePathCache)
    {
        int index = d->filePathHash.value(filePath, -1);
        if (index != -1)
            return createIndex(index, 0);
    }
    else
    {
        const int size = d->infos.size();
        for (int i=0; i<size; i++)
            if (d->infos[i].filePath() == filePath)
                return createIndex(i, 0);
    }
    return QModelIndex();
}

ImageInfo ImageModel::imageInfo(const QString &filePath) const
{
    if (d->keepFilePathCache)
    {
        int index = d->filePathHash.value(filePath, -1);
        if (index != -1)
            return d->infos[index];
    }
    else
    {
        foreach (const ImageInfo &info, d->infos)
            if (info.filePath() == filePath)
                return info;
    }
    return ImageInfo();
}

void ImageModel::addImageInfos(const QList<ImageInfo> &infos)
{
    if (infos.isEmpty())
        return;

    if (d->preprocessor)
        emit preprocess(infos);
    else
        appendInfos(infos);
}

void ImageModel::clearImageInfos()
{
    reset();
    d->infos.clear();
    d->idHash.clear();
    d->filePathHash.clear();
    imageInfosCleared();
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

bool ImageModel::hasImage(const ImageInfo &info) const
{
    return d->idHash.contains(info.id());
}

// ------------ Preprocessing -------------

void ImageModel::setPreprocessor(QObject *preprocessor)
{
    unsetPreprocessor(d->preprocessor);
    d->preprocessor = preprocessor;
}

void ImageModel::unsetPreprocessor(QObject *preprocessor)
{
    if (preprocessor && d->preprocessor == preprocessor)
    {
        disconnect(this, SIGNAL(preprocess(const QList<ImageInfo> &)), 0, 0);
        disconnect(d->preprocessor, 0, this, SLOT(appendInfos(const QList<ImageInfo> &)));
    }
}

void ImageModel::reAddImageInfos(const QList<ImageInfo> &infos)
{
    appendInfos(infos);
}

void ImageModel::appendInfos(const QList<ImageInfo> &infos)
{
    emit imageInfosAboutToBeAdded(infos);
    int firstNewIndex = d->infos.size();
    int lastNewIndex = d->infos.size() + infos.size() - 1;
    beginInsertRows(QModelIndex(), firstNewIndex, lastNewIndex);
    d->infos << infos;
    for (int i=firstNewIndex; i<=lastNewIndex; i++)
    {
        ImageInfo &info = d->infos[i];
        d->idHash[info.id()] = i;
        if (d->keepFilePathCache)
            d->filePathHash[info.filePath()] = i;
    }
    endInsertRows();
    emit imageInfosAdded(infos);
}

// ------------ QAbstractItemModel implementation -------------

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return d->infos[index.row()].name();
        case ImageModelPointerRole:
            return QVariant::fromValue(const_cast<ImageModel*>(this));
        case ImageModelInternalId:
            return index.row();
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

int ImageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return d->infos.size();
}

Qt::ItemFlags ImageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (d->itemDrag)
        f |= Qt::ItemIsDragEnabled;
    if (d->itemDrop)
        f |= Qt::ItemIsDropEnabled;
    return f;
}

QModelIndex ImageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0 || row < 0 || parent.isValid() || row >= d->infos.size())
        return QModelIndex();

    return createIndex(row, 0);
}

// ------------ Drag and Drop -------------

Qt::DropActions ImageModel::supportedDropActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}

QStringList ImageModel::mimeTypes() const
{
    if (d->dragDropHandler)
        return d->dragDropHandler->mimeTypes();
    return QStringList();
}

bool ImageModel::dropMimeData(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &)
{
    // we require custom solutions
    return false;
}

QMimeData *ImageModel::mimeData(const QModelIndexList &indexes) const
{
    if (!d->dragDropHandler)
        return 0;

    QList<ImageInfo> infos = imageInfos(indexes);
    return d->dragDropHandler->createMimeData(infos);
}

void ImageModel::setEnableDrag(bool enable)
{
    d->itemDrag = enable;
}

void ImageModel::setEnableDrop(bool enable)
{
    d->itemDrop = enable;
}

void ImageModel::setDragDropHandler(ImageModelDragDropHandler *handler)
{
    d->dragDropHandler = handler;
}

ImageModelDragDropHandler *ImageModel::dragDropHandler() const
{
    return d->dragDropHandler;
}

// ------------ Database watch -------------

void ImageModel::slotImageChange(const ImageChangeset &changeset)
{
    if (d->infos.isEmpty())
        return;

    if (d->watchFlags & changeset.changes())
    {
        QItemSelection items;
        foreach(qlonglong id, changeset.ids())
        {
            QModelIndex index = indexForImageId(id);
            if (index.isValid())
                items.select(index, index);
        }
        if (!items.isEmpty())
        {
            foreach (const QItemSelectionRange &range, items)
                emit dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

} // namespace Digikam
