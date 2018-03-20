/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#include "enfusesettings.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QApplication>
#include <QSpinBox>
#include <QDoubleSpinBox>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

QString EnfuseSettings::asCommentString() const
{
    QString ret;

    ret.append(hardMask ? i18n("Hardmask: enabled") : i18n("Hardmask: disabled"));
    ret.append(QChar::fromLatin1('\n'));
    ret.append(ciecam02 ? i18n("CIECAM02: enabled") : i18n("CIECAM02: disabled"));
    ret.append(QChar::fromLatin1('\n'));
    ret.append(autoLevels ? i18n("Levels: auto") : i18n("Levels: %1", QString::number(levels)));
    ret.append(QChar::fromLatin1('\n'));
    ret.append(i18n("Exposure: %1",   QString::number(exposure)));
    ret.append(QChar::fromLatin1('\n'));
    ret.append(i18n("Saturation: %1", QString::number(saturation)));
    ret.append(QChar::fromLatin1('\n'));
    ret.append(i18n("Contrast: %1",   QString::number(contrast)));

    return ret;
}

QString EnfuseSettings::inputImagesList() const
{
    QString ret;

    foreach(const QUrl& url, inputUrls)
    {
        ret.append(url.fileName() + QLatin1String(" ; "));
    }

    ret.truncate(ret.length()-3);
    return ret;
}

class EnfuseSettingsWidget::Private
{
public:

    explicit Private()
      : autoLevelsCB(0),
        hardMaskCB(0),
        ciecam02CB(0),
        levelsLabel(0),
        exposureLabel(0),
        saturationLabel(0),
        contrastLabel(0),
        levelsInput(0),
        exposureInput(0),
        saturationInput(0),
        contrastInput(0)
    {
    }

public:

    QCheckBox*       autoLevelsCB;
    QCheckBox*       hardMaskCB;
    QCheckBox*       ciecam02CB;

    QLabel*          levelsLabel;
    QLabel*          exposureLabel;
    QLabel*          saturationLabel;
    QLabel*          contrastLabel;

    QSpinBox*        levelsInput;

    QDoubleSpinBox*  exposureInput;
    QDoubleSpinBox*  saturationInput;
    QDoubleSpinBox*  contrastInput;
};

EnfuseSettingsWidget::EnfuseSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(this);

    // ------------------------------------------------------------------------

    d->autoLevelsCB = new QCheckBox(i18nc("@option:check Enfuse setting", "Automatic Local/Global Image Features Balance (Levels)"), this);
    d->autoLevelsCB->setToolTip(i18nc("@info:tooltip",
                                      "Optimise image features (contrast, saturation, . . .) to be as global as possible."));
    d->autoLevelsCB->setWhatsThis(i18nc("@info:whatsthis",
                                        "Set automatic level selection (maximized) for pyramid blending, "
                                        "i.e. optimise image features (contrast, saturation, . . .) to be as global as possible."));

    d->levelsLabel  = new QLabel(i18nc("@label:slider Enfuse settings", "Image Features Balance:"));
    d->levelsInput  = new QSpinBox(this);
    d->levelsInput->setRange(1, 29);
    d->levelsInput->setSingleStep(1);
    d->levelsInput->setToolTip(i18nc("@info:tooltip",
                                     "Balances between local features (small number) or global features (high number)."));
    d->levelsInput->setWhatsThis(i18nc("@info:whatsthis",
                                       "Set the number of levels for pyramid blending. "
                                       "Balances towards local features (small number) or global features (high number). "
                                       "Additionally, a low number trades off quality of results for faster "
                                       "execution time and lower memory usage."));

    d->hardMaskCB = new QCheckBox(i18nc("@option:check", "Hard Mask"), this);
    d->hardMaskCB->setToolTip(i18nc("@info:tooltip",
                                    "Useful only for focus stack to improve sharpness."));
    d->hardMaskCB->setWhatsThis(i18nc("@info:whatsthis",
                                      "Force hard blend masks without averaging on finest "
                                      "scale. This is only useful for focus "
                                      "stacks with thin and high contrast features. "
                                      "It improves sharpness at the expense of increased noise."));

    d->exposureLabel = new QLabel(i18nc("@label:slider Enfuse settings", "Well-Exposedness Contribution:"));
    d->exposureInput = new QDoubleSpinBox(this);
    d->exposureInput->setRange(0.0, 1.0);
    d->exposureInput->setSingleStep(0.01);
    d->exposureInput->setToolTip(i18nc("@info:tooltip",
                                       "Contribution of well exposed pixels to the blending process."));
    d->exposureInput->setWhatsThis(i18nc("@info:whatsthis",
                                         "Set the well-exposedness criterion contribution for the blending process. "
                                         "Higher values will favor well-exposed pixels."));

    d->saturationLabel = new QLabel(i18nc("@label:slider enfuse settings", "High-Saturation Contribution:"));
    d->saturationInput = new QDoubleSpinBox(this);
    d->saturationInput->setDecimals(2);
    d->saturationInput->setRange(0.0, 1.0);
    d->saturationInput->setSingleStep(0.01);
    d->saturationInput->setToolTip(i18nc("@info:tooltip",
                                         "Contribution of highly saturated pixels to the blending process."));
    d->saturationInput->setWhatsThis(i18nc("@info:whatsthis",
                                           "Increasing this value makes pixels with high "
                                           "saturation contribute more to the final output."));

    d->contrastLabel = new QLabel(i18nc("@label:slider enfuse settings", "High-Contrast Contribution:"));
    d->contrastInput = new QDoubleSpinBox(this);
    d->contrastInput->setDecimals(2);
    d->contrastInput->setRange(0.0, 1.0);
    d->contrastInput->setSingleStep(0.01);
    d->contrastInput->setToolTip(i18nc("@info:tooltip",
                                       "Contribution of highly contrasted pixels to the blending process."));
    d->contrastInput->setWhatsThis(i18nc("@info:whatsthis",
                                         "Sets the relative weight of high-contrast pixels. "
                                         "Increasing this weight makes pixels with neighboring differently colored "
                                         "pixels contribute more to the final output. Particularly useful for focus stacks."));

    d->ciecam02CB = new QCheckBox(i18nc("@option:check", "Use Color Appearance Model (CIECAM02)"), this);
    d->ciecam02CB->setToolTip(i18nc("@info:tooltip",
                                    "Convert to CIECAM02 color appearance model during the blending process instead of RGB."));
    d->ciecam02CB->setWhatsThis(i18nc("@info:whatsthis",
                                      "Use Color Appearance Modelling (CIECAM02) to render detailed colors. "
                                      "Your input files should have embedded ICC profiles. If no ICC profile is present, "
                                      "sRGB color space will be assumed. The difference between using this option "
                                      "and default color blending algorithm is very slight, and will be most noticeable "
                                      "when you need to blend areas of different primary colors together."));

    // ------------------------------------------------------------------------

    grid->addWidget(d->autoLevelsCB,    0, 0, 1, 2);
    grid->addWidget(d->levelsLabel,     1, 0, 1, 1);
    grid->addWidget(d->levelsInput,     1, 1, 1, 1);
    grid->addWidget(d->hardMaskCB,      2, 0, 1, 2);
    grid->addWidget(d->exposureLabel,   3, 0, 1, 1);
    grid->addWidget(d->exposureInput,   3, 1, 1, 1);
    grid->addWidget(d->saturationLabel, 4, 0, 1, 1);
    grid->addWidget(d->saturationInput, 4, 1, 1, 1);
    grid->addWidget(d->contrastLabel,   5, 0, 1, 1);
    grid->addWidget(d->contrastInput,   5, 1, 1, 1);
    grid->addWidget(d->ciecam02CB,      6, 0, 1, 2);
    grid->setRowStretch(7, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // ------------------------------------------------------------------------

    connect(d->autoLevelsCB, SIGNAL(toggled(bool)),
            d->levelsLabel, SLOT(setDisabled(bool)));

    connect(d->autoLevelsCB, SIGNAL(toggled(bool)),
            d->levelsInput, SLOT(setDisabled(bool)));
}

