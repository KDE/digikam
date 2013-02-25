/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Item properties
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

#include "tableview_column_item.moc"
#include <QFormLayout>

// Qt includes

// KDE includes

#include <kdebug.h>

// local includes

namespace Digikam
{

namespace TableViewColumns
{

ColumnItemProperties::ColumnItemProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(SubColumnWidth)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    if (subColumnSetting == "width")
    {
        subColumn = SubColumnWidth;
    }
    else if (subColumnSetting == "height")
    {
        subColumn = SubColumnHeight;
    }
}

ColumnItemProperties::~ColumnItemProperties()
{

}

TableViewColumnDescription ColumnItemProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("item-properties"), i18n("Item properties"));

    description.addSubColumn(
        TableViewColumnDescription("item-properties", i18n("Width"), "subcolumn", "width")
    );

    description.addSubColumn(
        TableViewColumnDescription("item-properties", i18n("Height"), "subcolumn", "height")
    );

    return description;
}

QString ColumnItemProperties::getTitle()
{
    switch (subColumn)
    {
    case SubColumnWidth:
        return i18n("Width");
    case SubColumnHeight:
        return i18n("Height");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnItemProperties::getColumnFlags() const
{
    if (   (subColumn == SubColumnHeight)
        || (subColumn == SubColumnWidth) )
    {
        return ColumnCustomSorting;
    }

    return ColumnNoFlags;
}

QVariant ColumnItemProperties::data(const QModelIndex& sourceIndex, const int role)
{
    if (role != Qt::DisplayRole)
    {
        /// @todo is this correct or does sourceIndex have column!=0?
        return sourceIndex.data(role);
    }

    const ImageInfo info = getImageInfo(sourceIndex);

    switch (subColumn)
    {
    case SubColumnWidth:
        /// @todo Needs custom sorting
        return KGlobal::locale()->formatNumber(info.dimensions().width(), 0);
        break;

    case SubColumnHeight:
        /// @todo Needs custom sorting
        return KGlobal::locale()->formatNumber(info.dimensions().height(), 0);
        break;

    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnItemProperties::compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
{
    const ImageInfo infoA = getImageInfo(sourceA);
    const ImageInfo infoB = getImageInfo(sourceB);

    if (subColumn == SubColumnHeight)
    {
        const int heightA = infoA.dimensions().height();
        const int heightB = infoB.dimensions().height();

        return compareHelper<int>(heightA, heightB);
    }
    else if (subColumn == SubColumnWidth)
    {
        const int widthA = infoA.dimensions().width();
        const int widthB = infoB.dimensions().width();

        return compareHelper<int>(widthA, widthB);
    }

    kWarning() << "item: unimplemented comparison, subColumn=" << subColumn;
    return CmpEqual;
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

