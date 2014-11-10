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

// Qt includes

#include <QModelIndex>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// local includes

#include "imageinfo.h"
#include "databaseinfocontainers.h"
#include "imagepropertiestab.h"

namespace Digikam
{

namespace TableViewColumns
{

ColumnItemProperties::ColumnItemProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        const SubColumn pSubColumn,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(pSubColumn)
{

}

ColumnItemProperties::~ColumnItemProperties()
{

}

QStringList ColumnItemProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("width") << QLatin1String("height")
            << QLatin1String("dimensions") << QLatin1String("pixelcount")
            << QLatin1String("bitdepth") << QLatin1String("colormode")
            << QLatin1String("itemtype") << QLatin1String("itemcreationdatetime")
            << QLatin1String("itemdigitizationtime") << QLatin1String("itemaspectratio");

    return columns;
}

TableViewColumnDescription ColumnItemProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("item-properties"), i18n("Item properties"));
    description.setIcon("image-x-generic");

    description.addSubColumn(
        TableViewColumnDescription("width", i18n("Width"))
    );

    description.addSubColumn(
        TableViewColumnDescription("height", i18n("Height"))
    );

    description.addSubColumn(
        TableViewColumnDescription("dimensions", i18n("Dimensions"))
    );

    description.addSubColumn(
        TableViewColumnDescription("pixelcount", i18n("Pixel count"))
    );

    description.addSubColumn(
        TableViewColumnDescription("bitdepth", i18n("Bit depth"))
    );

    description.addSubColumn(
        TableViewColumnDescription("colormode", i18n("Color mode"))
    );

    description.addSubColumn(
        TableViewColumnDescription("itemtype", i18n("Type"))
    );

    description.addSubColumn(
        TableViewColumnDescription("itemcreationdatetime", i18n("Creation date/time"))
    );

    description.addSubColumn(
        TableViewColumnDescription("itemdigitizationtime", i18n("Digitization date/time"))
    );

    description.addSubColumn(
        TableViewColumnDescription("itemaspectratio", i18n("Aspect ratio"))
    );

    return description;
}

QString ColumnItemProperties::getTitle() const
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
        case SubColumnBitDepth:
            return i18n("Bit depth");
        case SubColumnColorMode:
            return i18n("Color mode");
        case SubColumnType:
            return i18n("Type");
        case SubColumnCreationDateTime:
            return i18n("Creation date/time");
        case SubColumnDigitizationDateTime:
            return i18n("Digitization date/time");
        case SubColumnAspectRatio:
            return i18n("Aspect ratio");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnItemProperties::getColumnFlags() const
{
    ColumnFlags flags(ColumnNoFlags);

    if (   (subColumn == SubColumnHeight)
        || (subColumn == SubColumnWidth)
        || (subColumn == SubColumnDimensions)
        || (subColumn == SubColumnBitDepth)
        || (subColumn == SubColumnPixelCount)
        || (subColumn == SubColumnCreationDateTime)
        || (subColumn == SubColumnDigitizationDateTime)
        || (subColumn == SubColumnAspectRatio) )
    {
        flags|=ColumnCustomSorting;
    }

    return flags;
}

