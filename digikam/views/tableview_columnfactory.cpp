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
        const TableViewColumnConfiguration& pConfiguration,
        QObject* const parent
    )
  : QObject(parent),
    s(tableViewShared),
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

    /// @todo extract column ids from column class
    if (columnId=="file-properties")
    {
        return new TableViewColumns::ColumnFileProperties(s, columnConfiguration);
    }
    else if (columnId=="item-properties")
    {
        return new TableViewColumns::ColumnItemProperties(s, columnConfiguration);
    }
    else if (columnId=="digikam-properties")
    {
        return new TableViewColumns::ColumnDigikamProperties(s, columnConfiguration);
    }
    else if (columnId=="geo-properties")
    {
        return new TableViewColumns::ColumnGeoProperties(s, columnConfiguration);
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

    descriptionList << TableViewColumns::ColumnThumbnail::getDescription();
    descriptionList << TableViewColumns::ColumnFileProperties::getDescription();
    descriptionList << TableViewColumns::ColumnItemProperties::getDescription();
    descriptionList << TableViewColumns::ColumnDigikamProperties::getDescription();
    descriptionList << TableViewColumns::ColumnGeoProperties::getDescription();

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

TableViewColumnProfile::TableViewColumnProfile()
{

}

TableViewColumnProfile::~TableViewColumnProfile()
{

}

void TableViewColumnProfile::loadSettings(const KConfigGroup& configGroup)
{
    name = configGroup.readEntry("Profile Name", QString());
    headerState = configGroup.readEntry("Header State", QByteArray());
    const int nColumns = configGroup.readEntry("Column Count", int(0));

    for (int i=0; i<nColumns; ++i)
    {
        /// @todo check for invalid column configurations
        const QString configSubGroupName = QString("Column %1").arg(i);
        const KConfigGroup subGroup = configGroup.group(configSubGroupName);

        /// @todo move loading into TableViewColumnConfiguration
        TableViewColumnConfiguration columnConfiguration;
        columnConfiguration.loadSettings(subGroup);

        columnConfigurationList << columnConfiguration;
    }

    if (columnConfigurationList.isEmpty())
    {
        // no data loaded, create default entries
        columnConfigurationList << TableViewColumnConfiguration("thumbnail");
        columnConfigurationList << TableViewColumnConfiguration("filename");
        columnConfigurationList << TableViewColumnConfiguration("coordinates");
    }
}

void TableViewColumnProfile::saveSettings(KConfigGroup& configGroup)
{
    configGroup.writeEntry("Profile Name", name);
    const int nColumns = columnConfigurationList.count();
    configGroup.writeEntry("Column Count", nColumns);
    configGroup.writeEntry("Header State", headerState);

    for (int i=0; i<nColumns; ++i)
    {
        const QString configSubGroupName = QString("Column %1").arg(i);
        KConfigGroup subGroup = configGroup.group(configSubGroupName);

        const TableViewColumnConfiguration& columnConfiguration = columnConfigurationList.at(i);;
        columnConfiguration.saveSettings(subGroup);
    }
}

void TableViewColumnConfiguration::loadSettings(const KConfigGroup& configGroup)
{
    columnId = configGroup.readEntry("Column Id", QString());

    const int nSettings = configGroup.readEntry("NSettings", int(0));
    for (int i=0; i<nSettings; ++i)
    {
        const QString& key = configGroup.readEntry(QString("Key %1").arg(i), QString());
        const QString& value = configGroup.readEntry(QString("Value %1").arg(i), QString());

        if (!key.isEmpty())
        {
            columnSettings.insert(key, value);
        }
    }
}

void TableViewColumnConfiguration::saveSettings(KConfigGroup& configGroup) const
{
    configGroup.writeEntry("Column Id", columnId);

    const int nSettings = columnSettings.count();
    configGroup.writeEntry("NSettings", nSettings);

    QHashIterator<QString, QString> settingsIterator(columnSettings);
    for (int i=0; settingsIterator.hasNext(); ++i)
    {
        settingsIterator.next();
        configGroup.writeEntry(QString("Key %1").arg(i), settingsIterator.key());
        configGroup.writeEntry(QString("Value %1").arg(i), settingsIterator.value());
    }
}

TableViewColumn::ColumnFlags TableViewColumn::getColumnFlags() const
{
    return ColumnNoFlags;
}

ImageInfo TableViewColumn::getImageInfo(const QModelIndex sourceIndex) const
{
    return s->imageFilterModel->imageInfo(sourceIndex);
}

TableViewColumnConfiguration TableViewColumn::getConfiguration() const
{
    return configuration;
}

/**
 * This function should never be called, because subclasses have to do the comparison on their own. But it can not be
 * pure, since then every subclass which does not do custom comparison would have to implement an empty stub.
 */
TableViewColumn::ColumnCompareResult TableViewColumn::compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
{
    Q_UNUSED(sourceA)
    Q_UNUSED(sourceB)

    kWarning()<<"Unimplemented custom comparison. Make sure getColumnFlags() does not return ColumnCustomSorting.";

    return CmpEqual;
}

TableViewColumnConfigurationWidget* TableViewColumn::getConfigurationWidget(QWidget* const parentWidget) const
{
    return 0;
}

TableViewColumnConfigurationWidget::TableViewColumnConfigurationWidget(
        TableViewShared* const sharedObject,
        const TableViewColumnConfiguration& currentConfiguration,
        QWidget* const parent
    )
  : QWidget(parent),
    s(sharedObject),
    configuration(currentConfiguration)
{

}

TableViewColumnConfigurationWidget::~TableViewColumnConfigurationWidget()
{

}

void TableViewColumn::setConfiguration(const TableViewColumnConfiguration& newConfiguration)
{
    Q_UNUSED(newConfiguration)
}

void TableViewColumn::setThumbnailSize(const ThumbnailSize& size)
{
    Q_UNUSED(size)
}

} /* namespace Digikam */


