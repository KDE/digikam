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

#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtimer.h>
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

// Digikam includes.

#include "colorgradientwidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "listboxpreviewitem.h"
#include "whitebalance.h"

// Local includes.

#include "autocorrectiontool.h"
#include "autocorrectiontool.moc"

using namespace Digikam;

namespace DigikamImagesPluginCore
{

AutoCorrectionTool::AutoCorrectionTool(QObject* parent)
                  : EditorTool(parent)
{
    setName("autocorrection");
    setToolName(i18n("Auto-Correction"));
    setToolIcon(SmallIcon("autocorrection"));
    setToolHelp("autocolorcorrectiontool.anchor");

    // -------------------------------------------------------------

    m_previewWidget = new ImageWidget("autocorrection Tool", 0,
                                      i18n("<p>Here you can see the auto-color correction tool "
                                           "preview. You can pick color on image "
                                           "to see the color level corresponding on histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    ImageIface iface(0, 0);
    m_thumbnailImage         = iface.getOriginalImg()->smoothScale(128, 128, QSize::ScaleMin);
    m_destinationPreviewData = 0;

    EditorToolSettings *gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                                              EditorToolSettings::Ok|
                                                              EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout(gboxSettings->plainPage(), 2, 4);

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings->plainPage());
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gboxSettings->plainPage() );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select the histogram channel to display here:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    m_scaleBG = new QHButtonGroup(gboxSettings->plainPage());
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select the histogram scale here.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton(m_scaleBG);
    QToolTip::add(linHistoButton, i18n("<p>Linear"));
    m_scaleBG->insert(linHistoButton, HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
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
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing "
                                             "of the selected image channel. This one is re-computed at any "
                                             "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new ColorGradientWidget(ColorGradientWidget::Horizontal, 10, histoBox);
    m_hGradient->setColors(QColor("black"), QColor("white"));

    // -------------------------------------------------------------

    m_correctionTools = new QListBox(gboxSettings->plainPage());
    m_correctionTools->setColumnMode(1);
    m_correctionTools->setVariableWidth(false);
    m_correctionTools->setVariableHeight(false);
    ListBoxWhatsThis* whatsThis = new ListBoxWhatsThis(m_correctionTools);

    QPixmap pix = getThumbnailForEffect(AutoLevelsCorrection);
    ListBoxPreviewItem *item = new ListBoxPreviewItem(pix, i18n("Auto Levels"));
    whatsThis->add( item, i18n("<b>Auto Levels</b>:"
                               "<p>This option maximizes the tonal range in the Red, "
                               "Green, and Blue channels. It searches the image shadow and highlight "
                               "limit values and adjusts the Red, Green, and Blue channels "
                               "to a full histogram range.</p>"));
    m_correctionTools->insertItem(item, AutoLevelsCorrection);

    pix = getThumbnailForEffect(NormalizeCorrection);
    item = new ListBoxPreviewItem(pix, i18n("Normalize"));
    whatsThis->add( item, i18n("<b>Normalize</b>:"
                               "<p>This option scales brightness values across the active "
                               "image so that the darkest point becomes black, and the "
                               "brightest point becomes as bright as possible without "
                               "altering its hue. This is often a \"magic fix\" for "
                               "images that are dim or washed out.</p>"));
    m_correctionTools->insertItem(item, NormalizeCorrection);

    pix = getThumbnailForEffect(EqualizeCorrection);
    item = new ListBoxPreviewItem(pix, i18n("Equalize"));
    whatsThis->add( item, i18n("<b>Equalize</b>:"
                               "<p>This option adjusts the brightness of colors across the "
                               "active image so that the histogram for the value channel "
                               "is as nearly as possible flat, that is, so that each possible "
                               "brightness value appears at about the same number of pixels "
                               "as each other value. Sometimes Equalize works wonderfully at "
                               "enhancing the contrasts in an image. Other times it gives "
                               "garbage. It is a very powerful operation, which can either work "
                               "miracles on an image or destroy it.</p>"));
    m_correctionTools->insertItem(item, EqualizeCorrection);

    pix = getThumbnailForEffect(StretchContrastCorrection);
    item = new ListBoxPreviewItem(pix, i18n("Stretch Contrast"));
    whatsThis->add( item, i18n("<b>Stretch Contrast</b>:"
                               "<p>This option enhances the contrast and brightness "
                               "of the RGB values of an image by stretching the lowest "
                               "and highest values to their fullest range, adjusting "
                               "everything in between.</p>"));
    m_correctionTools->insertItem(item, StretchContrastCorrection);

    pix = getThumbnailForEffect(AutoExposureCorrection);
    item = new ListBoxPreviewItem(pix, i18n("Auto Exposure"));
    whatsThis->add( item, i18n("<b>Auto Exposure</b>:"
                               "<p>This option enhances the contrast and brightness "
                               "of the RGB values of an image to calculate optimal "
                               "exposition and black level using image histogram "
                               "properties.</p>"));
    m_correctionTools->insertItem(item, AutoExposureCorrection);

    // -------------------------------------------------------------

    m_correctionTools->setFocus();
    gridSettings->addMultiCellLayout(l1,                0, 0, 0, 4);
    gridSettings->addMultiCellWidget(histoBox,          1, 1, 0, 4);
    gridSettings->addMultiCellWidget(m_correctionTools, 2, 2, 0, 4);
    gridSettings->setRowStretch(2, 10);
    gridSettings->setSpacing(gboxSettings->spacingHint());
    gridSettings->setMargin(gboxSettings->spacingHint());

    setToolSettings(gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget(const DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const DColor&)));

    connect(m_correctionTools, SIGNAL(highlighted(int)),
            this, SLOT(slotEffect()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));
}

AutoCorrectionTool::~AutoCorrectionTool()
{
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void AutoCorrectionTool::slotChannelChanged(int channel)
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

    m_histogramWidget->repaint(false);
}

void AutoCorrectionTool::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void AutoCorrectionTool::slotColorSelectedFromTarget(const DColor& color)
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void AutoCorrectionTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("autocorrection Tool");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    m_correctionTools->setCurrentItem(config->readNumEntry("Auto Correction Filter", AutoLevelsCorrection));
    m_histogramWidget->reset();
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void AutoCorrectionTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("autocorrection Tool");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("Auto Correction Filter", m_correctionTools->currentItem());
    m_previewWidget->writeSettings();
    config->sync();
}

