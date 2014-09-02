/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : lens auto-fix batch tool.
 *
 * Copyright (C) 2010-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lensautofix.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kseparator.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "lensfunsettings.h"
#include "lensfuncameraselector.h"
#include "lensfuniface.h"

namespace Digikam
{

class LensAutoFix::Private
{
public:

    Private()
        : settingsView(0),
          cameraSelector(0)
    {}

    LensFunSettings*       settingsView;
    LensFunCameraSelector* cameraSelector;
};

LensAutoFix::LensAutoFix(QObject* const parent)
    : BatchTool("LensAutoFix", EnhanceTool, parent),
      d(new Private)
{
    setToolTitle(i18n("Lens Auto-Correction"));
    setToolDescription(i18n("Fix automatically lens distortions"));
    setToolIconName("lensautofix");
}

LensAutoFix::~LensAutoFix()
{
    delete d;
}

void LensAutoFix::registerSettingsWidget()
{
    m_settingsWidget   = new QWidget;
    QLabel* const note = new QLabel(i18n("<b>Use Metadata</b> option will parse images' information at "
                                         "queue run-time to find relevant lens features."));
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    d->cameraSelector      = new LensFunCameraSelector();
    KSeparator* const line = new KSeparator(Qt::Horizontal);
    d->settingsView        = new LensFunSettings();
    d->cameraSelector->setPassiveMetadataUsage(true);
    d->cameraSelector->setEnabledUseMetadata(true);

    QGridLayout* const grid = new QGridLayout(m_settingsWidget);
    grid->addWidget(note,              0, 0, 1, 2);
    grid->addWidget(d->cameraSelector, 1, 0, 1, 2);
    grid->addWidget(line,              2, 0, 1, 2);
    grid->addWidget(d->settingsView,   3, 0, 1, 2);
    grid->setRowStretch(4, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings LensAutoFix::defaultSettings()
{
    BatchToolSettings prm;

    prm.insert("UseMetadata",     true);
    prm.insert("filterCCA",       true);
    prm.insert("filterVIG",       true);
    prm.insert("filterDST",       true);
    prm.insert("filterGEO",       true);

    prm.insert("cropFactor",      -1.0);
    prm.insert("focalLength",     -1.0);
    prm.insert("aperture",        -1.0);
    prm.insert("subjectDistance", -1.0);

    prm.insert("cameraMake",      QString());
    prm.insert("cameraModel",     QString());
    prm.insert("lensModel",       QString());

    return prm;
}

void LensAutoFix::slotAssignSettings2Widget()
{
    d->cameraSelector->setUseMetadata(settings()["UseMetadata"].toBool());
    LensFunContainer lfPrm;
    lfPrm.filterCCA       = settings()["filterCCA"].toBool();
    lfPrm.filterVIG       = settings()["filterVIG"].toBool();
    lfPrm.filterDST       = settings()["filterDST"].toBool();
    lfPrm.filterGEO       = settings()["filterGEO"].toBool();

    lfPrm.cropFactor      = settings()["cropFactor"].toDouble();
    lfPrm.focalLength     = settings()["focalLength"].toDouble();
    lfPrm.aperture        = settings()["aperture"].toDouble();
    lfPrm.subjectDistance = settings()["subjectDistance"].toDouble();

    lfPrm.cameraMake      = settings()["cameraMake"].toString();
    lfPrm.cameraModel     = settings()["cameraModel"].toString();
    lfPrm.lensModel       = settings()["lensModel"].toString();

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

    prm.insert("UseMetadata", (bool)d->cameraSelector->useMetadata());

    prm.insert("filterCCA", (bool)settings.filterCCA);
    prm.insert("filterVIG", (bool)settings.filterVIG);
    prm.insert("filterDST", (bool)settings.filterDST);
    prm.insert("filterGEO", (bool)settings.filterGEO);

    prm.insert("cropFactor", (double)settings.cropFactor);
    prm.insert("focalLength", (double)settings.focalLength);
    prm.insert("aperture", (double)settings.aperture);
    prm.insert("subjectDistance", (double)settings.subjectDistance);

    prm.insert("cameraMake",      settings.cameraMake);
    prm.insert("cameraModel",     settings.cameraModel);
    prm.insert("lensModel",       settings.lensModel);

    BatchTool::slotSettingsChanged(prm);
}

bool LensAutoFix::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    LensFunContainer prm;

    bool useMeta = settings()["UseMetadata"].toBool();

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
        prm.filterCCA       = settings()["filterCCA"].toBool();
        prm.filterVIG       = settings()["filterVIG"].toBool();
        prm.filterDST       = settings()["filterDST"].toBool();
        prm.filterGEO       = settings()["filterGEO"].toBool();

        prm.cropFactor      = settings()["cropFactor"].toDouble();
        prm.focalLength     = settings()["focalLength"].toDouble();
        prm.aperture        = settings()["aperture"].toDouble();
        prm.subjectDistance = settings()["subjectDistance"].toDouble();

        QString cameraMake  = settings()["cameraMake"].toString();
        QString cameraModel = settings()["cameraModel"].toString();
        QString lensModel   = settings()["lensModel"].toString();
    }

    LensFunFilter filter(&image(), 0L, prm);
    applyFilter(&filter);

    KExiv2Data data = image().getMetadata();
    filter.registerSettingsToXmp(data);
    image().setMetadata(data);

    return savefromDImg();
}

} // namespace Digikam
