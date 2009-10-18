/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
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


#include "rgbtool.h"
#include "rgbtool.moc"

// Qt includes

#include <QCheckBox>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
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

#include "colorgradientwidget.h"
#include "colormodifier.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imagewidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class RGBToolPriv
{
public:

    RGBToolPriv() :
        destinationPreviewData(0),
        rSlider(0),
        gSlider(0),
        bSlider(0),
        channelCB(0),
        rInput(0),
        gInput(0),
        bInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    uchar*               destinationPreviewData;

    QSlider*             rSlider;
    QSlider*             gSlider;
    QSlider*             bSlider;

    KComboBox*           channelCB;

    RIntNumInput*        rInput;
    RIntNumInput*        gInput;
    RIntNumInput*        bInput;

    ImageWidget*         previewWidget;
    EditorToolSettings*  gboxSettings;
};

RGBTool::RGBTool(QObject* parent)
       : EditorTool(parent),
         d(new RGBToolPriv)
{
    setObjectName("colorbalance");
    setToolName(i18n("Color Balance"));
    setToolIcon(SmallIcon("adjustrgb"));

    d->destinationPreviewData = 0;

    d->previewWidget = new ImageWidget("colorbalance Tool", 0,
                                      i18n("The image color-balance adjustment preview "
                                           "is shown here. "
                                           "Picking a color on the image will show the "
                                           "corresponding color level on the histogram."));
    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *labelCyan = new QLabel(i18n("Cyan"), d->gboxSettings->plainPage());
    labelCyan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->rSlider = new QSlider(Qt::Horizontal, d->gboxSettings->plainPage());
    d->rSlider->setValue(0);
    d->rSlider->setRange(-100, 100);
    d->rSlider->setPageStep(1);
    d->rSlider->setTickPosition(QSlider::TicksBelow);
    d->rSlider->setTickInterval(20);
    d->rSlider->setWhatsThis(i18n("Set here the cyan/red color adjustment of the image."));

    QLabel *labelRed = new QLabel(i18n("Red"), d->gboxSettings->plainPage());
    labelRed->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->rInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->rInput->setRange(-100, 100, 1);
    d->rInput->setSliderEnabled(false);
    d->rInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelMagenta = new QLabel(i18n("Magenta"), d->gboxSettings->plainPage());
    labelMagenta->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->gSlider = new QSlider(Qt::Horizontal, d->gboxSettings->plainPage());
    d->gSlider->setValue(0);
    d->gSlider->setRange(-100, 100);
    d->gSlider->setPageStep(1);
    d->gSlider->setTickPosition(QSlider::TicksBelow);
    d->gSlider->setTickInterval(20);
    d->gSlider->setWhatsThis(i18n("Set here the magenta/green color adjustment of the image."));

    QLabel *labelGreen = new QLabel(i18n("Green"), d->gboxSettings->plainPage());
    labelGreen->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->gInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->gInput->setRange(-100, 100, 1);
    d->gInput->setSliderEnabled(false);
    d->gInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelYellow = new QLabel(i18n("Yellow"), d->gboxSettings->plainPage());
    labelYellow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->bSlider = new QSlider(Qt::Horizontal, d->gboxSettings->plainPage());
    d->bSlider->setValue(0);
    d->bSlider->setRange(-100, 100);
    d->bSlider->setPageStep(1);
    d->bSlider->setTickPosition(QSlider::TicksBelow);
    d->bSlider->setTickInterval(20);
    d->bSlider->setWhatsThis(i18n("Set here the yellow/blue color adjustment of the image."));

    QLabel *labelBlue = new QLabel(i18n("Blue"), d->gboxSettings->plainPage());
    labelBlue->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->bInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->bInput->setRange(-100, 100, 1);
    d->bInput->setSliderEnabled(false);
    d->bInput->setDefaultValue(0);

    // -------------------------------------------------------------

    gridSettings->addWidget(labelCyan,      0, 0, 1, 1);
    gridSettings->addWidget(d->rSlider,     0, 1, 1, 1);
    gridSettings->addWidget(labelRed,       0, 2, 1, 1);
    gridSettings->addWidget(d->rInput,      0, 3, 1, 1);
    gridSettings->addWidget(labelMagenta,   1, 0, 1, 1);
    gridSettings->addWidget(d->gSlider,     1, 1, 1, 1);
    gridSettings->addWidget(labelGreen,     1, 2, 1, 1);
    gridSettings->addWidget(d->gInput,      1, 3, 1, 1);
    gridSettings->addWidget(labelYellow,    2, 0, 1, 1);
    gridSettings->addWidget(d->bSlider,     2, 1, 1, 1);
    gridSettings->addWidget(labelBlue,      2, 2, 1, 1);
    gridSettings->addWidget(d->bInput,      2, 3, 1, 1);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());
    gridSettings->setRowStretch(3, 10);

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(d->rSlider, SIGNAL(valueChanged(int)),
            d->rInput, SLOT(setValue(int)));

    connect(d->rInput, SIGNAL(valueChanged (int)),
            d->rSlider, SLOT(setValue(int)));

    connect(d->rInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->gSlider, SIGNAL(valueChanged(int)),
            d->gInput, SLOT(setValue(int)));

    connect(d->gInput, SIGNAL(valueChanged (int)),
            d->gSlider, SLOT(setValue(int)));

    connect(d->gInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->bSlider, SIGNAL(valueChanged(int)),
            d->bInput, SLOT(setValue(int)));

    connect(d->bInput, SIGNAL(valueChanged (int)),
            d->bSlider, SLOT(setValue(int)));

    connect(d->bInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    d->gboxSettings->enableButton(EditorToolSettings::Ok, false);
}

