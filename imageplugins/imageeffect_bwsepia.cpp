/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-06
 * Description : Black and White conversion tool.
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

#include "imageeffect_bwsepia.h"

namespace DigikamImagesPluginCore
{

ImageEffect_BWSepia::ImageEffect_BWSepia(QWidget* parent)
                   : Digikam::ImageDlgBase(parent, i18n("Convert to Black & White"), "convertbw", false)
{
    m_destinationPreviewData = 0L;
    setHelp("blackandwhitetool.anchor", "digikam");

    // -------------------------------------------------------------

    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageGuideWidget(480, 320, frame, true, 
                                                    Digikam::ImageGuideWidget::PickColorMode,
                                                    Qt::red, 1, false,
                                                    Digikam::ImageGuideWidget::TargetPreviewImage);
    l->addWidget(m_previewWidget, 0);
    QWhatsThis::add( m_previewWidget, i18n("<p>Here you can see the black and white conversion tool preview. "
                                           "You can pick color on image "
                                           "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(frame); 

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
    
    QLabel *labelType = new QLabel(i18n("Black and white conversion tool:"), gboxSettings);
    m_typeCB = new QComboBox( false, gboxSettings );
    m_typeCB->insertItem( previewEffectPic("neutralbw"), i18n("Neutral Black & White") );
    m_typeCB->insertItem( previewEffectPic("bwgreen"),   i18n("Black & White with Green Filter") );
    m_typeCB->insertItem( previewEffectPic("bworange"),  i18n("Black & White with Orange Filter") );
    m_typeCB->insertItem( previewEffectPic("bwred"),     i18n("Black & White with Red Filter") );
    m_typeCB->insertItem( previewEffectPic("bwyellow"),  i18n("Black & White with Yellow Filter") );
    m_typeCB->insertItem( previewEffectPic("sepia"),     i18n("Black & White with Sepia Tone") );
    m_typeCB->insertItem( previewEffectPic("browntone"), i18n("Black & White with Brown Tone") );
    m_typeCB->insertItem( previewEffectPic("coldtone"),  i18n("Black & White with Cold Tone") );
    m_typeCB->insertItem( previewEffectPic("selenium"),  i18n("Black & White with Selenium Tone") );
    m_typeCB->insertItem( previewEffectPic("platinum"),  i18n("Black & White with Platinum Tone") );
    m_typeCB->setCurrentText( i18n("Neutral Black & White") );
    QWhatsThis::add( m_typeCB, i18n("<p>Select here the black and white conversion type:<p>"
                                    "<b>Neutral</b>: simulate black and white neutral film exposure.<p>"
                                    "<b>Green Filter</b>: simulate black and white film exposure using green filter. "
                                    "This provides an universal asset for all scenics, especially suited for portraits "
                                    "photographed against sky.<p>"
                                    "<b>Orange Filter</b>: simulate black and white film exposure using orange filter. "
                                    "This will enhances landscapes, marine scenes and aerial photography.<p>"
                                    "<b>Red Filter</b>: simulate black and white film exposure using red filter. "
                                    "Using this one creates dramatic sky effects and simulates moonlight scenes in daytime.<p>"
                                    "<b>Yellow Filter</b>: simulate black and white film exposure using yellow filter. "
                                    "Most natural tonal correction and improves contrast. Ideal for landscapes.<p>"
                                    "<b>Sepia Tone</b>: gives a warm highlight and mid-tone while adding a bit of coolness to "
                                    "the shadows-very similar to the process of bleaching a print and re-developing in a sepia "
                                    "toner.<p>"
                                    "<b>Brown Tone</b>: more neutral than Sepia Tone filter.<p>"
                                    "<b>Cold Tone</b>: start subtle and replicate printing on a cold tone black and white "
                                    "paper such as a bromide enlarging paper.<p>"
                                    "<b>Selenium Tone</b>: effect that replicate traditional selenium chemical toning done "
                                    "in the darkroom.<p>"
                                    "<b>Platinum Tone</b>: effect that replicate traditional platinum chemical toning done "
                                    "in the darkroom."
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

ImageEffect_BWSepia::~ImageEffect_BWSepia()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_BWSepia::slotChannelChanged(int channel)
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

void ImageEffect_BWSepia::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_BWSepia::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

QPixmap ImageEffect_BWSepia::previewEffectPic(QString name)
{
    KGlobal::dirs()->addResourceType(name.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return ( QPixmap::QPixmap(KGlobal::dirs()->findResourceDir(name.ascii(), name + ".png") + name + ".png") );
}

void ImageEffect_BWSepia::slotDefault()
{
    m_typeCB->blockSignals(true);
    m_typeCB->setCurrentItem( BWNeutral );
    m_typeCB->blockSignals(false);
    slotEffect();
}

void ImageEffect_BWSepia::slotEffect()
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

    blackAndWhiteConversion(m_destinationPreviewData, w, h, sb, m_typeCB->currentItem());

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.
   
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_BWSepia::slotOk()
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
       blackAndWhiteConversion(data, w, h, sb, type);
       QString name;
       
       switch (type)
       {
          case BWNeutral:
             name = i18n("Neutral Black && White");
          break;

          case BWGreenFilter:
             name = i18n("Black && White With Green Filter");
          break;

          case BWOrangeFilter:
             name = i18n("Black && White With Orange Filter");
          break;

          case BWRedFilter:
             name = i18n("Black && White With Red Filter");
          break;

          case BWYellowFilter:
             name = i18n("Black && White With Yellow Filter");
          break;

          case BWSepia:
             name = i18n("Black && White Sepia");
          break;

          case BWBrown:
             name = i18n("Black && White Brown");
          break;

          case BWCold:
             name = i18n("Black && White Cold");
          break;

          case BWSelenium:
             name = i18n("Black && White Selenium");
          break;
          
          case BWPlatinum:
             name = i18n("Black && White Platinum");
          break;
       }
          
       iface->putOriginalImage(name, data);
       delete [] data;
    }

    kapp->restoreOverrideCursor();
    accept();
}

// This method is based on the Convert to Black & White tutorial (channel mixer method) 
// from GimpGuru.org web site available at this url : http://www.gimpguru.org/Tutorials/Color2BW/

void ImageEffect_BWSepia::blackAndWhiteConversion(uchar *data, int w, int h, bool sb, int type)
{
    // Value to multiply RGB 8 bits component of mask used by changeTonality() method.
    int mul = sb ? 255 : 1;

    switch (type)
    {
       case BWNeutral:
          Digikam::ImageFilters::channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.3, 0.59 , 0.11,                                // Red channel gains.
                   0.0, 1.0,   0.0,                                 // Green channel gains (not used).
                   0.0, 0.0,   1.0);                                // Blue channel gains (not used).
          break;
       
       case BWGreenFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.1, 0.7, 0.2,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWOrangeFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.78, 0.22, 0.0,                                 // Red channel gains.
                   0.0,  1.0,  0.0,                                 // Green channel gains (not used).
                   0.0,  0.0,  1.0);                                // Blue channel gains (not used).
          break;
       
       case BWRedFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.9, 0.1, 0.0,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWYellowFilter:
          Digikam::ImageFilters::channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.6, 0.28, 0.12,                                 // Red channel gains.
                   0.0, 1.0,  0.0,                                  // Green channel gains (not used).
                   0.0, 0.0,  1.0);                                 // Blue channel gains (not used).
          break;
       
       case BWSepia:
          Digikam::ImageFilters::changeTonality(data, w, h, sb, 162*mul, 132*mul, 101*mul);
          break;
       
       case BWBrown:
          Digikam::ImageFilters::changeTonality(data, w, h, sb, 129*mul, 115*mul, 104*mul);
          break;
       
       case BWCold:
          Digikam::ImageFilters::changeTonality(data, w, h, sb, 102*mul, 109*mul, 128*mul);
          break;
       
       case BWSelenium:
          Digikam::ImageFilters::changeTonality(data, w, h, sb, 122*mul, 115*mul, 122*mul);
          break;
       
       case BWPlatinum:
          Digikam::ImageFilters::changeTonality(data, w, h, sb, 115*mul, 110*mul, 106*mul);
          break;
    }
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_bwsepia.moc"

