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

// Qt includes.

#include <QButtonGroup>
#include <QColor>
#include <QComboBox>
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
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khuesaturationselect.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "hslmodifier.h"
#include "dimg.h"

// Local includes.

#include "hspreviewwidget.h"
#include "imageeffect_hsl.h"
#include "imageeffect_hsl.moc"

using namespace KDcrawIface;

namespace DigikamImagesPluginCore
{

ImageEffect_HSL::ImageEffect_HSL(QWidget* parent)
               : Digikam::ImageDlgBase(parent, i18n("Hue/Saturation/Lightness"), "hsladjust", false)
{
    m_destinationPreviewData = 0L;
    setHelp("hsladjusttool.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget("hsladjust Tool Dialog", mainWidget(),
                                               i18n("<p>Here you can see the image "
                                                    "Hue/Saturation/Lightness adjustments preview. "
                                                    "You can pick color on image "
                                                    "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings );

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( gboxSettings );
    m_channelCB->addItem( i18n("Luminosity") );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->setWhatsThis( i18n("<p>Select the histogram channel to display:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"));

    // -------------------------------------------------------------

    QWidget *scaleBox = new QWidget(gboxSettings);
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    m_scaleBG         = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale.<p>"
                                "If the image's maximal counts are small, you can use the linear scale.<p>"
                                "Logarithmic scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph."));

    QToolButton *linHistoButton = new QToolButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);

    QToolButton *logHistoButton = new QToolButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(linHistoButton);
    hlay->addWidget(logHistoButton);

    m_scaleBG->setExclusive(true);
    logHistoButton->setChecked(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(scaleBox);

    // -------------------------------------------------------------

    KVBox *histoBox   = new KVBox(gboxSettings);
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );


    // -------------------------------------------------------------

    m_HSSelector = new KHueSaturationSelector(gboxSettings);
    m_HSSelector->setWhatsThis( i18n("<p>Select the hue and saturation adjustments of the image."));
    m_HSSelector->setMinimumSize(256, 142);

    m_HSPreview = new HSPreviewWidget(gboxSettings, spacingHint());
    m_HSPreview->setWhatsThis( i18n("<p>You can see here a colour preview of the hue and "
                                    "saturation adjustments."));
    m_HSPreview->setMinimumSize(256, 15);

    QLabel *label2 = new QLabel(i18n("Hue:"), gboxSettings);
    m_hInput       = new KDoubleNumInput(gboxSettings);
    m_hInput->setDecimals(0);
    m_hInput->setRange(-180.0, 180.0, 1.0, true);
    m_hInput->setValue(0.0);
    m_hInput->setWhatsThis( i18n("<p>Set here the hue adjustment of the image."));

    QLabel *label3 = new QLabel(i18n("Saturation:"), gboxSettings);
    m_sInput       = new KDoubleNumInput(gboxSettings);
    m_sInput->setDecimals(2);
    m_sInput->setRange(-100.0, 100.0, 0.01, true);
    m_sInput->setValue(0.0);
    m_sInput->setWhatsThis( i18n("<p>Set here the saturation adjustment of the image."));

    QLabel *label4 = new QLabel(i18n("Lightness:"), gboxSettings);
    m_lInput       = new KDoubleNumInput(gboxSettings);
    m_lInput->setDecimals(2);
    m_lInput->setRange(-100.0, 100.0, 0.01, true);
    m_lInput->setValue(0.0);
    m_lInput->setWhatsThis( i18n("<p>Set here the lightness adjustment of the image."));

    // -------------------------------------------------------------

    gridSettings->addLayout(l1,           0, 0, 1, 5);
    gridSettings->addWidget(histoBox,     1, 0, 2, 5);
    gridSettings->addWidget(m_HSSelector, 3, 0, 1, 5);
    gridSettings->addWidget(m_HSPreview,  4, 0, 1, 5);
    gridSettings->addWidget(label2,       5, 0, 1, 5);
    gridSettings->addWidget(m_hInput,     6, 0, 1, 5);
    gridSettings->addWidget(label3,       7, 0, 1, 5);
    gridSettings->addWidget(m_sInput,     8, 0, 1, 5);
    gridSettings->addWidget(label4,       9, 0, 1, 5);
    gridSettings->addWidget(m_lInput,    10, 0, 1, 5);
    gridSettings->setRowStretch(11, 10);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_HSSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

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

    enableButtonOk( false );
}

ImageEffect_HSL::~ImageEffect_HSL()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_HSL::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case LuminosityChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            break;

        case RedChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            break;

        case GreenChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            break;

        case BlueChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            break;
    }

    m_histogramWidget->repaint();
}

void ImageEffect_HSL::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ImageEffect_HSL::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_HSL::slotHSChanged(int h, int s)
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

void ImageEffect_HSL::slotHChanged(double h)
{
    int hue = int(h);
    if (h >= -180 && h < 0)
        hue = int(h) + 359;

    m_HSSelector->blockSignals(true);
    m_HSSelector->setXValue(hue);
    m_HSSelector->blockSignals(false);
}

void ImageEffect_HSL::slotSChanged(double s)
{
    int sat = (int)((s + 100.0) * (255.0/200.0));

    m_HSSelector->blockSignals(true);
    m_HSSelector->setYValue(sat);
    m_HSSelector->blockSignals(false);
}

void ImageEffect_HSL::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hsladjust Tool Dialog");
    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    m_hInput->setValue(group.readEntry("HueAjustment", 0.0));
    m_sInput->setValue(group.readEntry("SaturationAjustment", 0.0));
    m_lInput->setValue(group.readEntry("LighnessAjustment", 0.0));
    slotHChanged(m_hInput->value());
    slotSChanged(m_sInput->value());
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ImageEffect_HSL::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hsladjust Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());
    group.writeEntry("HueAjustment", m_hInput->value());
    group.writeEntry("SaturationAjustment", m_sInput->value());
    group.writeEntry("LighnessAjustment", m_lInput->value());
    config->sync();
}

void ImageEffect_HSL::resetValues()
{
    m_hInput->blockSignals(true);
    m_sInput->blockSignals(true);
    m_lInput->blockSignals(true);
    m_hInput->setValue(0.0);
    m_sInput->setValue(0.0);
    m_lInput->setValue(0.0);
    slotHChanged(0.0);
    slotSChanged(0.0);
    m_hInput->blockSignals(false);
    m_sInput->blockSignals(false);
    m_lInput->blockSignals(false);
}

void ImageEffect_HSL::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double hu  = m_hInput->value();
    double sa  = m_sInput->value();
    double lu  = m_lInput->value();

    enableButtonOk( hu != 0.0 || sa != 0.0 || lu != 0.0);

    m_HSPreview->setHS(hu, sa);
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
    Digikam::HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.

    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_HSL::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double hu  = m_hInput->value();
    double sa  = m_sInput->value();
    double lu  = m_lInput->value();

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();
    Digikam::DImg original(w, h, sb, a, data);
    delete [] data;

    Digikam::HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(original);

    iface->putOriginalImage(i18n("HSL Adjustments"), original.bits());
    kapp->restoreOverrideCursor();
    accept();
}

}  // NameSpace DigikamImagesPluginCore

