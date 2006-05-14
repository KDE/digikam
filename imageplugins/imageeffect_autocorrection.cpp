/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-31
 * Description : Auto-Color correction tool.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h> 
#include <qradiobutton.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h> 
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimgimagefilters.h"
#include "dimg.h"
#include "listboxpreviewitem.h"

// Local includes.

#include "imageeffect_autocorrection.h"

namespace DigikamImagesPluginCore
{

ImageEffect_AutoCorrection::ImageEffect_AutoCorrection(QWidget* parent)
                          : Digikam::ImageDlgBase(parent, i18n("Auto Color Correction"), "autocorrection", false)
                          ,m_destinationPreviewData(0L)
{
    setHelp("autocolorcorrectiontool.anchor", "digikam");
    
    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget(plainPage(),
                                               i18n("<p>Here you can see the auto-color correction tool preview. "
                                                    "You can pick color on image "
                                                    "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);
    
    // -------------------------------------------------------------

    Digikam::ImageIface iface(0, 0);
    m_thumbnailImage = iface.getOriginalImg()->smoothScale(128, 128, QSize::ScaleMin);

    
    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 4, 4, marginHint(), spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gboxSettings );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    m_scaleBG = new QHButtonGroup(gboxSettings);
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));
    
    QPushButton *linHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    m_scaleBG->insert(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);
    
    QPushButton *logHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    m_scaleBG->insert(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);       

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addWidget(m_scaleBG);
    l1->addStretch(10);
    
    gridSettings->addMultiCellLayout(l1, 0, 0, 0, 4);

    // -------------------------------------------------------------

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, gboxSettings, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing of the "
                                             "selected image channel. This one is re-computed at any "
                                             "settings changes."));
    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, gboxSettings );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    
    gridSettings->addMultiCellWidget(m_histogramWidget, 1, 1, 0, 4);
    gridSettings->addMultiCellWidget(m_hGradient, 2, 2, 0, 4);

    // -------------------------------------------------------------
    
    m_correctionTools = new QListBox(gboxSettings);
    m_correctionTools->setColumnMode(1);
    m_correctionTools->setVariableWidth(false);
    m_correctionTools->setVariableHeight(false);
    Digikam::ListBoxWhatsThis* whatsThis = new Digikam::ListBoxWhatsThis(m_correctionTools);

    QPixmap pix = getThumbnailForEffect(AutoLevelsCorrection);
    
    QListBoxItem *item = new Digikam::ListBoxPreviewItem(pix, i18n("Auto Levels"));
    
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Auto Levels</b>:"
                               "<p>This option maximizes the tonal range in the Red, "
                               "Green, and Blue channels. It search the image shadow and highlight "
                               "limit values and adjust the Red, Green, and Blue channels "
                               "to a full histogram range.</p>").arg(previewEffectPic("autolevels")));
    m_correctionTools->insertItem(item, AutoLevelsCorrection);

    pix = getThumbnailForEffect(NormalizeCorrection);
    item = new Digikam::ListBoxPreviewItem(pix, i18n("Normalize"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Normalize</b>:"
                               "<p>This option scales brightness values across the active "
                               "image so that the darkest point becomes black, and the "
                               "brightest point becomes as bright as possible without "
                               "altering its hue. This is often a \"magic fix\" for "
                               "images that are dim or washed out.</p>").arg(previewEffectPic("normalize")));
    m_correctionTools->insertItem(item, NormalizeCorrection);

    pix = getThumbnailForEffect(EqualizeCorrection);
    item = new Digikam::ListBoxPreviewItem(pix, i18n("Equalize"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Equalize</b>:"
                                    "<p>This option adjusts the brightness of colors across the "
                                    "active image so that the histogram for the value channel "
                                    "is as nearly as possible flat, that is, so that each possible "
                                    "brightness value appears at about the same number of pixels "
                                    "as each other value. Sometimes Equalize works wonderfully at "
                                    "enhancing the contrasts in an image. Other times it gives "
                                    "garbage. It is a very powerful operation, which can either work "
                                    "miracles on an image or destroy it.</p>").arg(previewEffectPic("equalize")));
    m_correctionTools->insertItem(item, EqualizeCorrection);

    pix = getThumbnailForEffect(StretchContrastCorrection);
    item = new Digikam::ListBoxPreviewItem(pix, i18n("Stretch Contrast"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Stretch Contrast</b>:"
                               "<p>This option enhances the contrast and brightness "
                               "of the RGB values of an image by stretching the lowest "
                               "and highest values to their fullest range, adjusting "
                               "everything in between.</p>").arg(previewEffectPic("stretchcontrast")));
    m_correctionTools->insertItem(item, StretchContrastCorrection);

    // -------------------------------------------------------------
    
    m_correctionTools->setFocus();
    gridSettings->addMultiCellWidget(m_correctionTools, 3, 3, 0, 4);
    gridSettings->setRowStretch(3, 10);
    setUserAreaWidget(gboxSettings);
   
    // Reset all parameters to the default values.
    QTimer::singleShot(0, this, SLOT(slotDefault()));
    
    // -------------------------------------------------------------
    
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_correctionTools, SIGNAL(highlighted(int)),
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

    m_histogramWidget->repaint(false);
}

void ImageEffect_AutoCorrection::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_AutoCorrection::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

QString ImageEffect_AutoCorrection::previewEffectPic(QString name)
{
    KGlobal::dirs()->addResourceType(name.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return ( KGlobal::dirs()->findResourceDir(name.ascii(), name + ".png") + name + ".png" );
}

void ImageEffect_AutoCorrection::slotDefault()
{
    m_correctionTools->blockSignals(true);
    m_correctionTools->setCurrentItem( AutoLevelsCorrection );
    m_correctionTools->blockSignals(false);
    slotEffect();
}

void ImageEffect_AutoCorrection::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    Digikam::ImageIface* iface      = m_previewWidget->imageIface();
    uchar *m_destinationPreviewData = iface->getPreviewImage();
    int w                           = iface->previewWidth();
    int h                           = iface->previewHeight();
    bool sb                         = iface->previewSixteenBit();

    autoCorrection(m_destinationPreviewData, w, h, sb, m_correctionTools->currentItem());

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
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

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
    }
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_autocorrection.moc"
