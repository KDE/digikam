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

namespace Digikam
{

class TableViewColumnProfile;
class TableViewColumn;
class TableViewColumnDescription;
class TableViewColumnConfiguration;
class ImageFilterModel;
class TableViewColumnFactory;
class ImageChangeset;

class TableViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit TableViewModel(TableViewColumnFactory* const tableViewColumnFactory, ImageFilterModel* const sourceModel, QObject* parent = 0);
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
    QModelIndex toImageFilterModelIndex(const QModelIndex& i) const;
    void loadColumnProfile(const TableViewColumnProfile& columnProfile);
    TableViewColumnProfile getColumnProfile() const;

private Q_SLOTS:

    void slotColumnDataChanged(const QModelIndex& sourceIndex);

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

    class Private;
    const QScopedPointer<Private> d;
};

class TableViewSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    TableViewSortFilterProxyModel(TableViewShared* const sPointer, QObject* parent = 0);
    virtual ~TableViewSortFilterProxyModel();

    virtual bool lessThan(const QModelIndex& tableViewIndexLeft, const QModelIndex& tableViewIndexRight) const;

    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    QModelIndex toImageModelIndex(const QModelIndex& index) const;

private:

    class Private;
    const QScopedPointer<Private> d;
    TableViewShared* const s;
};

class TableViewCurrentToSortedSyncer : public QObject
{
    Q_OBJECT

public:

    explicit TableViewCurrentToSortedSyncer(TableViewShared* const sharedObject, QObject* const parent = 0);
    virtual ~TableViewCurrentToSortedSyncer();

private Q_SLOTS:

    void slotSortedModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotTableViewModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

private:

    class Private;
    class QScopedPointer<Private> d;
    TableViewShared* const s;
};

} /* namespace Digikam */

#endif // TABLEVIEW_MODEL_H

