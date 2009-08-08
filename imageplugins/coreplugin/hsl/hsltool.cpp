/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-16
 * Description : digiKam image editor to adjust Hue, Saturation,
 *               and Lightness of picture.
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


#include "hsltool.h"
#include "hsltool.moc"

// Qt includes

#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kcolordialog.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khuesaturationselect.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "hslmodifier.h"
#include "hspreviewwidget.h"
#include "imageiface.h"
#include "imagewidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class HSLToolPriv
{
public:

    HSLToolPriv()
    {
        destinationPreviewData    = 0;
        HSSelector                = 0;
        hInput                    = 0;
        sInput                    = 0;
        lInput                    = 0;
        HSPreview                 = 0;
        previewWidget             = 0;
        gboxSettings              = 0;
    }

    uchar*                  destinationPreviewData;

    KHueSaturationSelector* HSSelector;

    RDoubleNumInput*        hInput;
    RDoubleNumInput*        sInput;
    RDoubleNumInput*        lInput;

    HSPreviewWidget*        HSPreview;

    ImageWidget*            previewWidget;
    EditorToolSettings*     gboxSettings;
};

HSLTool::HSLTool(QObject* parent)
       : EditorTool(parent),
         d(new HSLToolPriv)
{
    setObjectName("adjusthsl");
    setToolName(i18n("Hue / Saturation / Lightness"));
    setToolIcon(SmallIcon("adjusthsl"));
    setToolHelp("hsladjusttool.anchor");

    d->destinationPreviewData = 0;

    d->previewWidget = new ImageWidget("hsladjust Tool", 0,
                                      i18n("The Hue/Saturation/Lightness adjustment preview "
                                           "is shown here. "
                                           "Picking a color on the image will show the "
                                           "corresponding color level on the histogram."));
    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);

    // -------------------------------------------------------------

    d->HSSelector = new KHueSaturationSelector();
    d->HSSelector->setWhatsThis(i18n("Select the hue and saturation adjustments of the image."));
    d->HSSelector->setMinimumSize(256, 142);

    d->HSPreview = new HSPreviewWidget(d->gboxSettings->plainPage(), d->gboxSettings->spacingHint());
    d->HSPreview->setWhatsThis(i18n("You can see here a color preview of the hue and "
                                   "saturation adjustments."));
    d->HSPreview->setMinimumSize(256, 15);

    QLabel *label2 = new QLabel(i18n("Hue:"));
    d->hInput      = new RDoubleNumInput();
    d->hInput->setDecimals(0);
    d->hInput->input()->setRange(-180.0, 180.0, 1.0, true);
    d->hInput->setDefaultValue(0.0);
    d->hInput->setWhatsThis(i18n("Set here the hue adjustment of the image."));

    QLabel *label3 = new QLabel(i18n("Saturation:"));
    d->sInput      = new RDoubleNumInput();
    d->sInput->setDecimals(2);
    d->sInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->sInput->setDefaultValue(0.0);
    d->sInput->setWhatsThis(i18n("Set here the saturation adjustment of the image."));

    QLabel *label4 = new QLabel(i18n("Lightness:"));
    d->lInput      = new RDoubleNumInput();
    d->lInput->setDecimals(2);
    d->lInput->input()->setRange(-100.0, 100.0, 0.01, true);
    d->lInput->setDefaultValue(0.0);
    d->lInput->setWhatsThis(i18n("Set here the lightness adjustment of the image."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->HSSelector, 0, 0, 1, 5);
    mainLayout->addWidget(d->HSPreview,  1, 0, 1, 5);
    mainLayout->addWidget(label2,        2, 0, 1, 5);
    mainLayout->addWidget(d->hInput,     3, 0, 1, 5);
    mainLayout->addWidget(label3,        4, 0, 1, 5);
    mainLayout->addWidget(d->sInput,     5, 0, 1, 5);
    mainLayout->addWidget(label4,        6, 0, 1, 5);
    mainLayout->addWidget(d->lInput,     7, 0, 1, 5);
    mainLayout->setRowStretch(8, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->HSSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(d->hInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->hInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotHChanged(double)));

    connect(d->sInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->sInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotSChanged(double)));

    connect(d->lInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    d->gboxSettings->enableButton(EditorToolSettings::Ok, false);
}

