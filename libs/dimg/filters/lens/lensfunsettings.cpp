/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lensfunsettings.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

namespace Digikam
{

class LensFunSettings::LensFunSettingsPriv
{
public:

    LensFunSettingsPriv() :
        configCCAEntry("CCA"),
        configVignettingEntry("Vignetting"),
        configCCIEntry("CCI"),
        configDistortionEntry("Distortion"),
        configGeometryEntry("Geometry"),

        filterCCA(0),
        filterVig(0),
        filterCCI(0),
        filterDist(0),
        filterGeom(0)
        {}

    const QString configCCAEntry;
    const QString configVignettingEntry;
    const QString configCCIEntry;
    const QString configDistortionEntry;
    const QString configGeometryEntry;

    QCheckBox*    filterCCA;
    QCheckBox*    filterVig;
    QCheckBox*    filterCCI;
    QCheckBox*    filterDist;
    QCheckBox*    filterGeom;
};

LensFunSettings::LensFunSettings(QWidget* parent)
               : QWidget(parent),
                 d(new LensFunSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(this);

    d->filterCCA  = new QCheckBox(i18n("Chromatic Aberration"));
    d->filterCCA->setWhatsThis(i18n("Chromatic aberration is easily recognized as color fringes "
                                    "towards the image corners. CA is due to a varying lens focus "
                                    "for different colors."));
    d->filterVig  = new QCheckBox(i18n("Vignetting"));
    d->filterVig->setWhatsThis(i18n("Vignetting refers to an image darkening, mostly in the corners. "
                                    "Optical and natural vignetting can be canceled out with this option, "
                                    "whereas mechanical vignetting will not be cured."));
    d->filterCCI  = new QCheckBox(i18n("Color Correction"));
    d->filterCCI->setWhatsThis(i18n("All lenses have a slight color tinge to them, "
                                    "mostly due to the anti-reflective coating. "
                                    "The tinge can be canceled when the respective data is known for the lens."));
    d->filterDist = new QCheckBox(i18n("Distortion"));
    d->filterDist->setWhatsThis(i18n("Distortion refers to an image deformation, which is most pronounced "
                                     "towards the corners. These Seidel aberrations are known as pincushion "
                                     "and barrel distortions."));
    d->filterGeom = new QCheckBox(i18n("Geometry"));
    d->filterGeom->setWhatsThis(i18n("Four geometries are handled here: Rectilinear (99 percent of all lenses), "
                                     "Fisheye, Cylindrical, Equirectangular."));

    grid->addWidget(d->filterCCA,  0, 0, 1, 2);
    grid->addWidget(d->filterVig,  1, 0, 1, 2);
    grid->addWidget(d->filterCCI,  2, 0, 1, 2);
    grid->addWidget(d->filterDist, 3, 0, 1, 2);
    grid->addWidget(d->filterGeom, 4, 0, 1, 2);
    grid->setRowStretch(5, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    connect(d->filterCCA, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->filterVig, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->filterCCI, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->filterDist, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->filterGeom, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));
}

LensFunSettings::~LensFunSettings()
{
    delete d;
}

void LensFunSettings::setEnabledCCA(bool b)
{
    d->filterCCA->setEnabled(b);
}

void LensFunSettings::setEnabledVig(bool b)
{
    d->filterVig->setEnabled(b);
}

void LensFunSettings::setEnabledCCI(bool b)
{
    d->filterCCI->setEnabled(b);
}

void LensFunSettings::setEnabledDist(bool b)
{
    d->filterDist->setEnabled(b);
}

void LensFunSettings::setEnabledGeom(bool b)
{
    d->filterGeom->setEnabled(b);
}

LensFunContainer LensFunSettings::settings() const
{
    LensFunContainer prm;

    prm.filterCCA  = (d->filterCCA->isChecked()  && d->filterCCA->isEnabled());
    prm.filterVig  = (d->filterVig->isChecked()  && d->filterVig->isEnabled());
    prm.filterCCI  = (d->filterCCI->isChecked()  && d->filterCCI->isEnabled());
    prm.filterDist = (d->filterDist->isChecked() && d->filterDist->isEnabled());
    prm.filterGeom = (d->filterGeom->isChecked() && d->filterGeom->isEnabled());

    return prm;
}

void LensFunSettings::setSettings(const LensFunContainer& settings)
{
    blockSignals(true);
    d->filterCCA->setChecked(settings.filterCCA);
    d->filterVig->setChecked(settings.filterVig);
    d->filterCCI->setChecked(settings.filterCCI);
    d->filterDist->setChecked(settings.filterDist);
    d->filterGeom->setChecked(settings.filterGeom);
    blockSignals(false);
}

void LensFunSettings::resetToDefault()
{
    setSettings(LensFunContainer());
}

LensFunContainer LensFunSettings::defaultSettings() const
{
    LensFunContainer prm;
    return prm;
}

void LensFunSettings::readSettings(KConfigGroup& group)
{
    LensFunContainer prm;
    LensFunContainer defaultPrm = defaultSettings();
    d->filterCCA->setChecked(group.readEntry(d->configCCAEntry,         true));
    d->filterVig->setChecked(group.readEntry(d->configVignettingEntry,  true));
    d->filterCCI->setChecked(group.readEntry(d->configCCIEntry,         true));
    d->filterDist->setChecked(group.readEntry(d->configDistortionEntry, true));
    d->filterGeom->setChecked(group.readEntry(d->configGeometryEntry,   true));
    setSettings(prm);
}

void LensFunSettings::writeSettings(KConfigGroup& group)
{
    LensFunContainer prm = settings();
    if ( d->filterCCA->isEnabled() )  group.writeEntry(d->configCCAEntry,        (d->filterCCA->isChecked()));
    if ( d->filterVig->isEnabled() )  group.writeEntry(d->configVignettingEntry, (d->filterVig->isChecked()));
    if ( d->filterCCI->isEnabled() )  group.writeEntry(d->configCCIEntry,        (d->filterCCI->isChecked()));
    if ( d->filterDist->isEnabled() ) group.writeEntry(d->configDistortionEntry, (d->filterDist->isChecked()));
    if ( d->filterGeom->isEnabled() ) group.writeEntry(d->configGeometryEntry,   (d->filterGeom->isChecked()));
}

}  // namespace Digikam
