/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-17
 * Description : resize image batch tool.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2019 by Sambhav Dusad  <sambhavdusad24 at gmail dot com>
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

#include "resize.h"

// Qt includes

#include <QGridLayout>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QSize>
#include <QWidget>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace DigikamBqmResizePlugin
{

class Q_DECL_HIDDEN Resize::Private
{
public:

    enum WidthPreset
    {
        Tiny = 0,
        Small,
        Medium,
        Big,
        Large,
        Huge,
        FHD,
        QHD,
        Profile
    };

public:

    explicit Private()
      : labelPreset(0),
        useCustom(0),
        usePixel(0),
        usePercent(0),
        useCM(0),
        useInches(0),
        customLength(0),
        presetCBox(0),
        changeSettings(true)
    {
    }

    QPair<int, int> presetLengthValue(WidthPreset preset);

public:

    QLabel*          labelPreset;

    QWidget*         vbox;
    QCheckBox*       useCustom;
    QCheckBox*       preserveRatioBox;
    QRadioButton*    usePixel;
    QRadioButton*    usePercent;
    QRadioButton*    useCM;
    QRadioButton*    useInches;

    DIntNumInput*    customLength;

    QComboBox*       presetCBox;

    bool             changeSettings;
};

QPair<int, int> Resize::Private::presetLengthValue(WidthPreset preset)
{
    QPair<int, int> dimension;

    switch (preset)
    {
        case Private::Tiny:
            dimension.first  = 320;
            dimension.second = 200;
            break;

        case Private::Small:
            dimension.first  = 640;
            dimension.second = 480;
            break;

        case Private::Medium:
            dimension.first  = 800;
            dimension.second = 600;
            break;

        case Private::Big:
            dimension.first  = 1024;
            dimension.second = 768;
            break;

        case Private::Large:
            dimension.first  = 1280;
            dimension.second = 720;
            break;

        case Private::Huge:  // Private::Huge
            dimension.first  = 1600;
            dimension.second = 900;
            break;

        case Private::FHD:
            dimension.first  = 1920;
            dimension.second = 1080;
            break;

        case Private::QHD:
            dimension.first  = 2560;
            dimension.second = 1440;
            break;

        default:
            dimension.first  = 1080;
            dimension.second = 1080;
            break;
    }

    return dimension;
}

// ------------------------------------------------------------------------------

Resize::Resize(QObject* const parent)
    : BatchTool(QLatin1String("Resize"), TransformTool, parent),
      d(new Private)
{
}

Resize::~Resize()
{
    delete d;
}

void Resize::registerSettingsWidget()
{
    d->vbox                 = new QWidget();
    QGridLayout* const grid = new QGridLayout(d->vbox);
    d->labelPreset          = new QLabel(i18n("Preset Resolutions:"), d->vbox);
    d->presetCBox           = new QComboBox(d->vbox);
    d->presetCBox->insertItem(Private::Tiny,    i18np("Tiny (1 pixel)",    "Tiny (%1*%2 pixels)",
                                                d->presetLengthValue(Private::Tiny).first,
                                                d->presetLengthValue(Private::Tiny).second));
    d->presetCBox->insertItem(Private::Small,   i18np("Small (1 pixel)",   "Small (%1*%2 pixels)",
                                                d->presetLengthValue(Private::Small).first,
                                                d->presetLengthValue(Private::Small).second));
    d->presetCBox->insertItem(Private::Medium,  i18np("Medium (1 pixel)",  "Medium (%1*%2 pixels)",
                                                d->presetLengthValue(Private::Medium).first,
                                                d->presetLengthValue(Private::Medium).second));
    d->presetCBox->insertItem(Private::Big,     i18np("Big (1 pixel)",     "Big (%1*%2 pixels)",
                                                d->presetLengthValue(Private::Big).first,
                                                d->presetLengthValue(Private::Big).second));
    d->presetCBox->insertItem(Private::Large,   i18np("Large (1 pixel)",   "Large (%1*%2 pixels)",
                                                d->presetLengthValue(Private::Large).first,
                                                d->presetLengthValue(Private::Large).second));
    d->presetCBox->insertItem(Private::Huge,    i18np("Huge (1 pixel)",    "Huge (%1*%2 pixels)",
                                                d->presetLengthValue(Private::Huge).first,
                                                d->presetLengthValue(Private::Huge).second));
    d->presetCBox->insertItem(Private::FHD,     i18np("FHD (1 pixel)",     "FHD (%1*%2 pixels)",
                                                d->presetLengthValue(Private::FHD).first,
                                                d->presetLengthValue(Private::FHD).second));
    d->presetCBox->insertItem(Private::QHD,     i18np("QHD (1 pixel)",     "QHD (%1*%2 pixels)",
                                                d->presetLengthValue(Private::QHD).first,
                                                d->presetLengthValue(Private::QHD).second));
    d->presetCBox->insertItem(Private::Profile, i18np("Profile (1 pixel)", "Profile (%1*%2 pixels)",
                                                d->presetLengthValue(Private::Profile).first,
                                                d->presetLengthValue(Private::Profile).second));

    d->useCustom            = new QCheckBox(i18n("Use Custom Length"),     d->vbox);
    d->preserveRatioBox     = new QCheckBox(i18n("Maintain aspect ratio"), d->vbox);
    d->usePixel             = new QRadioButton(i18n("Use Pixels"),         d->vbox);
    d->usePercent           = new QRadioButton(i18n("Use Percentage"),     d->vbox);
    d->useCM                = new QRadioButton(i18n("Use Centimetres"),    d->vbox);
    d->useInches            = new QRadioButton(i18n("Use Inches"),         d->vbox);
    QLabel* const label1    = new QLabel(i18n("Width:"),                   d->vbox);
    d->customLength         = new DIntNumInput(d->vbox);
    d->customLength->setSuffix(i18n(" Pixels"));
    d->customLength->setRange(10, 10000, 1);
    d->customLength->setDefaultValue(1024);

    grid->addWidget(d->labelPreset,      0, 0, 1, 7);
    grid->addWidget(d->presetCBox,       1, 0, 1, 6);
    grid->addWidget(d->useCustom,        2, 0, 1, 7);
    grid->addWidget(d->usePixel,         3, 0, 1, 7);
    grid->addWidget(d->usePercent,       4, 0, 1, 7);
    grid->addWidget(d->useCM,            5, 0, 1, 7);
    grid->addWidget(d->useInches,        6, 0, 1, 7);
    grid->addWidget(d->preserveRatioBox, 7, 0, 1, 7);
    grid->addWidget(label1,              8, 0, 1, 1);
    grid->addWidget(d->customLength,     8, 1, 1, 7);
    grid->setRowStretch(9, 10);

    m_settingsWidget        = d->vbox;

    connect(d->presetCBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->customLength, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useCustom, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->usePixel, SIGNAL(toggled(bool)),
            this, SLOT(slotUnitsChanged()));

    connect(d->usePercent, SIGNAL(toggled(bool)),
            this, SLOT(slotUnitsChanged()));

    connect(d->useCM, SIGNAL(toggled(bool)),
            this, SLOT(slotUnitsChanged()));

    connect(d->useInches, SIGNAL(toggled(bool)),
            this, SLOT(slotUnitsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Resize::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("UseCustom"),        false);
    settings.insert(QLatin1String("PreserveRatioBox"), false);
    settings.insert(QLatin1String("UsePixel"),         false);
    settings.insert(QLatin1String("UsePercent"),       false);
    settings.insert(QLatin1String("UseCM"),            false);
    settings.insert(QLatin1String("UseInches"),        false);
    settings.insert(QLatin1String("LengthCustom"),     1024);
    settings.insert(QLatin1String("LengthPreset"),     Private::Medium);
    return settings;
}

void Resize::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->useCustom->setChecked(settings()[QLatin1String("UseCustom")].toBool());
    d->preserveRatioBox->setChecked(true);
    d->usePixel->setChecked(true);
    d->usePercent->setChecked(settings()[QLatin1String("UsePercent")].toBool());
    d->useCM->setChecked(settings()[QLatin1String("UseCM")].toBool());
    d->useInches->setChecked(settings()[QLatin1String("UseInches")].toBool());
    d->customLength->setValue(settings()[QLatin1String("LengthCustom")].toInt());
    d->presetCBox->setCurrentIndex(settings()[QLatin1String("LengthPreset")].toInt());
    d->changeSettings = true;
}

void Resize::slotSettingsChanged()
{
    d->labelPreset->setEnabled(!d->useCustom->isChecked());
    d->customLength->setEnabled(d->useCustom->isChecked());
    d->preserveRatioBox->setEnabled(d->useCustom->isChecked());
    d->presetCBox->setEnabled(!d->useCustom->isChecked());
    d->usePixel->setEnabled(d->useCustom->isChecked());
    d->usePercent->setEnabled(d->useCustom->isChecked());
    d->useCM->setEnabled(d->useCustom->isChecked());
    d->useInches->setEnabled(d->useCustom->isChecked());

    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("UseCustom"),        d->useCustom->isChecked());
        settings.insert(QLatin1String("PreserveRatioBox"), d->preserveRatioBox->isChecked());
        settings.insert(QLatin1String("UsePixel"),         d->usePixel->isChecked());
        settings.insert(QLatin1String("UsePercent"),       d->usePercent->isChecked());
        settings.insert(QLatin1String("UseCM"),            d->useCM->isChecked());
        settings.insert(QLatin1String("UseInches"),        d->useInches->isChecked());
        settings.insert(QLatin1String("LengthCustom"),     d->customLength->value());
        settings.insert(QLatin1String("LengthPreset"),     d->presetCBox->currentIndex());
        BatchTool::slotSettingsChanged(settings);
    }
}