EnfuseSettingsWidget::~EnfuseSettingsWidget()
{
    delete d;
}

void EnfuseSettingsWidget::resetToDefault()
{
    d->autoLevelsCB->setChecked(true);
    d->levelsInput->setValue(20);
    d->hardMaskCB->setChecked(false);
    d->exposureInput->setValue(1.0);
    d->saturationInput->setValue(0.2);
    d->contrastInput->setValue(0.0);
    d->ciecam02CB->setChecked(false);
}

void EnfuseSettingsWidget::setSettings(const EnfuseSettings& settings)
{
    d->autoLevelsCB->setChecked(settings.autoLevels);
    d->levelsInput->setValue(settings.levels);
    d->hardMaskCB->setChecked(settings.hardMask);
    d->exposureInput->setValue(settings.exposure);
    d->saturationInput->setValue(settings.saturation);
    d->contrastInput->setValue(settings.contrast);
    d->ciecam02CB->setChecked(settings.ciecam02);
}

EnfuseSettings EnfuseSettingsWidget::settings() const
{
    EnfuseSettings settings;
    settings.autoLevels = d->autoLevelsCB->isChecked();
    settings.levels     = d->levelsInput->value();
    settings.hardMask   = d->hardMaskCB->isChecked();
    settings.exposure   = d->exposureInput->value();
    settings.saturation = d->saturationInput->value();
    settings.contrast   = d->contrastInput->value();
    settings.ciecam02   = d->ciecam02CB->isChecked();
    return settings;
}

void EnfuseSettingsWidget::readSettings(KConfigGroup& group)
{
    d->autoLevelsCB->setChecked(group.readEntry("Auto Levels",       true));
    d->levelsInput->setValue(group.readEntry("Levels Value",         20));
    d->hardMaskCB->setChecked(group.readEntry("Hard Mask",           false));
    d->exposureInput->setValue(group.readEntry("Exposure Value",     1.0));
    d->saturationInput->setValue(group.readEntry("Saturation Value", 0.2));
    d->contrastInput->setValue(group.readEntry("Contrast Value",     0.0));
    d->ciecam02CB->setChecked(group.readEntry("CIECAM02",            false));
}

void EnfuseSettingsWidget::writeSettings(KConfigGroup& group)
{
    group.writeEntry("Auto Levels",      d->autoLevelsCB->isChecked());
    group.writeEntry("Levels Value",     d->levelsInput->value());
    group.writeEntry("Hard Mask",        d->hardMaskCB->isChecked());
    group.writeEntry("Exposure Value",   d->exposureInput->value());
    group.writeEntry("Saturation Value", d->saturationInput->value());
    group.writeEntry("Contrast Value",   d->contrastInput->value());
    group.writeEntry("CIECAM02",         d->ciecam02CB->isChecked());
}

} // namespace Digikam
