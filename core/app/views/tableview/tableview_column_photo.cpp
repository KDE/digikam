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

#include "tableview_column_photo.h"

// Qt includes

#include <QFormLayout>
#include <QComboBox>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredbinfocontainers.h"
#include "imagepropertiestab.h"
#include "dmetadata.h"
#include "imageinfo.h"

namespace Digikam
{

namespace TableViewColumns
{

ColumnPhotoProperties::ColumnPhotoProperties(TableViewShared* const tableViewShared,
                                             const TableViewColumnConfiguration& pConfiguration,
                                             const SubColumn pSubColumn,
                                             QObject* const parent)
    : TableViewColumn(tableViewShared, pConfiguration, parent),
      subColumn(pSubColumn)
{
}

ColumnPhotoProperties::~ColumnPhotoProperties()
{
}

QStringList ColumnPhotoProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("cameramaker") << QLatin1String("cameramodel")
            << QLatin1String("lens")        << QLatin1String("aperture")
            << QLatin1String("focal")       << QLatin1String("exposure")
            << QLatin1String("sensitivity") << QLatin1String("modeprogram")
            << QLatin1String("flash")       << QLatin1String("whitebalance");

    return columns;
}

TableViewColumnDescription ColumnPhotoProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("photo-properties"), i18n("Photo properties"));
    description.setIcon(QLatin1String("camera-photo"));

    description.addSubColumn(TableViewColumnDescription(QLatin1String("cameramaker"),  i18n("Camera maker")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("cameramodel"),  i18n("Camera model")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("lens"),         i18n("Lens")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("aperture"),     i18n("Aperture")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("focal"),        i18n("Focal length")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("exposure"),     i18n("Exposure")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("sensitivity"),  i18n("Sensitivity")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("modeprogram"),  i18n("Mode/program")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("flash"),        i18n("Flash")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("bordertool"),   i18n("White balance")));

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

    if ((subColumn == SubColumnAperture)  ||
        (subColumn == SubColumnFocal)     ||
        (subColumn == SubColumnExposure)  ||
        (subColumn == SubColumnSensitivity) )
    {
        flags |= ColumnCustomSorting;
    }

    if (subColumn == SubColumnExposure)
    {
        flags |= ColumnHasConfigurationWidget;
    }

    return flags;
}

