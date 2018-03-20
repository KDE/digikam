/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : rotate image batch tool.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "rotate.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dcombobox.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "freerotationfilter.h"
#include "freerotationsettings.h"
#include "loadsavethread.h"



namespace Digikam
{

class Rotate::Private
{
public:

    Private() :
        CUSTOM_ANGLE(DImg::ROT270 + 1),
        label(0),
        useExif(0),
        comboBox(0),
        frSettings(0)
    {
    }

    const int             CUSTOM_ANGLE;

    QLabel*               label;

    QCheckBox*            useExif;

    DComboBox*            comboBox;

    FreeRotationSettings* frSettings;
};

Rotate::Rotate(QObject* const parent)
    : BatchTool(QLatin1String("Rotate"), TransformTool, parent),
      d(new Private)
{
    setToolTitle(i18n("Rotate"));
    setToolDescription(i18n("Rotate images."));
    setToolIconName(QLatin1String("object-rotate-right"));
}

Rotate::~Rotate()
{
    delete d;
}

void Rotate::registerSettingsWidget()
{

    DVBox* const vbox = new DVBox;
    d->useExif        = new QCheckBox(i18n("Use Exif Orientation"), vbox);

    d->label     = new QLabel(vbox);
    d->comboBox  = new DComboBox(vbox);
    d->comboBox->insertItem(DImg::ROT90,     i18n("90 degrees"));
    d->comboBox->insertItem(DImg::ROT180,    i18n("180 degrees"));
    d->comboBox->insertItem(DImg::ROT270,    i18n("270 degrees"));
    d->comboBox->insertItem(d->CUSTOM_ANGLE, i18n("Custom"));
    d->comboBox->setDefaultIndex(DImg::ROT90);
    d->label->setText(i18n("Angle:"));

    d->frSettings = new FreeRotationSettings(vbox);

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget = vbox;

    setNeedResetExifOrientation(true);

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useExif, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->frSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    slotSettingsChanged();

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Rotate::defaultSettings()
{
    BatchToolSettings settings;
    FreeRotationContainer defaultPrm = d->frSettings->defaultSettings();

    settings.insert(QLatin1String("useExif"),   true);
    settings.insert(QLatin1String("rotation"),  d->comboBox->defaultIndex());
    settings.insert(QLatin1String("angle"),     defaultPrm.angle);
    settings.insert(QLatin1String("antiAlias"), defaultPrm.antiAlias);
    settings.insert(QLatin1String("autoCrop"),  defaultPrm.autoCrop);
    return settings;
}

void Rotate::slotAssignSettings2Widget()
{
    d->useExif->setChecked(settings()[QLatin1String("useExif")].toBool());
    d->comboBox->setCurrentIndex(settings()[QLatin1String("rotation")].toInt());
    FreeRotationContainer prm;
    prm.angle     = settings()[QLatin1String("angle")].toDouble();
    prm.antiAlias = settings()[QLatin1String("antiAlias")].toBool();
    prm.autoCrop  = settings()[QLatin1String("autoCrop")].toInt();
    d->frSettings->setSettings(prm);
}

void Rotate::slotSettingsChanged()
{
    d->label->setEnabled(!d->useExif->isChecked());
    d->comboBox->setEnabled(!d->useExif->isChecked());
    d->frSettings->setEnabled(d->comboBox->isEnabled() && d->comboBox->currentIndex() == d->CUSTOM_ANGLE);

    BatchToolSettings settings;
    FreeRotationContainer currentPrm = d->frSettings->settings();

    settings.insert(QLatin1String("useExif"),   d->useExif->isChecked());
    settings.insert(QLatin1String("rotation"),  d->comboBox->currentIndex());
    settings.insert(QLatin1String("angle"),     currentPrm.angle);
    settings.insert(QLatin1String("antiAlias"), currentPrm.antiAlias);
    settings.insert(QLatin1String("autoCrop"),  currentPrm.autoCrop);

    BatchTool::slotSettingsChanged(settings);
}

bool Rotate::toolOperations()
{
    FreeRotationContainer prm;
    bool useExif  = settings()[QLatin1String("useExif")].toBool();
    int rotation  = settings()[QLatin1String("rotation")].toInt();
    prm.angle     = settings()[QLatin1String("angle")].toDouble();
    prm.antiAlias = settings()[QLatin1String("antiAlias")].toBool();
    prm.autoCrop  = settings()[QLatin1String("autoCrop")].toInt();

    // JPEG image : lossless method if non-custom rotation angle.

    if (JPEGUtils::isJpegImage(inputUrl().toLocalFile()) && image().isNull())
    {
        JPEGUtils::JpegRotator rotator(inputUrl().toLocalFile());
        rotator.setDestinationFile(outputUrl().toLocalFile());

        if (useExif)
        {
            return rotator.autoExifTransform();
        }
        else
        {
            switch (rotation)
            {
                case DImg::ROT90:
                    return rotator.exifTransform(MetaEngineRotation::Rotate90);
                    break;

                case DImg::ROT180:
                    return rotator.exifTransform(MetaEngineRotation::Rotate180);
                    break;

                case DImg::ROT270:
                    return rotator.exifTransform(MetaEngineRotation::Rotate270);
                    break;

                default:
                    // there is no lossless method to turn JPEG image with a custom angle.
                    // fall through
                    break;
            }
        }
    }

    // Non-JPEG image: DImg

    if (!loadToDImg())
    {
        return false;
    }

    if (useExif)
    {
        // Exif rotation is currently not recorded to image history
        image().rotateAndFlip(LoadSaveThread::exifOrientation(image(), inputUrl().toLocalFile()));
    }
    else
    {
        DImgBuiltinFilter filter;
        switch (rotation)
        {
            case DImg::ROT90:
            {
                DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate90);
                applyFilter(&filter);
                break;
            }
            case DImg::ROT180:
            {
                DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate180);
                applyFilter(&filter);
                break;
            }
            case DImg::ROT270:
            {
                DImgBuiltinFilter filter(DImgBuiltinFilter::Rotate270);
                applyFilter(&filter);
                break;
            }
            default:      // Custom value
            {
                FreeRotationFilter fr(&image(), 0L, prm);
                applyFilterChangedProperties(&fr);
                break;
            }
        }
    }

    return (savefromDImg());
}

}  // namespace Digikam
