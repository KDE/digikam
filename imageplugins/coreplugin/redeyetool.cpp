/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include <qcolor.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <kapplication.h>
#include <kcolordialog.h>
#include <kcolordialog.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Digikam includes.

#include "bcgmodifier.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imagewidget.h"

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "redeyetool.h"
#include "redeyetool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

RedEyeTool::RedEyeTool(QObject* parent)
          : EditorTool(parent)
{
    setName("redeye");
    setToolName(i18n("Red Eye"));
    setToolIcon(SmallIcon("redeyes"));
    setToolHelp("redeyecorrectiontool.anchor");

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("redeye Tool", 0,
                                      i18n("<p>Here you can see the image selection preview with "
                                           "red eye reduction applied."),
                                           true, ImageGuideWidget::PickColorMode, true, true);
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    EditorToolSettings *gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                                              EditorToolSettings::Ok|
                                                              EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout(gboxSettings->plainPage(), 11, 4);

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings->plainPage());
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox(false, gboxSettings->plainPage());
    m_channelCB->insertItem(i18n("Luminosity"));
    m_channelCB->insertItem(i18n("Red"));
    m_channelCB->insertItem(i18n("Green"));
    m_channelCB->insertItem(i18n("Blue"));
    QWhatsThis::add( m_channelCB, i18n("<p>Select the histogram channel to display here:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image channel values.<p>"
                                       "<b>Green</b>: display the green image channel values.<p>"
                                       "<b>Blue</b>: display the blue image channel values.<p>"));

    m_scaleBG = new QHButtonGroup(gboxSettings->plainPage());
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin(0);
    QWhatsThis::add( m_scaleBG, i18n("<p>Select the histogram scale here.<p>"
                                     "If the image's maximum counts are small, you can use the linear scale.<p>"
                                     "The logarithmic scale can be used when the maximal counts are big "
                                     "to show all values (small and large) on the graph."));

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

    QVBox *histoBox   = new QVBox(gboxSettings->plainPage());
    m_histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram "
                                             "of the selected image channel. It is "
                                             "updated upon setting changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient   = new ColorGradientWidget(ColorGradientWidget::Horizontal, 10, histoBox);
    m_hGradient->setColors(QColor("black"), QColor("white"));

    // -------------------------------------------------------------

    m_thresholdLabel = new QLabel(i18n("Sensitivity:"), gboxSettings->plainPage());
    m_redThreshold   = new RIntNumInput(gboxSettings->plainPage());
    m_redThreshold->setRange(10, 90, 1);
    m_redThreshold->setDefaultValue(20);
    QWhatsThis::add(m_redThreshold, i18n("<p>Sets the red color pixels selection threshold. "
                                         "Low values will select more red color pixels (agressive correction), high "
                                         "values less (mild correction). Use low value if eye have been selected "
                                         "exactly. Use high value if other parts of the face are also selected."));

    m_smoothLabel = new QLabel(i18n("Smooth:"), gboxSettings->plainPage());
    m_smoothLevel = new RIntNumInput(gboxSettings->plainPage());
    m_smoothLevel->setRange(0, 5, 1);
    m_smoothLevel->setDefaultValue(1);
    QWhatsThis::add(m_smoothLevel, i18n("<p>Sets the smoothness value when blurring the border "
                                        "of the changed pixels. "
                                        "This leads to a more naturally looking pupil."));

    QLabel *label3 = new QLabel(i18n("Coloring Tint:"), gboxSettings->plainPage());
    m_HSSelector   = new KHSSelector(gboxSettings->plainPage());
    m_VSelector    = new KValueSelector(gboxSettings->plainPage());
    m_HSSelector->setMinimumSize(200, 142);
    m_VSelector->setMinimumSize(26, 142);
    QWhatsThis::add(m_HSSelector, i18n("<p>Sets a custom color to re-colorize the eyes."));

    QLabel *label4 = new QLabel(i18n("Tint Level:"), gboxSettings->plainPage());
    m_tintLevel    = new RIntNumInput(gboxSettings->plainPage());
    m_tintLevel->setRange(1, 200, 1);
    m_tintLevel->setDefaultValue(128);
    QWhatsThis::add(m_tintLevel, i18n("<p>Set the tint level to adjust the luminosity of "
                                      "the new color of the pupil."));

    gridSettings->addMultiCellLayout(l1,               0,  0, 0, 4);
    gridSettings->addMultiCellWidget(histoBox,         1,  2, 0, 4);
    gridSettings->addMultiCellWidget(m_thresholdLabel, 3,  3, 0, 4);
    gridSettings->addMultiCellWidget(m_redThreshold,   4,  4, 0, 4);
    gridSettings->addMultiCellWidget(m_smoothLabel,    5,  5, 0, 4);
    gridSettings->addMultiCellWidget(m_smoothLevel,    6,  6, 0, 4);
    gridSettings->addMultiCellWidget(label3,           7,  7, 0, 4);
    gridSettings->addMultiCellWidget(m_HSSelector,     8,  8, 0, 3);
    gridSettings->addMultiCellWidget(m_VSelector,      8,  8, 4, 4);
    gridSettings->addMultiCellWidget(label4,           9,  9, 0, 4);
    gridSettings->addMultiCellWidget(m_tintLevel,     10, 10, 0, 4);
    gridSettings->setRowStretch(11, 10);
    gridSettings->setColStretch(3, 10);

    setToolSettings(gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(m_redThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_smoothLevel, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_HSSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(m_VSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_tintLevel, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

RedEyeTool::~RedEyeTool()
{
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void RedEyeTool::slotHSChanged(int h, int s)
{
    m_VSelector->blockSignals(true);
    m_VSelector->setHue(h);
    m_VSelector->setSaturation(s);
    m_VSelector->updateContents();
    m_VSelector->repaint(false);
    m_VSelector->blockSignals(false);
    slotTimer();
}

void RedEyeTool::slotChannelChanged(int channel)
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

void RedEyeTool::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void RedEyeTool::slotColorSelectedFromTarget(const DColor& color)
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void RedEyeTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("redeye Tool");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0)); // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    m_redThreshold->setValue(config->readNumEntry("RedThreshold", m_redThreshold->defaultValue()));
    m_smoothLevel->setValue(config->readNumEntry("SmoothLevel", m_smoothLevel->defaultValue()));
    m_HSSelector->setXValue(config->readNumEntry("HueColoringTint", 0));
    m_HSSelector->setYValue(config->readNumEntry("SatColoringTint", 0));
    m_VSelector->setValue(config->readNumEntry("ValColoringTint", 0));
    m_tintLevel->setValue(config->readNumEntry("TintLevel", m_tintLevel->defaultValue()));

    slotHSChanged(m_HSSelector->xValue(), m_HSSelector->yValue());
    m_histogramWidget->reset();
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void RedEyeTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("redeye Tool");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("RedThreshold", m_redThreshold->value());
    config->writeEntry("SmoothLevel", m_smoothLevel->value());
    config->writeEntry("HueColoringTint", m_HSSelector->xValue());
    config->writeEntry("SatColoringTint", m_HSSelector->yValue());
    config->writeEntry("ValColoringTint", m_VSelector->value());
    config->writeEntry("TintLevel", m_tintLevel->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void RedEyeTool::slotResetSettings()
{
    m_redThreshold->blockSignals(true);
    m_HSSelector->blockSignals(true);
    m_VSelector->blockSignals(true);
    m_tintLevel->blockSignals(true);

    m_redThreshold->slotReset();
    m_smoothLevel->slotReset();

    // Black color by default
    m_HSSelector->setXValue(0);
    m_HSSelector->setYValue(0);
    m_VSelector->setValue(0);

    m_tintLevel->slotReset();

    m_redThreshold->blockSignals(false);
    m_HSSelector->blockSignals(false);
    m_VSelector->blockSignals(false);
    m_tintLevel->blockSignals(false);
}

void RedEyeTool::slotEffect()
{
    kapp->setOverrideCursor(KCursor::waitCursor());

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    // Here, we need to use the real selection image data because we will apply
    // a Gaussian blur filter on pixels and we cannot use directly the preview scaled image
    // else the blur radius will not give the same result between preview and final rendering.
    ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getImageSelection();
    int w                      = iface->selectedWidth();
    int h                      = iface->selectedHeight();
    bool sb                    = iface->originalSixteenBit();
    bool a                     = iface->originalHasAlpha();
    DImg selection(w, h, sb, a, m_destinationPreviewData);

    redEyeFilter(selection);

    DImg preview = selection.smoothScale(iface->previewWidth(), iface->previewHeight());

    iface->putPreviewImage(preview.bits());
    m_previewWidget->updatePreview();

    // Update histogram.

    memcpy(m_destinationPreviewData, selection.bits(), selection.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void RedEyeTool::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getImageSelection();
    int w                      = iface->selectedWidth();
    int h                      = iface->selectedHeight();
    bool sixteenBit            = iface->originalSixteenBit();
    bool hasAlpha              = iface->originalHasAlpha();
    DImg selection(w, h, sixteenBit, hasAlpha, data);
    delete [] data;

    redEyeFilter(selection);

    iface->putImageSelection(i18n("Red Eyes Correction"), selection.bits());

    kapp->restoreOverrideCursor();
}

void RedEyeTool::redEyeFilter(DImg& selection)
{
    DImg mask(selection.width(), selection.height(), selection.sixteenBit(), true,
                       selection.bits(), true);

    selection          = mask.copy();
    float redThreshold = m_redThreshold->value()/10.0;
    int hue            = m_HSSelector->xValue();
    int sat            = m_HSSelector->yValue();
    int val            = m_VSelector->value();
    KColor coloring;
    coloring.setHsv(hue, sat, val);

    struct channel
    {
        float red_gain;
        float green_gain;
        float blue_gain;
    };

    channel red_chan, green_chan, blue_chan;

    red_chan.red_gain     = 0.1;
    red_chan.green_gain   = 0.6;
    red_chan.blue_gain    = 0.3;

    green_chan.red_gain   = 0.0;
    green_chan.green_gain = 1.0;
    green_chan.blue_gain  = 0.0;

    blue_chan.red_gain    = 0.0;
    blue_chan.green_gain  = 0.0;
    blue_chan.blue_gain   = 1.0;

    float red_norm, green_norm, blue_norm;
    int   level = 201 - m_tintLevel->value();

    red_norm   = 1.0 / (red_chan.red_gain   + red_chan.green_gain   + red_chan.blue_gain);
    green_norm = 1.0 / (green_chan.red_gain + green_chan.green_gain + green_chan.blue_gain);
    blue_norm  = 1.0 / (blue_chan.red_gain  + blue_chan.green_gain  + blue_chan.blue_gain);

    red_norm   *= coloring.red()   / level;
    green_norm *= coloring.green() / level;
    blue_norm  *= coloring.blue()  / level;

    // Perform a red color pixels detection in selection image and create a correction mask using an alpha channel.

    if (!selection.sixteenBit())         // 8 bits image.
    {
        uchar* ptr  = selection.bits();
        uchar* mptr = mask.bits();
        uchar  r, g, b, r1, g1, b1;

        for (uint i = 0 ; i < selection.width() * selection.height() ; i++)
        {
            b       = ptr[0];
            g       = ptr[1];
            r       = ptr[2];
            mptr[3] = 255;

            if (r >= ( redThreshold * g))
            {
                r1 = QMIN(255, (int)(red_norm * (red_chan.red_gain   * r +
                                                 red_chan.green_gain * g +
                                                 red_chan.blue_gain  * b)));

                g1 = QMIN(255, (int)(green_norm * (green_chan.red_gain   * r +
                                                   green_chan.green_gain * g +
                                                   green_chan.blue_gain  * b)));

                b1 = QMIN(255, (int)(blue_norm * (blue_chan.red_gain   * r +
                                                  blue_chan.green_gain * g +
                                                  blue_chan.blue_gain  * b)));

                mptr[0] = b1;
                mptr[1] = g1;
                mptr[2] = r1;
                mptr[3] = QMIN( (int)((r-g) / 150.0 * 255.0), 255);
            }

            ptr += 4;
            mptr+= 4;
        }
    }
    else                                 // 16 bits image.
    {
        unsigned short* ptr  = (unsigned short*)selection.bits();
        unsigned short* mptr = (unsigned short*)mask.bits();
        unsigned short  r, g, b, r1, g1, b1;

        for (uint i = 0 ; i < selection.width() * selection.height() ; i++)
        {
            b       = ptr[0];
            g       = ptr[1];
            r       = ptr[2];
            mptr[3] = 65535;

            if (r >= ( redThreshold * g))
            {
                r1 = QMIN(65535, (int)(red_norm * (red_chan.red_gain   * r +
                                                         red_chan.green_gain * g +
                                                         red_chan.blue_gain  * b)));

                g1 = QMIN(65535, (int)(green_norm * (green_chan.red_gain   * r +
                                                            green_chan.green_gain * g +
                                                            green_chan.blue_gain  * b)));

                b1 = QMIN(65535, (int)(blue_norm * (blue_chan.red_gain   * r +
                                                          blue_chan.green_gain * g +
                                                          blue_chan.blue_gain  * b)));

                mptr[0] = b1;
                mptr[1] = g1;
                mptr[2] = r1;
                mptr[3] = QMIN( (int)((r-g) / 38400.0 * 65535.0), 65535);;
            }

            ptr += 4;
            mptr+= 4;
        }
    }

    // Now, we will blur only the transparency pixels from the mask.

    DImg mask2 = mask.copy();
    DImgImageFilters filter;
    filter.gaussianBlurImage(mask2.bits(), mask2.width(), mask2.height(),
                             mask2.sixteenBit(), m_smoothLevel->value());

    if (!selection.sixteenBit())         // 8 bits image.
    {
        uchar* mptr  = mask.bits();
        uchar* mptr2 = mask2.bits();

        for (uint i = 0 ; i < mask2.width() * mask2.height() ; i++)
        {
            if (mptr2[3] < 255)
            {
                mptr[0] = mptr2[0];
                mptr[1] = mptr2[1];
                mptr[2] = mptr2[2];
                mptr[3] = mptr2[3];
            }

            mptr += 4;
            mptr2+= 4;
        }
    }
    else                                // 16 bits image.
    {
        unsigned short* mptr  = (unsigned short*)mask.bits();
        unsigned short* mptr2 = (unsigned short*)mask2.bits();

        for (uint i = 0 ; i < mask2.width() * mask2.height() ; i++)
        {
            if (mptr2[3] < 65535)
            {
                mptr[0] = mptr2[0];
                mptr[1] = mptr2[1];
                mptr[2] = mptr2[2];
                mptr[3] = mptr2[3];
            }

            mptr += 4;
            mptr2+= 4;
        }
    }

    // - Perform pixels blending using alpha channel between the mask and the selection.

    DColorComposer *composer = DColorComposer::getComposer(DColorComposer::PorterDuffSrcOver);

    // NOTE: 'mask' is the Source image, 'selection' is the Destination image.

    selection.bitBlendImage(composer, &mask,
                            0, 0, mask.width(), mask.height(),
                            0, 0);
}

}  // NameSpace DigikamImagesPluginCore