QVariant ColumnPhotoProperties::data(TableViewModel::Item* const item, const int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    switch (subColumn)
    {
        case SubColumnCameraMaker:
        {
            QString cameraMaker = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Make).toString();
            ImagePropertiesTab::shortenedMakeInfo(cameraMaker);
            return cameraMaker;
        }
        case SubColumnCameraModel:
        {
            QString cameraModel = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Model).toString();
            ImagePropertiesTab::shortenedModelInfo(cameraModel);
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
            const QString apertureString   = DMetadata::valueToString(apertureVariant, MetadataInfo::Aperture);

            return apertureString;
        }
        case SubColumnFocal:
        {
            /// @todo Make this configurable
            const DatabaseFields::Set requiredSet                 = DatabaseFields::FocalLength | DatabaseFields::FocalLength35;
            const TableViewModel::DatabaseFieldsHashRaw rawFields = s->tableViewModel->itemDatabaseFieldsRaw(item, requiredSet);
            const QVariant focalLengthVariant                     = rawFields.value(DatabaseFields::FocalLength);
            const QString focalLengthString                       = DMetadata::valueToString(focalLengthVariant, MetadataInfo::FocalLength);
            const QVariant focalLength35Variant                   = rawFields.value(DatabaseFields::FocalLength35);
            const QString focalLength35String                     = DMetadata::valueToString(focalLength35Variant, MetadataInfo::FocalLengthIn35mm);

            if (focalLength35String.isEmpty())
            {
                return focalLengthString;
            }

            if (focalLengthString.isEmpty())
            {
                return QString();
            }

            /// @todo What if only 35 mm is set?
            return i18n("%1 (%2)", focalLengthString, focalLength35String);
        }
        case SubColumnExposure:
        {
            /// @todo Add a configuration option for fraction vs number, units s vs ms vs mus
            const QVariant exposureVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::ExposureTime);

            if (configuration.getSetting(QLatin1String("format"), QLatin1String("fraction")) == QLatin1String("fraction"))
            {
                const QString exposureString = DMetadata::valueToString(exposureVariant, MetadataInfo::ExposureTime);
                return exposureString;
            }

            if (!exposureVariant.isValid())
            {
                return QString();
            }

            const QString unitKey = configuration.getSetting(QLatin1String("unit"), QLatin1String("seconds"));
            double multiplier     = 1.0;
            KLocalizedString exposureTimeLocalizedString = ki18n("%1 s");

            if (unitKey == QLatin1String("milliseconds"))
            {
                multiplier                  = 1000.0;
                exposureTimeLocalizedString = ki18n("%1 ms");
            }
            else if (unitKey == QLatin1String("microseconds"))
            {
                multiplier                  = 1000000.0;
                exposureTimeLocalizedString = ki18n("%1 µs");
            }

            const double exposureTime = exposureVariant.toDouble() * multiplier;
            /// @todo Seems like we have to check for 0 here, too, as an invalid value

            if (exposureTime == 0)
            {
                return QString();
            }

            /// @todo Remove trailing zeros?
            /// @todo Align right? --> better align at decimal point
            const QString exposureTimeString = exposureTimeLocalizedString.subs(QLocale().toString(exposureTime, 'g', 3)).toString();
            return exposureTimeString;
        }
        case SubColumnSensitivity:
        {
            const QVariant sensitivityVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Sensitivity);
            const QString sensitivityString   = DMetadata::valueToString(sensitivityVariant, MetadataInfo::Sensitivity);

            if (sensitivityString.isEmpty())
            {
                return QString();
            }

            return i18n("%1 ISO", sensitivityString);
        }
        case SubColumnModeProgram:
        {
            const DatabaseFields::Set requiredSet                 = DatabaseFields::ExposureMode | DatabaseFields::ExposureProgram;
            const TableViewModel::DatabaseFieldsHashRaw rawFields = s->tableViewModel->itemDatabaseFieldsRaw(item, requiredSet);
            const QVariant exposureModeVariant                    = rawFields.value(DatabaseFields::ExposureMode);
            const QString exposureModeString                      = DMetadata::valueToString(exposureModeVariant, MetadataInfo::ExposureMode);
            const QVariant exposureProgramVariant                 = rawFields.value(DatabaseFields::ExposureProgram);
            const QString exposureProgramString                   = DMetadata::valueToString(exposureProgramVariant, MetadataInfo::ExposureProgram);

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

            return QString::fromUtf8("%1 / %2").arg(exposureModeString).arg(exposureProgramString);
        }
        case SubColumnFlash:
        {
            const QVariant flashModeVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::FlashMode);
            const QString flashModeString   = DMetadata::valueToString(flashModeVariant, MetadataInfo::FlashMode);

            return flashModeString;
        }
        case SubColumnWhiteBalance:
        {
            const QVariant whiteBalanceVariant = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::WhiteBalance);
            const QString whiteBalanceString   = DMetadata::valueToString(whiteBalanceVariant, MetadataInfo::WhiteBalance);

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
            const double apertureA  = variantA.toDouble();
            const double apertureB  = variantB.toDouble();

            return compareHelper<double>(apertureA, apertureB);
        }

        case SubColumnFocal:
        {
            /// @todo This just works if both have focal length set, not if focal length 35 has to be used
            const QVariant variantA   = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::FocalLength);
            const QVariant variantB   = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::FocalLength);
            const double focalLengthA = variantA.toDouble();
            const double focalLengthB = variantB.toDouble();

            return compareHelper<double>(focalLengthA, focalLengthB);
        }

        case SubColumnExposure:
        {
            const QVariant variantA    = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::ExposureTime);
            const QVariant variantB    = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::ExposureTime);
            const double exposureTimeA = variantA.toDouble();
            const double exposureTimeB = variantB.toDouble();

            return compareHelper<double>(exposureTimeA, exposureTimeB);
        }

        case SubColumnSensitivity:
        {
            const QVariant variantA   = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::Sensitivity);
            const QVariant variantB   = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::Sensitivity);
            const double sensitivityA = variantA.toDouble();
            const double sensitivityB = variantB.toDouble();

            return compareHelper<double>(sensitivityA, sensitivityB);
        }

        default:
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "item: unimplemented comparison, subColumn=" << subColumn;
            return CmpEqual;
        }
    }
}

