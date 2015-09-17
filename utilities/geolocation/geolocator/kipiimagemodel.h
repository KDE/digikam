/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-21
 * @brief  A model to hold information about images.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef KIPIIMAGEMODEL_H
#define KIPIIMAGEMODEL_H

// Qt includes

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QPixmap>
#include <QSortFilterProxyModel>

// Libkipi includes

#include <KIPI/Interface>

// Local includes

#include "kipiimageitem.h"

namespace Digikam
{

class KipiImageModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    KipiImageModel(QObject* const parent = 0);
    ~KipiImageModel();

    // own functions:
    void addItem(KipiImageItem* const newItem);
    void setColumnCount(const int nColumns);
    KipiImageItem* itemFromIndex(const QModelIndex& index) const;
    KipiImageItem* itemFromUrl(const QUrl& url) const;
    QModelIndex indexFromUrl(const QUrl& url) const;

    QPixmap getPixmapForIndex(const QPersistentModelIndex& itemIndex, const int size);
    void setKipiInterface(KIPI::Interface* const interface);

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

    void itemChanged(KipiImageItem* const changedItem);

Q_SIGNALS:

    void signalThumbnailForIndexAvailable(const QPersistentModelIndex& index, const QPixmap& pixmap);

protected Q_SLOTS:

    void slotThumbnailFromInterface(const QUrl& url, const QPixmap& pixmap);

private:

    class Private;
    Private* const d;

    friend class KipiImageItem;
};

// --------------------------------------------------------------------------------------------------------------------

class KipiImageSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    KipiImageSortProxyModel(KipiImageModel* const kipiImageModel, QItemSelectionModel* const sourceSelectionModel);
    ~KipiImageSortProxyModel();

    QItemSelectionModel* mappedSelectionModel() const;

protected:

    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

private:

    class Private;
    Private* const d;
};

} /* Digikam */

#endif /* KIPIIMAGEMODEL_H */
