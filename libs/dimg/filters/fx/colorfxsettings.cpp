/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-11-08
 * Description : Color effects settings view.
 *
 * Copyright (C) 2012 by Alexander Dymo <adymo at kdevelop dot org>
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

#include "colorfxsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <kdebug.h>
#include <kdialog.h>

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

namespace Digikam
{

class ColorFXSettings::Private
{
public:

    Private() :
        effectTypeLabel(0),
        levelLabel(0),
        iterationLabel(0),
        effectType(0),
        levelInput(0),
        iterationInput(0)
    {}

    static const QString configEffectTypeEntry;
    static const QString configLevelAdjustmentEntry;
    static const QString configIterationAdjustmentEntry;

    QLabel*              effectTypeLabel;
    QLabel*              levelLabel;
    QLabel*              iterationLabel;

    RComboBox*           effectType;

    RIntNumInput*        levelInput;
    RIntNumInput*        iterationInput;
};

const QString ColorFXSettings::Private::configEffectTypeEntry("EffectType");
const QString ColorFXSettings::Private::configLevelAdjustmentEntry("LevelAdjustment");
const QString ColorFXSettings::Private::configIterationAdjustmentEntry("IterationAdjustment");


// --------------------------------------------------------

ColorFXSettings::ColorFXSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* grid = new QGridLayout(parent);

    d->effectTypeLabel = new QLabel(i18n("Type:"));
    d->effectType      = new RComboBox();
    d->effectType->addItem(i18n("Solarize"));
    d->effectType->addItem(i18n("Vivid"));
    d->effectType->addItem(i18n("Neon"));
    d->effectType->addItem(i18n("Find Edges"));
    d->effectType->setDefaultIndex(ColorFXFilter::Solarize);
    d->effectType->setWhatsThis(i18n("<p>Select the effect type to apply to the image here.</p>"
                                     "<p><b>Solarize</b>: simulates solarization of photograph.</p>"
                                     "<p><b>Vivid</b>: simulates the Velvia(tm) slide film colors.</p>"
                                     "<p><b>Neon</b>: coloring the edges in a photograph to "
                                     "reproduce a fluorescent light effect.</p>"
                                     "<p><b>Find Edges</b>: detects the edges in a photograph "
                                     "and their strength.</p>"));

    d->levelLabel = new QLabel(i18nc("level of the effect", "Level:"));
    d->levelInput = new RIntNumInput();
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setDefaultValue(0);
    d->levelInput->setWhatsThis( i18n("Set here the level of the effect."));

    d->iterationLabel = new QLabel(i18n("Iteration:"));
    d->iterationInput = new RIntNumInput();
    d->iterationInput->setRange(0, 100, 1);
    d->iterationInput->setSliderEnabled(true);
    d->iterationInput->setDefaultValue(0);
    d->iterationInput->setWhatsThis( i18n("This value controls the number of iterations "
                                          "to use with the Neon and Find Edges effects."));


    grid->addWidget(d->effectTypeLabel, 0, 0, 1, 5);
    grid->addWidget(d->effectType,      1, 0, 1, 5);
    grid->addWidget(d->levelLabel,      2, 0, 1, 5);
    grid->addWidget(d->levelInput,      3, 0, 1, 5);
    grid->addWidget(d->iterationLabel,  4, 0, 1, 5);
    grid->addWidget(d->iterationInput,  5, 0, 1, 5);
    grid->setRowStretch(6, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->levelInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalLevelOrIterationChanged()));

    connect(d->iterationInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalLevelOrIterationChanged()));

    connect(d->effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));
}

ColorFXSettings::~ColorFXSettings()
{
    delete d;
}

void ColorFXSettings::slotEffectTypeChanged(int type)
{
    d->levelInput->setEnabled(true);
    d->levelLabel->setEnabled(true);

    d->levelInput->blockSignals(true);
    d->iterationInput->blockSignals(true);
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setValue(25);

    switch (type)
    {
        case ColorFXFilter::Solarize:
            d->levelInput->setRange(0, 100, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(0);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Vivid:
            d->levelInput->setRange(0, 50, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(5);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Neon:
        case ColorFXFilter::FindEdges:
            d->levelInput->setRange(0, 5, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(3);
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            d->iterationInput->setRange(0, 5, 1);
            d->iterationInput->setSliderEnabled(true);
            d->iterationInput->setValue(2);
            break;
    }

    d->levelInput->blockSignals(false);
    d->iterationInput->blockSignals(false);

    emit signalSettingsChanged();
}


ColorFXContainer ColorFXSettings::settings() const
{
    ColorFXContainer prm;
    prm.colorFXType = d->effectType->currentIndex();
    prm.level       = d->levelInput->value();
    prm.iterations  = d->iterationInput->value();
    return prm;
}

void ColorFXSettings::setSettings(const ColorFXContainer& settings)
{
    blockSignals(true);

    d->effectType->setCurrentIndex(settings.colorFXType);
    slotEffectTypeChanged(settings.colorFXType);

    d->levelInput->setValue(settings.level);
    d->iterationInput->setValue(settings.iterations);

    blockSignals(false);
}

void ColorFXSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

ColorFXContainer ColorFXSettings::defaultSettings() const
{
    return ColorFXContainer();
}

void ColorFXSettings::readSettings(KConfigGroup& group)
{
    ColorFXContainer prm;
    ColorFXContainer defaultPrm = defaultSettings();

    d->effectType->setCurrentIndex(group.readEntry(d->configEffectTypeEntry,       d->effectType->defaultIndex()));
    d->levelInput->setValue(group.readEntry(d->configLevelAdjustmentEntry,         d->levelInput->defaultValue()));
    d->iterationInput->setValue(group.readEntry(d->configIterationAdjustmentEntry, d->iterationInput->defaultValue()));

    setSettings(prm);
}

void ColorFXSettings::writeSettings(KConfigGroup& group)
{
    ColorFXContainer prm = settings();

    group.writeEntry(d->configEffectTypeEntry,          d->effectType->currentIndex());
    group.writeEntry(d->configLevelAdjustmentEntry,     d->levelInput->value());
    group.writeEntry(d->configIterationAdjustmentEntry, d->iterationInput->value());
}

void ColorFXSettings::enable()
{
    d->effectTypeLabel->setEnabled(true);
    d->effectType->setEnabled(true);
    d->levelInput->setEnabled(true);
    d->levelLabel->setEnabled(true);
    d->iterationInput->setEnabled(true);
    d->iterationLabel->setEnabled(true);

    switch (d->effectType->currentIndex())
    {
        case ColorFXFilter::Solarize:
        case ColorFXFilter::Vivid:
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Neon:
        case ColorFXFilter::FindEdges:
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            break;
    }
}

void ColorFXSettings::disable()
{
    d->effectTypeLabel->setEnabled(false);
    d->effectType->setEnabled(false);
    d->levelInput->setEnabled(false);
    d->levelLabel->setEnabled(false);
    d->iterationInput->setEnabled(false);
    d->iterationLabel->setEnabled(false);
}

}  // namespace Digikam
