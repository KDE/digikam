/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Table view column helpers
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

#include "tableview_columnfactory.moc"

// Qt includes

// KDE includes

// local includes

#include "tableview_columns.h"

namespace Digikam
{

class TableViewColumnFactory::Private
{
public:
    TableViewColumnDataSource* dataSource;
};

TableViewColumn::TableViewColumn(TableViewColumnDataSource* const pDataSource)
  : dataSource(pDataSource)
{

}

TableViewColumn::~TableViewColumn()
{

}

TableViewColumnFactory::TableViewColumnFactory(TableViewColumnDataSource* const dataSource, QObject* parent)
  : QObject(parent),
    d(new Private())
{
    d->dataSource = dataSource;
}

QString TableViewColumn::getIdStatic()
{
    return QString("idstatic");
}

QString TableViewColumn::getId()
{
    return QString("id");
}

QString TableViewColumn::getTitle()
{
    return QString("Title");
}

QStringList TableViewColumnFactory::getColumnIds()
{
    QStringList columnIds;

    columnIds << TableViewColumns::ColumnFilename::getIdStatic();
    columnIds << TableViewColumns::ColumnCoordinates::getIdStatic();

    return columnIds;
}

TableViewColumn* TableViewColumnFactory::getColumn(const QString columnId)
{
    if (columnId=="filename")
    {
        return new TableViewColumns::ColumnFilename(d->dataSource);
    }
    else if (columnId=="coordinates")
    {
        return new TableViewColumns::ColumnCoordinates(d->dataSource);
    }

    return 0;
}

QVariant TableViewColumn::data(const QModelIndex& sourceIndex, const int role)
{
    Q_UNUSED(sourceIndex)
    Q_UNUSED(role)
    return QVariant();
}


} /* namespace Digikam */


