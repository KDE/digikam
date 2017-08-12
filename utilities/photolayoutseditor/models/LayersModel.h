/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LAYERS_MODEL_H
#define LAYERS_MODEL_H

// Qt includes

#include <QObject>
#include <QModelIndex>
#include <QAbstractItemModel>

namespace PhotoLayoutsEditor
{
class LayersModelItem;
class AbstractPhoto;

class LayersModel : public QAbstractItemModel
{
        Q_OBJECT

    public:

        enum ModelRoles
        {
            VisibilityIcon = Qt::UserRole,
            LockIcon,
            GraphicItem
        };

    public:

        explicit LayersModel(QObject* parent = 0);
        virtual ~LayersModel();

        QModelIndex index( int row, int column, const QModelIndex& parent ) const;
        QModelIndex parent( const QModelIndex& index ) const;
        int rowCount( const QModelIndex& parent = QModelIndex()) const;
        int columnCount( const QModelIndex& parent = QModelIndex()) const;
        QVariant data( const QModelIndex&, int role ) const;
        bool setData(const QModelIndex& index, const QVariant& value, int role);
        Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        bool appendItem(AbstractPhoto* item, const QModelIndex& parent = QModelIndex());
        bool insertItem(AbstractPhoto* item, int position, const QModelIndex& parent = QModelIndex());
        bool prependItem(AbstractPhoto* item, const QModelIndex& parent = QModelIndex());
        bool appendItems(const QList<AbstractPhoto*>& items, const QModelIndex& parent);
        bool insertItems(const QList<AbstractPhoto*>& items, int position, const QModelIndex& parent = QModelIndex());
        bool prependItems(const QList<AbstractPhoto*>& items, const QModelIndex& parent);
        QModelIndexList itemsToIndexes(const QList<AbstractPhoto*> & items) const;
        QList<AbstractPhoto*> indexesToItems(const QModelIndexList& indexes) const;
        QModelIndex findIndex(AbstractPhoto* item, const QModelIndex& parent = QModelIndex()) const;
        QModelIndex findIndex(LayersModelItem* item, const QModelIndex& parent = QModelIndex()) const;
        bool insertRows(int row, int count, const QModelIndex& parent);
        LayersModelItem* getItem(const QModelIndex& index) const;
        bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

        bool moveRows(int sourcePosition, const QModelIndex& sourceParent,
                      int destPosition, const QModelIndex& destinationParent);

        bool moveRows(const QModelIndex& sourceIndex, const QModelIndex& sourdeParent,
                      const QModelIndex& destinationIndex, const QModelIndex& destinationParent);

        bool moveRows(int sourcePosition, int sourceCount,
                      const QModelIndex& sourceParent, int destPosition,
                      const QModelIndex& destinationParent);

        bool moveRows(const QModelIndex& start1, const QModelIndex& end1,
                      const QModelIndex& parent1, const QModelIndex& start2,
                      const QModelIndex& parent2);

        void updateModel(const QModelIndex& topLeft, const QModelIndex& bottomRight);

        Qt::DropActions supportedDragActions() const;

    Q_SIGNALS:

        void rowsInserted(const QModelIndex& parent, int first, int last);

    private:

        LayersModelItem* root;
};

} // namespace PhotoLayoutsEditor

#endif // LAYERSMODEL_H
