/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Geographic columns
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

#include "tableview_column_geo.moc"
#include <klistwidget.h>
#include <QFormLayout>

// Qt includes

// KDE includes

// local includes

namespace Digikam
{

namespace TableViewColumns
{

ColumnGeoProperties::ColumnGeoProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(SubColumnCoordinates)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    subColumn = getSubColumnIndex<ColumnGeoProperties>(subColumnSetting, SubColumnCoordinates);
}

ColumnGeoProperties::~ColumnGeoProperties()
{

}

QStringList ColumnGeoProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("hascoordinates") << QLatin1String("coordinates")
            << QLatin1String("altitude");

    return columns;
}

TableViewColumnDescription ColumnGeoProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("geo-properties"), i18n("Geo properties"));
    description.setIcon("applications-internet");

    description.addSubColumn(
        TableViewColumnDescription("geo-properties", i18n("Geotagged"), "subcolumn", "hascoordinates")
    );

    description.addSubColumn(
        TableViewColumnDescription("geo-properties", i18n("Coordinates"), "subcolumn", "coordinates")
    );

    description.addSubColumn(
        TableViewColumnDescription("geo-properties", i18n("Altitude"), "subcolumn", "altitude")
    );

    return description;
}

QString ColumnGeoProperties::getTitle()
{
    switch (subColumn)
    {
    case SubColumnHasCoordinates:
        return i18n("Geotagged");
    case SubColumnCoordinates:
        return i18n("Coordinates");
    case SubColumnAltitude:
        return i18n("Altitude");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnGeoProperties::getColumnFlags() const
{
    ColumnFlags flags(ColumnNoFlags);

    if (subColumn == SubColumnAltitude)
    {
        flags |= ColumnCustomSorting | ColumnHasConfigurationWidget;
    }

    return flags;
}
QVariant ColumnGeoProperties::data(const QModelIndex& sourceIndex, const int role)
{
    if ( (role != Qt::DisplayRole) &&
         (role != Qt::TextAlignmentRole) )
    {
        return sourceIndex.data(role);
    }

    if (role==Qt::TextAlignmentRole)
    {
        switch (subColumn)
        {
            case SubColumnAltitude:
                return QVariant(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));

            default:
                return sourceIndex.data(role);
        }
    }

    const ImageInfo info = getImageInfo(sourceIndex);

    switch (subColumn)
    {
    case SubColumnHasCoordinates:
        return info.hasCoordinates() ? i18n("Yes") : i18n("No");
        break;

    case SubColumnCoordinates:
    {
        if (!info.hasCoordinates())
        {
            return QString();
        }

        return QString("%1,%2")
               .arg(
                   KGlobal::locale()->formatNumber(info.latitudeNumber(), 7)
               )
               .arg(
                   KGlobal::locale()->formatNumber(info.longitudeNumber(), 7)
               );
        break;
    }

    case SubColumnAltitude:
    {
        /// @todo Needs custom sorting
        if ((!info.hasCoordinates()) || (!info.hasAltitude()))
        {
            return QString();
        }
        return KGlobal::locale()->formatNumber(info.altitudeNumber());
        break;
    }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnGeoProperties::compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
{
    const ImageInfo infoA = getImageInfo(sourceA);
    const ImageInfo infoB = getImageInfo(sourceB);

    if (subColumn == SubColumnAltitude)
    {
        const bool hasAltitudeA = infoA.hasAltitude();
        const bool hasAltitudeB = infoB.hasAltitude();

        if (hasAltitudeA && hasAltitudeB)
        {
            const double altitudeA = infoA.altitudeNumber();
            const double altitudeB = infoB.altitudeNumber();

            return compareHelper<double>(altitudeA, altitudeB);
        }

        return compareHelper<int>(int(hasAltitudeA), int(hasAltitudeB));
    }

    kWarning() << "geo: unimplemented comparison, subColumn=" << subColumn;
    return CmpEqual;
}

TableViewColumnConfigurationWidget* ColumnGeoProperties::getConfigurationWidget(QWidget* const parentWidget) const
{
    TableViewColumnConfiguration myConfiguration = getConfiguration();
    return new ColumnGeoConfigurationWidget(s, myConfiguration, parentWidget);
}

ColumnGeoConfigurationWidget::ColumnGeoConfigurationWidget(
        TableViewShared* const sharedObject,
        const TableViewColumnConfiguration& columnConfiguration,
        QWidget* const parentWidget)
  : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget)
{

}

ColumnGeoConfigurationWidget::~ColumnGeoConfigurationWidget()
{

}

TableViewColumnConfiguration ColumnGeoConfigurationWidget::getNewConfiguration()
{
    return configuration;
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

