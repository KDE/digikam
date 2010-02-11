/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Color Balance settings view.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cbsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QSlider>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>

// KDE includes

#include <kdebug.h>
#include <kurl.h>
#include <kdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace Digikam
{

class CBSettingsPriv
{
public:

    CBSettingsPriv() :
        configRedAdjustmentEntry("RedAdjustment"),
        configGreenAdjustmentEntry("GreenAdjustment"),
        configBlueAdjustmentEntry("BlueAdjustment"),
        rSlider(0),
        gSlider(0),
        bSlider(0),
        rInput(0),
        gInput(0),
        bInput(0)
        {}

    const QString configRedAdjustmentEntry;
    const QString configGreenAdjustmentEntry;
    const QString configBlueAdjustmentEntry;

    QSlider*      rSlider;
    QSlider*      gSlider;
    QSlider*      bSlider;

    RIntNumInput* rInput;
    RIntNumInput* gInput;
    RIntNumInput* bInput;
};

CBSettings::CBSettings(QWidget* parent)
          : QWidget(parent),
            d(new CBSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    QLabel* labelCyan = new QLabel(i18n("Cyan"));
    labelCyan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->rSlider = new QSlider(Qt::Horizontal);
    d->rSlider->setValue(0);
    d->rSlider->setRange(-100, 100);
    d->rSlider->setPageStep(1);
    d->rSlider->setTickPosition(QSlider::TicksBelow);
    d->rSlider->setTickInterval(20);
    d->rSlider->setWhatsThis(i18n("Set here the cyan/red color adjustment of the image."));

    QLabel *labelRed = new QLabel(i18n("Red"));
    labelRed->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->rInput = new RIntNumInput();
    d->rInput->setRange(-100, 100, 1);
    d->rInput->setSliderEnabled(false);
    d->rInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelMagenta = new QLabel(i18n("Magenta"));
    labelMagenta->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->gSlider = new QSlider(Qt::Horizontal);
    d->gSlider->setValue(0);
    d->gSlider->setRange(-100, 100);
    d->gSlider->setPageStep(1);
    d->gSlider->setTickPosition(QSlider::TicksBelow);
    d->gSlider->setTickInterval(20);
    d->gSlider->setWhatsThis(i18n("Set here the magenta/green color adjustment of the image."));

    QLabel *labelGreen = new QLabel(i18n("Green"));
    labelGreen->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->gInput = new RIntNumInput();
    d->gInput->setRange(-100, 100, 1);
    d->gInput->setSliderEnabled(false);
    d->gInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelYellow = new QLabel(i18n("Yellow"));
    labelYellow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->bSlider = new QSlider(Qt::Horizontal);
    d->bSlider->setValue(0);
    d->bSlider->setRange(-100, 100);
    d->bSlider->setPageStep(1);
    d->bSlider->setTickPosition(QSlider::TicksBelow);
    d->bSlider->setTickInterval(20);
    d->bSlider->setWhatsThis(i18n("Set here the yellow/blue color adjustment of the image."));

    QLabel *labelBlue = new QLabel(i18n("Blue"));
    labelBlue->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->bInput = new RIntNumInput();
    d->bInput->setRange(-100, 100, 1);
    d->bInput->setSliderEnabled(false);
    d->bInput->setDefaultValue(0);

    // -------------------------------------------------------------

    grid->addWidget(labelCyan,    0, 0, 1, 1);
    grid->addWidget(d->rSlider,   0, 1, 1, 1);
    grid->addWidget(labelRed,     0, 2, 1, 1);
    grid->addWidget(d->rInput,    0, 3, 1, 1);
    grid->addWidget(labelMagenta, 1, 0, 1, 1);
    grid->addWidget(d->gSlider,   1, 1, 1, 1);
    grid->addWidget(labelGreen,   1, 2, 1, 1);
    grid->addWidget(d->gInput,    1, 3, 1, 1);
    grid->addWidget(labelYellow,  2, 0, 1, 1);
    grid->addWidget(d->bSlider,   2, 1, 1, 1);
    grid->addWidget(labelBlue,    2, 2, 1, 1);
    grid->addWidget(d->bInput,    2, 3, 1, 1);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    grid->setRowStretch(3, 10);

    // -------------------------------------------------------------

    connect(d->rSlider, SIGNAL(valueChanged(int)),
            d->rInput, SLOT(setValue(int)));

    connect(d->rInput, SIGNAL(valueChanged (int)),
            d->rSlider, SLOT(setValue(int)));

    connect(d->rInput, SIGNAL(valueChanged (int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->gSlider, SIGNAL(valueChanged(int)),
            d->gInput, SLOT(setValue(int)));

    connect(d->gInput, SIGNAL(valueChanged (int)),
            d->gSlider, SLOT(setValue(int)));

    connect(d->gInput, SIGNAL(valueChanged (int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->bSlider, SIGNAL(valueChanged(int)),
            d->bInput, SLOT(setValue(int)));

    connect(d->bInput, SIGNAL(valueChanged (int)),
            d->bSlider, SLOT(setValue(int)));

    connect(d->bInput, SIGNAL(valueChanged (int)),
            this, SIGNAL(signalSettingsChanged()));
}

CBSettings::~CBSettings()
{
    delete d;
}

CBContainer CBSettings::settings() const
{
    CBContainer prm;

    prm.red   = ((double)d->rInput->value() + 100.0)/100.0;
    prm.green = ((double)d->gInput->value() + 100.0)/100.0;
    prm.blue  = ((double)d->bInput->value() + 100.0)/100.0;

    return prm;
}

void CBSettings::setSettings(const CBContainer& settings)
{
    blockSignals(true);
    d->rInput->setValue((int)(settings.red   * 100.0) - 100.0);
    d->gInput->setValue((int)(settings.green * 100.0) - 100.0);
    d->bInput->setValue((int)(settings.blue  * 100.0) - 100.0);
    adjustSliders(settings);
    blockSignals(false);
}

void CBSettings::resetToDefault()
{
    blockSignals(true);
    d->rInput->slotReset();
    d->gInput->slotReset();
    d->bInput->slotReset();
    adjustSliders(defaultSettings());
    blockSignals(false);
}

CBContainer CBSettings::defaultSettings() const
{
    CBContainer prm;

    prm.red   = ((double)d->rInput->defaultValue() + 100.0)/100.0;
    prm.green = ((double)d->gInput->defaultValue() + 100.0)/100.0;
    prm.blue  = ((double)d->bInput->defaultValue() + 100.0)/100.0;

    return prm;
}

void CBSettings::readSettings(KConfigGroup& group)
{
    CBContainer prm;
    CBContainer defaultPrm = defaultSettings();

    prm.red   = group.readEntry(d->configRedAdjustmentEntry,   defaultPrm.red);
    prm.green = group.readEntry(d->configGreenAdjustmentEntry, defaultPrm.green);
    prm.blue  = group.readEntry(d->configBlueAdjustmentEntry,  defaultPrm.blue);

    setSettings(prm);
}

void CBSettings::writeSettings(KConfigGroup& group)
{
    CBContainer prm = settings();

    group.writeEntry(d->configRedAdjustmentEntry,   prm.red);
    group.writeEntry(d->configGreenAdjustmentEntry, prm.green);
    group.writeEntry(d->configBlueAdjustmentEntry,  prm.blue);
}

void CBSettings::adjustSliders(const CBContainer& settings)
{
    d->rSlider->blockSignals(true);
    d->gSlider->blockSignals(true);
    d->bSlider->blockSignals(true);
    d->rInput->blockSignals(true);
    d->gInput->blockSignals(true);
    d->bInput->blockSignals(true);

    d->rSlider->setValue(settings.red);
    d->gSlider->setValue(settings.green);
    d->bSlider->setValue(settings.blue);
    d->rInput->setValue(settings.red);
    d->gInput->setValue(settings.green);
    d->bInput->setValue(settings.blue);

    d->rSlider->blockSignals(false);
    d->gSlider->blockSignals(false);
    d->bSlider->blockSignals(false);
    d->rInput->blockSignals(false);
    d->gInput->blockSignals(false);
    d->bInput->blockSignals(false);
}

}  // namespace Digikam
