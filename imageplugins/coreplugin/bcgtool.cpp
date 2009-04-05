/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-05
 * Description : digiKam image editor to adjust Brightness,
                 Contrast, and Gamma of picture.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "bcgtool.h"
#include "bcgtool.moc"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "bcgmodifier.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imagewidget.h"


using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

BCGTool::BCGTool(QObject* parent)
       : EditorTool(parent)
{
    setObjectName("bcgadjust");
    setToolName(i18n("Brightness / Contrast / Gamma"));
    setToolIcon(SmallIcon("contrast"));
    setToolHelp("bcgadjusttool.anchor");

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("bcgadjust Tool", 0,
                                      i18n("Here you can see the image "
                                           "brightness-contrast-gamma adjustments preview. "
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

    QLabel *label2 = new QLabel(i18n("Brightness:"), m_gboxSettings->plainPage());
    m_bInput       = new RIntNumInput(m_gboxSettings->plainPage());
    m_bInput->setRange(-100, 100, 1);
    m_bInput->setSliderEnabled(true);
    m_bInput->setDefaultValue(0);
    m_bInput->setWhatsThis( i18n("Set here the brightness adjustment of the image."));

    QLabel *label3 = new QLabel(i18n("Contrast:"), m_gboxSettings->plainPage());
    m_cInput       = new RIntNumInput(m_gboxSettings->plainPage());
    m_cInput->setRange(-100, 100, 1);
    m_cInput->setSliderEnabled(true);
    m_cInput->setDefaultValue(0);
    m_cInput->setWhatsThis( i18n("Set here the contrast adjustment of the image."));

    QLabel *label4 = new QLabel(i18n("Gamma:"), m_gboxSettings->plainPage());
    m_gInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_gInput->setDecimals(2);
    m_gInput->input()->setRange(0.1, 3.0, 0.01, true);
    m_gInput->setDefaultValue(1.0);
    m_gInput->setWhatsThis( i18n("Set here the gamma adjustment of the image."));

    // -------------------------------------------------------------

    gridSettings->addWidget(label2,   0, 0, 1, 5);
    gridSettings->addWidget(m_bInput, 1, 0, 1, 5);
    gridSettings->addWidget(label3,   2, 0, 1, 5);
    gridSettings->addWidget(m_cInput, 3, 0, 1, 5);
    gridSettings->addWidget(label4,   4, 0, 1, 5);
    gridSettings->addWidget(m_gInput, 5, 0, 1, 5);
    gridSettings->setRowStretch(6, 10);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_bInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_cInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_gInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    m_gboxSettings->enableButton(EditorToolSettings::Ok, false);
}

BCGTool::~BCGTool()
{
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void BCGTool::slotColorSelectedFromTarget( const DColor &color )
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void BCGTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("bcgadjust Tool");

    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)HistogramWidget::LogScaleHistogram));

    m_bInput->setValue(group.readEntry("BrightnessAdjustment", m_bInput->defaultValue()));
    m_cInput->setValue(group.readEntry("ContrastAdjustment", m_cInput->defaultValue()));
    m_gInput->setValue(group.readEntry("GammaAdjustment", m_gInput->defaultValue()));
}

void BCGTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("bcgadjust Tool");
    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());
    group.writeEntry("BrightnessAdjustment", m_bInput->value());
    group.writeEntry("ContrastAdjustment", m_cInput->value());
    group.writeEntry("GammaAdjustment", m_gInput->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void BCGTool::slotResetSettings()
{
    m_bInput->blockSignals(true);
    m_cInput->blockSignals(true);
    m_gInput->blockSignals(true);

    m_bInput->slotReset();
    m_cInput->slotReset();
    m_gInput->slotReset();

    m_bInput->blockSignals(false);
    m_cInput->blockSignals(false);
    m_gInput->blockSignals(false);

    slotEffect();
}

void BCGTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double b = (double)m_bInput->value()/250.0;
    double c = (double)(m_cInput->value()/100.0) + 1.00;
    double g = m_gInput->value();

    m_gboxSettings->enableButton(EditorToolSettings::Ok,
                                 ( b != 0.0 || c != 1.0 || g != 1.0 ));

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
    BCGModifier cmod;
    cmod.setGamma(g);
    cmod.setBrightness(b);
    cmod.setContrast(c);
    cmod.applyBCG(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.

    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void BCGTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = m_previewWidget->imageIface();

    double b = (double)m_bInput->value()/250.0;
    double c = (double)(m_cInput->value()/100.0) + 1.00;
    double g = m_gInput->value();

    iface->setOriginalBCG(b, c, g);
    kapp->restoreOverrideCursor();
}

}  // namespace DigikamImagesPluginCore