void Resize::slotUnitsChanged()
{
    if (d->usePercent->isChecked())
    {
        d->customLength->setSuffix(QLatin1String("%"));
        d->customLength->setRange(1, 1000, 1);
        d->customLength->setDefaultValue(100);
        d->customLength->setValue(100);
    }
    else if (d->useCM->isChecked())
    {
        d->customLength->setSuffix(QLatin1String(" cm"));
        d->customLength->setRange(1, 500, 1);
        d->customLength->setDefaultValue(100);
        d->customLength->setValue(100);
    }
    else if (d->useInches->isChecked())
    {
        d->customLength->setSuffix(QLatin1String(" Inches"));
        d->customLength->setRange(1, 200, 1);
        d->customLength->setDefaultValue(50);
        d->customLength->setValue(50);
    }
    else
    {
        d->customLength->setSuffix(i18n(" Pixels"));
        d->customLength->setRange(10, 10000, 1);
        d->customLength->setDefaultValue(1024);
        d->customLength->setValue(1024);
    }

    slotSettingsChanged();
}

bool Resize::toolOperations()
{
    bool useCustom              = settings()[QLatin1String("UseCustom")].toBool();
    bool preserveRatioBox       = settings()[QLatin1String("PreserveRatioBox")].toBool();
    bool usePercent             = settings()[QLatin1String("UsePercent")].toBool();
    bool useCM                  = settings()[QLatin1String("UseCM")].toBool();
    bool useInches              = settings()[QLatin1String("UseInches")].toBool();
    int length                  = settings()[QLatin1String("LengthCustom")].toInt();
    Private::WidthPreset preset = (Private::WidthPreset)(settings()[QLatin1String("LengthPreset")].toInt());
    int width                   = 0;
    int height                  = 0;

    if (!loadToDImg())
    {
        return false;
    }

    if (useCustom)
    {
        width  = image().width();
        height = image().height();
    }
    if (!useCustom)
    {
        QPair<int, int> dimension = d->presetLengthValue(preset);
        width                     = dimension.first;
        height                    = dimension.second;
    }
    else if (usePercent)
    {
        width = (int)(width * (double)length / 100.0);
    }
    else if (useCM)
    {
        width = (int)(length * 37.795);    //1 cm = 37.795px
    }
    else if (useInches)
    {
        width = (int)(length * 96);        //1 inch = 96px
    }

    QSize newSize(image().size());

    if (!preserveRatioBox)
    {
        newSize.scale(QSize(width, height), Qt::IgnoreAspectRatio);
    }
    else
    {
        newSize.scale(QSize(width, width), Qt::KeepAspectRatio);
    }

    if (!newSize.isValid())
    {
        return false;
    }

    DImgBuiltinFilter filter(DImgBuiltinFilter::Resize, newSize);
    applyFilter(&filter);

    return (savefromDImg());
}

} // namespace DigikamBqmResizePlugin
