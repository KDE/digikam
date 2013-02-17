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

// KDE includes


// local includes

#include "tableview_shared.h"

namespace Digikam
{

class TableViewColumnProfile;
class TableViewColumn;
class TableViewColumnDescription;
class TableViewColumnConfiguration;
class ImageFilterModel;
class TableViewColumnFactory;

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

    void addColumnAt(const TableViewColumnDescription& description, const int targetColumn = -1);
    void addColumnAt(const TableViewColumnConfiguration& configuration, const int targetColumn = -1);
    void removeColumnAt(const int columnIndex);
    TableViewColumn* getColumnObject(const int columnIndex);
    QModelIndex toImageFilterModelIndex(const QModelIndex& i) const;
    void loadColumnProfile(const TableViewColumnProfile& columnProfile);
    TableViewColumnProfile getColumnProfile() const;

private:

    class Private;
    const QScopedPointer<Private> d;
};


} /* namespace Digikam */

#endif // TABLEVIEW_MODEL_H

