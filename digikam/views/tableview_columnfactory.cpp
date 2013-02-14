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
};

TableViewColumn::TableViewColumn(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration
    )
  : s(tableViewShared),
    configuration(pConfiguration)
{

}

TableViewColumn::~TableViewColumn()
{

}

TableViewColumnFactory::TableViewColumnFactory(TableViewShared* const tableViewShared, QObject* parent)
  : QObject(parent),
    d(new Private()),
    s(tableViewShared)
{
}

QString TableViewColumn::getTitle()
{
    return QString("Title");
}

TableViewColumn* TableViewColumnFactory::getColumn(const Digikam::TableViewColumnConfiguration& columnConfiguration)
{
    const QString& columnId = columnConfiguration.columnId;

    if (columnId=="filename")
    {
        return new TableViewColumns::ColumnFilename(s, columnConfiguration);
    }
    else if (columnId=="coordinates")
    {
        return new TableViewColumns::ColumnCoordinates(s, columnConfiguration);
    }
    else if (columnId=="thumbnail")
    {
        return new TableViewColumns::ColumnThumbnail(s, columnConfiguration);
    }

    return 0;
}

QVariant TableViewColumn::data(const QModelIndex& sourceIndex, const int role)
{
    Q_UNUSED(sourceIndex)
    Q_UNUSED(role)
    return QVariant();
}

QList<TableViewColumnDescription> TableViewColumnFactory::getColumnDescriptionList()
{
    QList<TableViewColumnDescription> descriptionList;

    descriptionList << TableViewColumns::ColumnFilename::getDescription();
    descriptionList << TableViewColumns::ColumnCoordinates::getDescription();
    descriptionList << TableViewColumns::ColumnThumbnail::getDescription();

    return descriptionList;
}

bool TableViewColumn::paint(QPainter*const painter, const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(sourceIndex)

    return false;
}

QSize TableViewColumn::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
{
    Q_UNUSED(option)
    Q_UNUSED(sourceIndex)

    return QSize();
}


} /* namespace Digikam */


