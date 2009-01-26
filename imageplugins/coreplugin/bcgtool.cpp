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
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bcgmodifier.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imagewidget.h"

// Local includes.

#include "bcgtool.h"
#include "bcgtool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

BCGTool::BCGTool(QObject* parent)
       : EditorTool(parent)
{
    setName("bcgadjust");
    setToolName(i18n("Brightness / Contrast / Gamma"));
    setToolIcon(SmallIcon("contrast"));
    setToolHelp("bcgadjusttool.anchor");

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("bcgadjust Tool", 0,
                                      i18n("<p>Here you can see the image "
                                           "brightness-contrast-gamma adjustments preview. "
                                           "You can pick color on image "
                                           "to see the color level corresponding on histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage(), 9, 4);

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

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Brightness:"), m_gboxSettings->plainPage());
    m_bInput       = new RIntNumInput(m_gboxSettings->plainPage());
    m_bInput->setRange(-100, 100, 1);
    m_bInput->setDefaultValue(0);
    QWhatsThis::add( m_bInput, i18n("<p>Set here the brightness adjustment of the image."));

    QLabel *label3 = new QLabel(i18n("Contrast:"), m_gboxSettings->plainPage());
    m_cInput       = new RIntNumInput(m_gboxSettings->plainPage());
    m_cInput->setRange(-100, 100, 1);
    m_cInput->setDefaultValue(0);
    QWhatsThis::add( m_cInput, i18n("<p>Set here the contrast adjustment of the image."));

    QLabel *label4 = new QLabel(i18n("Gamma:"), m_gboxSettings->plainPage());
    m_gInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_gInput->setPrecision(2);
    m_gInput->setRange(0.1, 3.0, 0.01);
    m_gInput->setDefaultValue(1.0);
    QWhatsThis::add( m_gInput, i18n("<p>Set here the gamma adjustment of the image."));

    // -------------------------------------------------------------

    gridSettings->addMultiCellLayout(l1,        0, 0, 0, 4);
    gridSettings->addMultiCellWidget(histoBox,  1, 2, 0, 4);
    gridSettings->addMultiCellWidget(label2,    3, 3, 0, 4);
    gridSettings->addMultiCellWidget(m_bInput,  4, 4, 0, 4);
    gridSettings->addMultiCellWidget(label3,    5, 5, 0, 4);
    gridSettings->addMultiCellWidget(m_cInput,  6, 6, 0, 4);
    gridSettings->addMultiCellWidget(label4,    7, 7, 0, 4);
    gridSettings->addMultiCellWidget(m_gInput,  8, 8, 0, 4);
    gridSettings->setRowStretch(9, 10);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

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

void BCGTool::slotChannelChanged(int channel)
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

void BCGTool::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void BCGTool::slotColorSelectedFromTarget(const DColor &color)
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void BCGTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("bcgadjust Tool");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    m_bInput->setValue(config->readNumEntry("BrightnessAjustment", m_bInput->defaultValue()));
    m_cInput->setValue(config->readNumEntry("ContrastAjustment", m_cInput->defaultValue()));
    m_gInput->setValue(config->readDoubleNumEntry("GammaAjustment", m_gInput->defaultValue()));
    m_histogramWidget->reset();
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void BCGTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("bcgadjust Tool");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("BrightnessAjustment", m_bInput->value());
    config->writeEntry("ContrastAjustment", m_cInput->value());
    config->writeEntry("GammaAjustment", m_gInput->value());
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
    kapp->setOverrideCursor(KCursor::waitCursor());

    double b = (double) m_bInput->value() / 250.0;
    double c = (double) (m_cInput->value() / 100.0) + 1.00;
    double g = m_gInput->value();

    m_gboxSettings->enableButton(EditorToolSettings::Ok,
                                ( b != 0.0 || c != 1.0 || g != 1.0 ));

    m_histogramWidget->stopHistogramComputation();

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
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void BCGTool::finalRendering()
{
    kapp->setOverrideCursor(KCursor::waitCursor());
    ImageIface* iface = m_previewWidget->imageIface();

    double b = (double) m_bInput->value() / 250.0;
    double c = (double) (m_cInput->value() / 100.0) + 1.00;
    double g = m_gInput->value();

    iface->setOriginalBCG(b, c, g);
    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamImagesPluginCore

