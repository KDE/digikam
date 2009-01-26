/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-16
 * Description : digiKam image editor to adjust Hue, Saturation,
 *               and Lightness of picture.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

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

// KDE includes.

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

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "colorgradientwidget.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "hslmodifier.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "hspreviewwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

HSLTool::HSLTool(QObject* parent)
       : EditorTool(parent)
{
    setObjectName("adjusthsl");
    setToolName(i18n("Hue / Saturation / Lightness"));
    setToolIcon(SmallIcon("adjusthsl"));
    setToolHelp("hsladjusttool.anchor");

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("hsladjust Tool", 0,
                                      i18n("Here you can see the image "
                                           "Hue/Saturation/Lightness adjustments preview. "
                                           "You can pick color on image "
                                           "to see the color level corresponding on histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    m_HSSelector = new KHueSaturationSelector(m_gboxSettings->plainPage());
    m_HSSelector->setWhatsThis(i18n("Select the hue and saturation adjustments of the image."));
    m_HSSelector->setMinimumSize(256, 142);

    m_HSPreview = new HSPreviewWidget(m_gboxSettings->plainPage(), m_gboxSettings->spacingHint());
    m_HSPreview->setWhatsThis(i18n("You can see here a color preview of the hue and "
                                   "saturation adjustments."));
    m_HSPreview->setMinimumSize(256, 15);

    QLabel *label2 = new QLabel(i18n("Hue:"), m_gboxSettings->plainPage());
    m_hInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_hInput->setDecimals(0);
    m_hInput->input()->setRange(-180.0, 180.0, 1.0, true);
    m_hInput->setDefaultValue(0.0);
    m_hInput->setWhatsThis(i18n("Set here the hue adjustment of the image."));

    QLabel *label3 = new QLabel(i18n("Saturation:"), m_gboxSettings->plainPage());
    m_sInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_sInput->setDecimals(2);
    m_sInput->input()->setRange(-100.0, 100.0, 0.01, true);
    m_sInput->setDefaultValue(0.0);
    m_sInput->setWhatsThis(i18n("Set here the saturation adjustment of the image."));

    QLabel *label4 = new QLabel(i18n("Lightness:"), m_gboxSettings->plainPage());
    m_lInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_lInput->setDecimals(2);
    m_lInput->input()->setRange(-100.0, 100.0, 0.01, true);
    m_lInput->setDefaultValue(0.0);
    m_lInput->setWhatsThis(i18n("Set here the lightness adjustment of the image."));

    // -------------------------------------------------------------

    gridSettings->addWidget(m_HSSelector, 0, 0, 1, 5);
    gridSettings->addWidget(m_HSPreview,  1, 0, 1, 5);
    gridSettings->addWidget(label2,       2, 0, 1, 5);
    gridSettings->addWidget(m_hInput,     3, 0, 1, 5);
    gridSettings->addWidget(label3,       4, 0, 1, 5);
    gridSettings->addWidget(m_sInput,     5, 0, 1, 5);
    gridSettings->addWidget(label4,       6, 0, 1, 5);
    gridSettings->addWidget(m_lInput,     7, 0, 1, 5);
    gridSettings->setRowStretch(8, 10);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_HSSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_hInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_hInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotHChanged(double)));

    connect(m_sInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_sInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotSChanged(double)));

    connect(m_lInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    m_gboxSettings->enableButton(EditorToolSettings::Ok, false);
}

HSLTool::~HSLTool()
{
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void HSLTool::slotColorSelectedFromTarget(const DColor &color)
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void HSLTool::slotHSChanged(int h, int s)
{
    double hue = double(h);
    if (h >= 180 && h <= 359)
        hue = double(h) - 359.0;

    double sat = ((double)s * (200.0/255.0)) - 100.0;

    m_hInput->blockSignals(true);
    m_sInput->blockSignals(true);
    m_hInput->setValue(hue);
    m_sInput->setValue(sat);
    m_hInput->blockSignals(false);
    m_sInput->blockSignals(false);
    slotTimer();
}

void HSLTool::slotHChanged(double h)
{
    int hue = int(h);
    if (h >= -180 && h < 0)
        hue = int(h) + 359;

    m_HSSelector->blockSignals(true);
    m_HSSelector->setXValue(hue);
    m_HSSelector->blockSignals(false);
}

void HSLTool::slotSChanged(double s)
{
    int sat = (int)((s + 100.0) * (255.0/200.0));

    m_HSSelector->blockSignals(true);
    m_HSSelector->setYValue(sat);
    m_HSSelector->blockSignals(false);
}

void HSLTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hsladjust Tool");

    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)HistogramWidget::LogScaleHistogram));

    m_hInput->setValue(group.readEntry("HueAdjustment", m_hInput->defaultValue()));
    m_sInput->setValue(group.readEntry("SaturationAdjustment", m_sInput->defaultValue()));
    m_lInput->setValue(group.readEntry("LighnessAdjustment", m_lInput->defaultValue()));
    slotHChanged(m_hInput->value());
    slotSChanged(m_sInput->value());
}

void HSLTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hsladjust Tool");
    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());
    group.writeEntry("HueAdjustment", m_hInput->value());
    group.writeEntry("SaturationAdjustment", m_sInput->value());
    group.writeEntry("LighnessAdjustment", m_lInput->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void HSLTool::slotResetSettings()
{
    m_hInput->blockSignals(true);
    m_sInput->blockSignals(true);
    m_lInput->blockSignals(true);

    m_hInput->slotReset();
    m_sInput->slotReset();
    m_lInput->slotReset();

    slotHChanged(m_hInput->defaultValue());
    slotSChanged(m_sInput->defaultValue());

    m_hInput->blockSignals(false);
    m_sInput->blockSignals(false);
    m_lInput->blockSignals(false);

    slotEffect();
}

void HSLTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double hu  = m_hInput->value();
    double sa  = m_sInput->value();
    double lu  = m_lInput->value();

    m_gboxSettings->enableButton(EditorToolSettings::Ok,
                                (hu != 0.0 || sa != 0.0 || lu != 0.0));

    m_HSPreview->setHS(hu, sa);
    m_gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    DImg preview(w, h, sb, a, m_destinationPreviewData);
    HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.

    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void HSLTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double hu  = m_hInput->value();
    double sa  = m_sInput->value();
    double lu  = m_lInput->value();

    ImageIface* iface = m_previewWidget->imageIface();
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

