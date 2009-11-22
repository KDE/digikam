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

#include <QCheckBox>
#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
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

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "dimgwaveletsnr.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"

using namespace KDcrawIface;
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
        softnessInput(0),
        thresholdInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configSoftnessAdjustmentEntry;
    const QString       configThresholdAdjustmentEntry;

    RDoubleNumInput*    softnessInput;
    RDoubleNumInput*    thresholdInput;
    ImagePanelWidget*   previewWidget;
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
                                EditorToolSettings::Try);

    d->gboxSettings->setTools(EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout( d->gboxSettings->plainPage() );

    // -------------------------------------------------------------

    QLabel *label1    = new QLabel(i18n("Threshold:"));
    d->thresholdInput = new RDoubleNumInput;
    d->thresholdInput->setDecimals(2);
    d->thresholdInput->input()->setRange(0.0, 10.0, 0.1, true);
    d->thresholdInput->setDefaultValue(1.2);
    d->thresholdInput->setWhatsThis(i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                                         "the image in a range from 0.0 (none) to 10.0. "
                                         "The threshold is the value below which everything is considered noise."));

    // -------------------------------------------------------------

    QLabel *label2   = new QLabel(i18n("Softness:"));
    d->softnessInput = new RDoubleNumInput;
    d->softnessInput->setDecimals(1);
    d->softnessInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->softnessInput->setDefaultValue(0.1);
    d->softnessInput->setWhatsThis(i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                                        "(soft as opposed to hard thresholding). The higher the softness "
                                        "the more noise remains in the image."));

    grid->addWidget(label1,            0, 0, 1, 1);
    grid->addWidget(d->thresholdInput, 0, 1, 1, 1);
    grid->addWidget(label2,            1, 0, 1, 1);
    grid->addWidget(d->softnessInput,  1, 1, 1, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

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
    d->thresholdInput->setEnabled(true);
    d->softnessInput->setEnabled(true);
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->thresholdInput->blockSignals(true);
    d->softnessInput->blockSignals(true);

    d->thresholdInput->setValue(group.readEntry(d->configThresholdAdjustmentEntry, d->thresholdInput->defaultValue()));
    d->softnessInput->setValue(group.readEntry(d->configSoftnessAdjustmentEntry,   d->softnessInput->defaultValue()));

    d->thresholdInput->blockSignals(false);
    d->softnessInput->blockSignals(false);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configThresholdAdjustmentEntry, d->thresholdInput->value());
    group.writeEntry(d->configSoftnessAdjustmentEntry,  d->softnessInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    d->thresholdInput->slotReset();
    d->softnessInput->slotReset();
}

void NoiseReductionTool::prepareEffect()
{
    d->thresholdInput->setEnabled(false);
    d->softnessInput->setEnabled(false);

    double th  = d->thresholdInput->value();
    double so  = d->softnessInput->value();
    DImg image = d->previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgWaveletsNR(&image, this, th, so)));
}

void NoiseReductionTool::prepareFinal()
{
    d->thresholdInput->setEnabled(false);
    d->softnessInput->setEnabled(false);

    double th = d->thresholdInput->value();
    double so = d->softnessInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter *>(new DImgWaveletsNR(iface.getOriginalImg(), this, th, so)));
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
