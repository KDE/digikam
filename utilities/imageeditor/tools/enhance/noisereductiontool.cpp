/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a tool to reduce CCD noise.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "noisereductiontool.h"

// Qt includes

#include <QString>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "nrestimate.h"
#include "nrsettings.h"
#include "nrfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace Digikam
{

class NoiseReductionTool::Private
{
public:

    Private() :
        configGroupName(QLatin1String("noisereduction Tool")),
        nrSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    const QString       configGroupName;

    NRSettings*         nrSettings;
    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

NoiseReductionTool::NoiseReductionTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("noisereduction"));
    setToolName(i18n("Noise Reduction"));
    setToolIcon(QIcon::fromTheme(QLatin1String("noisereduction")));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    d->nrSettings    = new NRSettings(d->gboxSettings->plainPage());
    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    connect(d->nrSettings, SIGNAL(signalEstimateNoise()),
            this, SLOT(slotEstimateNoise()));
}

NoiseReductionTool::~NoiseReductionTool()
{
    delete d;
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->nrSettings->readSettings(group);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->nrSettings->writeSettings(group);
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    d->nrSettings->resetToDefault();
}

void NoiseReductionTool::preparePreview()
{
    DImg image      = d->previewWidget->getOriginalRegionImage();
    NRContainer prm = d->nrSettings->settings();

    setFilter(new NRFilter(&image, this, prm));
}

void NoiseReductionTool::prepareFinal()
{
    NRContainer prm = d->nrSettings->settings();

    ImageIface iface;
    setFilter(new NRFilter(iface.original(), this, prm));
}

void NoiseReductionTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void NoiseReductionTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Noise Reduction"), filter()->filterAction(), filter()->getTargetImage());
}

void NoiseReductionTool::slotLoadSettings()
{
    d->nrSettings->loadSettings();
}

void NoiseReductionTool::slotSaveAsSettings()
{
    d->nrSettings->saveAsSettings();
}

void NoiseReductionTool::slotEstimateNoise()
{
    ImageIface iface;
    setAnalyser(new NREstimate(iface.original(), this));
}

void NoiseReductionTool::analyserCompleted()
{
    NREstimate* const tool = dynamic_cast<NREstimate*>(analyser());
    if (!tool) return;

    d->nrSettings->setSettings(tool->settings());
    qApp->restoreOverrideCursor();
    slotPreview();
}

}  // namespace Digikam
