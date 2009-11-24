/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QFile>
#include <QImage>
#include <QString>
#include <QTextStream>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "waveletsnr.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "noisereductionsettings.h"
#include "version.h"

using namespace Digikam;

namespace DigikamImagesPluginCore
{

class NoiseReductionToolPriv
{
public:

    NoiseReductionToolPriv() :
        configGroupName("noisereduction Tool"),
        configSoftnessAdjustmentEntry("SoftnessInputAdjustment"),
        configThresholdAdjustmentEntry("ThresholdAdjustment"),
        nrSettings(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString          configGroupName;
    const QString          configSoftnessAdjustmentEntry;
    const QString          configThresholdAdjustmentEntry;

    NoiseReductionSettings* nrSettings;
    ImagePanelWidget*       previewWidget;
    EditorToolSettings*     gboxSettings;
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
                                EditorToolSettings::Try);

    d->gboxSettings->setTools(EditorToolSettings::PanIcon);
    d->nrSettings = new NoiseReductionSettings(d->gboxSettings);

    setToolSettings(d->gboxSettings);

    d->previewWidget = new ImagePanelWidget(470, 350, "noisereduction Tool", d->gboxSettings->panIconView());
    setToolView(d->previewWidget);

    init();
}

NoiseReductionTool::~NoiseReductionTool()
{
    delete d;
}

void NoiseReductionTool::renderingFinished()
{
    d->nrSettings->thresholdInput()->setEnabled(true);
    d->nrSettings->softnessInput()->setEnabled(true);
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->nrSettings->thresholdInput()->blockSignals(true);
    d->nrSettings->softnessInput()->blockSignals(true);

    d->nrSettings->thresholdInput()->setValue(group.readEntry(d->configThresholdAdjustmentEntry, d->nrSettings->thresholdInput()->defaultValue()));
    d->nrSettings->softnessInput()->setValue(group.readEntry(d->configSoftnessAdjustmentEntry,   d->nrSettings->softnessInput()->defaultValue()));

    d->nrSettings->thresholdInput()->blockSignals(false);
    d->nrSettings->softnessInput()->blockSignals(false);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configThresholdAdjustmentEntry, d->nrSettings->thresholdInput()->value());
    group.writeEntry(d->configSoftnessAdjustmentEntry,  d->nrSettings->softnessInput()->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    d->nrSettings->resetToDefault();
}

void NoiseReductionTool::prepareEffect()
{
    d->nrSettings->thresholdInput()->setEnabled(false);
    d->nrSettings->softnessInput()->setEnabled(false);

    DImg image = d->previewWidget->getOriginalRegionImage();
    WaveletsNRContainer prm;
    prm.thresholds[0] = d->nrSettings->thresholdInput()->value();
    prm.thresholds[1] = d->nrSettings->thresholdInput()->value();
    prm.thresholds[2] = d->nrSettings->thresholdInput()->value();
    prm.softness[0]   = d->nrSettings->softnessInput()->value();
    prm.softness[1]   = d->nrSettings->softnessInput()->value();
    prm.softness[2]   = d->nrSettings->softnessInput()->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new WaveletsNR(&image, this, prm)));
}

void NoiseReductionTool::prepareFinal()
{
    d->nrSettings->thresholdInput()->setEnabled(false);
    d->nrSettings->softnessInput()->setEnabled(false);

    WaveletsNRContainer prm;
    prm.thresholds[0] = d->nrSettings->thresholdInput()->value();
    prm.thresholds[1] = d->nrSettings->thresholdInput()->value();
    prm.thresholds[2] = d->nrSettings->thresholdInput()->value();
    prm.softness[0]   = d->nrSettings->softnessInput()->value();
    prm.softness[1]   = d->nrSettings->softnessInput()->value();
    prm.softness[2]   = d->nrSettings->softnessInput()->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new WaveletsNR(iface.getOriginalImg(), this, prm)));
}

void NoiseReductionTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void NoiseReductionTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Noise Reduction"), filter()->getTargetImage().bits());
}

}  // namespace DigikamImagesPluginCore
