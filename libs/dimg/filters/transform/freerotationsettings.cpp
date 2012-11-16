/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : Free rotation settings view.
 *
 * Copyright (C) 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "freerotationsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>

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

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rexpanderbox.h>

using namespace KDcrawIface;

namespace Digikam
{

class FreeRotationSettings::Private
{
public:

    Private() :
        antialiasInput(0),
        angleInput(0),
        fineAngleInput(0),
        autoCropCB(0)
    {}

    static const QString configAutoCropTypeEntry;
    static const QString configAntiAliasingEntry;

public:
    
    QCheckBox*       antialiasInput;

    RIntNumInput*    angleInput;
    RDoubleNumInput* fineAngleInput;
    RComboBox*       autoCropCB;
};

const QString FreeRotationSettings::Private::configAutoCropTypeEntry("Auto Crop Type");
const QString FreeRotationSettings::Private::configAntiAliasingEntry("Anti Aliasing");

// --------------------------------------------------------

FreeRotationSettings::FreeRotationSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* grid = new QGridLayout(this);

    // --------------------------------------------------------

    QLabel* label3    = new QLabel(i18n("Main angle:"));
    d->angleInput     = new RIntNumInput;
    d->angleInput->setRange(-180, 180, 1);
    d->angleInput->setSliderEnabled(true);
    d->angleInput->setDefaultValue(0);
    d->angleInput->setWhatsThis(i18n("An angle in degrees by which to rotate the image. "
                                     "A positive angle rotates the image clockwise; "
                                     "a negative angle rotates it counter-clockwise."));

    QLabel* label4    = new QLabel(i18n("Fine angle:"));
    d->fineAngleInput = new RDoubleNumInput;
    d->fineAngleInput->input()->setRange(-1.0, 1.0, 0.01, true);
    d->fineAngleInput->setDefaultValue(0);
    d->fineAngleInput->setWhatsThis(i18n("This value in degrees will be added to main angle value "
                                         "to set fine target angle."));

    d->antialiasInput = new QCheckBox(i18n("Anti-Aliasing"));
    d->antialiasInput->setWhatsThis(i18n("Enable this option to apply the anti-aliasing filter "
                                         "to the rotated image. "
                                         "In order to smooth the target image, it will be blurred a little."));

    QLabel* label5    = new QLabel(i18n("Auto-crop:"));
    d->autoCropCB     = new RComboBox;
    d->autoCropCB->addItem(i18nc("no autocrop", "None"));
    d->autoCropCB->addItem(i18n("Widest Area"));
    d->autoCropCB->addItem(i18n("Largest Area"));
    d->autoCropCB->setDefaultIndex(FreeRotationContainer::NoAutoCrop);
    d->autoCropCB->setWhatsThis(i18n("Select the method to process image auto-cropping "
                                     "to remove black frames around a rotated image here."));

    // -------------------------------------------------------------

    grid->addWidget(label3,            0, 0, 1, 1);
    grid->addWidget(d->angleInput,     1, 0, 1, 2);
    grid->addWidget(label4,            2, 0, 1, 1);
    grid->addWidget(d->fineAngleInput, 3, 0, 1, 2);
    grid->addWidget(d->antialiasInput, 4, 0, 1, -1);
    grid->addWidget(label5,            5, 0, 1, 1);
    grid->addWidget(d->autoCropCB,     5, 1, 1, 1);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->antialiasInput, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->autoCropCB, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->angleInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->fineAngleInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
}

FreeRotationSettings::~FreeRotationSettings()
{
    delete d;
}

FreeRotationContainer FreeRotationSettings::settings() const
{
    FreeRotationContainer prm;
    prm.angle     = (double)d->angleInput->value() + d->fineAngleInput->value();
    prm.antiAlias = d->antialiasInput->isChecked();
    prm.autoCrop  = d->autoCropCB->currentIndex();
    return prm;
}

void FreeRotationSettings::setSettings(const FreeRotationContainer& settings)
{
    blockSignals(true);

    d->angleInput->setValue((int)(settings.angle));
    d->fineAngleInput->setValue(settings.angle - (double)d->angleInput->value());
    d->antialiasInput->setChecked(settings.antiAlias);
    d->autoCropCB->setCurrentIndex(settings.autoCrop);

    blockSignals(false);
}

void FreeRotationSettings::resetToDefault()
{
    blockSignals(true);
    d->angleInput->slotReset();
    d->fineAngleInput->slotReset();
    d->antialiasInput->setChecked(true);
    d->autoCropCB->slotReset();
    blockSignals(false);
}

FreeRotationContainer FreeRotationSettings::defaultSettings() const
{
    FreeRotationContainer prm;
    prm.angle     = d->angleInput->defaultValue();
    prm.antiAlias = true;
    prm.autoCrop  = d->autoCropCB->defaultIndex();
    return prm;
}

void FreeRotationSettings::readSettings(KConfigGroup& group)
{
    d->autoCropCB->setCurrentIndex(group.readEntry(d->configAutoCropTypeEntry, d->autoCropCB->defaultIndex()));
    d->antialiasInput->setChecked(group.readEntry(d->configAntiAliasingEntry, true));
    d->angleInput->slotReset();
    d->fineAngleInput->slotReset();
}

void FreeRotationSettings::writeSettings(KConfigGroup& group)
{
    FreeRotationContainer prm = settings();
    group.writeEntry(d->configAutoCropTypeEntry, d->autoCropCB->currentIndex());
    group.writeEntry(d->configAntiAliasingEntry, d->antialiasInput->isChecked());
}

}  // namespace Digikam
