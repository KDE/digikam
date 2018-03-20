/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-17
 * Description : Border settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bordersettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dnuminput.h"
#include "dcombobox.h"
#include "dcolorselector.h"
#include "digikam_debug.h"

namespace Digikam
{

class Private
{
public:

    Private() :
        preserveAspectRatio(0),
        labelBackground(0),
        labelBorderPercent(0),
        labelBorderWidth(0),
        labelForeground(0),
        firstColorButton(0),
        secondColorButton(0),
        borderType(0),
        borderPercent(0),
        borderWidth(0)
    {}

    static const QString configBorderTypeEntry;
    static const QString configBorderPercentEntry;
    static const QString configBorderWidthEntry;
    static const QString configPreserveAspectRatioEntry;
    static const QString configSolidColorEntry;
    static const QString configNiepceBorderColorEntry;
    static const QString configNiepceLineColorEntry;
    static const QString configBevelUpperLeftColorEntry;
    static const QString configBevelLowerRightColorEntry;
    static const QString configDecorativeFirstColorEntry;
    static const QString configDecorativeSecondColorEntry;

    QCheckBox*           preserveAspectRatio;

    QLabel*              labelBackground;
    QLabel*              labelBorderPercent;
    QLabel*              labelBorderWidth;
    QLabel*              labelForeground;

    QColor               bevelLowerRightColor;
    QColor               bevelUpperLeftColor;
    QColor               decorativeFirstColor;
    QColor               decorativeSecondColor;
    QColor               niepceBorderColor;
    QColor               niepceLineColor;
    QColor               solidColor;

    DColorSelector*      firstColorButton;
    DColorSelector*      secondColorButton;

    DComboBox*           borderType;
    DIntNumInput*        borderPercent;
    DIntNumInput*        borderWidth;
};

const QString Private::configBorderTypeEntry(QLatin1String("Border Type"));
const QString Private::configBorderPercentEntry(QLatin1String("Border Percent"));
const QString Private::configBorderWidthEntry(QLatin1String("Border Width"));
const QString Private::configPreserveAspectRatioEntry(QLatin1String("Preserve Aspect Ratio"));
const QString Private::configSolidColorEntry(QLatin1String("Solid Color"));
const QString Private::configNiepceBorderColorEntry(QLatin1String("Niepce Border Color"));
const QString Private::configNiepceLineColorEntry(QLatin1String("Niepce Line Color"));
const QString Private::configBevelUpperLeftColorEntry(QLatin1String("Bevel Upper Left Color"));
const QString Private::configBevelLowerRightColorEntry(QLatin1String("Bevel Lower Right Color"));
const QString Private::configDecorativeFirstColorEntry(QLatin1String("Decorative First Color"));
const QString Private::configDecorativeSecondColorEntry(QLatin1String("Decorative Second Color"));

// --------------------------------------------------------

BorderSettings::BorderSettings(QWidget* parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* grid = new QGridLayout(parent);

    QLabel* label1 = new QLabel(i18n("Type:"));
    d->borderType  = new DComboBox();
    d->borderType->addItem(i18nc("solid border type", "Solid"));
    // NOTE: Niepce is a real name. This is the first guy in the world to have built a camera.
    d->borderType->addItem(QLatin1String("Niepce"));
    d->borderType->addItem(i18nc("beveled border type", "Beveled"));
    d->borderType->addItem(i18n("Decorative Pine"));
    d->borderType->addItem(i18n("Decorative Wood"));
    d->borderType->addItem(i18n("Decorative Paper"));
    d->borderType->addItem(i18n("Decorative Parquet"));
    d->borderType->addItem(i18n("Decorative Ice"));
    d->borderType->addItem(i18n("Decorative Leaf"));
    d->borderType->addItem(i18n("Decorative Marble"));
    d->borderType->addItem(i18n("Decorative Rain"));
    d->borderType->addItem(i18n("Decorative Craters"));
    d->borderType->addItem(i18n("Decorative Dried"));
    d->borderType->addItem(i18n("Decorative Pink"));
    d->borderType->addItem(i18n("Decorative Stone"));
    d->borderType->addItem(i18n("Decorative Chalk"));
    d->borderType->addItem(i18n("Decorative Granite"));
    d->borderType->addItem(i18n("Decorative Rock"));
    d->borderType->addItem(i18n("Decorative Wall"));
    d->borderType->setDefaultIndex(BorderContainer::SolidBorder);
    d->borderType->setWhatsThis(i18n("Select the border type to add around the image here."));

    DLineWidget* line1 = new DLineWidget(Qt::Horizontal);

    // -------------------------------------------------------------------

    d->preserveAspectRatio = new QCheckBox();
    d->preserveAspectRatio->setText(i18n("Preserve Aspect Ratio"));
    d->preserveAspectRatio->setWhatsThis(i18n("Enable this option if you want to preserve the aspect "
                                              "ratio of image. If enabled, the border width will be "
                                              "a percentage of the image size, else the border width will be "
                                              "in pixels."));

    d->labelBorderPercent  = new QLabel(i18n("Width (%):"));
    d->borderPercent       = new DIntNumInput();
    d->borderPercent->setRange(1, 50, 1);
    d->borderPercent->setDefaultValue(10);
    d->borderPercent->setWhatsThis(i18n("Set here the border width as a percentage of the image size."));

    d->labelBorderWidth = new QLabel(i18n("Width (pixels):"));
    d->borderWidth      = new DIntNumInput();
    d->borderWidth->setRange(1, 1000, 1);
    d->borderWidth->setDefaultValue(100);
    d->borderWidth->setWhatsThis(i18n("Set here the border width in pixels to add around the image."));

    DLineWidget* line2 = new DLineWidget(Qt::Horizontal);

    // -------------------------------------------------------------------

    d->labelForeground   = new QLabel();
    d->firstColorButton  = new DColorSelector();
    d->firstColorButton->setColor(QColor(192, 192, 192));
    d->labelBackground   = new QLabel();
    d->secondColorButton = new DColorSelector();
    d->secondColorButton->setColor(QColor(128, 128, 128));

    // -------------------------------------------------------------------

    grid->addWidget(label1,                  0, 0, 1, 3);
    grid->addWidget(d->borderType,           1, 0, 1, 3);
    grid->addWidget(line1,                   2, 0, 1, 3);
    grid->addWidget(d->preserveAspectRatio,  3, 0, 1, 3);
    grid->addWidget(d->labelBorderPercent,   4, 0, 1, 3);
    grid->addWidget(d->borderPercent,        5, 0, 1, 3);
    grid->addWidget(d->labelBorderWidth,     6, 0, 1, 3);
    grid->addWidget(d->borderWidth,          7, 0, 1, 3);
    grid->addWidget(line2,                   8, 0, 1, 3);
    grid->addWidget(d->labelForeground,      9, 0, 1, 1);
    grid->addWidget(d->firstColorButton,     9, 1, 1, 2);
    grid->addWidget(d->labelBackground,     10, 0, 1, 1);
    grid->addWidget(d->secondColorButton,   10, 1, 1, 2);
    grid->setRowStretch(11, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->preserveAspectRatio, SIGNAL(toggled(bool)),
            this, SLOT(slotPreserveAspectRatioToggled(bool)));

    connect(d->borderType, SIGNAL(activated(int)),
            this, SLOT(slotBorderTypeChanged(int)));

    connect(d->borderPercent, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->borderWidth, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->firstColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotColorForegroundChanged(QColor)));

