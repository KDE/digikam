/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#include "lut3dsettings.h"

// Qt includes

#include <QStandardPaths>
#include <QApplication>
#include <QDirIterator>
#include <QGridLayout>
#include <QStringList>
#include <QString>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dexpanderbox.h"
#include "previewlist.h"
#include "imageiface.h"
#include "dnuminput.h"

namespace Digikam
{

class Lut3DSettings::Private
{
public:

    Private() :
        correctionTools(0),
        bIntensity(0)
    {
    }

    static const QString configLut3DFilterEntry;
    static const QString configLut3DIntensityEntry;

    PreviewList*         correctionTools;
    DIntNumInput*        bIntensity;
    QStringList          luts;
};

const QString Lut3DSettings::Private::configLut3DFilterEntry(QLatin1String("Lut3D Color Correction Filter"));
const QString Lut3DSettings::Private::configLut3DIntensityEntry(QLatin1String("Lut3D Color Correction Intensity"));

// --------------------------------------------------------

Lut3DSettings::Lut3DSettings(QWidget* const parent, bool useGenericImg)
    : QWidget(parent),
      d(new Private)
{
    int idx;
    DImg thumbImage;

    findLuts();

    if (!useGenericImg)
    {
        ImageIface iface;
        thumbImage = iface.original()->smoothScale(128, 128, Qt::KeepAspectRatio);
    }
    else
    {
        thumbImage = DImg(QIcon::fromTheme(QLatin1String("image-x-generic")).pixmap(128).toImage());
    }

    d->correctionTools = new PreviewList(this);

    for (idx = 0; idx < d->luts.count(); idx++)
    {
        const QString &path = d->luts[idx];
        QFileInfo fi(path);

        d->correctionTools->addItem(new Lut3DFilter(&thumbImage, Lut3DContainer(path)),
                                        fi.baseName(), idx);
    }

    d->correctionTools->setFocus();

    // -------------------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QHBoxLayout* const intensityLayout = new QHBoxLayout();
    QLabel* const label                = new QLabel(i18n("Intensity:"));
    d->bIntensity                      = new DIntNumInput();
    d->bIntensity->setRange(1, 100, 1);
    d->bIntensity->setDefaultValue(100);
    d->bIntensity->setWhatsThis(i18n("Set here the intensity of the filter."));

    intensityLayout->addWidget(label);
    intensityLayout->addWidget(d->bIntensity);

    // -------------------------------------------------------------

    QVBoxLayout* const mainLayout      = new QVBoxLayout(parent);
    mainLayout->addWidget(d->correctionTools);
    mainLayout->addLayout(intensityLayout);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);

    connect(d->correctionTools, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->bIntensity, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

Lut3DSettings::~Lut3DSettings()
{
    delete d;
}

void Lut3DSettings::startPreviewFilters()
{
    d->correctionTools->startFilters();
}

Lut3DContainer Lut3DSettings::settings() const
{
    Lut3DContainer prm;

    prm.path = d->luts[d->correctionTools->currentId()];
    prm.intensity = d->bIntensity->value();

    return prm;
}

void Lut3DSettings::setSettings(const Lut3DContainer& settings)
{
    blockSignals(true);

    int filterId = d->luts.indexOf(settings.path);

    if (filterId == -1)
    {
        filterId = 0;
    }

    d->correctionTools->setCurrentId(filterId);
    d->bIntensity->setValue(settings.intensity);

    blockSignals(false);
}

void Lut3DSettings::resetToDefault()
{
    blockSignals(true);

    d->correctionTools->setCurrentId(0);
    d->bIntensity->slotReset();

    blockSignals(false);
}

Lut3DContainer Lut3DSettings::defaultSettings() const
{
    Lut3DContainer prm;

    prm.path = d->luts[0];
    prm.intensity = d->bIntensity->defaultValue();

    return prm;
}

void Lut3DSettings::readSettings(KConfigGroup& group)
{
    Lut3DContainer prm;
    Lut3DContainer defaultPrm = defaultSettings();

    prm.path = group.readEntry(d->configLut3DFilterEntry,         defaultPrm.path);
    prm.intensity = group.readEntry(d->configLut3DIntensityEntry, defaultPrm.intensity);

    setSettings(prm);
}

void Lut3DSettings::writeSettings(KConfigGroup& group)
{
    Lut3DContainer prm = settings();

    group.writeEntry(d->configLut3DFilterEntry,    prm.path);
    group.writeEntry(d->configLut3DIntensityEntry, prm.intensity);
}

void Lut3DSettings::findLuts()
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
}

}  // namespace Digikam
