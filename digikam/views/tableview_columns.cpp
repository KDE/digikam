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

#include "tableview_columns.moc"
#include <klistwidget.h>
#include <QFormLayout>

// Qt includes

// KDE includes

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

ColumnGeoProperties::ColumnGeoProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(SubColumnCoordinates)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    if (subColumnSetting == "hascoordinates")
    {
        subColumn = SubColumnHasCoordinates;
    }
    else if (subColumnSetting == "coordinates")
    {
        subColumn = SubColumnCoordinates;
    }
    else if (subColumnSetting == "altitude")
    {
        subColumn = SubColumnAltitude;
    }
}

ColumnGeoProperties::~ColumnGeoProperties()
{

}

TableViewColumnDescription ColumnGeoProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("geo-properties"), i18n("Geo properties"));

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
    if (role != Qt::DisplayRole)
    {
        /// @todo is this correct or does sourceIndex have column!=0?
        return sourceIndex.data(role);
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

ColumnThumbnail::ColumnThumbnail(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent)
{
    connect(s->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription, QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription, QPixmap)));
}

ColumnThumbnail::~ColumnThumbnail()
{

}

TableViewColumnDescription ColumnThumbnail::getDescription()
{
    return TableViewColumnDescription(QLatin1String("thumbnail"), QLatin1String("Thumbnail"));
}

TableViewColumn::ColumnFlags ColumnThumbnail::getColumnFlags() const
{
    return ColumnCustomPainting;
}

QString ColumnThumbnail::getTitle()
{
    return i18n("Thumbnail");
}

QVariant ColumnThumbnail::data(const QModelIndex& sourceIndex, const int role)
{
    Q_UNUSED(sourceIndex)
    Q_UNUSED(role)

    // we do not return any data, but paint(...) something
    return QVariant();
}

bool ColumnThumbnail::paint(QPainter* const painter, const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
{
    /// @todo do we have to reset the column?
    const ImageInfo info = getImageInfo(sourceIndex);
    if (!info.isNull())
    {
        QSize size(60, 60);
        const QString path = info.filePath();
        QPixmap thumbnail;

        /// @todo handle unavailable thumbnails -> emit itemChanged(...) later
        if (s->thumbnailLoadThread->find(path, thumbnail, qMax(size.width() + 2, size.height() + 2)))
        {
            /// @todo Is slotThumbnailLoaded still called when the thumbnail is found right away?
            /// @todo remove borders
//                 thumbnail = thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2)
            const QSize availableSize = option.rect.size();
            const QSize pixmapSize    = thumbnail.size().boundedTo(availableSize);
            QPoint startPoint((availableSize.width() - pixmapSize.width()) / 2,
                              (availableSize.height() - pixmapSize.height()) / 2);
            startPoint += option.rect.topLeft();
            painter->drawPixmap(QRect(startPoint, pixmapSize), thumbnail, QRect(QPoint(0, 0), pixmapSize));

            return true;
        }
    }

    // we did not get to paint a thumbnail...
    return false;
}

QSize ColumnThumbnail::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
{
    return QSize(60, 60);
}

void ColumnThumbnail::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    const QModelIndex sourceIndex = s->imageFilterModel->indexForPath(loadingDescription.filePath);
    if (!sourceIndex.isValid())
    {
        return;
    }

    emit(signalDataChanged(sourceIndex));
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

