/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
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


#include "antivignettingtool.h"
#include "antivignettingtool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>

// KDE includes

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

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "antivignetting.h"
#include "bcgmodifier.h"
#include "daboutdata.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamAntiVignettingImagesPlugin
{

class AntiVignettingToolPriv
{
public:

    AntiVignettingToolPriv()
    {
        maskPreviewLabel  = 0;
        brightnessInput   = 0;
        contrastInput     = 0;
        gammaInput        = 0;
        densityInput      = 0;
        powerInput        = 0;
        radiusInput       = 0;
        previewWidget     = 0;
        gboxSettings      = 0;
    }

    QLabel*             maskPreviewLabel;

    RIntNumInput*       brightnessInput;
    RIntNumInput*       contrastInput;

    RDoubleNumInput*    gammaInput;
    RDoubleNumInput*    densityInput;
    RDoubleNumInput*    powerInput;
    RDoubleNumInput*    radiusInput;

    ImageWidget*        previewWidget;
    EditorToolSettings* gboxSettings;
};

AntiVignettingTool::AntiVignettingTool(QObject* parent)
                  : EditorToolThreaded(parent),
                    d(new AntiVignettingToolPriv)
{
    setObjectName("antivignetting");
    setToolName(i18n("Vignetting Correction"));
    setToolIcon(SmallIcon("antivignetting"));

    d->previewWidget = new ImageWidget("antivignetting Tool", 0, QString(),
                                      false, ImageGuideWidget::HVGuideMode, false);

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());

    d->maskPreviewLabel = new QLabel(d->gboxSettings->plainPage());
    d->maskPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->maskPreviewLabel->setPixmap(QPixmap(120, 120));
    d->maskPreviewLabel->setWhatsThis(i18n("You can see here a thumbnail preview of the anti-vignetting "
                                           "mask applied to the image."));

    // -------------------------------------------------------------

    QLabel *label1  = new QLabel(i18n("Density:"), d->gboxSettings->plainPage());
    d->densityInput = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->densityInput->setDecimals(1);
    d->densityInput->input()->setRange(1.0, 20.0, 0.1, true);
    d->densityInput->setDefaultValue(2.0);
    d->densityInput->setWhatsThis(i18n("This value controls the degree of intensity attenuation "
                                       "by the filter at its point of maximum density."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Power:"), d->gboxSettings->plainPage());
    d->powerInput  = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->powerInput->setDecimals(1);
    d->powerInput->input()->setRange(0.1, 2.0, 0.1, true);
    d->powerInput->setDefaultValue(1.0);
    d->powerInput->setWhatsThis(i18n("This value is used as the exponent controlling the "
                                     "fall-off in density from the center of the filter to the periphery."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Radius:"), d->gboxSettings->plainPage());
    d->radiusInput = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->radiusInput->setDecimals(1);
    d->radiusInput->input()->setRange(-100.0, 100.0, 0.1, true);
    d->radiusInput->setDefaultValue(1.0);
    d->radiusInput->setWhatsThis(i18n("This value is the radius of the center filter. It is a "
                                      "multiple of the half-diagonal measure of the image, at which "
                                      "the density of the filter falls to zero."));

    KSeparator *line = new KSeparator (Qt::Horizontal, d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *label4     = new QLabel(i18n("Brightness:"), d->gboxSettings->plainPage());
    d->brightnessInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->brightnessInput->setRange(0, 100, 1);
    d->brightnessInput->setSliderEnabled(true);
    d->brightnessInput->setDefaultValue(0);
    d->brightnessInput->setWhatsThis(i18n("Set here the brightness re-adjustment of the target image."));

    // -------------------------------------------------------------

    QLabel *label5   = new QLabel(i18n("Contrast:"), d->gboxSettings->plainPage());
    d->contrastInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->contrastInput->setRange(0, 100, 1);
    d->contrastInput->setSliderEnabled(true);
    d->contrastInput->setDefaultValue(0);
    d->contrastInput->setWhatsThis(i18n("Set here the contrast re-adjustment of the target image."));

    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Gamma:"), d->gboxSettings->plainPage());
    d->gammaInput  = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->gammaInput->setDecimals(2);
    d->gammaInput->input()->setRange(0.1, 3.0, 0.01, true);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setWhatsThis(i18n("Set here the gamma re-adjustment of the target image."));

    // -------------------------------------------------------------

    gridSettings->addWidget(d->maskPreviewLabel, 0, 0, 1, 3);
    gridSettings->addWidget(label1,              1, 0, 1, 3);
    gridSettings->addWidget(d->densityInput,     2, 0, 1, 3);
    gridSettings->addWidget(label2,              3, 0, 1, 3);
    gridSettings->addWidget(d->powerInput,       4, 0, 1, 3);
    gridSettings->addWidget(label3,              5, 0, 1, 3);
    gridSettings->addWidget(d->radiusInput,      6, 0, 1, 3);
    gridSettings->addWidget(line,                7, 0, 1, 3);
    gridSettings->addWidget(label4,              8, 0, 1, 3);
    gridSettings->addWidget(d->brightnessInput,  9, 0, 1, 3);
    gridSettings->addWidget(label5,             10, 0, 1, 3);
    gridSettings->addWidget(d->contrastInput,   11, 0, 1, 3);
    gridSettings->addWidget(label6,             12, 0, 1, 3);
    gridSettings->addWidget(d->gammaInput,      13, 0, 1, 3);
    gridSettings->setRowStretch(14, 10);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->densityInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->powerInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->brightnessInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->contrastInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->gammaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
}

AntiVignettingTool::~AntiVignettingTool()
{
    delete d;
}

void AntiVignettingTool::renderingFinished()
{
    d->densityInput->setEnabled(true);
    d->powerInput->setEnabled(true);
    d->radiusInput->setEnabled(true);
    d->brightnessInput->setEnabled(true);
    d->contrastInput->setEnabled(true);
    d->gammaInput->setEnabled(true);
}

void AntiVignettingTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("antivignetting Tool");

    blockWidgetSignals(true);

    d->densityInput->setValue(group.readEntry("DensityAdjustment", d->densityInput->defaultValue()));
    d->powerInput->setValue(group.readEntry("PowerAdjustment", d->powerInput->defaultValue()));
    d->radiusInput->setValue(group.readEntry("RadiusAdjustment", d->radiusInput->defaultValue()));
    d->brightnessInput->setValue(group.readEntry("BrightnessAdjustment", d->brightnessInput->defaultValue()));
    d->contrastInput->setValue(group.readEntry("ContrastAdjustment", d->contrastInput->defaultValue()));
    d->gammaInput->setValue(group.readEntry("GammaAdjustment", d->gammaInput->defaultValue()));

    blockWidgetSignals(false);

    slotEffect();
}

void AntiVignettingTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("antivignetting Tool");
    group.writeEntry("DensityAdjustment", d->densityInput->value());
    group.writeEntry("PowerAdjustment", d->powerInput->value());
    group.writeEntry("RadiusAdjustment", d->radiusInput->value());
    group.writeEntry("BrightnessAdjustment", d->brightnessInput->value());
    group.writeEntry("ContrastAdjustment", d->contrastInput->value());
    group.writeEntry("GammaAdjustment", d->gammaInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void AntiVignettingTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->densityInput->slotReset();
    d->powerInput->slotReset();
    d->radiusInput->slotReset();
    d->brightnessInput->slotReset();
    d->contrastInput->slotReset();
    d->gammaInput->slotReset();

    blockWidgetSignals(false);

    slotEffect();
}

void AntiVignettingTool::prepareEffect()
{
    d->densityInput->setEnabled(false);
    d->powerInput->setEnabled(false);
    d->radiusInput->setEnabled(false);
    d->brightnessInput->setEnabled(false);
    d->contrastInput->setEnabled(false);
    d->gammaInput->setEnabled(false);

    double dens  = d->densityInput->value();
    double power = d->powerInput->value();
    double rad   = d->radiusInput->value();

    ImageIface* iface = d->previewWidget->imageIface();
    int orgWidth               = iface->originalWidth();
    int orgHeight              = iface->originalHeight();
    QSize ps(orgWidth, orgHeight);
    ps.scale(QSize(120, 120), Qt::ScaleMin);

    // Calc mask preview.
    DImg preview(ps.width(), ps.height(), false);
    memset(preview.bits(), 255, preview.numBytes());
    AntiVignetting maskPreview(&preview, 0, dens, power, rad, 0, 0, false);
    maskPreview.startFilterDirectly();       // Run filter without to use multithreading.
    QPixmap pix = maskPreview.getTargetImage().convertToPixmap();
    QPainter pt(&pix);
    pt.setPen(QPen(Qt::black, 1));
    pt.drawRect(0, 0, pix.width(), pix.height());
    pt.end();
    d->maskPreviewLabel->setPixmap(pix);

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new AntiVignetting(iface->getOriginalImg(), this, dens, power, rad, 0, 0, true)));
}

void AntiVignettingTool::prepareFinal()
{
    d->densityInput->setEnabled(false);
    d->powerInput->setEnabled(false);
    d->radiusInput->setEnabled(false);
    d->brightnessInput->setEnabled(false);
    d->contrastInput->setEnabled(false);
    d->gammaInput->setEnabled(false);

    double dens  = d->densityInput->value();
    double power = d->powerInput->value();
    double rad   = d->radiusInput->value();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new AntiVignetting(iface.getOriginalImg(), this, dens, power, rad, 0, 0, true)));
}

void AntiVignettingTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    DImg imDest       = filter()->getTargetImage();

    // Adjust Image BCG.

    double b = (double)(d->brightnessInput->value() / 100.0);
    double c = (double)(d->contrastInput->value()   / 100.0) + (double)(1.00);
    double g = d->gammaInput->value();

    BCGModifier cmod;
    cmod.setGamma(g);
    cmod.setBrightness(b);
    cmod.setContrast(c);
    cmod.applyBCG(imDest);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(), iface->previewHeight())).bits());
    d->previewWidget->updatePreview();
}

void AntiVignettingTool::putFinalData()
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Vignetting Correction"),
                           filter()->getTargetImage().bits());

    double b = (double)(d->brightnessInput->value() / 100.0);
    double c = (double)(d->contrastInput->value()   / 100.0) + (double)(1.00);
    double g = d->gammaInput->value();

    // Adjust Image BCG.
    iface.setOriginalBCG(b, c, g);
}

void AntiVignettingTool::blockWidgetSignals(bool b)
{
    d->densityInput->blockSignals(b);
    d->powerInput->blockSignals(b);
    d->radiusInput->blockSignals(b);
    d->brightnessInput->blockSignals(b);
    d->contrastInput->blockSignals(b);
    d->gammaInput->blockSignals(b);
}

}  // namespace DigikamAntiVignettingImagesPlugin
