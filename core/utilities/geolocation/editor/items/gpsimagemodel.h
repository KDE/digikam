/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : A model to hold information about images.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPSIMAGEMODEL_H
#define GPSIMAGEMODEL_H

// Qt includes

#include <QAbstractItemModel>
#include <QPixmap>

// Local includes

#include "gpsimageitem.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class GPSImageModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    GPSImageModel(QObject* const parent = 0);
    ~GPSImageModel();

    // own functions:
    void addItem(GPSImageItem* const newItem);
    void setColumnCount(const int nColumns);
    GPSImageItem* itemFromIndex(const QModelIndex& index) const;
    GPSImageItem* itemFromUrl(const QUrl& url) const;
    QModelIndex indexFromUrl(const QUrl& url) const;

    QPixmap getPixmapForIndex(const QPersistentModelIndex& itemIndex, const int size);

    // QAbstractItemModel:
    virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual Qt::DropActions supportedDragActions() const;

protected:

    void itemChanged(GPSImageItem* const changedItem);

Q_SIGNALS:

    void signalThumbnailForIndexAvailable(const QPersistentModelIndex& index, const QPixmap& pixmap);

protected Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class Private;
    Private* const d;

    friend class GPSImageItem;
};

} // namespace Digikam

#endif // GPSIMAGEMODEL_H
