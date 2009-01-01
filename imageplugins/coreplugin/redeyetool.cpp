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


#include "redeyetool.h"
#include "redeyetool.moc"

// Qt includes.

#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes.

#include <kapplication.h>
#include <kcolordialog.h>
#include <kcolorvalueselector.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khuesaturationselect.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "bcgmodifier.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imagewidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

RedEyeTool::RedEyeTool(QObject* parent)
          : EditorTool(parent)
{
    setObjectName("redeye");
    setToolName(i18n("Red Eye"));
    setToolIcon(SmallIcon("redeyes"));
    setToolHelp("redeyecorrectiontool.anchor");

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("redeye Tool", 0,
                                      i18n("Here you can see the image selection preview with "
                                           "red eye reduction applied."),
                                      true, ImageGuideWidget::PickColorMode, true, true);
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    m_thresholdLabel = new QLabel(i18n("Sensitivity:"), m_gboxSettings->plainPage());
    m_redThreshold   = new RIntNumInput(m_gboxSettings->plainPage());
    m_redThreshold->setRange(10, 90, 1);
    m_redThreshold->setSliderEnabled(true);
    m_redThreshold->setDefaultValue(20);
    m_redThreshold->setWhatsThis(i18n("<p>Sets the red color pixels selection threshold.</p>"
                                      "<p>Low values will select more red color pixels "
                                      "(aggressive correction), high values less (mild correction). "
                                      "Use low value if eye have been selected exactly. "
                                      "Use high value if other parts of the face are also selected.</p>"));

    m_smoothLabel = new QLabel(i18nc("Smoothness when blurring border of changed pixels",
                                     "Smooth:"), m_gboxSettings->plainPage());
    m_smoothLevel = new RIntNumInput(m_gboxSettings->plainPage());
    m_smoothLevel->setRange(0, 5, 1);
    m_smoothLevel->setSliderEnabled(true);
    m_smoothLevel->setDefaultValue(1);
    m_smoothLevel->setWhatsThis(i18n("Sets the smoothness value when blurring the border "
                                     "of the changed pixels. "
                                     "This leads to a more naturally looking pupil."));

    QLabel *label3 = new QLabel(i18n("Coloring Tint:"), m_gboxSettings->plainPage());

    m_HSSelector   = new KHueSaturationSelector(m_gboxSettings->plainPage());
    m_HSSelector->setWhatsThis(i18n("Sets a custom color when re-colorizing the eyes."));
    m_HSSelector->setMinimumSize(200, 142);
    m_HSSelector->setChooserMode(ChooserValue);
    m_HSSelector->setColorValue(255);

    m_VSelector    = new KColorValueSelector(m_gboxSettings->plainPage());
    m_VSelector->setChooserMode(ChooserValue);
    m_VSelector->setMinimumSize(26, 142);
    m_VSelector->setIndent(false);

    QLabel *label4 = new QLabel(i18n("Tint Level:"), m_gboxSettings->plainPage());
    m_tintLevel    = new RIntNumInput(m_gboxSettings->plainPage());
    m_tintLevel->setRange(1, 200, 1);
    m_tintLevel->setSliderEnabled(true);
    m_tintLevel->setDefaultValue(128);
    m_tintLevel->setWhatsThis(i18n("Set the tint level to adjust the luminosity of "
                                   "the new color of the pupil."));

    // -------------------------------------------------------------

    gridSettings->addWidget(m_thresholdLabel, 0, 0, 1, 5);
    gridSettings->addWidget(m_redThreshold,   1, 0, 1, 5);
    gridSettings->addWidget(m_smoothLabel,    2, 0, 1, 5);
    gridSettings->addWidget(m_smoothLevel,    3, 0, 1, 5);
    gridSettings->addWidget(label3,           4, 0, 1, 5);
    gridSettings->addWidget(m_HSSelector,     5, 0, 1, 4);
    gridSettings->addWidget(m_VSelector,      5, 4, 1, 1);
    gridSettings->addWidget(label4,           6, 0, 1, 5);
    gridSettings->addWidget(m_tintLevel,      7, 0, 1, 5);
    gridSettings->setRowStretch(8, 10);
    gridSettings->setColumnStretch(3, 10);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

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
            this, SLOT(slotVChanged(int)));

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
    QColor color;

    int val = m_selColor.value();

    color.setHsv(h, s, val);
    setColor(color);
}

void RedEyeTool::slotVChanged(int v)
{
    QColor color;

    int hue = m_selColor.hue();
    int sat = m_selColor.saturation();

    color.setHsv(hue, sat, v);
    setColor(color);
}

