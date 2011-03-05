/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "noisereductiontool.moc"

// Qt includes

#include <QString>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes

#include "dimg.h"
#include "nrfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "nrsettings.h"

namespace DigikamEnhanceImagePlugin
{

class NoiseReductionTool::NoiseReductionToolPriv
{
public:

    NoiseReductionToolPriv() :
        configGroupName("noisereduction Tool"),
        nrSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    const QString       configGroupName;

    NRSettings*         nrSettings;
    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

NoiseReductionTool::NoiseReductionTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new NoiseReductionToolPriv)
{
    setObjectName("noisereduction");
    setToolName(i18n("Noise Reduction"));
    setToolIcon(SmallIcon("noisereduction"));

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

    init();
}

NoiseReductionTool::~NoiseReductionTool()
{
    delete d;
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->nrSettings->readSettings(group);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->nrSettings->writeSettings(group);
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    d->nrSettings->resetToDefault();
}

void NoiseReductionTool::prepareEffect()
{
    DImg image      = d->previewWidget->getOriginalRegionImage();
    NRContainer prm = d->nrSettings->settings();

    setFilter(new NRFilter(&image, this, prm));
}

void NoiseReductionTool::prepareFinal()
{
    NRContainer prm = d->nrSettings->settings();

    ImageIface iface(0, 0);
    setFilter(new NRFilter(iface.getOriginalImg(), this, prm));
}

void NoiseReductionTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void NoiseReductionTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Noise Reduction"), filter()->filterAction(), filter()->getTargetImage().bits());
}

void NoiseReductionTool::slotLoadSettings()
{
    d->nrSettings->loadSettings();
}

void NoiseReductionTool::slotSaveAsSettings()
{
    d->nrSettings->saveAsSettings();
}

}  // namespace DigikamEnhanceImagePlugin
