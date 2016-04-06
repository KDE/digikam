/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : lens auto-fix batch tool.
 *
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lensautofix.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dwidgetutils.h"
#include "dimg.h"
#include "lensfunsettings.h"
#include "lensfuncameraselector.h"
#include "lensfuniface.h"
#include "dexpanderbox.h"



namespace Digikam
{

class LensAutoFix::Private
{
public:

    Private()
        : settingsView(0),
          cameraSelector(0)
    {
    }

    LensFunSettings*       settingsView;
    LensFunCameraSelector* cameraSelector;
};

LensAutoFix::LensAutoFix(QObject* const parent)
    : BatchTool(QLatin1String("LensAutoFix"), EnhanceTool, parent),
      d(new Private)
{
    setToolTitle(i18n("Lens Auto-Correction"));
    setToolDescription(i18n("Fix automatically lens distortions"));
    setToolIconName(QLatin1String("lensautofix"));
}

LensAutoFix::~LensAutoFix()
{
    delete d;
}

void LensAutoFix::registerSettingsWidget()
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    m_settingsWidget   = new QWidget;
    QLabel* const note = new QLabel(i18n("<b>Use Metadata</b> option will parse images' information at "
                                         "queue run-time to find relevant lens features."));
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    d->cameraSelector       = new LensFunCameraSelector();
    DLineWidget* const line = new DLineWidget(Qt::Horizontal);
    d->settingsView         = new LensFunSettings();
    d->cameraSelector->setPassiveMetadataUsage(true);
    d->cameraSelector->setEnabledUseMetadata(true);

