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

#include "colorfxsettings.h"

// Qt includes

#include <QStandardPaths>
#include <QStackedWidget>
#include <QApplication>
#include <QDirIterator>
#include <QGridLayout>
#include <QStringList>
#include <QLabel>
#include <QStyle>
#include <QIcon>

// Local includes

#include "dexpanderbox.h"
#include "previewlist.h"
#include "imageiface.h"
#include "dcombobox.h"
#include "dnuminput.h"

namespace Digikam
{

class ColorFXSettings::Private
{
public:

    Private() :
        stack(0),
        effectType(0),
        levelInput(0),
        iterationInput(0),
        intensityInput(0),
        iterationLabel(0),
        correctionTools(0)
    {}

    static const QString configEffectTypeEntry;
    static const QString configLevelAdjustmentEntry;
    static const QString configIterationAdjustmentEntry;
    static const QString configLut3DFilterEntry;
    static const QString configLut3DIntensityEntry;

    QStackedWidget* stack;

    DComboBox*      effectType;

    DIntNumInput*   levelInput;
    DIntNumInput*   iterationInput;
    DIntNumInput*   intensityInput;

    QLabel*         iterationLabel;

    PreviewList*    correctionTools;

    QStringList     luts;
};

const QString ColorFXSettings::Private::configEffectTypeEntry(QLatin1String("EffectType"));
const QString ColorFXSettings::Private::configLevelAdjustmentEntry(QLatin1String("LevelAdjustment"));
const QString ColorFXSettings::Private::configIterationAdjustmentEntry(QLatin1String("IterationAdjustment"));
const QString ColorFXSettings::Private::configLut3DFilterEntry(QLatin1String("Lut3D Color Correction Filter"));
const QString ColorFXSettings::Private::configLut3DIntensityEntry(QLatin1String("Lut3D Color Correction Intensity"));

// --------------------------------------------------------

ColorFXSettings::ColorFXSettings(QWidget* const parent, bool useGenericImg)
    : QWidget(parent),
      d(new Private)
{
    DImg thumbImage;

    findLuts();

    if (useGenericImg)
    {
        thumbImage = DImg(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(128).toImage());
    }
    else
    {
        ImageIface iface;
        thumbImage = iface.original()->smoothScale(128, 128, Qt::KeepAspectRatio);
    }

    // -------------------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid       = new QGridLayout(parent);

    QLabel* const effectTypeLabel = new QLabel(i18n("Type:"), parent);
    d->effectType                 = new DComboBox(parent);
    d->effectType->addItem(i18n("Solarize"));
    d->effectType->addItem(i18n("Vivid"));
    d->effectType->addItem(i18n("Neon"));
    d->effectType->addItem(i18n("Find Edges"));
    d->effectType->addItem(i18n("Lut3D"));
    d->effectType->setDefaultIndex(ColorFXFilter::Solarize);
    d->effectType->setWhatsThis(i18n("<p>Select the effect type to apply to the image here.</p>"
                                     "<p><b>Solarize</b>: simulates solarization of photograph.</p>"
                                     "<p><b>Vivid</b>: simulates the Velvia(tm) slide film colors.</p>"
                                     "<p><b>Neon</b>: coloring the edges in a photograph to "
                                     "reproduce a fluorescent light effect.</p>"
                                     "<p><b>Find Edges</b>: detects the edges in a photograph "
                                     "and their strength.</p>"
                                     "<p><b>Lut3D</b>: coloring images with Lut3D filters</p>"));

    d->stack                      = new QStackedWidget(parent);

    grid->addWidget(effectTypeLabel,                         0, 0, 1, 1);
    grid->addWidget(d->effectType,                           1, 0, 1, 1);
    grid->addWidget(new DLineWidget(Qt::Horizontal, parent), 2, 0, 1, 1);
    grid->addWidget(d->stack,                                3, 0, 1, 1);
    grid->setRowStretch(3, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    QWidget* const solarizeSettings = new QWidget(d->stack);
    QGridLayout* const grid1        = new QGridLayout(solarizeSettings);

    QLabel* const levelLabel        = new QLabel(i18nc("level of the effect", "Level:"), solarizeSettings);
    d->levelInput                   = new DIntNumInput(solarizeSettings);
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setDefaultValue(3);
    d->levelInput->setWhatsThis( i18n("Set here the level of the effect."));

    d->iterationLabel               = new QLabel(i18n("Iteration:"), solarizeSettings);
    d->iterationInput               = new DIntNumInput(solarizeSettings);
    d->iterationInput->setRange(0, 100, 1);
    d->iterationInput->setDefaultValue(2);
    d->iterationInput->setWhatsThis(i18n("This value controls the number of iterations "
                                         "to use with the Neon and Find Edges effects."));

    grid1->addWidget(levelLabel,        0, 0, 1, 1);
    grid1->addWidget(d->levelInput,     1, 0, 1, 1);
    grid1->addWidget(d->iterationLabel, 2, 0, 1, 1);
    grid1->addWidget(d->iterationInput, 3, 0, 1, 1);
    grid1->setRowStretch(4, 10);
    grid1->setContentsMargins(QMargins());
    grid1->setSpacing(0);

    d->stack->insertWidget(0, solarizeSettings);

    // -------------------------------------------------------------

    QWidget* const lut3DSettings = new QWidget(d->stack);
    QGridLayout* const grid2     = new QGridLayout(lut3DSettings);

    d->correctionTools           = new PreviewList(lut3DSettings);

    for (int idx = 0; idx < d->luts.count(); idx++)
    {
        ColorFXContainer prm;
        prm.colorFXType = ColorFXFilter::Lut3D;
        prm.path        = d->luts[idx];

        QFileInfo fi(prm.path);

        d->correctionTools->addItem(new ColorFXFilter(&thumbImage, lut3DSettings, prm),
                                                      translateLuts(fi.baseName()), idx);
    }

    QLabel* const intensityLabel = new QLabel(i18n("Intensity:"), lut3DSettings);
    d->intensityInput            = new DIntNumInput(lut3DSettings);
    d->intensityInput->setRange(1, 100, 1);
    d->intensityInput->setDefaultValue(100);
    d->intensityInput->setWhatsThis(i18n("Set here the intensity of the filter."));

    grid2->addWidget(d->correctionTools, 0, 0, 1, 1);
    grid2->addWidget(intensityLabel,     1, 0, 1, 1);
    grid2->addWidget(d->intensityInput,  2, 0, 1, 1);
    grid2->setRowStretch(0, 10);
    grid2->setContentsMargins(QMargins());
    grid2->setSpacing(0);

    d->stack->insertWidget(1, lut3DSettings);

    // -------------------------------------------------------------

    connect(d->effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));

    connect(d->levelInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->iterationInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->correctionTools, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->intensityInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

ColorFXSettings::~ColorFXSettings()
{
    delete d;
}

void ColorFXSettings::startPreviewFilters()
{
    d->correctionTools->startFilters();
}

void ColorFXSettings::slotEffectTypeChanged(int type)
{
    d->iterationInput->blockSignals(true);
    d->levelInput->blockSignals(true);

    int w = (type == ColorFXFilter::Lut3D ? 1 : 0);
    d->stack->setCurrentWidget(d->stack->widget(w));

    switch (type)
    {
        case ColorFXFilter::Solarize:
            d->levelInput->setRange(0, 100, 1);
            d->levelInput->setValue(20);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Vivid:
            d->levelInput->setRange(0, 50, 1);
            d->levelInput->setValue(5);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Neon:
        case ColorFXFilter::FindEdges:
            d->levelInput->setRange(0, 5, 1);
            d->levelInput->setValue(3);
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            d->iterationInput->setRange(0, 5, 1);
            d->iterationInput->setValue(2);
            break;
    }

    d->iterationInput->blockSignals(false);
    d->levelInput->blockSignals(false);

    emit signalSettingsChanged();
}

ColorFXContainer ColorFXSettings::settings() const
{
    ColorFXContainer prm;

    prm.colorFXType = d->effectType->currentIndex();
    prm.level       = d->levelInput->value();
    prm.iterations  = d->iterationInput->value();
    prm.intensity   = d->intensityInput->value();
    prm.path        = d->luts.value(d->correctionTools->currentId());

    return prm;
}

void ColorFXSettings::setSettings(const ColorFXContainer& settings)
{
    blockSignals(true);

    d->effectType->setCurrentIndex(settings.colorFXType);
    slotEffectTypeChanged(settings.colorFXType);

    d->levelInput->setValue(settings.level);
    d->iterationInput->setValue(settings.iterations);

    int filterId = d->luts.indexOf(settings.path);

    if (filterId == -1)
    {
        filterId = 0;
    }

    d->intensityInput->setValue(settings.intensity);
    d->correctionTools->setCurrentId(filterId);

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

    prm.colorFXType = group.readEntry(d->configEffectTypeEntry,          defaultPrm.colorFXType);
    prm.level       = group.readEntry(d->configLevelAdjustmentEntry,     defaultPrm.level);
    prm.iterations  = group.readEntry(d->configIterationAdjustmentEntry, defaultPrm.iterations);
    prm.intensity   = group.readEntry(d->configLut3DIntensityEntry,      defaultPrm.intensity);
    prm.path        = group.readEntry(d->configLut3DFilterEntry,         defaultPrm.path);

    setSettings(prm);
}

void ColorFXSettings::writeSettings(KConfigGroup& group)
{
    ColorFXContainer prm = settings();

    group.writeEntry(d->configEffectTypeEntry,          prm.colorFXType);
    group.writeEntry(d->configLevelAdjustmentEntry,     prm.level);
    group.writeEntry(d->configIterationAdjustmentEntry, prm.iterations);
    group.writeEntry(d->configLut3DIntensityEntry,      prm.intensity);
    group.writeEntry(d->configLut3DFilterEntry,         prm.path);
}

void ColorFXSettings::findLuts()
{
    QStringList dirpaths;
    dirpaths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                          QLatin1String("digikam/data/lut3d"),
                                          QStandardPaths::LocateDirectory);

    foreach (const QString& dirpath, dirpaths)
    {
        QDirIterator dirIt(dirpath, QDirIterator::Subdirectories);

        while (dirIt.hasNext())
        {
            dirIt.next();

            if (QFileInfo(dirIt.filePath()).isFile())
            {
                d->luts << dirIt.filePath();
            }
        }
    }

    d->luts.sort();
}

QString ColorFXSettings::translateLuts(const QString& name) const
{
    if (name.toLower() == QLatin1String("bleach"))
    {
        return i18n("Bleach");
    }
    else if (name.toLower() == QLatin1String("blue_crush"))
    {
        return i18n("Blue Crush");
    }
    else if (name.toLower() == QLatin1String("bw_contrast"))
    {
        return i18n("BW Contrast");
    }
    else if (name.toLower() == QLatin1String("instant"))
    {
        return i18n("Instant");
    }
    else if (name.toLower() == QLatin1String("original"))
    {
        return i18n("Original");
    }
    else if (name.toLower() == QLatin1String("punch"))
    {
        return i18n("Punch");
    }
    else if (name.toLower() == QLatin1String("summer"))
    {
        return i18n("Summer");
    }
    else if (name.toLower() == QLatin1String("tokyo"))
    {
        return i18n("Tokyo");
    }
    else if (name.toLower() == QLatin1String("vintage"))
    {
        return i18n("Vintage");
    }
    else if (name.toLower() == QLatin1String("washout"))
    {
        return i18n("Washout");
    }
    else if (name.toLower() == QLatin1String("washout_color"))
    {
        return i18n("Washout Color");
    }
    else if (name.toLower() == QLatin1String("x_process"))
    {
        return i18n("X Process");
    }

    return name;
}

}  // namespace Digikam
