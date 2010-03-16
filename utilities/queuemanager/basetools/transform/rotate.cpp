/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : rotate image batch tool.
 *
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "freerotationfilter.h"

namespace Digikam
{

class RotatePriv
{
public:

    RotatePriv() :
        CUSTOMANGLE(DImg::ROT270+1),
        label(0),
        useExif(0),
        comboBox(0),
        angleInput(0)
        {}

    const int     CUSTOMANGLE;
    
    QLabel*       label;
    
    QCheckBox*    useExif;
    
    RComboBox*    comboBox;
    RIntNumInput* angleInput;
};  
  
Rotate::Rotate(QObject* parent)
      : BatchTool("Rotate", TransformTool, parent),
        d(new RotatePriv)
{
    setToolTitle(i18n("Rotate"));
    setToolDescription(i18n("A tool to rotate images."));
    setToolIcon(KIcon(SmallIcon("object-rotate-right")));

    KVBox* vbox  = new KVBox;
    d->useExif   = new QCheckBox(i18n("Use Exif Orientation"), vbox);

    d->label     = new QLabel(vbox);
    d->comboBox  = new RComboBox(vbox);
    d->comboBox->insertItem(DImg::ROT90,    i18n("90 degrees"));
    d->comboBox->insertItem(DImg::ROT180,   i18n("180 degrees"));
    d->comboBox->insertItem(DImg::ROT270,   i18n("270 degrees"));
    d->comboBox->insertItem(d->CUSTOMANGLE, i18n("Custom"));
    d->comboBox->setDefaultIndex(DImg::ROT90);    
    d->label->setText(i18n("Angle:"));

    d->angleInput = new RIntNumInput(vbox);
    d->angleInput->setRange(-180, 180, 1);
    d->angleInput->setSliderEnabled(true);
    d->angleInput->setDefaultValue(0);
    d->angleInput->setWhatsThis(i18n("An angle in degrees by which to rotate the image. "
                                     "A positive angle rotates the image clockwise; "
                                     "a negative angle rotates it counter-clockwise."));    
    
    QLabel* space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    setNeedResetExifOrientation(true);

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useExif, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->angleInput, SIGNAL(valueChanged(int)),
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
    settings.insert("UseExif",  true);
    settings.insert("Rotation", d->comboBox->defaultIndex());
    settings.insert("Angle",    d->angleInput->defaultValue());
    return settings;
}

void Rotate::slotAssignSettings2Widget()
{
    d->useExif->setChecked(settings()["UseExif"].toBool());
    d->comboBox->setCurrentIndex(settings()["Rotation"].toInt());
    d->angleInput->setValue(settings()["Angle"].toInt());
}

void Rotate::slotSettingsChanged()
{
    d->label->setEnabled(!d->useExif->isChecked());
    d->comboBox->setEnabled(!d->useExif->isChecked());
    d->angleInput->setEnabled(d->comboBox->isEnabled() && d->comboBox->currentIndex() == d->CUSTOMANGLE);
    
    BatchToolSettings settings;
    settings.insert("UseExif",  d->useExif->isChecked());
    settings.insert("Rotation", d->comboBox->currentIndex());
    settings.insert("Angle",    d->angleInput->value());
    BatchTool::slotSettingsChanged(settings);
}

bool Rotate::toolOperations()
{
    bool useExif = settings()["UseExif"].toBool();
    int rotation = settings()["Rotation"].toInt();
    int angle    = settings()["Angle"].toInt();
    
    // JPEG image : lossless method if non-custom rotation angle.

    if (isJpegImage(inputUrl().toLocalFile()) && image().isNull())
    {
        if (useExif)
        {
            if (!exifTransform(inputUrl().toLocalFile(), inputUrl().fileName(), outputUrl().toLocalFile(), Auto))
                return false;
        }
        else
        {
            switch(rotation)
            {
                case DImg::ROT90:
                    return (exifTransform(inputUrl().toLocalFile(), inputUrl().fileName(), outputUrl().toLocalFile(), Rotate90));
                    break;
                case DImg::ROT180:
                    return (exifTransform(inputUrl().toLocalFile(), inputUrl().fileName(), outputUrl().toLocalFile(), Rotate180));
                    break;
                case DImg::ROT270:
                    return (exifTransform(inputUrl().toLocalFile(), inputUrl().fileName(), outputUrl().toLocalFile(), Rotate270));
                    break;
                default:      // Custom value
                              // there is no loss less methode to turn JPEG image with a custom angle.
                    if (!loadToDImg()) return false;
                    FreeRotationContainer settings;
                    settings.angle = angle;
                    FreeRotationFilter fr(&image(), 0L, settings);
                    fr.startFilterDirectly();
                    DImg trg = fr.getTargetImage();
                    image().putImageData(trg.width(), trg.height(), trg.sixteenBit(), trg.hasAlpha(), trg.bits());
                    return (savefromDImg());
                    break;
            }
        }
    }

    // Non-JPEG image: DImg

    if (!loadToDImg()) return false;

    if (useExif)
    {
        DMetadata meta(inputUrl().toLocalFile());
        switch(meta.getImageOrientation())
        {
            case DMetadata::ORIENTATION_HFLIP:
                image().flip(DImg::HORIZONTAL);
                break;

            case DMetadata::ORIENTATION_ROT_180:
                image().rotate(DImg::ROT180);
                break;

            case DMetadata::ORIENTATION_VFLIP:
                image().flip(DImg::VERTICAL);
                break;

            case DMetadata::ORIENTATION_ROT_90_HFLIP:
                image().flip(DImg::HORIZONTAL);
                image().rotate(DImg::ROT90);
                break;

            case DMetadata::ORIENTATION_ROT_90:
                image().rotate(DImg::ROT90);
                break;

            case DMetadata::ORIENTATION_ROT_90_VFLIP:
                image().flip(DImg::VERTICAL);
                image().rotate(DImg::ROT90);
                break;

            case DMetadata::ORIENTATION_ROT_270:
                image().rotate(DImg::ROT270);
                break;

            default:
                // DMetadata::ORIENTATION_NORMAL
                // DMetadata::ORIENTATION_UNSPECIFIED
                // Nothing to do...
                break;
        }
    }
    else
    {
        switch(rotation)
        {      
            case DImg::ROT90:
                image().rotate(DImg::ROT90);
                break;
            case DImg::ROT180:
                image().rotate(DImg::ROT180);
                break;
            case DImg::ROT270:
                image().rotate(DImg::ROT270);
                break;
            default:      // Custom value
                FreeRotationContainer settings;
                settings.angle = angle;
                FreeRotationFilter fr(&image(), 0L, settings);
                fr.startFilterDirectly();
                DImg trg = fr.getTargetImage();
                image().putImageData(trg.width(), trg.height(), trg.sixteenBit(), trg.hasAlpha(), trg.bits());
                break;
        }
    }

    return (savefromDImg());
}

}  // namespace Digikam
