/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : lens auto-fix batch tool.
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

#include "lensautofix.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <kseparator.h>

// Local includes

#include "dimg.h"
#include "lensfunsettings.h"
#include "lensfuncameraselector.h"
#include "lensfuniface.h"

namespace Digikam
{

class LensAutoFix::LensAutoFixPriv
{
public:

    LensAutoFixPriv()
        : settingsView(0),
          cameraSelector(0)
    {}

    LensFunSettings*       settingsView;
    LensFunCameraSelector* cameraSelector;
};

LensAutoFix::LensAutoFix(QObject* parent)
           : BatchTool("LensAutoFix", EnhanceTool, parent),
             d(new LensAutoFixPriv)
{
    setToolTitle(i18n("Lens Auto-fix"));
    setToolDescription(i18n("A tool to fix automatically lens distorsions"));
    setToolIcon(KIcon(SmallIcon("lensautofix")));

    QWidget* box      = new QWidget;
    d->cameraSelector = new LensFunCameraSelector();
    KSeparator* line  = new KSeparator(Qt::Horizontal);
    d->settingsView   = new LensFunSettings();
    d->cameraSelector->setPassiveMetadataUsage(true);
    d->cameraSelector->enableUseMetadata(true);

    QGridLayout* grid = new QGridLayout(box);
    grid->addWidget(d->cameraSelector, 0, 0, 1, 2);
    grid->addWidget(line,              1, 0, 1, 2);
    grid->addWidget(d->settingsView,   2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    setSettingsWidget(box);

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

LensAutoFix::~LensAutoFix()
{
    delete d;
}

BatchToolSettings LensAutoFix::defaultSettings()
{
    BatchToolSettings prm;

    prm.insert("UseMetadata",     true);
    prm.insert("filterCCA",       true);
    prm.insert("filterVig",       true);
    prm.insert("filterCCI",       true);
    prm.insert("filterDist",      true);
    prm.insert("filterGeom",      true);

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
    lfPrm.filterVig       = settings()["filterVig"].toBool();
    lfPrm.filterCCI       = settings()["filterCCI"].toBool();
    lfPrm.filterDist      = settings()["filterDist"].toBool();
    lfPrm.filterGeom      = settings()["filterGeom"].toBool();

    lfPrm.cropFactor      = settings()["cropFactor"].toDouble();
    lfPrm.focalLength     = settings()["focalLength"].toDouble();
    lfPrm.aperture        = settings()["aperture"].toDouble();
    lfPrm.subjectDistance = settings()["subjectDistance"].toDouble();

    QString cameraMake    = settings()["cameraMake"].toString();
    QString cameraModel   = settings()["cameraModel"].toString();
    QString lensModel     = settings()["lensModel"].toString();

    lfPrm.usedCamera      = d->cameraSelector->iface()->findCamera(cameraMake, cameraModel);
    lfPrm.usedLens        = d->cameraSelector->iface()->findLens(lensModel);

    d->cameraSelector->setSettings(lfPrm);
}

void LensAutoFix::slotSettingsChanged()
{
    // Update checkbox options about Lens corrections available.
    d->settingsView->setEnabledCCA(d->cameraSelector->useMetadata()  ? true : d->cameraSelector->settings().supportsCCA());
    d->settingsView->setEnabledVig(d->cameraSelector->useMetadata()  ? true : d->cameraSelector->settings().supportsVig());
    d->settingsView->setEnabledCCI(d->cameraSelector->useMetadata()  ? true : d->cameraSelector->settings().supportsVig());
    d->settingsView->setEnabledDist(d->cameraSelector->useMetadata() ? true : d->cameraSelector->settings().supportsDistortion());
    d->settingsView->setEnabledGeom(d->cameraSelector->useMetadata() ? true : d->cameraSelector->settings().supportsDistortion());

    BatchToolSettings prm;
    LensFunContainer  settings = d->cameraSelector->settings();

    prm.insert("UseMetadata",     (bool)d->cameraSelector->useMetadata());

    prm.insert("filterCCA",       (bool)settings.filterCCA);
    prm.insert("filterVig",       (bool)settings.filterVig);
    prm.insert("filterCCI",       (bool)settings.filterCCI);
    prm.insert("filterDist",      (bool)settings.filterDist);
    prm.insert("filterGeom",      (bool)settings.filterGeom);

    prm.insert("cropFactor",      (double)settings.cropFactor);
    prm.insert("focalLength",     (double)settings.focalLength);
    prm.insert("aperture",        (double)settings.aperture);
    prm.insert("subjectDistance", (double)settings.subjectDistance);

    prm.insert("cameraMake",      settings.usedCamera ? settings.usedCamera->Maker : QString());
    prm.insert("cameraModel",     settings.usedCamera ? settings.usedCamera->Model : QString());
    prm.insert("lensModel",       settings.usedLens ?   settings.usedLens->Model   : QString());

    BatchTool::slotSettingsChanged(prm);
}

bool LensAutoFix::toolOperations()
{
    if (!loadToDImg())
        return false;

    LensFunIface     iface;
    LensFunContainer prm;

    bool useMeta = settings()["UseMetadata"].toBool();
    if (useMeta)
    {
        DMetadata meta(image().getMetadata());
        bool      ret = iface.findFromMetadata(meta, prm);
        if (!ret) return false;
    }
    else
    {
        prm.filterCCA       = settings()["filterCCA"].toBool();
        prm.filterVig       = settings()["filterVig"].toBool();
        prm.filterCCI       = settings()["filterCCI"].toBool();
        prm.filterDist      = settings()["filterDist"].toBool();
        prm.filterGeom      = settings()["filterGeom"].toBool();

        prm.cropFactor      = settings()["cropFactor"].toDouble();
        prm.focalLength     = settings()["focalLength"].toDouble();
        prm.aperture        = settings()["aperture"].toDouble();
        prm.subjectDistance = settings()["subjectDistance"].toDouble();

        QString cameraMake  = settings()["cameraMake"].toString();
        QString cameraModel = settings()["cameraModel"].toString();
        QString lensModel   = settings()["lensModel"].toString();

        prm.usedCamera      = iface.findCamera(cameraMake, cameraModel);
        prm.usedLens        = iface.findLens(lensModel);
    }

    iface.setSettings(prm);
    LensFunFilter filter(&image(), 0L, &iface);
    filter.startFilterDirectly();
    image().putImageData(filter.getTargetImage().bits());
    KExiv2Data data = image().getMetadata();
    filter.registerSettingsToXmp(data, iface.settings());
    image().setMetadata(data);
    return savefromDImg();
}

} // namespace Digikam