HSLTool::~HSLTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void HSLTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void HSLTool::slotHSChanged(int h, int s)
{
    double hue = double(h);
    if (h >= 180 && h <= 359)
        hue = double(h) - 359.0;

    double sat = ((double)s * (200.0/255.0)) - 100.0;

    d->hInput->blockSignals(true);
    d->sInput->blockSignals(true);
    d->hInput->setValue(hue);
    d->sInput->setValue(sat);
    d->hInput->blockSignals(false);
    d->sInput->blockSignals(false);
    slotTimer();
}

void HSLTool::slotHChanged(double h)
{
    int hue = int(h);
    if (h >= -180 && h < 0)
        hue = int(h) + 359;

    d->HSSelector->blockSignals(true);
    d->HSSelector->setXValue(hue);
    d->HSSelector->blockSignals(false);
}

void HSLTool::slotSChanged(double s)
{
    int sat = (int)((s + 100.0) * (255.0/200.0));

    d->HSSelector->blockSignals(true);
    d->HSSelector->setYValue(sat);
    d->HSSelector->blockSignals(false);
}

void HSLTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hsladjust Tool");

    d->gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)EditorToolSettings::LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)HistogramWidget::LogScaleHistogram));

    d->hInput->setValue(group.readEntry("HueAdjustment", d->hInput->defaultValue()));
    d->sInput->setValue(group.readEntry("SaturationAdjustment", d->sInput->defaultValue()));
    d->lInput->setValue(group.readEntry("LighnessAdjustment", d->lInput->defaultValue()));
    slotHChanged(d->hInput->value());
    slotSChanged(d->sInput->value());
}

void HSLTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hsladjust Tool");
    group.writeEntry("Histogram Channel", d->gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", d->gboxSettings->histogramBox()->scale());
    group.writeEntry("HueAdjustment", d->hInput->value());
    group.writeEntry("SaturationAdjustment", d->sInput->value());
    group.writeEntry("LighnessAdjustment", d->lInput->value());
    d->previewWidget->writeSettings();
    config->sync();
}

void HSLTool::slotResetSettings()
{
    d->hInput->blockSignals(true);
    d->sInput->blockSignals(true);
    d->lInput->blockSignals(true);

    d->hInput->slotReset();
    d->sInput->slotReset();
    d->lInput->slotReset();

    slotHChanged(d->hInput->defaultValue());
    slotSChanged(d->sInput->defaultValue());

    d->hInput->blockSignals(false);
    d->sInput->blockSignals(false);
    d->lInput->blockSignals(false);

    slotEffect();
}

void HSLTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double hu  = d->hInput->value();
    double sa  = d->sInput->value();
    double lu  = d->lInput->value();

    d->gboxSettings->enableButton(EditorToolSettings::Ok,
                                (hu != 0.0 || sa != 0.0 || lu != 0.0));

    d->HSPreview->setHS(hu, sa);
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    ImageIface* iface = d->previewWidget->imageIface();
    d->destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    DImg preview(w, h, sb, a, d->destinationPreviewData);
    HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(preview);
    iface->putPreviewImage(preview.bits());

    d->previewWidget->updatePreview();

    // Update histogram.

    memcpy(d->destinationPreviewData, preview.bits(), preview.numBytes());
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void HSLTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double hu  = d->hInput->value();
    double sa  = d->sInput->value();
    double lu  = d->lInput->value();

    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();
    DImg original(w, h, sb, a, data);
    delete [] data;

    HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(original);

    iface->putOriginalImage(i18n("HSL Adjustments"), original.bits());
    kapp->restoreOverrideCursor();
}

}  // namespace DigikamImagesPluginCore

