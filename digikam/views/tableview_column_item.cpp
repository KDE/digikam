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
    subColumn = SubColumn(getSubColumns().indexOf(subColumnSetting));
    if (subColumn<0)
    {
        subColumn = SubColumnWidth;
    }
}

ColumnItemProperties::~ColumnItemProperties()
{

}

QStringList ColumnItemProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("width") << QLatin1String("height")
            << QLatin1String("dimensions") << QLatin1String("pixelcount");

    return columns;
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

    description.addSubColumn(
        TableViewColumnDescription("item-properties", i18n("Dimensions"), "subcolumn", "dimensions")
    );

    description.addSubColumn(
        TableViewColumnDescription("item-properties", i18n("Pixel count"), "subcolumn", "pixelcount")
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
    case SubColumnDimensions:
        return i18n("Dimensions");
    case SubColumnPixelCount:
        return i18n("Pixel count");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnItemProperties::getColumnFlags() const
{
    ColumnFlags flags(ColumnNoFlags);

    if (   (subColumn == SubColumnHeight)
        || (subColumn == SubColumnWidth)
        || (subColumn == SubColumnDimensions)
        || (subColumn == SubColumnPixelCount) )
    {
        flags|=ColumnCustomSorting;
    }

    return flags;
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
        return KGlobal::locale()->formatNumber(info.dimensions().width(), 0);

    case SubColumnHeight:
        return KGlobal::locale()->formatNumber(info.dimensions().height(), 0);

    case SubColumnDimensions:
        {
            const QSize imgSize = info.dimensions();

            if (imgSize.isNull())
            {
                return QString();
            }

            const QString widthString = KGlobal::locale()->formatNumber(imgSize.width(), 0);
            const QString heightString = KGlobal::locale()->formatNumber(imgSize.height(), 0);
            return QString("%1x%2").arg(widthString).arg(heightString);
        }

    case SubColumnPixelCount:
        {
            const QSize imgSize = info.dimensions();
            const int pixelCount = imgSize.height() * imgSize.width();
            if (pixelCount==0)
            {
                return QString();
            }

            /// @todo make this configurable with si-prefixes
            return KGlobal::locale()->formatNumber(pixelCount, 0);
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnItemProperties::compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
{
    const ImageInfo infoA = getImageInfo(sourceA);
    const ImageInfo infoB = getImageInfo(sourceB);

    switch (subColumn)
    {
    case SubColumnHeight:
        {
        const int heightA = infoA.dimensions().height();
        const int heightB = infoB.dimensions().height();

        return compareHelper<int>(heightA, heightB);
        }
    case SubColumnWidth:
        {
        const int widthA = infoA.dimensions().width();
        const int widthB = infoB.dimensions().width();

        return compareHelper<int>(widthA, widthB);
        }

    case SubColumnDimensions:
        {
            const int widthA = infoA.dimensions().width();
            const int widthB = infoB.dimensions().width();

            const ColumnCompareResult widthResult = compareHelper<int>(widthA, widthB);
            if (widthResult!=CmpEqual)
            {
                return widthResult;
            }

            const int heightA = infoA.dimensions().height();
            const int heightB = infoB.dimensions().height();

            return compareHelper<int>(heightA, heightB);
        }

    case SubColumnPixelCount:
        {
            const int widthA = infoA.dimensions().width();
            const int widthB = infoB.dimensions().width();
            const int heightA = infoA.dimensions().height();
            const int heightB = infoB.dimensions().height();

            const int pixelCountA = widthA*heightA;
            const int pixelCountB = widthB*heightB;

            return compareHelper<int>(pixelCountA, pixelCountB);
        }


    default:
        kWarning() << "item: unimplemented comparison, subColumn=" << subColumn;
        return CmpEqual;
    }
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

