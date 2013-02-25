/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: File properties
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

#include "tableview_column_file.moc"
#include <QFormLayout>

// Qt includes

// KDE includes

#include <kdebug.h>

// local includes

namespace Digikam
{

namespace TableViewColumns
{

ColumnFileProperties::ColumnFileProperties(
    TableViewShared* const tableViewShared,
    const TableViewColumnConfiguration& pConfiguration,
    QObject* const parent)
  : TableViewColumn(tableViewShared,
    pConfiguration, parent),
    subColumn(SubColumnName)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    subColumn = SubColumn(getSubColumns().indexOf(subColumnSetting));
    if (subColumn<0)
    {
        subColumn = SubColumnName;
    }
}

TableViewColumnDescription ColumnFileProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("file-properties"), i18n("File properties"));

    description.addSubColumn(
        TableViewColumnDescription("file-properties", i18n("Filename"), "subcolumn", "name")
    );

    description.addSubColumn(
        TableViewColumnDescription("file-properties", i18n("Size"), "subcolumn", "size")
    );

    return description;
}

QString ColumnFileProperties::getTitle()
{
    switch (subColumn)
    {
    case SubColumnName:
        return i18n("Filename");
    case SubColumnSize:
        return i18n("Size");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnFileProperties::getColumnFlags() const
{
    if (subColumn == SubColumnSize)
    {
        return ColumnCustomSorting | ColumnHasConfigurationWidget;
    }

    return ColumnNoFlags;
}

QVariant ColumnFileProperties::data(const QModelIndex& sourceIndex, const int role)
{
    if (role != Qt::DisplayRole)
    {
        /// @todo is this correct or does sourceIndex have column!=0?
        return sourceIndex.data(role);
    }

    const ImageInfo info = getImageInfo(sourceIndex);

    switch (subColumn)
    {
    case SubColumnName:
        return info.fileUrl().fileName();
        break;

    case SubColumnSize:
    {
        /// @todo Add configuration options for SI-prefixes
        const QString formatKey = configuration.getSetting("format", "kde");
        if (formatKey=="kde")
        {
            return KGlobal::locale()->formatByteSize(info.fileSize());
        }
        else
        {
            // formatKey=="plain"
            return KGlobal::locale()->formatNumber(info.fileSize(), 0);
        }
        break;
    }

    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnFileProperties::compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
{
    const ImageInfo infoA = getImageInfo(sourceA);
    const ImageInfo infoB = getImageInfo(sourceB);

    if (subColumn == SubColumnSize)
    {
        const int sizeA = infoA.fileSize();
        const int sizeB = infoB.fileSize();

        return compareHelper<int>(sizeA, sizeB);
    }

    kWarning() << "file: unimplemented comparison, subColumn=" << subColumn;
    return CmpEqual;
}

ColumnFileConfigurationWidget::ColumnFileConfigurationWidget(
        TableViewShared* const sharedObject,
        const TableViewColumnConfiguration& columnConfiguration,
        QWidget* const parentWidget)
  : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    subColumn = ColumnFileProperties::SubColumn(ColumnFileProperties::getSubColumns().indexOf(subColumnSetting));
    if (subColumn<0)
    {
        subColumn = ColumnFileProperties::SubColumnName;
    }

    if (subColumn==ColumnFileProperties::SubColumnSize)
    {
        QFormLayout* const box1 = new QFormLayout();
        selectorSizeType = new QComboBox(this);
        selectorSizeType->addItem(i18n("KDE default"), QString("kde"));
        selectorSizeType->addItem(i18n("Plain"), QString("plain"));
        box1->addRow(i18n("Display format"), selectorSizeType);

        setLayout(box1);

        const int index = selectorSizeType->findData(configuration.getSetting("format", "kde"));
        selectorSizeType->setCurrentIndex(index>=0 ? index : 0);
    }
}

ColumnFileConfigurationWidget::~ColumnFileConfigurationWidget()
{

}

TableViewColumnConfiguration ColumnFileConfigurationWidget::getNewConfiguration()
{
    const QString formatKey = selectorSizeType->itemData(selectorSizeType->currentIndex()).toString();
    configuration.columnSettings.insert("format", formatKey);

    return configuration;
}

QStringList ColumnFileProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("name") << QLatin1String("size");

    return columns;
}

void ColumnFileProperties::setConfiguration(const TableViewColumnConfiguration& newConfiguration)
{
    configuration = newConfiguration;
    /// @todo emit datachanged for this row
}

TableViewColumnConfigurationWidget* ColumnFileProperties::getConfigurationWidget(QWidget* const parentWidget) const
{
    return new ColumnFileConfigurationWidget(s, configuration, parentWidget);
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

