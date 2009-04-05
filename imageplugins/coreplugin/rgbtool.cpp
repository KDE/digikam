/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imagewidget.h"


using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

RGBTool::RGBTool(QObject* parent)
       : EditorTool(parent)
{
    setObjectName("colorbalance");
    setToolName(i18n("Color Balance"));
    setToolIcon(SmallIcon("adjustrgb"));

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("colorbalance Tool", 0,
                                      i18n("The image color-balance adjustment preview "
                                           "is shown here. "
                                           "Picking a color on the image will show the "
                                           "corresponding color level on the histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *labelCyan = new QLabel(i18n("Cyan"), m_gboxSettings->plainPage());
    labelCyan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_rSlider = new QSlider(Qt::Horizontal, m_gboxSettings->plainPage());
    m_rSlider->setValue(0);
    m_rSlider->setRange(-100, 100);
    m_rSlider->setPageStep(1);
    m_rSlider->setTickPosition(QSlider::TicksBelow);
    m_rSlider->setTickInterval(20);
    m_rSlider->setWhatsThis(i18n("Set here the cyan/red color adjustment of the image."));

    QLabel *labelRed = new QLabel(i18n("Red"), m_gboxSettings->plainPage());
    labelRed->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_rInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_rInput->setRange(-100, 100, 1);
    m_rInput->setSliderEnabled(false);
    m_rInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelMagenta = new QLabel(i18n("Magenta"), m_gboxSettings->plainPage());
    labelMagenta->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_gSlider = new QSlider(Qt::Horizontal, m_gboxSettings->plainPage());
    m_gSlider->setValue(0);
    m_gSlider->setRange(-100, 100);
    m_gSlider->setPageStep(1);
    m_gSlider->setTickPosition(QSlider::TicksBelow);
    m_gSlider->setTickInterval(20);
    m_gSlider->setWhatsThis(i18n("Set here the magenta/green color adjustment of the image."));

    QLabel *labelGreen = new QLabel(i18n("Green"), m_gboxSettings->plainPage());
    labelGreen->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_gInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_gInput->setRange(-100, 100, 1);
    m_gInput->setSliderEnabled(false);
    m_gInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelYellow = new QLabel(i18n("Yellow"), m_gboxSettings->plainPage());
    labelYellow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_bSlider = new QSlider(Qt::Horizontal, m_gboxSettings->plainPage());
    m_bSlider->setValue(0);
    m_bSlider->setRange(-100, 100);
    m_bSlider->setPageStep(1);
    m_bSlider->setTickPosition(QSlider::TicksBelow);
    m_bSlider->setTickInterval(20);
    m_bSlider->setWhatsThis(i18n("Set here the yellow/blue color adjustment of the image."));

    QLabel *labelBlue = new QLabel(i18n("Blue"), m_gboxSettings->plainPage());
    labelBlue->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_bInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_bInput->setRange(-100, 100, 1);
    m_bInput->setSliderEnabled(false);
    m_bInput->setDefaultValue(0);

    // -------------------------------------------------------------

    gridSettings->addWidget(labelCyan,      0, 0, 1, 1);
    gridSettings->addWidget(m_rSlider,      0, 1, 1, 1);
    gridSettings->addWidget(labelRed,       0, 2, 1, 1);
    gridSettings->addWidget(m_rInput,       0, 3, 1, 1);
    gridSettings->addWidget(labelMagenta,   1, 0, 1, 1);
    gridSettings->addWidget(m_gSlider,      1, 1, 1, 1);
    gridSettings->addWidget(labelGreen,     1, 2, 1, 1);
    gridSettings->addWidget(m_gInput,       1, 3, 1, 1);
    gridSettings->addWidget(labelYellow,    2, 0, 1, 1);
    gridSettings->addWidget(m_bSlider,      2, 1, 1, 1);
    gridSettings->addWidget(labelBlue,      2, 2, 1, 1);
    gridSettings->addWidget(m_bInput,       2, 3, 1, 1);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());
    gridSettings->setRowStretch(3, 10);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_rSlider, SIGNAL(valueChanged(int)),
            m_rInput, SLOT(setValue(int)));
    connect(m_rInput, SIGNAL(valueChanged (int)),
            m_rSlider, SLOT(setValue(int)));
    connect(m_rInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_gSlider, SIGNAL(valueChanged(int)),
            m_gInput, SLOT(setValue(int)));
    connect(m_gInput, SIGNAL(valueChanged (int)),
            m_gSlider, SLOT(setValue(int)));
    connect(m_gInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_bSlider, SIGNAL(valueChanged(int)),
            m_bInput, SLOT(setValue(int)));
    connect(m_bInput, SIGNAL(valueChanged (int)),
            m_bSlider, SLOT(setValue(int)));
    connect(m_bInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    m_gboxSettings->enableButton(EditorToolSettings::Ok, false);
}

RGBTool::~RGBTool()
{
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void RGBTool::slotColorSelectedFromTarget( const DColor &color )
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void RGBTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("colorbalance Tool");

    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)HistogramWidget::LogScaleHistogram));

    int r = group.readEntry("RedAdjustment", m_rInput->defaultValue());
    int g = group.readEntry("GreenAdjustment", m_gInput->defaultValue());
    int b = group.readEntry("BlueAdjustment", m_bInput->defaultValue());
    adjustSliders(r, g, b);
}

void RGBTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("colorbalance Tool");
    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());
    group.writeEntry("RedAdjustment", m_rSlider->value());
    group.writeEntry("GreenAdjustment", m_gInput->value());
    group.writeEntry("BlueAdjustment", m_bInput->value());
    m_previewWidget->writeSettings();
    group.sync();
}

void RGBTool::slotResetSettings()
{
    int r = m_rInput->defaultValue();
    int g = m_gInput->defaultValue();
    int b = m_bInput->defaultValue();

    adjustSliders(r, g, b);
}

void RGBTool::adjustSliders(int r, int g, int b)
{
    m_rSlider->blockSignals(true);
    m_gSlider->blockSignals(true);
    m_bSlider->blockSignals(true);
    m_rInput->blockSignals(true);
    m_gInput->blockSignals(true);
    m_bInput->blockSignals(true);

    m_rSlider->setValue(r);
    m_gSlider->setValue(g);
    m_bSlider->setValue(b);
    m_rInput->setValue(r);
    m_gInput->setValue(g);
    m_bInput->setValue(b);

    m_rSlider->blockSignals(false);
    m_gSlider->blockSignals(false);
    m_bSlider->blockSignals(false);
    m_rInput->blockSignals(false);
    m_gInput->blockSignals(false);
    m_bInput->blockSignals(false);

    slotEffect();
}

void RGBTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    m_gboxSettings->enableButton(EditorToolSettings::Ok,
                                (m_rInput->value() != 0 ||
                                 m_gInput->value() != 0 ||
                                 m_bInput->value() != 0));

    m_gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool alpha                 = iface->previewHasAlpha();
    bool sixteenBit            = iface->previewSixteenBit();

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
    double a = 1.0;

    DImg preview(w, h, sixteenBit, alpha, m_destinationPreviewData);
    ColorModifier cmod;
    cmod.applyColorModifier(preview, r, g, b, a);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData,
                                            w, h, sixteenBit, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void RGBTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
    double a = 1.0;

    ImageIface* iface = m_previewWidget->imageIface();
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