RGBTool::~RGBTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void RGBTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void RGBTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("colorbalance Tool");

    d->gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)LogScaleHistogram));

    int r = group.readEntry("RedAdjustment", d->rInput->defaultValue());
    int g = group.readEntry("GreenAdjustment", d->gInput->defaultValue());
    int b = group.readEntry("BlueAdjustment", d->bInput->defaultValue());
    adjustSliders(r, g, b);
}

void RGBTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("colorbalance Tool");
    group.writeEntry("Histogram Channel", d->gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", d->gboxSettings->histogramBox()->scale());
    group.writeEntry("RedAdjustment", d->rSlider->value());
    group.writeEntry("GreenAdjustment", d->gInput->value());
    group.writeEntry("BlueAdjustment", d->bInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void RGBTool::slotResetSettings()
{
    int r = d->rInput->defaultValue();
    int g = d->gInput->defaultValue();
    int b = d->bInput->defaultValue();

    adjustSliders(r, g, b);
}

void RGBTool::adjustSliders(int r, int g, int b)
{
    d->rSlider->blockSignals(true);
    d->gSlider->blockSignals(true);
    d->bSlider->blockSignals(true);
    d->rInput->blockSignals(true);
    d->gInput->blockSignals(true);
    d->bInput->blockSignals(true);

    d->rSlider->setValue(r);
    d->gSlider->setValue(g);
    d->bSlider->setValue(b);
    d->rInput->setValue(r);
    d->gInput->setValue(g);
    d->bInput->setValue(b);

    d->rSlider->blockSignals(false);
    d->gSlider->blockSignals(false);
    d->bSlider->blockSignals(false);
    d->rInput->blockSignals(false);
    d->gInput->blockSignals(false);
    d->bInput->blockSignals(false);

    slotEffect();
}

void RGBTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    d->gboxSettings->enableButton(EditorToolSettings::Ok,
                                (d->rInput->value() != 0 ||
                                 d->gInput->value() != 0 ||
                                 d->bInput->value() != 0));

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    ImageIface* iface = d->previewWidget->imageIface();
    d->destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool alpha                 = iface->previewHasAlpha();
    bool sixteenBit            = iface->previewSixteenBit();

    double r = ((double)d->rInput->value() + 100.0)/100.0;
    double g = ((double)d->gInput->value() + 100.0)/100.0;
    double b = ((double)d->bInput->value() + 100.0)/100.0;
    double a = 1.0;

    DImg preview(w, h, sixteenBit, alpha, d->destinationPreviewData);
    ColorModifier cmod;
    cmod.applyColorModifier(preview, r, g, b, a);
    iface->putPreviewImage(preview.bits());

    d->previewWidget->updatePreview();

    // Update histogram.
    memcpy(d->destinationPreviewData, preview.bits(), preview.numBytes());
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
                                            w, h, sixteenBit, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void RGBTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double r = ((double)d->rInput->value() + 100.0)/100.0;
    double g = ((double)d->gInput->value() + 100.0)/100.0;
    double b = ((double)d->bInput->value() + 100.0)/100.0;
    double a = 1.0;

    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool alpha                 = iface->originalHasAlpha();
    bool sixteenBit            = iface->originalSixteenBit();
    DImg original(w, h, sixteenBit, alpha, data);
    delete [] data;

    ColorModifier cmod;
    cmod.applyColorModifier(original, r, g, b, a);

    iface->putOriginalImage(i18n("Color Balance"), original.bits());
    kapp->restoreOverrideCursor();
}

}  // namespace DigikamImagesPluginCore

