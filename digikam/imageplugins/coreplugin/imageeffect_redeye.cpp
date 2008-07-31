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

#include <QColor>
#include <QGroupBox>
#include <QButtonGroup> 
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <kcolordialog.h>
#include <kglobal.h>
#include <kvbox.h>
#include <khuesaturationselect.h>
#include <kcolorvalueselector.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "bcgmodifier.h"
#include "dimg.h"
#include "dimgimagefilters.h"

// Local includes.

#include "imageeffect_redeye.h"
#include "imageeffect_redeye.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_RedEye::ImageEffect_RedEye(QWidget* parent)
                  : Digikam::ImageDlgBase(parent, i18n("Red Eye Reduction"), "redeye", false)
{
    m_destinationPreviewData = 0L;
    setHelp("redeyecorrectiontool.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget("redeye Tool Dialog", mainWidget(),
                                               i18n("<p>Here you can see the image selection preview with "
                                                    "red eye reduction applied."),
                                               true, Digikam::ImageGuideWidget::PickColorMode, true, true);
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( gboxSettings );
    m_channelCB->addItem( i18n("Luminosity") );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->setWhatsThis( i18n("<p>Select the histogram channel to display:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image channel values.<p>"
                                    "<b>Green</b>: display the green image channel values.<p>"
                                    "<b>Blue</b>: display the blue image channel values.<p>"));

    // -------------------------------------------------------------

    QWidget *scaleBox = new QWidget(gboxSettings);
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    m_scaleBG         = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale.<p>"
                                "If the image's maximal counts are small, you can use the linear scale.<p>"
                                "Logarithmic scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/histogram-lin.png")));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);

    QPushButton *logHistoButton = new QPushButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/histogram-log.png")));
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
    m_histogramWidget->setWhatsThis(i18n("<p>Here you can see the target preview image histogram "
                                         "of the selected image channel. It is "
                                         "updated upon setting changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox);
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // -------------------------------------------------------------

    m_thresholdLabel = new QLabel(i18n("Sensitivity:"), gboxSettings);
    m_redThreshold   = new KIntNumInput(gboxSettings);
    m_redThreshold->setRange(10, 90, 1);
    m_redThreshold->setSliderEnabled(true);
    m_redThreshold->setValue(20);
    m_redThreshold->setWhatsThis(i18n("<p>Sets the red color pixels selection threshold. "
                                      "Low values will select more red color pixels (agressive correction), high "
                                      "values less (mild correction). Use low value if eye have been selected "
                                      "exactly. Use high value if other parts of the face are also selected."));

    m_smoothLabel = new QLabel(i18n("Smooth:"), gboxSettings);
    m_smoothLevel = new KIntNumInput(gboxSettings);
    m_smoothLevel->setRange(0, 5, 1);
    m_smoothLevel->setSliderEnabled(true);
    m_smoothLevel->setValue(1);
    m_smoothLevel->setWhatsThis(i18n("<p>Sets the smoothness value when blurring the border "
                                     "of the changed pixels. "
                                     "This leads to a more naturally looking pupil."));

    QLabel *label3 = new QLabel(i18n("Coloring Tint:"), gboxSettings);
    m_HSSelector   = new KHueSaturationSelector(gboxSettings);
    m_VSelector    = new KColorValueSelector(gboxSettings);
    m_HSSelector->setWhatsThis(i18n("<p>Sets a custom color when re-colorizing the eyes."));
    m_HSSelector->setMinimumSize(200, 142);
    m_HSSelector->setChooserMode(ChooserValue);
    m_HSSelector->setColorValue(255);
    m_VSelector->setChooserMode(ChooserValue);
    m_VSelector->setMinimumSize(26, 142);

    QLabel *label4 = new QLabel(i18n("Tint Level:"), gboxSettings);
    m_tintLevel    = new KIntNumInput(gboxSettings);
    m_tintLevel->setRange(1, 200, 1);
    m_tintLevel->setSliderEnabled(true);
    m_tintLevel->setValue(128);
    m_tintLevel->setWhatsThis(i18n("<p>Set the tint level to adjust the luminosity of "
                                   "the new color of the pupil."));

    // -------------------------------------------------------------

    gridSettings->addLayout(l1,               0, 0, 1, 5);
    gridSettings->addWidget(histoBox,         1, 0, 2, 5);
    gridSettings->addWidget(m_thresholdLabel, 3, 0, 1, 5);
    gridSettings->addWidget(m_redThreshold,   4, 0, 1, 5);
    gridSettings->addWidget(m_smoothLabel,    5, 0, 1, 5);
    gridSettings->addWidget(m_smoothLevel,    6, 0, 1, 5);
    gridSettings->addWidget(label3,           7, 0, 1, 5);
    gridSettings->addWidget(m_HSSelector,     8, 0, 1, 4);
    gridSettings->addWidget(m_VSelector,      8, 4, 1, 1);
    gridSettings->addWidget(label4,           9, 0, 1, 5);
    gridSettings->addWidget(m_tintLevel,     10, 0, 1, 5);
    gridSettings->setRowStretch(11, 10);
    gridSettings->setColumnStretch(3, 10);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
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

ImageEffect_RedEye::~ImageEffect_RedEye()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_RedEye::slotHSChanged(int h, int s)
{
    m_VSelector->blockSignals(true);
    m_VSelector->setHue(h);
    m_VSelector->setSaturation(s);
    m_VSelector->updateContents();
    m_VSelector->repaint();
    m_VSelector->blockSignals(false);
    slotTimer();
}

void ImageEffect_RedEye::slotChannelChanged(int channel)
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

void ImageEffect_RedEye::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ImageEffect_RedEye::slotColorSelectedFromTarget(const Digikam::DColor& color)
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_RedEye::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("redeye Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0)); // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale", 
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    m_redThreshold->setValue(group.readEntry("RedThreshold", 20));
    m_smoothLevel->setValue(group.readEntry("SmoothLevel", 1));
    m_HSSelector->setXValue(group.readEntry("HueColoringTint", 0));
    m_HSSelector->setYValue(group.readEntry("SatColoringTint", 0));
    m_VSelector->setValue(group.readEntry("ValColoringTint", 0));
    m_tintLevel->setValue(group.readEntry("TintLevel", 128));

    slotHSChanged(m_HSSelector->xValue(), m_HSSelector->yValue());
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ImageEffect_RedEye::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("redeye Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());
    group.writeEntry("RedThreshold", m_redThreshold->value());
    group.writeEntry("SmoothLevel", m_smoothLevel->value());
    group.writeEntry("HueColoringTint", m_HSSelector->xValue());
    group.writeEntry("SatColoringTint", m_HSSelector->yValue());
    group.writeEntry("ValColoringTint", m_VSelector->value());
    group.writeEntry("TintLevel", m_tintLevel->value());
    config->sync();
}

void ImageEffect_RedEye::resetValues()
{
    m_redThreshold->blockSignals(true);
    m_HSSelector->blockSignals(true);
    m_VSelector->blockSignals(true);
    m_tintLevel->blockSignals(true);

    m_redThreshold->setValue(20);
    m_smoothLevel->setValue(1);

    // Black color by default
    m_HSSelector->setXValue(0);
    m_HSSelector->setYValue(0);
    m_VSelector->setValue(0);

    m_tintLevel->setValue(128);

    m_redThreshold->blockSignals(false);
    m_HSSelector->blockSignals(false);
    m_VSelector->blockSignals(false);
    m_tintLevel->blockSignals(false);
} 

void ImageEffect_RedEye::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    // Here, we need to use the real selection image data because we will apply 
    // a Gaussian blur filter on pixels and we cannot use directly the preview scaled image 
    // else the blur radius will not give the same result between preview and final rendering.  
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getImageSelection();
    int w                      = iface->selectedWidth();
    int h                      = iface->selectedHeight();
    bool sb                    = iface->originalSixteenBit();
    bool a                     = iface->originalHasAlpha();
    Digikam::DImg selection(w, h, sb, a, m_destinationPreviewData);

    redEyeFilter(selection);

    Digikam::DImg preview = selection.smoothScale(iface->previewWidth(), iface->previewHeight());

    iface->putPreviewImage(preview.bits());
    m_previewWidget->updatePreview();

    // Update histogram.

    memcpy(m_destinationPreviewData, selection.bits(), selection.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_RedEye::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getImageSelection();
    int w                      = iface->selectedWidth();
    int h                      = iface->selectedHeight();
    bool sixteenBit            = iface->originalSixteenBit();
    bool hasAlpha              = iface->originalHasAlpha();
    Digikam::DImg selection(w, h, sixteenBit, hasAlpha, data);
    delete [] data;

    redEyeFilter(selection);

    iface->putImageSelection(i18n("Red Eyes Correction"), selection.bits());

    kapp->restoreOverrideCursor();
    accept();
}

void ImageEffect_RedEye::redEyeFilter(Digikam::DImg& selection)
{
    Digikam::DImg mask(selection.width(), selection.height(), selection.sixteenBit(), true,
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

    Digikam::DImg mask2 = mask.copy();
    Digikam::DImgImageFilters filter;
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

    Digikam::DColorComposer *composer = Digikam::DColorComposer::getComposer(Digikam::DColorComposer::PorterDuffSrcOver);

    // NOTE: 'mask' is the Source image, 'selection' is the Destination image.

    selection.bitBlendImage(composer, &mask,
                            0, 0, mask.width(), mask.height(),
                            0, 0);
}

}  // NameSpace DigikamImagesPluginCore
