/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : Auto-Color correction tool.
 *
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

#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
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
#include "dimgimagefilters.h"
#include "whitebalance.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_autocorrection.h"
#include "imageeffect_autocorrection.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_AutoCorrection::ImageEffect_AutoCorrection(QWidget* parent)
                          : Digikam::ImageDlgBase(parent, i18n("Auto Color Correction"),
                            "autocorrection", false), m_destinationPreviewData(0L)
{
    setHelp("autocolorcorrectiontool.anchor", "digikam");

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("autocorrection Tool Dialog", mainWidget(),
                                               i18n("<p>Here you can see the auto-color correction tool "
                                                    "preview. You can pick color on image "
                                                    "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------

    Digikam::ImageIface iface(0, 0);
    m_thumbnailImage = iface.getOriginalImg()->smoothScale(128, 128, Qt::ScaleMin);

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

    m_correctionTools = new QListWidget(gboxSettings);
    m_correctionTools->setIconSize(QSize(128, 128));

    QPixmap pix = getThumbnailForEffect(AutoLevelsCorrection);
    QListWidgetItem *item = new QListWidgetItem(QIcon(pix), i18n("Auto Levels"));
    item->setWhatsThis(i18n("<b>Auto Levels</b>:"
                            "<p>This option maximizes the tonal range in the Red, "
                            "Green, and Blue channels. It searches the image shadow and highlight "
                            "limit values and adjusts the Red, Green, and Blue channels "
                            "to a full histogram range.</p>"));
    m_correctionTools->insertItem(AutoLevelsCorrection, item);

    pix = getThumbnailForEffect(NormalizeCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Normalize"));
    item->setWhatsThis(i18n("<b>Normalize</b>:"
                            "<p>This option scales brightness values across the active "
                            "image so that the darkest point becomes black, and the "
                            "brightest point becomes as bright as possible without "
                            "altering its hue. This is often a \"magic fix\" for "
                            "images that are dim or washed out.</p>"));
    m_correctionTools->insertItem(NormalizeCorrection, item);

    pix = getThumbnailForEffect(EqualizeCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Equalize"));
    item->setWhatsThis(i18n("<b>Equalize</b>:"
                            "<p>This option adjusts the brightness of colors across the "
                            "active image so that the histogram for the value channel "
                            "is as nearly as possible flat, that is, so that each possible "
                            "brightness value appears at about the same number of pixels "
                            "as each other value. Sometimes Equalize works wonderfully at "
                            "enhancing the contrasts in an image. Other times it gives "
                            "garbage. It is a very powerful operation, which can either work "
                            "miracles on an image or destroy it.</p>"));
    m_correctionTools->insertItem(EqualizeCorrection, item);

    pix = getThumbnailForEffect(StretchContrastCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Stretch Contrast"));
    item->setWhatsThis(i18n("<b>Stretch Contrast</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image by stretching the lowest "
                            "and highest values to their fullest range, adjusting "
                            "everything in between.</p>"));
    m_correctionTools->insertItem(StretchContrastCorrection, item);

    pix = getThumbnailForEffect(AutoExposureCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Auto Exposure"));
    item->setWhatsThis(i18n("<b>Auto Exposure</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image to calculate optimal "
                            "exposition and black level using image histogram "
                            "properties.</p>"));
    m_correctionTools->insertItem(AutoExposureCorrection, item);

    // -------------------------------------------------------------

    m_correctionTools->setFocus();

    gridSettings->addLayout(l1,                0, 0, 1, 5);
    gridSettings->addWidget(histoBox,          1, 0, 2, 5);
    gridSettings->addWidget(m_correctionTools, 3, 0, 1, 5);
    gridSettings->setRowStretch(3, 10);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_correctionTools, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotEffect()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));
}

ImageEffect_AutoCorrection::~ImageEffect_AutoCorrection()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_AutoCorrection::slotChannelChanged(int channel)
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

void ImageEffect_AutoCorrection::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ImageEffect_AutoCorrection::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_AutoCorrection::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("autocorrection Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    m_correctionTools->setCurrentRow(group.readEntry("Auto Correction Filter", (int)AutoLevelsCorrection));
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ImageEffect_AutoCorrection::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("autocorrection Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());
    group.writeEntry("Auto Correction Filter", m_correctionTools->currentRow());
    config->sync();
}

void ImageEffect_AutoCorrection::resetValues()
{
    m_correctionTools->blockSignals(true);
    m_correctionTools->setCurrentRow(AutoLevelsCorrection);
    m_correctionTools->blockSignals(false);
}

void ImageEffect_AutoCorrection::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface      = m_previewWidget->imageIface();
    uchar *m_destinationPreviewData = iface->getPreviewImage();
    int w                           = iface->previewWidth();
    int h                           = iface->previewHeight();
    bool sb                         = iface->previewSixteenBit();

    autoCorrection(m_destinationPreviewData, w, h, sb, m_correctionTools->currentRow());

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.

    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

QPixmap ImageEffect_AutoCorrection::getThumbnailForEffect(AutoCorrectionType type)
{
    Digikam::DImg thumb = m_thumbnailImage.copy();
    autoCorrection(thumb.bits(), thumb.width(), thumb.height(), thumb.sixteenBit(), type);
    return (thumb.convertToPixmap());
}


void ImageEffect_AutoCorrection::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    if (data)
    {
       int type = m_correctionTools->currentRow();
       autoCorrection(data, w, h, sb, type);
       QString name;

       switch (type)
       {
          case AutoLevelsCorrection:
             name = i18n("Auto Levels");
          break;

          case NormalizeCorrection:
             name = i18n("Normalize");
          break;

          case EqualizeCorrection:
             name = i18n("Equalize");
          break;

          case StretchContrastCorrection:
             name = i18n("Stretch Contrast");
          break;

          case AutoExposureCorrection:
             name = i18n("Auto Exposure");
          break;
       }

       iface->putOriginalImage(name, data);
       delete [] data;
    }

    kapp->restoreOverrideCursor();
    accept();
}

void ImageEffect_AutoCorrection::autoCorrection(uchar *data, int w, int h, bool sb, int type)
{
    Digikam::DImgImageFilters filter;

    switch (type)
    {
        case AutoLevelsCorrection:
            filter.autoLevelsCorrectionImage(data, w, h, sb);
            break;

        case NormalizeCorrection:
            filter.normalizeImage(data, w, h, sb);
            break;

        case EqualizeCorrection:
            filter.equalizeImage(data, w, h, sb);
            break;

        case StretchContrastCorrection:
            filter.stretchContrastImage(data, w, h, sb);
            break;

        case AutoExposureCorrection:
            Digikam::WhiteBalance wbFilter(sb);
            double blackLevel;
            double exposureLevel;
            wbFilter.autoExposureAdjustement(data, w, h, sb, blackLevel, exposureLevel);
            wbFilter.whiteBalance(data, w, h, sb, blackLevel, exposureLevel);
        break;
    }
}

}  // NameSpace DigikamImagesPluginCore