QVariant ColumnItemProperties::data(TableViewModel::Item* const item, const int role) const
{
    if ( (role != Qt::DisplayRole) &&
         (role != Qt::TextAlignmentRole) )
    {
        return QVariant();
    }

    if (role==Qt::TextAlignmentRole)
    {
        switch (subColumn)
        {
            case SubColumnHeight:
            case SubColumnWidth:
            case SubColumnPixelCount:
                return QVariant(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));

            default:
                return QVariant();
        }
    }

    const ImageInfo info = s->tableViewModel->infoFromItem(item);

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

            const QString widthString  = KGlobal::locale()->formatNumber(imgSize.width(),  0);
            const QString heightString = KGlobal::locale()->formatNumber(imgSize.height(), 0);

            return QString("%1x%2").arg(widthString).arg(heightString);
        }

        case SubColumnPixelCount:
        {
            const QSize imgSize  = info.dimensions();
            const int pixelCount = imgSize.height() * imgSize.width();

            if (pixelCount==0)
            {
                return QString();
            }

            /// @todo make this configurable with si-prefixes
            return KGlobal::locale()->formatNumber(pixelCount, 0);
        }

        case SubColumnAspectRatio:
        {
            const QSize imgSize = info.dimensions();
            QString aspectRatioString;

            if (!ImagePropertiesTab::aspectRatioToString(imgSize.width(), imgSize.height(), aspectRatioString))
            {
                return QString();
            }

            return aspectRatioString;
        }

        case SubColumnBitDepth:
        {
            const ImageCommonContainer commonInfo = info.imageCommonContainer();
            const int bitDepth = commonInfo.colorDepth;

            return QString("%1 bpp").arg(bitDepth);
        }

        case SubColumnColorMode:
        {
            const ImageCommonContainer commonInfo = info.imageCommonContainer();

            return commonInfo.colorModel;
        }

        case SubColumnType:
        {
            const ImageCommonContainer commonInfo = info.imageCommonContainer();

            return commonInfo.format;
        }

        case SubColumnCreationDateTime:
        {
            const QDateTime creationDateTime = info.dateTime();

            return KGlobal::locale()->formatDateTime(creationDateTime, KLocale::ShortDate, true);
        }

        case SubColumnDigitizationDateTime:
        {
            const ImageCommonContainer commonInfo = info.imageCommonContainer();
            const QDateTime digitizationDateTime = commonInfo.digitizationDate;

            return KGlobal::locale()->formatDateTime(digitizationDateTime, KLocale::ShortDate, true);
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnItemProperties::compare(
    TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const
{
    const ImageInfo infoA = s->tableViewModel->infoFromItem(itemA);
    const ImageInfo infoB = s->tableViewModel->infoFromItem(itemB);

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

        case SubColumnAspectRatio:
        {
            const int widthA = infoA.dimensions().width();
            const int widthB = infoB.dimensions().width();
            const int heightA = infoA.dimensions().height();
            const int heightB = infoB.dimensions().height();

            if ( (heightA==0) || (heightB==0) )
            {
                // at least one of the two does not have valid data,
                // sort based on which one has data at all
                return compareHelper<int>(heightA, heightB);
            }

            const qreal aspectRatioA = qreal(widthA) / qreal(heightA);
            const qreal aspectRatioB = qreal(widthB) / qreal(heightB);

            /// @todo use fuzzy compare?
            return compareHelper<qreal>(aspectRatioA, aspectRatioB);
        }

        case SubColumnBitDepth:
        {
            const ImageCommonContainer commonInfoA = infoA.imageCommonContainer();
            const int bitDepthA = commonInfoA.colorDepth;
            const ImageCommonContainer commonInfoB = infoB.imageCommonContainer();
            const int bitDepthB = commonInfoB.colorDepth;

            return compareHelper<int>(bitDepthA, bitDepthB);
        }

        case SubColumnCreationDateTime:
        {
            const QDateTime dtA = infoA.dateTime();
            const QDateTime dtB = infoB.dateTime();

            return compareHelper<QDateTime>(dtA, dtB);
        }

        case SubColumnDigitizationDateTime:
        {
            const ImageCommonContainer commonInfoA = infoA.imageCommonContainer();
            const ImageCommonContainer commonInfoB = infoB.imageCommonContainer();
            const QDateTime dtA = commonInfoA.digitizationDate;
            const QDateTime dtB = commonInfoB.digitizationDate;

            return compareHelper<QDateTime>(dtA, dtB);
        }

        default:
            kWarning() << "item: unimplemented comparison, subColumn=" << subColumn;
            return CmpEqual;
    }
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

