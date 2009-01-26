/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
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


#include "antivignettingtool.h"
#include "antivignettingtool.moc"

// Qt includes.

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "bcgmodifier.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "dimgimagefilters.h"
#include "antivignetting.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamAntiVignettingImagesPlugin
{

AntiVignettingTool::AntiVignettingTool(QObject* parent)
                  : EditorToolThreaded(parent)
{
    setObjectName("antivignetting");
    setToolName(i18n("Vignetting Correction"));
    setToolIcon(SmallIcon("antivignetting"));

    m_previewWidget = new ImageWidget("antivignetting Tool", 0, QString(),
                                      false, ImageGuideWidget::HVGuideMode, false);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    m_maskPreviewLabel = new QLabel(m_gboxSettings->plainPage());
    m_maskPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_maskPreviewLabel->setPixmap(QPixmap(120, 120));
    m_maskPreviewLabel->setWhatsThis(i18n("You can see here a thumbnail preview of the anti-vignetting "
                                          "mask applied to the image."));

    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Density:"), m_gboxSettings->plainPage());

    m_densityInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_densityInput->setDecimals(1);
    m_densityInput->input()->setRange(1.0, 20.0, 0.1, true);
    m_densityInput->setDefaultValue(2.0);
    m_densityInput->setWhatsThis(i18n("This value controls the degree of intensity attenuation "
                                      "by the filter at its point of maximum density."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Power:"), m_gboxSettings->plainPage());

    m_powerInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_powerInput->setDecimals(1);
    m_powerInput->input()->setRange(0.1, 2.0, 0.1, true);
    m_powerInput->setDefaultValue(1.0);
    m_powerInput->setWhatsThis(i18n("This value is used as the exponent controlling the "
                                    "fall-off in density from the center of the filter to the periphery."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Radius:"), m_gboxSettings->plainPage());

    m_radiusInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_radiusInput->setDecimals(1);
    m_radiusInput->input()->setRange(-100.0, 100.0, 0.1, true);
    m_radiusInput->setDefaultValue(1.0);
    m_radiusInput->setWhatsThis(i18n("This value is the radius of the center filter. It is a "
                                     "multiple of the half-diagonal measure of the image, at which "
                                     "the density of the filter falls to zero."));

    KSeparator *line = new KSeparator (Qt::Horizontal, m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("Brightness:"), m_gboxSettings->plainPage());

    m_brightnessInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_brightnessInput->setRange(0, 100, 1);
    m_brightnessInput->setSliderEnabled(true);
    m_brightnessInput->setDefaultValue(0);
    m_brightnessInput->setWhatsThis(i18n("Set here the brightness re-adjustment of the target image."));

    // -------------------------------------------------------------

    QLabel *label5 = new QLabel(i18n("Contrast:"), m_gboxSettings->plainPage());

    m_contrastInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_contrastInput->setRange(0, 100, 1);
    m_contrastInput->setSliderEnabled(true);
    m_contrastInput->setDefaultValue(0);
    m_contrastInput->setWhatsThis(i18n("Set here the contrast re-adjustment of the target image."));

    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Gamma:"), m_gboxSettings->plainPage());

    m_gammaInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_gammaInput->setDecimals(2);
    m_gammaInput->input()->setRange(0.1, 3.0, 0.01, true);
    m_gammaInput->setDefaultValue(1.0);
    m_gammaInput->setWhatsThis(i18n("Set here the gamma re-adjustment of the target image."));

    // -------------------------------------------------------------

    gridSettings->addWidget(m_maskPreviewLabel, 0,  0, 1, 3);
    gridSettings->addWidget(label1,             1,  0, 1, 3);
    gridSettings->addWidget(m_densityInput,     2,  0, 1, 3);
    gridSettings->addWidget(label2,             3,  0, 1, 3);
    gridSettings->addWidget(m_powerInput,       4,  0, 1, 3);
    gridSettings->addWidget(label3,             5,  0, 1, 3);
    gridSettings->addWidget(m_radiusInput,      6,  0, 1, 3);
    gridSettings->addWidget(line,               7,  0, 1, 3);
    gridSettings->addWidget(label4,             8,  0, 1, 3);
    gridSettings->addWidget(m_brightnessInput,  9,  0, 1, 3);
    gridSettings->addWidget(label5,             10, 0, 1, 3);
    gridSettings->addWidget(m_contrastInput,    11, 0, 1, 3);
    gridSettings->addWidget(label6,             12, 0, 1, 3);
    gridSettings->addWidget(m_gammaInput,       13, 0, 1, 3);
    gridSettings->setRowStretch(14, 10);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_densityInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_powerInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_brightnessInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_contrastInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_gammaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
}

AntiVignettingTool::~AntiVignettingTool()
{
}

void AntiVignettingTool::renderingFinished()
{
    m_densityInput->setEnabled(true);
    m_powerInput->setEnabled(true);
    m_radiusInput->setEnabled(true);
    m_brightnessInput->setEnabled(true);
    m_contrastInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
}

void AntiVignettingTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("antivignetting Tool");

    m_densityInput->blockSignals(true);
    m_powerInput->blockSignals(true);
    m_radiusInput->blockSignals(true);
    m_brightnessInput->blockSignals(true);
    m_contrastInput->blockSignals(true);
    m_gammaInput->blockSignals(true);

    m_densityInput->setValue(group.readEntry("DensityAdjustment", m_densityInput->defaultValue()));
    m_powerInput->setValue(group.readEntry("PowerAdjustment", m_powerInput->defaultValue()));
    m_radiusInput->setValue(group.readEntry("RadiusAdjustment", m_radiusInput->defaultValue()));
    m_brightnessInput->setValue(group.readEntry("BrightnessAdjustment", m_brightnessInput->defaultValue()));
    m_contrastInput->setValue(group.readEntry("ContrastAdjustment", m_contrastInput->defaultValue()));
    m_gammaInput->setValue(group.readEntry("GammaAdjustment", m_gammaInput->defaultValue()));

    m_densityInput->blockSignals(false);
    m_powerInput->blockSignals(false);
    m_radiusInput->blockSignals(false);
    m_brightnessInput->blockSignals(false);
    m_contrastInput->blockSignals(false);
    m_gammaInput->blockSignals(false);

    slotEffect();
}

void AntiVignettingTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("antivignetting Tool");
    group.writeEntry("DensityAdjustment", m_densityInput->value());
    group.writeEntry("PowerAdjustment", m_powerInput->value());
    group.writeEntry("RadiusAdjustment", m_radiusInput->value());
    group.writeEntry("BrightnessAdjustment", m_brightnessInput->value());
    group.writeEntry("ContrastAdjustment", m_contrastInput->value());
    group.writeEntry("GammaAdjustment", m_gammaInput->value());
    m_previewWidget->writeSettings();
    group.sync();
}

void AntiVignettingTool::slotResetSettings()
{
    m_densityInput->blockSignals(true);
    m_powerInput->blockSignals(true);
    m_radiusInput->blockSignals(true);
    m_brightnessInput->blockSignals(true);
    m_contrastInput->blockSignals(true);
    m_gammaInput->blockSignals(true);

    m_densityInput->slotReset();
    m_powerInput->slotReset();
    m_radiusInput->slotReset();
    m_brightnessInput->slotReset();
    m_contrastInput->slotReset();
    m_gammaInput->slotReset();

    m_densityInput->blockSignals(false);
    m_powerInput->blockSignals(false);
    m_radiusInput->blockSignals(false);
    m_brightnessInput->blockSignals(false);
    m_contrastInput->blockSignals(false);
    m_gammaInput->blockSignals(false);

    slotEffect();
}

void AntiVignettingTool::prepareEffect()
{
    m_densityInput->setEnabled(false);
    m_powerInput->setEnabled(false);
    m_radiusInput->setEnabled(false);
    m_brightnessInput->setEnabled(false);
    m_contrastInput->setEnabled(false);
    m_gammaInput->setEnabled(false);

    double d = m_densityInput->value();
    double p = m_powerInput->value();
    double r = m_radiusInput->value();

    ImageIface* iface = m_previewWidget->imageIface();
    int orgWidth               = iface->originalWidth();
    int orgHeight              = iface->originalHeight();
    QSize ps(orgWidth, orgHeight);
    ps.scale(QSize(120, 120), Qt::ScaleMin);

    // Calc mask preview.
    DImg preview(ps.width(), ps.height(), false);
    memset(preview.bits(), 255, preview.numBytes());
    AntiVignetting maskPreview(&preview, 0, d, p, r, 0, 0, false);
    maskPreview.startFilterDirectly();       // Run filter without to use multithreading.
    QPixmap pix = maskPreview.getTargetImage().convertToPixmap();
    QPainter pt(&pix);
    pt.setPen(QPen(Qt::black, 1));
    pt.drawRect(0, 0, pix.width(), pix.height());
    pt.end();
    m_maskPreviewLabel->setPixmap(pix);

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new AntiVignetting(iface->getOriginalImg(), this, d, p, r, 0, 0, true)));
}

void AntiVignettingTool::prepareFinal()
{
    m_densityInput->setEnabled(false);
    m_powerInput->setEnabled(false);
    m_radiusInput->setEnabled(false);
    m_brightnessInput->setEnabled(false);
    m_contrastInput->setEnabled(false);
    m_gammaInput->setEnabled(false);

    double d = m_densityInput->value();
    double p = m_powerInput->value();
    double r = m_radiusInput->value();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new AntiVignetting(iface.getOriginalImg(), this, d, p, r, 0, 0, true)));
}

void AntiVignettingTool::putPreviewData()
{
    ImageIface* iface = m_previewWidget->imageIface();
    DImg imDest       = filter()->getTargetImage();

    // Adjust Image BCG.

    double b = (double)(m_brightnessInput->value() / 100.0);
    double c = (double)(m_contrastInput->value()   / 100.0) + (double)(1.00);
    double g = m_gammaInput->value();

    BCGModifier cmod;
    cmod.setGamma(g);
    cmod.setBrightness(b);
    cmod.setContrast(c);
    cmod.applyBCG(imDest);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(), iface->previewHeight())).bits());
    m_previewWidget->updatePreview();
}

void AntiVignettingTool::putFinalData()
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Vignetting Correction"),
                           filter()->getTargetImage().bits());

    double b = (double)(m_brightnessInput->value() / 100.0);
    double c = (double)(m_contrastInput->value()   / 100.0) + (double)(1.00);
    double g = m_gammaInput->value();

    // Adjust Image BCG.
    iface.setOriginalBCG(b, c, g);
}

}  // namespace DigikamAntiVignettingImagesPlugin