    QGridLayout* const grid = new QGridLayout(m_settingsWidget);
    grid->addWidget(note,              0, 0, 1, 2);
    grid->addWidget(d->cameraSelector, 1, 0, 1, 2);
    grid->addWidget(line,              2, 0, 1, 2);
    grid->addWidget(d->settingsView,   3, 0, 1, 2);
    grid->setRowStretch(4, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings LensAutoFix::defaultSettings()
{
    BatchToolSettings prm;

    prm.insert(QLatin1String("UseMetadata"),     true);
    prm.insert(QLatin1String("filterCCA"),       true);
    prm.insert(QLatin1String("filterVIG"),       true);
    prm.insert(QLatin1String("filterDST"),       true);
    prm.insert(QLatin1String("filterGEO"),       true);

    prm.insert(QLatin1String("cropFactor"),      -1.0);
    prm.insert(QLatin1String("focalLength"),     -1.0);
    prm.insert(QLatin1String("aperture"),        -1.0);
    prm.insert(QLatin1String("subjectDistance"), -1.0);

    prm.insert(QLatin1String("cameraMake"),      QString());
    prm.insert(QLatin1String("cameraModel"),     QString());
    prm.insert(QLatin1String("lensModel"),       QString());

    return prm;
}

void LensAutoFix::slotAssignSettings2Widget()
{
    d->cameraSelector->setUseMetadata(settings()[QLatin1String("UseMetadata")].toBool());
    LensFunContainer lfPrm;

    lfPrm.filterCCA       = settings()[QLatin1String("filterCCA")].toBool();
    lfPrm.filterVIG       = settings()[QLatin1String("filterVIG")].toBool();
    lfPrm.filterDST       = settings()[QLatin1String("filterDST")].toBool();
    lfPrm.filterGEO       = settings()[QLatin1String("filterGEO")].toBool();

    lfPrm.cropFactor      = settings()[QLatin1String("cropFactor")].toDouble();
    lfPrm.focalLength     = settings()[QLatin1String("focalLength")].toDouble();
    lfPrm.aperture        = settings()[QLatin1String("aperture")].toDouble();
    lfPrm.subjectDistance = settings()[QLatin1String("subjectDistance")].toDouble();

    lfPrm.cameraMake      = settings()[QLatin1String("cameraMake")].toString();
    lfPrm.cameraModel     = settings()[QLatin1String("cameraModel")].toString();
    lfPrm.lensModel       = settings()[QLatin1String("lensModel")].toString();

    d->cameraSelector->setSettings(lfPrm);
}

void LensAutoFix::slotSettingsChanged()
{
    // Update checkbox options about Lens corrections available.
    d->settingsView->setEnabledCCA(d->cameraSelector->useMetadata()  ? true : d->cameraSelector->iface()->supportsCCA());
    d->settingsView->setEnabledVig(d->cameraSelector->useMetadata()  ? true : d->cameraSelector->iface()->supportsVig());
    d->settingsView->setEnabledDist(d->cameraSelector->useMetadata() ? true : d->cameraSelector->iface()->supportsDistortion());
    d->settingsView->setEnabledGeom(d->cameraSelector->useMetadata() ? true : d->cameraSelector->iface()->supportsDistortion());

    BatchToolSettings prm;
    LensFunContainer  settings = d->cameraSelector->settings();

    prm.insert(QLatin1String("UseMetadata"),     (bool)d->cameraSelector->useMetadata());

    prm.insert(QLatin1String("filterCCA"),       (bool)settings.filterCCA);
    prm.insert(QLatin1String("filterVIG"),       (bool)settings.filterVIG);
    prm.insert(QLatin1String("filterDST"),       (bool)settings.filterDST);
    prm.insert(QLatin1String("filterGEO"),       (bool)settings.filterGEO);

    prm.insert(QLatin1String("cropFactor"),      (double)settings.cropFactor);
    prm.insert(QLatin1String("focalLength"),     (double)settings.focalLength);
    prm.insert(QLatin1String("aperture"),        (double)settings.aperture);
    prm.insert(QLatin1String("subjectDistance"), (double)settings.subjectDistance);

    prm.insert(QLatin1String("cameraMake"),      settings.cameraMake);
    prm.insert(QLatin1String("cameraModel"),     settings.cameraModel);
    prm.insert(QLatin1String("lensModel"),       settings.lensModel);

    BatchTool::slotSettingsChanged(prm);
}

bool LensAutoFix::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    LensFunContainer prm;

    bool useMeta = settings()[QLatin1String("UseMetadata")].toBool();

    if (useMeta)
    {
        LensFunIface iface;
        DMetadata    meta(image().getMetadata());
        LensFunIface::MetadataMatch ret = iface.findFromMetadata(meta);
        prm                             = iface.settings();

        if (ret != LensFunIface::MetadataExactMatch)
        {
            setErrorDescription(i18n("Cannot find all lens information to process lens auto-corrections"));
            return false;
        }
    }
    else
    {
        prm.filterCCA       = settings()[QLatin1String("filterCCA")].toBool();
        prm.filterVIG       = settings()[QLatin1String("filterVIG")].toBool();
        prm.filterDST       = settings()[QLatin1String("filterDST")].toBool();
        prm.filterGEO       = settings()[QLatin1String("filterGEO")].toBool();

        prm.cropFactor      = settings()[QLatin1String("cropFactor")].toDouble();
        prm.focalLength     = settings()[QLatin1String("focalLength")].toDouble();
        prm.aperture        = settings()[QLatin1String("aperture")].toDouble();
        prm.subjectDistance = settings()[QLatin1String("subjectDistance")].toDouble();

        prm.cameraMake      = settings()[QLatin1String("cameraMake")].toString();
        prm.cameraModel     = settings()[QLatin1String("cameraModel")].toString();
        prm.lensModel       = settings()[QLatin1String("lensModel")].toString();
    }

    LensFunFilter filter(&image(), 0L, prm);
    applyFilter(&filter);

    MetaEngineData data = image().getMetadata();
    filter.registerSettingsToXmp(data);
    image().setMetadata(data);

    return savefromDImg();
}

} // namespace Digikam