    connect(d->secondColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotColorBackgroundChanged(QColor)));
}

BorderSettings::~BorderSettings()
{
    delete d;
}

BorderContainer BorderSettings::settings() const
{
    BorderContainer prm;

    prm.preserveAspectRatio   = d->preserveAspectRatio->isChecked();
    prm.borderType            = d->borderType->currentIndex();
    prm.borderWidth1          = d->borderWidth->value();
    prm.borderWidth2          = 15;
    prm.borderWidth3          = 15;
    prm.borderWidth4          = 10;
    prm.borderPercent         = d->borderPercent->value() / 100.0;
    prm.borderPath            = getBorderPath(d->borderType->currentIndex());
    prm.solidColor            = d->solidColor;
    prm.niepceBorderColor     = d->niepceBorderColor;
    prm.niepceLineColor       = d->niepceLineColor;
    prm.bevelUpperLeftColor   = d->bevelUpperLeftColor;
    prm.bevelLowerRightColor  = d->bevelLowerRightColor;
    prm.decorativeFirstColor  = d->decorativeFirstColor;
    prm.decorativeSecondColor = d->decorativeSecondColor;

    return prm;
}

void BorderSettings::setSettings(const BorderContainer& settings)
{
    blockSignals(true);

    d->preserveAspectRatio->setChecked(settings.preserveAspectRatio);
    d->borderType->setCurrentIndex(settings.borderType);
    d->borderWidth->setValue(settings.borderWidth1);
    d->borderPercent->setValue((int)(settings.borderPercent * 100.0));

    d->solidColor            = settings.solidColor;
    d->niepceBorderColor     = settings.niepceBorderColor;
    d->niepceLineColor       = settings.niepceLineColor;
    d->bevelUpperLeftColor   = settings.bevelUpperLeftColor;
    d->bevelLowerRightColor  = settings.bevelLowerRightColor;
    d->decorativeFirstColor  = settings.decorativeFirstColor;
    d->decorativeSecondColor = settings.decorativeSecondColor;

    slotBorderTypeChanged(settings.borderType);

    blockSignals(false);
}

void BorderSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

BorderContainer BorderSettings::defaultSettings() const
{
    BorderContainer prm;
    return prm;
}

void BorderSettings::readSettings(KConfigGroup& group)
{
    blockSignals(true);

    d->borderType->setCurrentIndex(group.readEntry(d->configBorderTypeEntry, d->borderType->defaultIndex()));
    d->borderPercent->setValue(group.readEntry(d->configBorderPercentEntry,  d->borderPercent->defaultValue()));
    d->borderWidth->setValue(group.readEntry(d->configBorderWidthEntry,      d->borderWidth->defaultValue()));
    d->preserveAspectRatio->setChecked(group.readEntry(d->configPreserveAspectRatioEntry, true));

    d->solidColor            = group.readEntry(d->configSolidColorEntry,            QColor(0, 0, 0));
    d->niepceBorderColor     = group.readEntry(d->configNiepceBorderColorEntry,     QColor(255, 255, 255));
    d->niepceLineColor       = group.readEntry(d->configNiepceLineColorEntry,       QColor(0, 0, 0));
    d->bevelUpperLeftColor   = group.readEntry(d->configBevelUpperLeftColorEntry,   QColor(192, 192, 192));
    d->bevelLowerRightColor  = group.readEntry(d->configBevelLowerRightColorEntry,  QColor(128, 128, 128));
    d->decorativeFirstColor  = group.readEntry(d->configDecorativeFirstColorEntry,  QColor(0, 0, 0));
    d->decorativeSecondColor = group.readEntry(d->configDecorativeSecondColorEntry, QColor(0, 0, 0));

    blockSignals(false);
    slotBorderTypeChanged(d->borderType->currentIndex());
}

void BorderSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configBorderTypeEntry,            d->borderType->currentIndex());
    group.writeEntry(d->configBorderPercentEntry,         d->borderPercent->value());
    group.writeEntry(d->configBorderWidthEntry,           d->borderWidth->value());
    group.writeEntry(d->configPreserveAspectRatioEntry,   d->preserveAspectRatio->isChecked());
    group.writeEntry(d->configSolidColorEntry,            d->solidColor);
    group.writeEntry(d->configNiepceBorderColorEntry,     d->niepceBorderColor);
    group.writeEntry(d->configNiepceLineColorEntry,       d->niepceLineColor);
    group.writeEntry(d->configBevelUpperLeftColorEntry,   d->bevelUpperLeftColor);
    group.writeEntry(d->configBevelLowerRightColorEntry,  d->bevelLowerRightColor);
    group.writeEntry(d->configDecorativeFirstColorEntry,  d->decorativeFirstColor);
    group.writeEntry(d->configDecorativeSecondColorEntry, d->decorativeSecondColor);
}

void BorderSettings::slotColorForegroundChanged(const QColor& color)
{
    switch (d->borderType->currentIndex())
    {
        case BorderContainer::SolidBorder:
            d->solidColor = color;
            break;

        case BorderContainer::NiepceBorder:
            d->niepceBorderColor = color;
            break;

        case BorderContainer::BeveledBorder:
            d->bevelUpperLeftColor = color;
            break;

        case BorderContainer::PineBorder:
        case BorderContainer::WoodBorder:
        case BorderContainer::PaperBorder:
        case BorderContainer::ParqueBorder:
        case BorderContainer::IceBorder:
        case BorderContainer::LeafBorder:
        case BorderContainer::MarbleBorder:
        case BorderContainer::RainBorder:
        case BorderContainer::CratersBorder:
        case BorderContainer::DriedBorder:
        case BorderContainer::PinkBorder:
        case BorderContainer::StoneBorder:
        case BorderContainer::ChalkBorder:
        case BorderContainer::GraniteBorder:
        case BorderContainer::RockBorder:
        case BorderContainer::WallBorder:
            d->decorativeFirstColor = color;
            break;
    }

    emit signalSettingsChanged();
}

void BorderSettings::slotColorBackgroundChanged(const QColor& color)
{
    switch (d->borderType->currentIndex())
    {
        case BorderContainer::SolidBorder:
            d->solidColor = color;
            break;

        case BorderContainer::NiepceBorder:
            d->niepceLineColor = color;
            break;

        case BorderContainer::BeveledBorder:
            d->bevelLowerRightColor = color;
            break;

        case BorderContainer::PineBorder:
        case BorderContainer::WoodBorder:
        case BorderContainer::PaperBorder:
        case BorderContainer::ParqueBorder:
        case BorderContainer::IceBorder:
        case BorderContainer::LeafBorder:
        case BorderContainer::MarbleBorder:
        case BorderContainer::RainBorder:
        case BorderContainer::CratersBorder:
        case BorderContainer::DriedBorder:
        case BorderContainer::PinkBorder:
        case BorderContainer::StoneBorder:
        case BorderContainer::ChalkBorder:
        case BorderContainer::GraniteBorder:
        case BorderContainer::RockBorder:
        case BorderContainer::WallBorder:
            d->decorativeSecondColor = color;
            break;
    }

    emit signalSettingsChanged();
}

