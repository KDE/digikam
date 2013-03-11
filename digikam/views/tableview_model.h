/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Wrapper model for table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TABLEVIEW_MODEL_H
#define TABLEVIEW_MODEL_H

// Qt includes
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

// KDE includes


// local includes

#include "databasechangesets.h"
#include "tableview_shared.h"

class QMimeData;

namespace Digikam
{

class ImageChangeset;
class ImageFilterModel;
class TableViewColumn;
class TableViewColumnConfiguration;
class TableViewColumnDescription;
class TableViewColumnFactory;
class TableViewColumnProfile;
class TableViewModelItem;

class TableViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit TableViewModel(TableViewShared* const sharedObject, QObject* parent = 0);
    virtual ~TableViewModel();

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& parent) const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual int columnCount(const QModelIndex& i) const;
    virtual QVariant data(const QModelIndex& i, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    void addColumnAt(const TableViewColumnDescription& description, const int targetColumn = -1);
    void addColumnAt(const TableViewColumnConfiguration& configuration, const int targetColumn = -1);
    void removeColumnAt(const int columnIndex);
    TableViewColumn* getColumnObject(const int columnIndex);
    QList<TableViewColumn*> getColumnObjects();
    QModelIndex fromImageFilterModelIndex(const QModelIndex& imageFilterModelIndex);
    QModelIndex toImageFilterModelIndex(const QModelIndex& i) const;
    void loadColumnProfile(const TableViewColumnProfile& columnProfile);
    TableViewColumnProfile getColumnProfile() const;

private Q_SLOTS:
    
    void slotPopulateModel();

    void slotColumnDataChanged(const QModelIndex& sourceIndex);
    void slotColumnAllDataChanged();

    void slotSourceModelAboutToBeReset();
    void slotSourceModelReset();
    void slotSourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void slotSourceRowsInserted(const QModelIndex& parent, int start, int end);
    void slotSourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotSourceRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotSourceRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                      const QModelIndex& destinationParent, int destinationRow);
    void slotSourceRowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                               const QModelIndex& destinationParent, int destinationRow);
    void slotSourceLayoutAboutToBeChanged();
    void slotSourceLayoutChanged();
    void slotSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    void slotDatabaseImageChanged(const ImageChangeset& imageChangeset);

private:

    TableViewModelItem* createItemFromSourceIndex(const QModelIndex& imageFilterModelIndex);
    TableViewModelItem* itemFromImageId(const qlonglong imageId) const;
    TableViewModelItem* itemFromIndex(const QModelIndex& i) const;
    void addSourceModelIndex(const QModelIndex& imageFilterModelIndex);

    TableViewShared* const s;
    class Private;
    const QScopedPointer<Private> d;
};

} /* namespace Digikam */

#endif // TABLEVIEW_MODEL_H