void RedEyeTool::setColor(QColor c)
{
    if (c.isValid())
    {
        m_selColor = c;

        // set values
        m_HSSelector->setValues(c.hue(), c.saturation());
        m_VSelector->setValue(c.value());

        // set colors
        m_HSSelector->blockSignals(true);
        m_HSSelector->setHue(c.hue());
        m_HSSelector->setSaturation(c.saturation());
        m_HSSelector->setColorValue(c.value());
        m_HSSelector->updateContents();
        m_HSSelector->blockSignals(false);
        m_HSSelector->repaint();

        m_VSelector->blockSignals(true);
        m_VSelector->setHue(c.hue());
        m_VSelector->setSaturation(c.saturation());
        m_VSelector->setColorValue(c.value());
        m_VSelector->updateContents();
        m_VSelector->blockSignals(false);
        m_VSelector->repaint();

        slotTimer();
    }
}

void RedEyeTool::slotColorSelectedFromTarget(const DColor& color)
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void RedEyeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("redeye Tool");

    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)HistogramWidget::LogScaleHistogram));

    m_redThreshold->setValue(group.readEntry("RedThreshold", m_redThreshold->defaultValue()));
    m_smoothLevel->setValue(group.readEntry("SmoothLevel", m_smoothLevel->defaultValue()));
    m_HSSelector->setHue(group.readEntry("HueColoringTint", 0));
    m_HSSelector->setSaturation(group.readEntry("SatColoringTint", 128));
    m_VSelector->setValue(group.readEntry("ValColoringTint", 255));
    m_tintLevel->setValue(group.readEntry("TintLevel", m_tintLevel->defaultValue()));

    QColor col;
    col.setHsv(m_HSSelector->hue(),
               m_HSSelector->saturation(),
               m_VSelector->value());
    setColor(col);
}

void RedEyeTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("redeye Tool");
    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());
    group.writeEntry("RedThreshold", m_redThreshold->value());
    group.writeEntry("SmoothLevel", m_smoothLevel->value());
    group.writeEntry("HueColoringTint", m_HSSelector->hue());
    group.writeEntry("SatColoringTint", m_HSSelector->saturation());
    group.writeEntry("ValColoringTint", m_VSelector->value());
    group.writeEntry("TintLevel", m_tintLevel->value());
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
    m_tintLevel->slotReset();

    // Black color by default
    QColor col;
    col.setHsv(0, 0, 0);
    setColor(col);

    m_redThreshold->blockSignals(false);
    m_HSSelector->blockSignals(false);
    m_VSelector->blockSignals(false);
    m_tintLevel->blockSignals(false);

    slotEffect();
}

void RedEyeTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    m_gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

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
    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void RedEyeTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

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
    int hue            = m_VSelector->hue();
    int sat            = m_VSelector->saturation();
    int val            = m_VSelector->value();
    QColor coloring    = QColor::fromHsv(hue, sat, val);

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
                r1 = qMin(255, (int)(red_norm * (red_chan.red_gain   * r +
                                                 red_chan.green_gain * g +
                                                 red_chan.blue_gain  * b)));

                g1 = qMin(255, (int)(green_norm * (green_chan.red_gain   * r +
                                                   green_chan.green_gain * g +
                                                   green_chan.blue_gain  * b)));

                b1 = qMin(255, (int)(blue_norm * (blue_chan.red_gain   * r +
                                                  blue_chan.green_gain * g +
                                                  blue_chan.blue_gain  * b)));

                mptr[0] = b1;
                mptr[1] = g1;
                mptr[2] = r1;
                mptr[3] = qMin( (int)((r-g) / 150.0 * 255.0), 255);
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
                r1 = qMin(65535, (int)(red_norm * (red_chan.red_gain   * r +
                                                         red_chan.green_gain * g +
                                                         red_chan.blue_gain  * b)));

                g1 = qMin(65535, (int)(green_norm * (green_chan.red_gain   * r +
                                                            green_chan.green_gain * g +
                                                            green_chan.blue_gain  * b)));

                b1 = qMin(65535, (int)(blue_norm * (blue_chan.red_gain   * r +
                                                          blue_chan.green_gain * g +
                                                          blue_chan.blue_gain  * b)));

                mptr[0] = b1;
                mptr[1] = g1;
                mptr[2] = r1;
                mptr[3] = qMin( (int)((r-g) / 38400.0 * 65535.0), 65535);
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

}  // namespace DigikamImagesPluginCore