void BorderSettings::slotBorderTypeChanged(int borderType)
{
    d->labelForeground->setText(i18nc("first color for border effect", "First:"));
    d->labelBackground->setText(i18nc("second color for border effect", "Second:"));
    d->firstColorButton->setWhatsThis(i18n("Set here the foreground color of the border."));
    d->secondColorButton->setWhatsThis(i18n("Set here the Background color of the border."));
    d->firstColorButton->setEnabled(true);
    d->secondColorButton->setEnabled(true);
    d->labelForeground->setEnabled(true);
    d->labelBackground->setEnabled(true);
    d->borderPercent->setEnabled(true);

    switch (borderType)
    {
        case BorderContainer::SolidBorder:
            d->firstColorButton->setColor(d->solidColor);
            d->secondColorButton->setEnabled(false);
            d->labelBackground->setEnabled(false);
            break;

        case BorderContainer::NiepceBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the main border."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the line."));
            d->firstColorButton->setColor(d->niepceBorderColor);
            d->secondColorButton->setColor(d->niepceLineColor);
            break;

        case BorderContainer::BeveledBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the upper left area."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the lower right area."));
            d->firstColorButton->setColor(d->bevelUpperLeftColor);
            d->secondColorButton->setColor(d->bevelLowerRightColor);
            break;

        case BorderContainer::PineBorder:
        case BorderContainer::WoodBorder:
        case BorderContainer::PaperBorder:
        case BorderContainer::ParqueBorder:
        case BorderContainer::IceBorder:
        case BorderContainer::LeafBorder:
        case BorderContainer::MarbleBorder:
        case BorderContainer::RainBorder:
        case BorderContainer::CratersBorder:
        case BorderContainer::DriedBorder:
        case BorderContainer::PinkBorder:
        case BorderContainer::StoneBorder:
        case BorderContainer::ChalkBorder:
        case BorderContainer::GraniteBorder:
        case BorderContainer::RockBorder:
        case BorderContainer::WallBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the first line."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the second line."));
            d->firstColorButton->setColor(d->decorativeFirstColor);
            d->secondColorButton->setColor(d->decorativeSecondColor);
            break;
    }

    emit signalSettingsChanged();
}

void BorderSettings::slotPreserveAspectRatioToggled(bool b)
{
    toggleBorderSlider(b);
    emit signalSettingsChanged();
}

QString BorderSettings::getBorderPath(int border)
{
    QString pattern;

    switch (border)
    {
        case BorderContainer::PineBorder:
            pattern = QLatin1String("pine-pattern");
            break;

        case BorderContainer::WoodBorder:
            pattern = QLatin1String("wood-pattern");
            break;

        case BorderContainer::PaperBorder:
            pattern = QLatin1String("paper-pattern");
            break;

        case BorderContainer::ParqueBorder:
            pattern = QLatin1String("parque-pattern");
            break;

        case BorderContainer::IceBorder:
            pattern = QLatin1String("ice-pattern");
            break;

        case BorderContainer::LeafBorder:
            pattern = QLatin1String("leaf-pattern");
            break;

        case BorderContainer::MarbleBorder:
            pattern = QLatin1String("marble-pattern");
            break;

        case BorderContainer::RainBorder:
            pattern = QLatin1String("rain-pattern");
            break;

        case BorderContainer::CratersBorder:
            pattern = QLatin1String("craters-pattern");
            break;

        case BorderContainer::DriedBorder:
            pattern = QLatin1String("dried-pattern");
            break;

        case BorderContainer::PinkBorder:
            pattern = QLatin1String("pink-pattern");
            break;

        case BorderContainer::StoneBorder:
            pattern = QLatin1String("stone-pattern");
            break;

        case BorderContainer::ChalkBorder:
            pattern = QLatin1String("chalk-pattern");
            break;

        case BorderContainer::GraniteBorder:
            pattern = QLatin1String("granit-pattern");
            break;

        case BorderContainer::RockBorder:
            pattern = QLatin1String("rock-pattern");
            break;

        case BorderContainer::WallBorder:
            pattern = QLatin1String("wall-pattern");
            break;

        default:
            return pattern;
            break;
    }

    return (QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/") + pattern + QLatin1String(".png")));
}

void BorderSettings::toggleBorderSlider(bool b)
{
    d->borderPercent->setEnabled(b);
    d->borderWidth->setEnabled(!b);
    d->labelBorderPercent->setEnabled(b);
    d->labelBorderWidth->setEnabled(!b);
}

}  // namespace Digikam
