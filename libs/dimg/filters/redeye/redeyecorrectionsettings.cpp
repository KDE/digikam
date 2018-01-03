/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Red Eyes auto correction settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#include "redeyecorrectionsettings.h"

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

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "digikam_debug.h"
#include "dexpanderbox.h"

namespace Digikam
{

class RedEyeCorrectionSettings::Private
{
public:

    Private() :
        redtoavgratioInput(0)
    {
    }

    static const QString configRedToAvgRatioAdjustmentEntry;

    DDoubleNumInput*     redtoavgratioInput;
};

const QString RedEyeCorrectionSettings::Private::configRedToAvgRatioAdjustmentEntry(QLatin1String("RedToAvgRatioAdjustment"));

// --------------------------------------------------------

RedEyeCorrectionSettings::RedEyeCorrectionSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing     = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* grid     = new QGridLayout(parent);

    QLabel* const label2  = new QLabel(i18n("Red Level:"));
    d->redtoavgratioInput = new DDoubleNumInput();
    d->redtoavgratioInput->setRange(1.5, 3.0, 0.1);
    d->redtoavgratioInput->setDefaultValue(2.1);
    d->redtoavgratioInput->setWhatsThis(i18n("Set here the reducing level of red to the average of blue and green."));

    // -------------------------------------------------------------

    grid->addWidget(label2,                0, 0, 1, 1);
    grid->addWidget(d->redtoavgratioInput, 1, 0, 1, 1);
    grid->setRowStretch(6, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->redtoavgratioInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
}

RedEyeCorrectionSettings::~RedEyeCorrectionSettings()
{
    delete d;
}

RedEyeCorrectionContainer RedEyeCorrectionSettings::settings() const
{
    RedEyeCorrectionContainer prm;

    prm.m_redToAvgRatio = (double)d->redtoavgratioInput->value();

    return prm;
}

void RedEyeCorrectionSettings::setSettings(const RedEyeCorrectionContainer& settings)
{
    blockSignals(true);
    d->redtoavgratioInput->setValue(settings.m_redToAvgRatio);
    blockSignals(false);
}

void RedEyeCorrectionSettings::resetToDefault()
{
    blockSignals(true);
    d->redtoavgratioInput->slotReset();
    blockSignals(false);
}

RedEyeCorrectionContainer RedEyeCorrectionSettings::defaultSettings() const
{
    RedEyeCorrectionContainer prm;

    prm.m_redToAvgRatio = (double)(d->redtoavgratioInput->defaultValue());

    return prm;
}

void RedEyeCorrectionSettings::readSettings(KConfigGroup& group)
{
    RedEyeCorrectionContainer prm;
    RedEyeCorrectionContainer defaultPrm = defaultSettings();

    prm.m_redToAvgRatio = group.readEntry(d->configRedToAvgRatioAdjustmentEntry, defaultPrm.m_redToAvgRatio);

    setSettings(prm);
}

void RedEyeCorrectionSettings::writeSettings(KConfigGroup& group)
{
    RedEyeCorrectionContainer prm = settings();

    group.writeEntry(d->configRedToAvgRatioAdjustmentEntry, prm.m_redToAvgRatio);
}

}  // namespace Digikam