void AutoCorrectionTool::slotResetSettings()
{
    m_correctionTools->blockSignals(true);
    m_correctionTools->setCurrentItem(AutoLevelsCorrection);
    m_correctionTools->blockSignals(false);

    slotEffect();
}

void AutoCorrectionTool::slotEffect()
{
    kapp->setOverrideCursor(KCursor::waitCursor());

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    ImageIface* iface        = m_previewWidget->imageIface();
    m_destinationPreviewData = iface->getPreviewImage();
    int w                    = iface->previewWidth();
    int h                    = iface->previewHeight();
    bool sb                  = iface->previewSixteenBit();

    autoCorrection(m_destinationPreviewData, w, h, sb, m_correctionTools->currentItem());

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.

    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

QPixmap AutoCorrectionTool::getThumbnailForEffect(AutoCorrectionType type)
{
    DImg thumb = m_thumbnailImage.copy();
    autoCorrection(thumb.bits(), thumb.width(), thumb.height(), thumb.sixteenBit(), type);
    return (thumb.convertToPixmap());
}


void AutoCorrectionTool::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *data       = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    if (data)
    {
       int type = m_correctionTools->currentItem();
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
}

void AutoCorrectionTool::autoCorrection(uchar *data, int w, int h, bool sb, int type)
{
    DImgImageFilters filter;

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
            WhiteBalance wbFilter(sb);
            double blackLevel;
            double exposureLevel;
            wbFilter.autoExposureAdjustement(data, w, h, sb, blackLevel, exposureLevel);
            wbFilter.whiteBalance(data, w, h, sb, blackLevel, exposureLevel);
        break;
    }
}

}  // NameSpace DigikamImagesPluginCore

