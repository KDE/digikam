/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-09
 * Description : DTrash item info model
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef DTRASHITEMMODEL_H
#define DTRASHITEMMODEL_H

// Qt includes

#include <QAbstractTableModel>

// Local includes

#include "dtrashiteminfo.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class DTrashItemModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    DTrashItemModel(QObject* parent = 0);
    ~DTrashItemModel();

    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;

    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool pixmapForItem(const QString& path, QPixmap& pix) const;
    void clearCurrentData();

    void loadItemsForCollection(const QString& colPath);
    DTrashItemInfo itemForIndex(const QModelIndex& index);
    DTrashItemInfoList itemsForIndexes(const QList<QModelIndex>& indexes);
    DTrashItemInfoList allItems();
    bool isEmpty();

    void changeThumbSize(int size);

public Q_SLOTS:

    void append(const DTrashItemInfo& itemInfo);
    void removeItems(const QModelIndexList& indexes);
    void refreshLayout();

Q_SIGNALS:

    void dataChange();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DTRASHITEMMODEL_H
