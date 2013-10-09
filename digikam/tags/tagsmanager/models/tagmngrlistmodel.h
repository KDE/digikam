/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : List View Model with support for mime data and drag-n-drop
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef LISTMODEL_H
#define LISTMODEL_H

// Qt includes

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace Digikam
{

class ListItem;

class TagMngrListModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    TagMngrListModel(QObject* const parent = 0);
    ~TagMngrListModel();

    /**
     * @brief addItem   - add new item to list
     * @param values    - A list of data for item: Name as QString, QBrush as
     *                    background and qlonglong as id
     * @return          - pointer to newly created listitem
     */
    ListItem* addItem(QList<QVariant> values);

    /**
     * @brief allItems  - return all items from List, usually to be saved
     *                    in KConfig
     */
    QList<ListItem*> allItems() const;

    void deleteItem(ListItem* const item);

    /**
     * Standard methods to be implemented when subcassing QAbstractItemModel
     */
    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    QModelIndex parent(const QModelIndex& index) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    bool setData(const QModelIndex& index, const QVariant& value, int role);

    /**
     * Reimplemented methods for handling drag-n-drop, encoding and decoding
     * mime types
     */
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    QList<int> getDragNewSelection() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // LISTMODEL_H
