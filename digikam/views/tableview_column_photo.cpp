/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-03-14
 * Description : Table view column helpers: Photo properties
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

#include "tableview_column_photo.moc"

// Qt includes

#include <QModelIndex>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// local includes

#include <databaseinfocontainers.h>
#include <dmetadata.h>
#include <imageinfo.h>

namespace Digikam
{

namespace TableViewColumns
{

ColumnPhotoProperties::ColumnPhotoProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(SubColumnCameraMaker)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    subColumn = getSubColumnIndex<ColumnPhotoProperties>(subColumnSetting, SubColumnCameraMaker);
}

ColumnPhotoProperties::~ColumnPhotoProperties()
{

}

QStringList ColumnPhotoProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("cameramaker") << QLatin1String("cameramodel")
            << QLatin1String("lens") << QLatin1String("aperture")
            << QLatin1String("focal") << QLatin1String("exposure")
            << QLatin1String("sensitivity") << QLatin1String("modeprogram")
            << QLatin1String("flash") << QLatin1String("whitebalance");

    return columns;
}

TableViewColumnDescription ColumnPhotoProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("photo-properties"), i18n("Photo properties"));
    description.setIcon("camera-photo");

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Camera maker"), "subcolumn", "cameramaker")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Camera model"), "subcolumn", "cameramodel")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Lens"), "subcolumn", "lens")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Aperture"), "subcolumn", "aperture")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Focal length"), "subcolumn", "focal")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Exposure"), "subcolumn", "exposure")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Sensitivity"), "subcolumn", "sensitivity")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Mode/program"), "subcolumn", "modeprogram")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("Flash"), "subcolumn", "flash")
    );

    description.addSubColumn(
        TableViewColumnDescription("photo-properties", i18n("White balance"), "subcolumn", "whitebalance")
    );

    return description;
}

