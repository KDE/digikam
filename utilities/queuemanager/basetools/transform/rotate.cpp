/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : rotate image batch tool.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rotate.moc"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kvbox.h>
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "freerotationfilter.h"
#include "freerotationsettings.h"

using namespace KDcrawIface;

namespace Digikam
{

class Rotate::RotatePriv
{
public:

    RotatePriv() :
        CUSTOM_ANGLE(DImg::ROT270+1),
        label(0),
        useExif(0),
        comboBox(0),
        frSettings(0)
    {}

    const int             CUSTOM_ANGLE;

    QLabel*               label;

    QCheckBox*            useExif;

    RComboBox*            comboBox;

    FreeRotationSettings* frSettings;
};

Rotate::Rotate(QObject* parent)
    : BatchTool("Rotate", TransformTool, parent),
      d(new RotatePriv)
{
    setToolTitle(i18n("Rotate"));
    setToolDescription(i18n("Rotate images."));
    setToolIcon(KIcon(SmallIcon("object-rotate-right")));

    KVBox* vbox  = new KVBox;
    d->useExif   = new QCheckBox(i18n("Use Exif Orientation"), vbox);

    d->label     = new QLabel(vbox);
    d->comboBox  = new RComboBox(vbox);
    d->comboBox->insertItem(DImg::ROT90,     i18n("90 degrees"));
    d->comboBox->insertItem(DImg::ROT180,    i18n("180 degrees"));
    d->comboBox->insertItem(DImg::ROT270,    i18n("270 degrees"));
    d->comboBox->insertItem(d->CUSTOM_ANGLE, i18n("Custom"));
    d->comboBox->setDefaultIndex(DImg::ROT90);
    d->label->setText(i18n("Angle:"));

    d->frSettings = new FreeRotationSettings(vbox);

    QLabel* space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    setNeedResetExifOrientation(true);

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useExif, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->frSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    slotSettingsChanged();
}

Rotate::~Rotate()
{
    delete d;
}

BatchToolSettings Rotate::defaultSettings()
{
    BatchToolSettings settings;
    FreeRotationContainer defaultPrm = d->frSettings->defaultSettings();

    settings.insert("useExif",   true);
    settings.insert("rotation",  d->comboBox->defaultIndex());
    settings.insert("angle",     defaultPrm.angle);
    settings.insert("antiAlias", defaultPrm.antiAlias);
    settings.insert("autoCrop",  defaultPrm.autoCrop);
    return settings;
}

void Rotate::slotAssignSettings2Widget()
{
    d->useExif->setChecked(settings()["useExif"].toBool());
    d->comboBox->setCurrentIndex(settings()["rotation"].toInt());
    FreeRotationContainer prm;
    prm.angle     = settings()["angle"].toDouble();
    prm.antiAlias = settings()["antiAlias"].toBool();
    prm.autoCrop  = settings()["autoCrop"].toInt();
    d->frSettings->setSettings(prm);
}

void Rotate::slotSettingsChanged()
{
    d->label->setEnabled(!d->useExif->isChecked());
    d->comboBox->setEnabled(!d->useExif->isChecked());
    d->frSettings->setEnabled(d->comboBox->isEnabled() && d->comboBox->currentIndex() == d->CUSTOM_ANGLE);

    BatchToolSettings settings;
    FreeRotationContainer currentPrm = d->frSettings->settings();

    settings.insert("useExif",   d->useExif->isChecked());
    settings.insert("rotation",  d->comboBox->currentIndex());
    settings.insert("angle",     currentPrm.angle);
    settings.insert("antiAlias", currentPrm.antiAlias);
    settings.insert("autoCrop",  currentPrm.autoCrop);

    BatchTool::slotSettingsChanged(settings);
}

bool Rotate::toolOperations()
{
    FreeRotationContainer prm;
    bool useExif  = settings()["useExif"].toBool();
    int rotation  = settings()["rotation"].toInt();
    prm.angle     = settings()["angle"].toDouble();
    prm.antiAlias = settings()["antiAlias"].toBool();
    prm.autoCrop  = settings()["autoCrop"].toInt();

    // JPEG image : lossless method if non-custom rotation angle.

    if (isJpegImage(inputUrl().toLocalFile()) && image().isNull())
    {
        JpegRotator rotator(inputUrl().toLocalFile());
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
                    return rotator.exifTransform(KExiv2Iface::RotationMatrix::Rotate90);
                    break;
                case DImg::ROT180:
                    return rotator.exifTransform(KExiv2Iface::RotationMatrix::Rotate180);
                    break;
                case DImg::ROT270:
                    return rotator.exifTransform(KExiv2Iface::RotationMatrix::Rotate270);
                    break;
                default:

                    // there is no loss less methode to turn JPEG image with a custom angle.
                    if (!loadToDImg())
                    {
                        return false;
                    }

                    FreeRotationFilter fr(&image(), 0L, prm);
                    fr.startFilterDirectly();
                    DImg trg = fr.getTargetImage();
                    image().putImageData(trg.width(), trg.height(), trg.sixteenBit(), trg.hasAlpha(), trg.bits());
                    return (savefromDImg());
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
        DMetadata meta(inputUrl().toLocalFile());
        image().rotateAndFlip(meta.getImageOrientation());
    }
    else
    {
        switch (rotation)
        {
            case DImg::ROT90:
            case DImg::ROT180:
            case DImg::ROT270:
                image().rotate((DImg::ANGLE)rotation);
                break;
            default:      // Custom value
                FreeRotationFilter fr(&image(), 0L, prm);
                fr.startFilterDirectly();
                DImg trg = fr.getTargetImage();
                image().putImageData(trg.width(), trg.height(), trg.sixteenBit(), trg.hasAlpha(), trg.bits());
                break;
        }
    }

    return (savefromDImg());
}

}  // namespace Digikam
