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

// Qt includes

#include <QFormLayout>

// KDE includes

#include <kcombobox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

// local includes

#include "imageinfo.h"

namespace
{

QString FormatAltitude(const qreal altitudeInMeters, const KLocale::MeasureSystem& measureSystem)
{
    if (measureSystem==KLocale::Metric)
    {
        const QString altitudeInMetersString = KGlobal::locale()->formatNumber(altitudeInMeters, 2);
        return QString("%1 m").arg(altitudeInMetersString);
    }
    else
    {
        const qreal altitudeInFeet = altitudeInMeters /* m */ / ( 0.3048 /* m/foot */ );
        const QString altitudeInFeetString = KGlobal::locale()->formatNumber(altitudeInFeet, 2);
        return QString("%1 ft").arg(altitudeInFeetString);
    }
}

}

namespace Digikam
{

namespace TableViewColumns
{

ColumnGeoProperties::ColumnGeoProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        const SubColumn pSubColumn,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(pSubColumn)
{

}

ColumnGeoProperties::~ColumnGeoProperties()
{

}

QStringList ColumnGeoProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("geohascoordinates") << QLatin1String("geocoordinates")
            << QLatin1String("geoaltitude");

    return columns;
}

TableViewColumnDescription ColumnGeoProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("geo-properties"), i18n("Geo properties"));
    description.setIcon("applications-internet");

    description.addSubColumn(
        TableViewColumnDescription("geohascoordinates", i18n("Geotagged"))
    );

    description.addSubColumn(
        TableViewColumnDescription("geocoordinates", i18n("Coordinates"))
    );

    description.addSubColumn(
        TableViewColumnDescription("geoaltitude", i18n("Altitude"))
    );

    return description;
}

QString ColumnGeoProperties::getTitle() const
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
QVariant ColumnGeoProperties::data(TableViewModel::Item* const item, const int role) const
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
            case SubColumnAltitude:
                return QVariant(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));

            default:
                return QVariant();
        }
    }

    const ImageInfo info = s->tableViewModel->infoFromItem(item);

    switch (subColumn)
    {
    case SubColumnHasCoordinates:
        return info.hasCoordinates() ? i18n("Yes") : i18n("No");

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
        }

    case SubColumnAltitude:
        {
            /// @todo Needs custom sorting
            if ((!info.hasCoordinates()) || (!info.hasAltitude()))
            {
                return QString();
            }

            /// @todo Use an enum instead to avoid lots of string comparisons
            const QString formatKey = configuration.getSetting("format", "kde");
            KLocale::MeasureSystem measureSystem = KGlobal::locale()->measureSystem();
            if (formatKey=="metric")
            {
                measureSystem = KLocale::Metric;
            }
            else if (formatKey=="imperial")
            {
                measureSystem = KLocale::Imperial;
            }

            const QString formattedAltitude = FormatAltitude(info.altitudeNumber(), measureSystem);
            return formattedAltitude;
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnGeoProperties::compare(
    TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const
{
    const ImageInfo infoA = s->tableViewModel->infoFromItem(itemA);
    const ImageInfo infoB = s->tableViewModel->infoFromItem(itemB);

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
  : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget),
    subColumn(ColumnGeoProperties::SubColumnHasCoordinates),
    selectorAltitudeUnit(0)
{
    ColumnGeoProperties::getSubColumnIndex<ColumnGeoProperties>(configuration.columnId, &subColumn);

    switch (subColumn)
    {
    case ColumnGeoProperties::SubColumnAltitude:
        {
            QFormLayout* const box1 = new QFormLayout();
            selectorAltitudeUnit = new KComboBox(this);
            selectorAltitudeUnit->addItem(i18n("KDE default"), QString("kde"));
            selectorAltitudeUnit->addItem(i18n("Metric units"), QString("metric"));
            selectorAltitudeUnit->addItem(i18n("Imperial units"), QString("imperial"));
            box1->addRow(i18n("Display format"), selectorAltitudeUnit);

            setLayout(box1);

            const int index = selectorAltitudeUnit->findData(configuration.getSetting("format", "kde"));
            selectorAltitudeUnit->setCurrentIndex(index>=0 ? index : 0);
            break;
        }

    default:
        break;
    }
}

ColumnGeoConfigurationWidget::~ColumnGeoConfigurationWidget()
{

}

TableViewColumnConfiguration ColumnGeoConfigurationWidget::getNewConfiguration()
{
    const QString formatKey = selectorAltitudeUnit->itemData(selectorAltitudeUnit->currentIndex()).toString();
    configuration.columnSettings.insert("format", formatKey);

    return configuration;
}

void ColumnGeoProperties::setConfiguration(const TableViewColumnConfiguration& newConfiguration)
{
    configuration = newConfiguration;

    emit(signalAllDataChanged());
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

