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

// Qt includes

#include <QComboBox>
#include <QFormLayout>
#include <QModelIndex>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// local includes

#include "imageinfo.h"

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
    subColumn = getSubColumnIndex<ColumnFileProperties>(subColumnSetting, SubColumnName);
}

TableViewColumnDescription ColumnFileProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("file-properties"), i18n("File properties"));
    description.setIcon("dialog-information");

    description.addSubColumn(
        TableViewColumnDescription("file-properties", i18n("Filename"), "subcolumn", "name")
    );

    description.addSubColumn(
        TableViewColumnDescription("file-properties", i18n("Size"), "subcolumn", "size")
    );

    return description;
}

QString ColumnFileProperties::getTitle() const
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

QVariant ColumnFileProperties::data(const QModelIndex& sourceIndex, const int role) const
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
            case SubColumnSize:
                return QVariant(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));

            default:
                return sourceIndex.data(role);
        }
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
        /// @todo Use an enum instead to avoid lots of string comparisons
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
  : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget),
    subColumn(ColumnFileProperties::SubColumnName),
    selectorSizeType(0)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    subColumn = ColumnFileProperties::getSubColumnIndex<ColumnFileProperties>(subColumnSetting, ColumnFileProperties::SubColumnName);

    switch (subColumn)
    {
    case ColumnFileProperties::SubColumnSize:
        {
            QFormLayout* const box1 = new QFormLayout();
            selectorSizeType = new QComboBox(this);
            selectorSizeType->addItem(i18n("KDE default"), QString("kde"));
            selectorSizeType->addItem(i18n("Plain"), QString("plain"));
            box1->addRow(i18n("Display format"), selectorSizeType);

            setLayout(box1);

            const int index = selectorSizeType->findData(configuration.getSetting("format", "kde"));
            selectorSizeType->setCurrentIndex(index>=0 ? index : 0);
            break;
        }

    default:
        break;
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

    emit(signalAllDataChanged());
}

TableViewColumnConfigurationWidget* ColumnFileProperties::getConfigurationWidget(QWidget* const parentWidget) const
{
    return new ColumnFileConfigurationWidget(s, configuration, parentWidget);
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

