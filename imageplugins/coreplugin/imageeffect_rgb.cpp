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

// Qt includes.

#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "colormodifier.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_rgb.h"
#include "imageeffect_rgb.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_RGB::ImageEffect_RGB(QWidget* parent)
               : Digikam::ImageDlgBase(parent, i18n("Color Balance"), "colorbalance", false)
{
    m_destinationPreviewData = 0L;
    setHelp("colorbalancetool.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget("colorbalance Tool Dialog", mainWidget(),
                                               i18n("<p>Here you can see the image "
                                                    "color-balance adjustments preview. "
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

    QLabel *labelCyan = new QLabel(i18n("Cyan"), gboxSettings);
    labelCyan->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    m_rSlider = new QSlider(Qt::Horizontal, gboxSettings);
    m_rSlider->setValue(0);
    m_rSlider->setRange(-100, 100);
    m_rSlider->setPageStep(1);
    m_rSlider->setTickPosition(QSlider::TicksBelow);
    m_rSlider->setTickInterval(20);
    m_rSlider->setWhatsThis( i18n("<p>Set here the cyan/red color adjustment of the image."));

    QLabel *labelRed = new QLabel(i18n("Red"), gboxSettings);
    labelRed->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );

    m_rInput = new QSpinBox(gboxSettings);
    m_rInput->setMinimum(-100);
    m_rInput->setMaximum(100);
    m_rInput->setSingleStep(1);
    m_rInput->setValue(0);

    // -------------------------------------------------------------

    QLabel *labelMagenta = new QLabel(i18n("Magenta"), gboxSettings);
    labelMagenta->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    m_gSlider = new QSlider(Qt::Horizontal, gboxSettings);
    m_gSlider->setValue(0);
    m_gSlider->setRange(-100, 100);
    m_gSlider->setPageStep(1);
    m_gSlider->setTickPosition(QSlider::TicksBelow);
    m_gSlider->setTickInterval(20);
    m_gSlider->setWhatsThis( i18n("<p>Set here the magenta/green color adjustment of the image."));

    QLabel *labelGreen = new QLabel(i18n("Green"), gboxSettings);
    labelGreen->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );

    m_gInput = new QSpinBox(gboxSettings);
    m_gInput->setMinimum(-100);
    m_gInput->setMaximum(100);
    m_gInput->setSingleStep(1);
    m_gInput->setValue(0);

    // -------------------------------------------------------------

    QLabel *labelYellow = new QLabel(i18n("Yellow"), gboxSettings);
    labelYellow->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    m_bSlider = new QSlider(Qt::Horizontal, gboxSettings);
    m_bSlider->setValue(0);
    m_bSlider->setRange(-100, 100);
    m_bSlider->setPageStep(1);
    m_bSlider->setTickPosition(QSlider::TicksBelow);
    m_bSlider->setTickInterval(20);
    m_bSlider->setWhatsThis( i18n("<p>Set here the yellow/blue color adjustment of the image."));

    QLabel *labelBlue = new QLabel(i18n("Blue"), gboxSettings);
    labelBlue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );

    m_bInput = new QSpinBox(gboxSettings);
    m_bInput->setMinimum(-100);
    m_bInput->setMaximum(100);
    m_bInput->setSingleStep(1);
    m_bInput->setValue(0);

    // -------------------------------------------------------------

    gridSettings->addLayout(l1, 0, 0, 1, 5 );
    gridSettings->addWidget(histoBox, 1, 0, 2, 5 );
    gridSettings->addWidget(labelCyan, 3, 0, 1, 1);
    gridSettings->addWidget(m_rSlider, 3, 1, 1, 1);
    gridSettings->addWidget(labelRed, 3, 2, 1, 1);
    gridSettings->addWidget(m_rInput, 3, 3, 1, 1);
    gridSettings->addWidget(labelMagenta, 4, 0, 1, 1);
    gridSettings->addWidget(m_gSlider, 4, 1, 1, 1);
    gridSettings->addWidget(labelGreen, 4, 2, 1, 1);
    gridSettings->addWidget(m_gInput, 4, 3, 1, 1);
    gridSettings->addWidget(labelYellow, 5, 0, 1, 1);
    gridSettings->addWidget(m_bSlider, 5, 1, 1, 1);
    gridSettings->addWidget(labelBlue, 5, 2, 1, 1);
    gridSettings->addWidget(m_bInput, 5, 3, 1, 1);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());
    gridSettings->setRowStretch(6, 10);

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

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

    enableButtonOk( false );
}

ImageEffect_RGB::~ImageEffect_RGB()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_RGB::slotChannelChanged(int channel)
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

void ImageEffect_RGB::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ImageEffect_RGB::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_RGB::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("colorbalance Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    int r = group.readEntry("RedAjustment", 0);
    int g = group.readEntry("GreenAjustment", 0);
    int b = group.readEntry("BlueAjustment", 0);
    adjustSliders(r, g, b);
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ImageEffect_RGB::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("colorbalance Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());
    group.writeEntry("RedAjustment", m_rSlider->value());
    group.writeEntry("GreenAjustment", m_gInput->value());
    group.writeEntry("BlueAjustment", m_bInput->value());
    group.sync();
}

void ImageEffect_RGB::resetValues()
{
    adjustSliders(0, 0, 0);
}

void ImageEffect_RGB::adjustSliders(int r, int g, int b)
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

void ImageEffect_RGB::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    enableButtonOk(m_rInput->value() != 0 ||
                   m_gInput->value() != 0 ||
                   m_bInput->value() != 0);

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool alpha                 = iface->previewHasAlpha();
    bool sixteenBit            = iface->previewSixteenBit();

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
    double a = 1.0;

    Digikam::DImg preview(w, h, sixteenBit, alpha, m_destinationPreviewData);
    Digikam::ColorModifier cmod;
    cmod.applyColorModifier(preview, r, g, b, a);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.

    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sixteenBit, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_RGB::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
    double a = 1.0;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool alpha                 = iface->originalHasAlpha();
    bool sixteenBit            = iface->originalSixteenBit();
    Digikam::DImg original(w, h, sixteenBit, alpha, data);
    delete [] data;

    Digikam::ColorModifier cmod;
    cmod.applyColorModifier(original, r, g, b, a);

    iface->putOriginalImage(i18n("Color Balance"), original.bits());
    kapp->restoreOverrideCursor();
    accept();
}

}  // NameSpace DigikamImagesPluginCore