TableViewColumnConfigurationWidget* ColumnPhotoProperties::getConfigurationWidget(QWidget* const parentWidget) const
{
    TableViewColumnConfiguration myConfiguration = getConfiguration();
    return new ColumnPhotoConfigurationWidget(s, myConfiguration, parentWidget);
}

// ---------------------------------------------------------------------------------------------------------------------

ColumnPhotoConfigurationWidget::ColumnPhotoConfigurationWidget(TableViewShared* const sharedObject,
                                                               const TableViewColumnConfiguration& columnConfiguration,
                                                               QWidget* const parentWidget)
    : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget),
      subColumn(ColumnPhotoProperties::SubColumnExposure),
      selectorExposureTimeFormat(0),
      selectorExposureTimeUnit(0)
{
    ColumnPhotoProperties::getSubColumnIndex<ColumnPhotoProperties>(configuration.columnId, &subColumn);

    switch (subColumn)
    {
        case ColumnPhotoProperties::SubColumnExposure:
        {
            QFormLayout* const box1    = new QFormLayout();
            selectorExposureTimeFormat = new QComboBox(this);
            selectorExposureTimeFormat->addItem(i18n("Fraction"), QLatin1String("fraction"));
            selectorExposureTimeFormat->addItem(i18n("Rational"), QLatin1String("rational"));
            box1->addRow(i18n("Format"), selectorExposureTimeFormat);

            selectorExposureTimeUnit = new QComboBox(this);
            selectorExposureTimeUnit->addItem(i18n("Seconds"),      QLatin1String("seconds"));
            selectorExposureTimeUnit->addItem(i18n("Milliseconds"), QLatin1String("milliseconds"));
            selectorExposureTimeUnit->addItem(i18n("Microseconds"), QLatin1String("microseconds"));
            box1->addRow(i18n("Unit"), selectorExposureTimeUnit);

            setLayout(box1);

            const int indexF = selectorExposureTimeFormat->findData(configuration.getSetting(QLatin1String("format"), QLatin1String("fraction")));
            selectorExposureTimeFormat->setCurrentIndex(indexF >= 0 ? indexF : 0);
            const int indexU = selectorExposureTimeUnit->findData(configuration.getSetting(QLatin1String("unit"), QLatin1String("seconds")));
            selectorExposureTimeUnit->setCurrentIndex(indexU >= 0 ? indexU : 0);

            slotUpdateUI();

            connect(selectorExposureTimeFormat, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotUpdateUI()));
            break;
        }

        default:
            break;
    }
}

ColumnPhotoConfigurationWidget::~ColumnPhotoConfigurationWidget()
{
}

TableViewColumnConfiguration ColumnPhotoConfigurationWidget::getNewConfiguration()
{
    const QString formatKey = selectorExposureTimeFormat->itemData(selectorExposureTimeFormat->currentIndex()).toString();
    configuration.columnSettings.insert(QLatin1String("format"), formatKey);

    const QString unitKey   = selectorExposureTimeUnit->itemData(selectorExposureTimeUnit->currentIndex()).toString();
    configuration.columnSettings.insert(QLatin1String("unit"), unitKey);

    return configuration;
}

void ColumnPhotoProperties::setConfiguration(const TableViewColumnConfiguration& newConfiguration)
{
    configuration = newConfiguration;

    emit(signalAllDataChanged());
}

void ColumnPhotoConfigurationWidget::slotUpdateUI()
{
    if (selectorExposureTimeFormat)
    {
        const QString currentKey = selectorExposureTimeFormat->itemData(selectorExposureTimeFormat->currentIndex()).toString();
        const bool needsUnits    = (currentKey == QLatin1String("rational"));

        selectorExposureTimeUnit->setEnabled(needsUnits);
    }
}

} /* namespace TableViewColumns */

} /* namespace Digikam */
