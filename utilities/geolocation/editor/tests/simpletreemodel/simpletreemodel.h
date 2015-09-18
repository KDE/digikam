/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-21
 * @brief  A simple model to hold a tree structure.
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

#ifndef SIMPLETREEMODEL_H
#define SIMPLETREEMODEL_H

// Qt includes

#include <QAbstractItemModel>

class SimpleTreeModelPrivate;
class SimpleTreeModel : public QAbstractItemModel
{
Q_OBJECT

public:

    class Item
    {
    public:
        Item()
        : dataColumns(),
          parent(0),
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
        Item* parent;
        QList<Item*> children;

        friend class SimpleTreeModel;
    };

    SimpleTreeModel(const int columnCount, QObject* const parent = 0);
    ~SimpleTreeModel();

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

    Item* addItem(Item* const parentItem = 0, const int rowNumber = -1);
    Item* indexToItem(const QModelIndex& itemIndex) const;
    Item* rootItem() const;
    QModelIndex itemToIndex(const Item* const item) const;

private:
    SimpleTreeModelPrivate* const d;
};

#endif /* SIMPLETREEMODEL_H */
