/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : A simple model to hold a tree structure.
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_SIMPLE_TREE_MODEL_H
#define DIGIKAM_SIMPLE_TREE_MODEL_H

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SimpleTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    class Item
    {
    public:

        explicit Item()
          : dataColumns(),
            parent(nullptr),
            children()
        {
        }

        ~Item()
        {
            qDeleteAll(children);
        }

        QString data;

    private:

        QList<QMap<int, QVariant> > dataColumns;
        Item*                       parent;
        QList<Item*>                children;

        friend class SimpleTreeModel;
    };

    explicit SimpleTreeModel(const int columnCount, QObject* const parent = nullptr);
    ~SimpleTreeModel();

    // QAbstractItemModel:
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    Item* addItem(Item* const parentItem = nullptr, const int rowNumber = -1);
    Item* indexToItem(const QModelIndex& itemIndex) const;
    Item* rootItem() const;
    QModelIndex itemToIndex(const Item* const item) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SIMPLE_TREE_MODEL_H
