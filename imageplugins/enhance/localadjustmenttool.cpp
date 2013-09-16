/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to edit portions of a image.
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
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

#include "localadjustmenttool.moc"

// Qt includes

#include <QString>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes

#include "dimg.h"
#include "localadjustmentfilter.h"
#include "localadjustmentsettings.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "localadjustmenttool.h"

namespace DigikamEnhanceImagePlugin
{

class LocalAdjustmentTool::Private
{
public:

    Private() :
        configGroupName("localadjustment Tool"),
        laSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    const QString       configGroupName;

    LASettings*         laSettings;
    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

LocalAdjustmentTool::LocalAdjustmentTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("localadjustment");
    setToolName(i18n("Local Adjustment"));
    setToolIcon(SmallIcon("localadjustment"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    d->laSettings    = new LASettings(d->gboxSettings->plainPage());
    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

//    connect(d->laSettings, SIGNAL(signalEstimateNoise()),
//            this, SLOT(slotEstimateNoise()));

    init();
}

LocalAdjustmentTool::~LocalAdjustmentTool()
{
    delete d;
}

void LocalAdjustmentTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->laSettings->readSettings(group);
}

void LocalAdjustmentTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->laSettings->writeSettings(group);
    group.sync();
}

void LocalAdjustmentTool::slotResetSettings()
{
    d->laSettings->resetToDefault();
}

void LocalAdjustmentTool::preparePreview()
{
    DImg image      = d->previewWidget->getOriginalRegionImage();
    LAContainer prm = d->laSettings->settings();

    setFilter(new LocalAdjustments(&image, this, prm));
}

void LocalAdjustmentTool::prepareFinal()
{
    LAContainer prm = d->laSettings->settings();

    ImageIface iface;
    setFilter(new LocalAdjustments(iface.original(), this, prm));
}

void LocalAdjustmentTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void LocalAdjustmentTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Local Adjustment"), filter()->filterAction(), filter()->getTargetImage());
}

/*
void LocalAdjustmentTool::slotLoadSettings()
{
    d->laSettings->loadSettings();
}

void LocalAdjustmentTool::slotSaveAsSettings()
{
    d->laSettings->saveAsSettings();
}


void LocalAdjustmentTool::slotEstimateNoise()
{
    ImageIface iface;
    setAnalyser(new NREstimate(iface.original(), this));
}

void LocalAdjustmentTool::analyserCompleted()
{
    NREstimate* const tool = dynamic_cast<NREstimate*>(analyser());
    if (!tool) return;

    d->laSettings->setSettings(tool->settings());
    kapp->restoreOverrideCursor();
    slotPreview();
}
*/

}  // namespace DigikamEnhanceImagePlugin

