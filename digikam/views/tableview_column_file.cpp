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

#include <QFormLayout>

// KDE includes

#include <kcombobox.h>
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
        const SubColumn pSubColumn,
        QObject* const parent
    )
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(pSubColumn)
{

}

TableViewColumnDescription ColumnFileProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("file-properties"), i18n("File properties"));
    description.setIcon("dialog-information");

    description.addSubColumn(
        TableViewColumnDescription("filename", i18n("Filename"))
    );

    description.addSubColumn(
        TableViewColumnDescription("filepath", i18n("Path"))
    );

    description.addSubColumn(
        TableViewColumnDescription("filesize", i18n("Size"))
    );

    description.addSubColumn(
        TableViewColumnDescription("filelastmodified", i18n("Last modified"))
    );

    return description;
}

QStringList ColumnFileProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("filename") << QLatin1String("filepath")
            << QLatin1String("filesize") << QLatin1String("filelastmodified");

    return columns;
}

QString ColumnFileProperties::getTitle() const
{
    switch (subColumn)
    {
    case SubColumnName:
        return i18n("Filename");
    case SubColumnFilePath:
        return i18n("Path");
    case SubColumnSize:
        return i18n("Size");
    case SubColumnLastModified:
        return i18n("Last modified");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnFileProperties::getColumnFlags() const
{
    if (   (subColumn == SubColumnSize)
        || (subColumn == SubColumnLastModified) )
    {
        return ColumnCustomSorting | ColumnHasConfigurationWidget;
    }

    return ColumnNoFlags;
}

QVariant ColumnFileProperties::data(TableViewModel::Item* const item, const int role) const
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
            case SubColumnSize:
                return QVariant(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));

            default:
                return QVariant();
        }
    }

    const ImageInfo info = s->tableViewModel->infoFromItem(item);

    switch (subColumn)
    {
    case SubColumnName:
        return info.fileUrl().fileName();
        break;

    case SubColumnFilePath:
        return info.fileUrl().path();
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

    case SubColumnLastModified:
    {
        const QDateTime lastModifiedTime = info.modDateTime();

        return KGlobal::locale()->formatDateTime(lastModifiedTime, KLocale::ShortDate, true);
    }

    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnFileProperties::compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const
{
    const ImageInfo infoA = s->tableViewModel->infoFromItem(itemA);
    const ImageInfo infoB = s->tableViewModel->infoFromItem(itemB);

    switch (subColumn)
    {
    case SubColumnSize:
    {
        const int sizeA = infoA.fileSize();
        const int sizeB = infoB.fileSize();

        return compareHelper<int>(sizeA, sizeB);
    }

    case SubColumnLastModified:
    {
        const QDateTime dtA = infoA.modDateTime();
        const QDateTime dtB = infoB.modDateTime();

        return compareHelper<QDateTime>(dtA, dtB);
    }

    default:

        kWarning() << "file: unimplemented comparison, subColumn=" << subColumn;
        return CmpEqual;
    }

}

ColumnFileConfigurationWidget::ColumnFileConfigurationWidget(
        TableViewShared* const sharedObject,
        const TableViewColumnConfiguration& columnConfiguration,
        QWidget* const parentWidget)
  : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget),
    subColumn(ColumnFileProperties::SubColumnName),
    selectorSizeType(0)
{
    ColumnFileProperties::getSubColumnIndex<ColumnFileProperties>(configuration.columnId, &subColumn);

    switch (subColumn)
    {
    case ColumnFileProperties::SubColumnSize:
        {
            QFormLayout* const box1 = new QFormLayout();
            selectorSizeType = new KComboBox(this);
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

