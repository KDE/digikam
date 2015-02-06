/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-17
 * Description : resize image batch tool.
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

#include "resize.moc"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QSize>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <klocale.h>
#include <knuminput.h>
#include <kvbox.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace Digikam
{

class Resize::Private
{
public:

    enum WidthPreset
    {
        Tiny = 0,
        Small,
        Medium,
        Big,
        Large,
        Huge
    };

public:

    Private() :
        labelPreset(0),
        useCustom(0),
        customLength(0),
        comboBox(0),
        changeSettings(true)
    {
    }

    int presetLengthValue(WidthPreset preset);

public:

    QLabel*       labelPreset;

    QCheckBox*    useCustom;

    KIntNumInput* customLength;

    KComboBox*    comboBox;

    bool          changeSettings;         
};

int Resize::Private::presetLengthValue(WidthPreset preset)
{
    int length;

    switch (preset)
    {
        case Private::Tiny:
            length = 480;
            break;

        case Private::Small:
            length = 640;
            break;

        case Private::Medium:
            length = 800;
            break;

        case Private::Big:
            length = 1024;
            break;

        case Private::Large:
            length = 1280;
            break;

        default:   // Private::Huge
            length = 1600;
            break;
    }

    return length;
}

// ------------------------------------------------------------------------------

Resize::Resize(QObject* const parent)
    : BatchTool("Resize", TransformTool, parent),
      d(new Private)
{
    setToolTitle(i18n("Resize"));
    setToolDescription(i18n("Resize images with a customized length."));
    setToolIconName("transform-scale");
}

Resize::~Resize()
{
    delete d;
}

void Resize::registerSettingsWidget()
{
    KVBox* const vbox   = new KVBox;
    d->labelPreset      = new QLabel(i18n("Preset Length:"), vbox);
    d->comboBox         = new KComboBox(vbox);
    d->comboBox->insertItem(Private::Tiny,   i18np("Tiny (1 pixel)",   "Tiny (%1 pixels)",   d->presetLengthValue(Private::Tiny)));
    d->comboBox->insertItem(Private::Small,  i18np("Small (1 pixel)",  "Small (%1 pixels)",  d->presetLengthValue(Private::Small)));
    d->comboBox->insertItem(Private::Medium, i18np("Medium (1 pixel)", "Medium (%1 pixels)", d->presetLengthValue(Private::Medium)));
    d->comboBox->insertItem(Private::Big,    i18np("Big (1 pixel)",    "Big (%1 pixels)",    d->presetLengthValue(Private::Big)));
    d->comboBox->insertItem(Private::Large,  i18np("Large (1 pixel)",  "Large (%1 pixels)",  d->presetLengthValue(Private::Large)));
    d->comboBox->insertItem(Private::Huge,   i18np("Huge (1 pixel)",   "Huge (%1 pixels)",   d->presetLengthValue(Private::Huge)));

    d->useCustom        = new QCheckBox(i18n("Use Custom Length"), vbox);
    d->customLength     = new KIntNumInput(vbox);
    d->customLength->setRange(10, 10000);
    d->customLength->setSliderEnabled(true);

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget    = vbox;

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->customLength, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useCustom, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Resize::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("UseCustom",    false);
    settings.insert("LengthCustom", 1024);
    settings.insert("LengthPreset", Private::Medium);
    return settings;
}

void Resize::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->comboBox->setCurrentIndex(settings()["LengthPreset"].toInt());
    d->useCustom->setChecked(settings()["UseCustom"].toBool());
    d->customLength->setValue(settings()["LengthCustom"].toInt());
    d->changeSettings = true;
}

void Resize::slotSettingsChanged()
{
    d->customLength->setEnabled(d->useCustom->isChecked());
    d->labelPreset->setEnabled(!d->useCustom->isChecked());
    d->comboBox->setEnabled(!d->useCustom->isChecked());

    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert("LengthPreset", d->comboBox->currentIndex());
        settings.insert("UseCustom",    d->useCustom->isChecked());
        settings.insert("LengthCustom", d->customLength->value());
        BatchTool::slotSettingsChanged(settings);
    }
}

bool Resize::toolOperations()
{
    bool useCustom              = settings()["UseCustom"].toBool();
    Private::WidthPreset preset = (Private::WidthPreset)(settings()["LengthPreset"].toInt());
    int length                  = settings()["LengthCustom"].toInt();

    if (!useCustom)
    {
        length = d->presetLengthValue(preset);
    }

    if (!loadToDImg())
    {
        return false;
    }

    QSize newSize(image().size());
    newSize.scale(QSize(length, length), Qt::KeepAspectRatio);

    if (!newSize.isValid())
    {
        return false;
    }

    DImgBuiltinFilter filter(DImgBuiltinFilter::Resize, newSize);
    applyFilter(&filter);

    return (savefromDImg());
}

}  // namespace Digikam