QString ColumnPhotoProperties::getTitle() const
{
    switch (subColumn)
    {
    case SubColumnCameraMaker:
        return i18n("Camera maker");
    case SubColumnCameraModel:
        return i18n("Camera model");
    case SubColumnLens:
        return i18n("Lens");
    case SubColumnAperture:
        return i18n("Aperture");
    case SubColumnFocal:
        return i18n("Focal length");
    case SubColumnExposure:
        return i18n("Exposure");
    case SubColumnSensitivity:
        return i18n("Sensitivity");
    case SubColumnModeProgram:
        return i18n("Mode/program");
    case SubColumnFlash:
        return i18n("Flash");
    case SubColumnWhiteBalance:
        return i18n("White balance");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnPhotoProperties::getColumnFlags() const
{
    ColumnFlags flags(ColumnNoFlags);

    if (   (subColumn==SubColumnAperture)
        || (subColumn==SubColumnFocal)
        || (subColumn==SubColumnExposure)
        || (subColumn==SubColumnSensitivity) )
    {
        flags|=ColumnCustomSorting;
    }

    return flags;
}

QVariant ColumnPhotoProperties::data(TableViewModel::Item* const item, const int role) const
{
    if (role != Qt::DisplayRole)
    {
        return item->imageFilterModelIndex.data(role);
    }

    switch (subColumn)
    {
    case SubColumnCameraMaker:
        {
            const QString cameraMaker = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Make).toString();
            return cameraMaker;
        }
    case SubColumnCameraModel:
        {
            const QString cameraModel = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Model).toString();
            return cameraModel;
        }
    case SubColumnLens:
        {
            const QString cameraLens = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Lens).toString();
            return cameraLens;
        }
    case SubColumnAperture:
        {
            const QVariant apertureVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Aperture);
            const QString apertureString = DMetadata::valueToString(apertureVariant, MetadataInfo::Aperture);

            return apertureString;
        }
    case SubColumnFocal:
        {
            /// @todo Make this configurable
            const QVariant focalLengthVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::FocalLength);
            const QString focalLengthString = DMetadata::valueToString(focalLengthVariant, MetadataInfo::FocalLength);
            const QVariant focalLength35Variant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::FocalLength35);
            const QString focalLength35String = DMetadata::valueToString(focalLength35Variant, MetadataInfo::FocalLengthIn35mm);

            if (focalLength35String.isEmpty())
            {
                return focalLengthString;
            }
            if (focalLengthString.isEmpty())
            {
                return QString();
            }

            /// @todo What if only 35 mm is set?
            return i18n("%1 (35mm: %2)", focalLengthString, focalLength35String);
        }
    case SubColumnExposure:
        {
            /// @todo Add a configuration option for fraction vs number, units s vs ms vs mus
            const QVariant exposureVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::ExposureTime);
            const QString exposureString = DMetadata::valueToString(exposureVariant, MetadataInfo::ExposureTime);

            return exposureString;
        }
    case SubColumnSensitivity:
        {
            const QVariant sensitivityVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Sensitivity);
            const QString sensitivityString = DMetadata::valueToString(sensitivityVariant, MetadataInfo::Sensitivity);
            if (sensitivityString.isEmpty())
            {
                return QString();
            }

            return i18n("%1 ISO", sensitivityString);
        }
    case SubColumnModeProgram:
        {
            const QVariant exposureModeVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::ExposureMode);
            const QString exposureModeString = DMetadata::valueToString(exposureModeVariant, MetadataInfo::ExposureMode);
            const QVariant exposureProgramVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::ExposureProgram);
            const QString exposureProgramString = DMetadata::valueToString(exposureProgramVariant, MetadataInfo::ExposureProgram);

            if (exposureModeString.isEmpty() && exposureProgramString.isEmpty())
            {
                return QString();
            }
            else if (!exposureModeString.isEmpty() && exposureProgramString.isEmpty())
            {
                return exposureModeString;
            }
            else if (exposureModeString.isEmpty() && !exposureProgramString.isEmpty())
            {
                return exposureProgramString;
            }

            return QString("%1 / %2").arg(exposureModeString).arg(exposureProgramString);
        }
    case SubColumnFlash:
        {
            const QVariant flashModeVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::FlashMode);
            const QString flashModeString = DMetadata::valueToString(flashModeVariant, MetadataInfo::FlashMode);

            return flashModeString;
        }
    case SubColumnWhiteBalance:
        {
            const QVariant whiteBalanceVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::WhiteBalance);
            const QString whiteBalanceString = DMetadata::valueToString(whiteBalanceVariant, MetadataInfo::WhiteBalance);

            return whiteBalanceString;
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnPhotoProperties::compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const
{
    const ImageInfo infoA = s->tableViewModel->infoFromItem(itemA);
    const ImageInfo infoB = s->tableViewModel->infoFromItem(itemB);

    switch (subColumn)
    {

    case SubColumnAperture:
        {
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::Aperture);
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::Aperture);
            const double apertureA = variantA.toDouble();
            const double apertureB = variantB.toDouble();

            return compareHelper<double>(apertureA, apertureB);
        }

    case SubColumnFocal:
        {
            /// @todo This just works if both have focal length set, not if focal length 35 has to be used
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::FocalLength);
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::FocalLength);
            const double focalLengthA = variantA.toDouble();
            const double focalLengthB = variantB.toDouble();

            return compareHelper<double>(focalLengthA, focalLengthB);
        }

    case SubColumnExposure:
        {
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::ExposureTime);
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::ExposureTime);
            const double exposureTimeA = variantA.toDouble();
            const double exposureTimeB = variantB.toDouble();

            return compareHelper<double>(exposureTimeA, exposureTimeB);
        }

    case SubColumnSensitivity:
        {
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::Sensitivity);
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::Sensitivity);
            const double sensitivityA = variantA.toDouble();
            const double sensitivityB = variantB.toDouble();

            return compareHelper<double>(sensitivityA, sensitivityB);
        }

    default:
        kWarning() << "item: unimplemented comparison, subColumn=" << subColumn;
        return CmpEqual;
    }
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

