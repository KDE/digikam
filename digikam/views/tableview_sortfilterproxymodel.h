/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-03-11
 * Description : Sort filter proxy model for the table view model
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

#ifndef TABLEVIEW_SORTFILTERPROXYMODEL_H
#define TABLEVIEW_SORTFILTERPROXYMODEL_H

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

class TableViewSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit TableViewSortFilterProxyModel(TableViewShared* const sPointer, QObject* parent = 0);
    virtual ~TableViewSortFilterProxyModel();

    virtual bool lessThan(const QModelIndex& tableViewIndexLeft, const QModelIndex& tableViewIndexRight) const;

    // drag-and-drop related functions
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    QModelIndex toImageModelIndex(const QModelIndex& index) const;
    QModelIndex toImageFilterModelIndex(const QModelIndex& index) const;

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

#endif // TABLEVIEW_SORTFILTERPROXYMODEL_H

