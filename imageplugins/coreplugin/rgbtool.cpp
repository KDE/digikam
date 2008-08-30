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
#include <QToolButton>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Digikam includes.

#include "colorgradientwidget.h"
#include "colormodifier.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imagewidget.h"

// Local includes.

#include "rgbtool.h"
#include "rgbtool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

RGBTool::RGBTool(QObject* parent)
       : EditorTool(parent)
//       , i18n("Color Balance"), "colorbalance", false)
{
    setObjectName("colorbalance");
    setToolName(i18n("Color Balance"));
    setToolIcon(SmallIcon("adjustrgb"));

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("colorbalance Tool", 0,
                                      i18n("<p>Here you can see the image "
                                           "color-balance adjustments preview. "
                                           "You can pick color on image "
                                           "to see the color level corresponding on histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1 = new QLabel(i18n("Channel:"), m_gboxSettings->plainPage());
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox(m_gboxSettings->plainPage());
    m_channelCB->addItem(i18n("Luminosity"));
    m_channelCB->addItem(i18n("Red"));
    m_channelCB->addItem(i18n("Green"));
    m_channelCB->addItem(i18n("Blue"));
    m_channelCB->setWhatsThis( i18n("<p>Select the histogram channel to display:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"));

    QWidget *scaleBox = new QWidget(m_gboxSettings->plainPage());
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
    m_scaleBG->addButton(linHistoButton, HistogramWidget::LinScaleHistogram);

    QToolButton *logHistoButton = new QToolButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, HistogramWidget::LogScaleHistogram);

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

    KVBox *histoBox   = new KVBox(m_gboxSettings->plainPage());
    m_histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // -------------------------------------------------------------

    QLabel *labelCyan = new QLabel(i18n("Cyan"), m_gboxSettings->plainPage());
    labelCyan->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    m_rSlider = new QSlider(Qt::Horizontal, m_gboxSettings->plainPage());
    m_rSlider->setValue(0);
    m_rSlider->setRange(-100, 100);
    m_rSlider->setPageStep(1);
    m_rSlider->setTickPosition(QSlider::TicksBelow);
    m_rSlider->setTickInterval(20);
    m_rSlider->setWhatsThis( i18n("<p>Set here the cyan/red color adjustment of the image."));

    QLabel *labelRed = new QLabel(i18n("Red"), m_gboxSettings->plainPage());
    labelRed->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );

    m_rInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_rInput->setRange(-100, 100, 1);
    m_rInput->setSliderEnabled(false);
    m_rInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelMagenta = new QLabel(i18n("Magenta"), m_gboxSettings->plainPage());
    labelMagenta->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    m_gSlider = new QSlider(Qt::Horizontal, m_gboxSettings->plainPage());
    m_gSlider->setValue(0);
    m_gSlider->setRange(-100, 100);
    m_gSlider->setPageStep(1);
    m_gSlider->setTickPosition(QSlider::TicksBelow);
    m_gSlider->setTickInterval(20);
    m_gSlider->setWhatsThis( i18n("<p>Set here the magenta/green color adjustment of the image."));

    QLabel *labelGreen = new QLabel(i18n("Green"), m_gboxSettings->plainPage());
    labelGreen->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );

    m_gInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_gInput->setRange(-100, 100, 1);
    m_gInput->setSliderEnabled(false);
    m_gInput->setDefaultValue(0);

    // -------------------------------------------------------------

    QLabel *labelYellow = new QLabel(i18n("Yellow"), m_gboxSettings->plainPage());
    labelYellow->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    m_bSlider = new QSlider(Qt::Horizontal, m_gboxSettings->plainPage());
    m_bSlider->setValue(0);
    m_bSlider->setRange(-100, 100);
    m_bSlider->setPageStep(1);
    m_bSlider->setTickPosition(QSlider::TicksBelow);
    m_bSlider->setTickInterval(20);
    m_bSlider->setWhatsThis( i18n("<p>Set here the yellow/blue color adjustment of the image."));

    QLabel *labelBlue = new QLabel(i18n("Blue"), m_gboxSettings->plainPage());
    labelBlue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );

    m_bInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_bInput->setRange(-100, 100, 1);
    m_bInput->setSliderEnabled(false);
    m_bInput->setDefaultValue(0);

    // -------------------------------------------------------------

    gridSettings->addLayout(l1,             0, 0, 1, 5 );
    gridSettings->addWidget(histoBox,       1, 0, 2, 5 );
    gridSettings->addWidget(labelCyan,      3, 0, 1, 1);
    gridSettings->addWidget(m_rSlider,      3, 1, 1, 1);
    gridSettings->addWidget(labelRed,       3, 2, 1, 1);
    gridSettings->addWidget(m_rInput,       3, 3, 1, 1);
    gridSettings->addWidget(labelMagenta,   4, 0, 1, 1);
    gridSettings->addWidget(m_gSlider,      4, 1, 1, 1);
    gridSettings->addWidget(labelGreen,     4, 2, 1, 1);
    gridSettings->addWidget(m_gInput,       4, 3, 1, 1);
    gridSettings->addWidget(labelYellow,    5, 0, 1, 1);
    gridSettings->addWidget(m_bSlider,      5, 1, 1, 1);
    gridSettings->addWidget(labelBlue,      5, 2, 1, 1);
    gridSettings->addWidget(m_bInput,       5, 3, 1, 1);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());
    gridSettings->setRowStretch(6, 10);

    setToolSettings(m_gboxSettings);

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

    m_gboxSettings->enableButton(EditorToolSettings::Ok, false);
}

RGBTool::~RGBTool()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    delete m_histogramWidget;
    delete m_previewWidget;
}

void RGBTool::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case LuminosityChannel:
            m_histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            break;

        case RedChannel:
            m_histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            break;

        case GreenChannel:
            m_histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            break;

        case BlueChannel:
            m_histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            break;
    }

    m_histogramWidget->repaint();
}

void RGBTool::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void RGBTool::slotColorSelectedFromTarget( const DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void RGBTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("colorbalance Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)HistogramWidget::LogScaleHistogram))->setChecked(true);

    int r = group.readEntry("RedAjustment", m_rInput->defaultValue());
    int g = group.readEntry("GreenAjustment", m_gInput->defaultValue());
    int b = group.readEntry("BlueAjustment", m_bInput->defaultValue());
    adjustSliders(r, g, b);
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void RGBTool::writeSettings()
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

    m_histogramWidget->stopHistogramComputation();

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
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sixteenBit, 0, 0, 0, false);

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

}  // NameSpace DigikamImagesPluginCore

