/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at free.fr>
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
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// Digikam includes.

#include "imageiface.h"
#include "imagefilters.h"
#include "imageguidewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_autocorrection.h"

namespace DigikamImagesPluginCore
{

ImageEffect_AutoCorrection::ImageEffect_AutoCorrection(QWidget* parent)
                          : Digikam::ImageDlgBase(parent, i18n("Auto Color Correction"), "autocorrection", false)
{
    m_destinationPreviewData = 0L;
    setHelp("autocolorcorrectiontool.anchor", "digikam");
    
    // -------------------------------------------------------------

    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageGuideWidget(480, 320, frame, true, 
                                                    Digikam::ImageGuideWidget::PickColorMode,
                                                    Qt::red, 1, false,
                                                    Digikam::ImageGuideWidget::TargetPreviewImage);
    l->addWidget(m_previewWidget, 0);
    QWhatsThis::add( m_previewWidget, i18n("<p>Here you can see the auto-color correction tool preview. "
                                           "You can pick color on image "
                                           "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(frame); 
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 5, 4, marginHint(), spacingHint());

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
    
    QLabel *labelType = new QLabel(i18n("Auto-Color correction tool:"), gboxSettings);
    m_typeCB = new QComboBox( false, gboxSettings );
    m_typeCB->insertItem( previewEffectPic("autolevels"),      i18n("Auto Levels") );
    m_typeCB->insertItem( previewEffectPic("normalize"),       i18n("Normalize") );
    m_typeCB->insertItem( previewEffectPic("equalize"),        i18n("Equalize") );
    m_typeCB->insertItem( previewEffectPic("stretchcontrast"), i18n("Stretch Contrast") );
    m_typeCB->setCurrentItem( AutoLevelsCorrection );
    QWhatsThis::add( m_typeCB, i18n("<p>Select here the auto-color correction tool to use:<p>"
                                    "<b>Auto Levels</b>: This option maximizes the tonal range in the Red, "
                                    "Green, and Blue channels. It search the image shadow and highlight "
                                    "limit values and adjust the Red, Green, and Blue channels "
                                    "to a full histogram range.<p>"
                                    "<b>Normalize</b>: this option scales brightness values across the active "
                                    "image so that the darkest point becomes black, and the "
                                    "brightest point becomes as bright as possible without "
                                    "altering its hue. This is often a \"magic fix\" for "
                                    "images that are dim or washed out.<p>"
                                    "<b>Equalize</b>: this option adjusts the brightness of colors across the "
                                    "active image so that the histogram for the value channel "
                                    "is as nearly as possible flat, that is, so that each possible "
                                    "brightness value appears at about the same number of pixels "
                                    "as each other value. Sometimes Equalize works wonderfully at "
                                    "enhancing the contrasts in an image. Other times it gives "
                                    "garbage. It is a very powerful operation, which can either work "
                                    "miracles on an image or destroy it.<p>"
                                    "<b>Stretch Contrast</b>: this option enhances the contrast and brightness "
                                    "of the RGB values of an image by stretching the lowest "
                                    "and highest values to their fullest range, adjusting "
                                    "everything in between."
                                    ));

    gridSettings->addMultiCellWidget(labelType, 3, 3, 0, 4);
    gridSettings->addMultiCellWidget(m_typeCB, 4, 4, 0, 4);
    
    gridSettings->setRowStretch(5, 10);
    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChanged( const Digikam::DColor &, bool, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_typeCB, SIGNAL(activated(int)),
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

QPixmap ImageEffect_AutoCorrection::previewEffectPic(QString name)
{
    KGlobal::dirs()->addResourceType(name.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return ( QPixmap::QPixmap(KGlobal::dirs()->findResourceDir(name.ascii(), name + ".png") + name + ".png") );
}

void ImageEffect_AutoCorrection::slotDefault()
{
    m_typeCB->blockSignals(true);
    m_typeCB->setCurrentItem( AutoLevelsCorrection );
    m_typeCB->blockSignals(false);
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

    autoCorrection(m_destinationPreviewData, w, h, sb, m_typeCB->currentItem());

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.
   
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_AutoCorrection::slotOk()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    if (data)
    {
       int type = m_typeCB->currentItem();
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
    switch (type)
    {
       case AutoLevelsCorrection:
          Digikam::ImageFilters::autoLevelsCorrectionImage(data, w, h, sb);
          break;
       
       case NormalizeCorrection:
          Digikam::ImageFilters::normalizeImage(data, w, h, sb);
          break;
       
       case EqualizeCorrection:
          Digikam::ImageFilters::equalizeImage(data, w, h, sb);
          break;
       
       case StretchContrastCorrection:
          Digikam::ImageFilters::stretchContrastImage(data, w, h, sb);
          break;
    }
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_autocorrection.moc"
