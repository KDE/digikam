/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
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

#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

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
{
    setName("colorbalance");
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

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage(), 7, 4);

    QLabel *label1 = new QLabel(i18n("Channel:"), m_gboxSettings->plainPage());
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox(false, m_gboxSettings->plainPage());
    m_channelCB->insertItem(i18n("Luminosity"));
    m_channelCB->insertItem(i18n("Red"));
    m_channelCB->insertItem(i18n("Green"));
    m_channelCB->insertItem(i18n("Blue"));
    QWhatsThis::add( m_channelCB, i18n("<p>Select the histogram channel to display here:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    m_scaleBG = new QHButtonGroup(m_gboxSettings->plainPage());
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin(0);
    QWhatsThis::add( m_scaleBG, i18n("<p>Select the histogram scale here.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton(m_scaleBG);
    QToolTip::add(linHistoButton, i18n("<p>Linear"));
    m_scaleBG->insert(linHistoButton, HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap(QPixmap(directory + "histogram-lin.png"));
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton(m_scaleBG);
    QToolTip::add(logHistoButton, i18n("<p>Logarithmic"));
    m_scaleBG->insert(logHistoButton, HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap(QPixmap(directory + "histogram-log.png"));
    logHistoButton->setToggleButton(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(m_scaleBG);

    gridSettings->addMultiCellLayout(l1, 0, 0, 0, 4);

    // -------------------------------------------------------------

    QVBox *histoBox   = new QVBox(m_gboxSettings->plainPage());
    m_histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing "
                                             "of the selected image channel. This one is re-computed at any "
                                             "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new ColorGradientWidget(ColorGradientWidget::Horizontal, 10, histoBox);
    m_hGradient->setColors(QColor("black"), QColor("white"));

    gridSettings->addMultiCellWidget(histoBox, 1, 2, 0, 4);

    // -------------------------------------------------------------

    QLabel *labelLeft = new QLabel(i18n("Cyan"), m_gboxSettings->plainPage());
    labelLeft->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_rSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, m_gboxSettings->plainPage(), "m_rSlider");
    m_rSlider->setTickmarks(QSlider::Below);
    m_rSlider->setTickInterval(20);
    QWhatsThis::add( m_rSlider, i18n("<p>Set here the cyan/red color adjustment of the image."));
    QLabel *labelRight = new QLabel(i18n("Red"), m_gboxSettings->plainPage());
    labelRight->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
    m_rInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_rInput->setDefaultValue(0);
    m_rInput->input()->setRange(-100, 100, 1, false);

    gridSettings->addMultiCellWidget(labelLeft,     3, 3, 0, 0);
    gridSettings->addMultiCellWidget(m_rSlider,     3, 3, 1, 1);
    gridSettings->addMultiCellWidget(labelRight,    3, 3, 2, 2);
    gridSettings->addMultiCellWidget(m_rInput,      3, 3, 3, 3);

    // -------------------------------------------------------------

    labelLeft = new QLabel(i18n("Magenta"), m_gboxSettings->plainPage());
    labelLeft->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_gSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, m_gboxSettings->plainPage(), "m_gSlider");
    m_gSlider->setTickmarks(QSlider::Below);
    m_gSlider->setTickInterval(20);
    QWhatsThis::add( m_gSlider, i18n("<p>Set here the magenta/green color adjustment of the image."));
    labelRight = new QLabel(i18n("Green"), m_gboxSettings->plainPage());
    labelRight->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_gInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_gInput->setDefaultValue(0);
    m_gInput->input()->setRange(-100, 100, 1, false);

    gridSettings->addMultiCellWidget(labelLeft,     4, 4, 0, 0);
    gridSettings->addMultiCellWidget(m_gSlider,     4, 4, 1, 1);
    gridSettings->addMultiCellWidget(labelRight,    4, 4, 2, 2);
    gridSettings->addMultiCellWidget(m_gInput,      4, 4, 3, 3);

    // -------------------------------------------------------------

    labelLeft = new QLabel(i18n("Yellow"), m_gboxSettings->plainPage());
    labelLeft->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_bSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, m_gboxSettings->plainPage(), "m_bSlider");
    m_bSlider->setTickmarks(QSlider::Below);
    m_bSlider->setTickInterval(20);
    QWhatsThis::add( m_bSlider, i18n("<p>Set here the yellow/blue color adjustment of the image."));
    labelRight = new QLabel(i18n("Blue"), m_gboxSettings->plainPage());
    labelRight->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_bInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_bInput->setDefaultValue(0);
    m_bInput->input()->setRange(-100, 100, 1, false);

    gridSettings->addMultiCellWidget(labelLeft,     5, 5, 0, 0);
    gridSettings->addMultiCellWidget(m_bSlider,     5, 5, 1, 1);
    gridSettings->addMultiCellWidget(labelRight,    5, 5, 2, 2);
    gridSettings->addMultiCellWidget(m_bInput,      5, 5, 3, 3);

    m_rInput->setValue(0);
    m_gInput->setValue(0);
    m_bInput->setValue(0);

    gridSettings->setRowStretch(6, 10);
    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
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
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void RGBTool::slotChannelChanged(int channel)
{
    switch (channel)
    {
        case LuminosityChannel:
            m_histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            m_hGradient->setColors(QColor("black"), QColor("white"));
            break;

        case RedChannel:
            m_histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            m_hGradient->setColors(QColor("black"), QColor("red"));
            break;

        case GreenChannel:
            m_histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            m_hGradient->setColors(QColor("black"), QColor("green"));
            break;

        case BlueChannel:
            m_histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            m_hGradient->setColors(QColor("black"), QColor("blue"));
            break;
    }

    m_histogramWidget->repaint(false);
}

void RGBTool::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void RGBTool::slotColorSelectedFromTarget(const DColor &color)
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void RGBTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("colorbalance Tool");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    int r = config->readNumEntry("RedAjustment", m_rInput->defaultValue());
    int g = config->readNumEntry("GreenAjustment", m_gInput->defaultValue());
    int b = config->readNumEntry("BlueAjustment", m_bInput->defaultValue());
    adjustSliders(r, g, b);
    m_histogramWidget->reset();
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void RGBTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("colorbalance Tool");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("RedAjustment", m_rSlider->value());
    config->writeEntry("GreenAjustment", m_gInput->value());
    config->writeEntry("BlueAjustment", m_bInput->value());
    m_previewWidget->writeSettings();
    config->sync();
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
    kapp->setOverrideCursor( KCursor::waitCursor() );

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

    double r = ((double) m_rInput->value() + 100.0) / 100.0;
    double g = ((double) m_gInput->value() + 100.0) / 100.0;
    double b = ((double) m_bInput->value() + 100.0) / 100.0;
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
    kapp->setOverrideCursor( KCursor::waitCursor() );

    double r = ((double) m_rInput->value() + 100.0) / 100.0;
    double g = ((double) m_gInput->value() + 100.0) / 100.0;
    double b = ((double) m_bInput->value() + 100.0) / 100.0;
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

