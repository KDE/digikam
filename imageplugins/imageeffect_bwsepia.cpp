/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
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
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <knuminput.h>

// Digikam includes.

#include "imageiface.h"
#include "dimgimagefilters.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "bcgmodifier.h"

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

    m_previewWidget = new Digikam::ImageWidget(plainPage(),
                                               i18n("<p>Here you can see the black and white conversion tool preview. "
                                                    "You can pick color on image "
                                                    "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);

    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 6, 4, marginHint(), spacingHint());

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
    
    m_bwTools = new QButtonGroup(1, Qt::Horizontal, i18n("Black and white tool:"), gboxSettings);

    QRadioButton *neutral = new QRadioButton(i18n("Neutral"), m_bwTools);
    QWhatsThis::add( neutral, i18n("<img source=\"%1\"> <b>Neutral Black & White</b>:"
                                     "<p>Simulate black and white neutral film exposure.</p>").arg(previewEffectPic("neutralbw")));
    m_bwTools->insert(neutral, BWNeutral);
    
    QRadioButton *green = new QRadioButton(i18n("Green Filter"), m_bwTools);
    QWhatsThis::add( green, i18n("<img source=\"%1\"> <b>Black & White with Green Filter</b>:"
                                     "<p>Simulate black and white film exposure using green filter. "
                                     "This provides an universal asset for all scenics, especially suited for portraits "
                                     "photographed against sky.</p>").arg(previewEffectPic("bwgreen")));
    m_bwTools->insert(green, BWGreenFilter);

    QRadioButton *orange = new QRadioButton(i18n("Orange Filter"), m_bwTools);
    QWhatsThis::add( orange, i18n("<img source=\"%1\"> <b>Black & White with Orange Filter</b>:"
                                     "<p>Simulate black and white film exposure using orange filter. "
                                     "This will enhances landscapes, marine scenes and aerial "
                                     "photography.</p>").arg(previewEffectPic("bworange")));
    m_bwTools->insert(orange, BWOrangeFilter);

    QRadioButton *red = new QRadioButton(i18n("Red Filter"), m_bwTools);
    QWhatsThis::add( red, i18n("<img source=\"%1\"> <b>Black & White with Red Filter</b>:"
                                     "<p>Simulate black and white film exposure using red filter. "
                                     "Using this one creates dramatic sky effects and simulates moonlight scenes "
                                     "in daytime.</p>").arg(previewEffectPic("bwred")));
    m_bwTools->insert(red, BWRedFilter);

    QRadioButton *yellow = new QRadioButton(i18n("Yellow Filter"), m_bwTools);
    QWhatsThis::add( yellow, i18n("<img source=\"%1\"> <b>Black & White with Yellow Filter</b>:"
                                      "<p>Simulate black and white film exposure using yellow filter. "
                                      "Most natural tonal correction and improves contrast. Ideal for "
                                      "landscapes.</p>").arg(previewEffectPic("bwyellow")));
    m_bwTools->insert(yellow, BWYellowFilter);

    QRadioButton *sepia = new QRadioButton(i18n("Sepia Tone"), m_bwTools);
    QWhatsThis::add( sepia, i18n("<img source=\"%1\"> <b>Black & White with Sepia Tone</b>:"
                                     "<p>Gives a warm highlight and mid-tone while adding a bit of coolness to "
                                     "the shadows-very similar to the process of bleaching a print and re-developing in a sepia "
                                     "toner.</p>").arg(previewEffectPic("sepia")));
    m_bwTools->insert(sepia, BWSepia);

    QRadioButton *brown = new QRadioButton(i18n("Brown Tone"), m_bwTools);
    QWhatsThis::add( brown, i18n("<img source=\"%1\"> <b>Black & White with Brown Tone</b>:"
                                      "<p>This filter is more neutral than Sepia Tone filter.</p>").arg(previewEffectPic("browntone")));
    m_bwTools->insert(brown, BWBrown);

    QRadioButton *cold = new QRadioButton(i18n("Cold Tone"), m_bwTools);
    QWhatsThis::add( cold, i18n("<img source=\"%1\"> <b>Black & White with Cold Tone</b>:"
                                      "<p>Start subtle and replicate printing on a cold tone black and white "
                                      "paper such as a bromide enlarging paper.</p>").arg(previewEffectPic("coldtone")));
    m_bwTools->insert(cold, BWCold);    

    QRadioButton *selenium = new QRadioButton(i18n("Selenium Tone"), m_bwTools);
    QWhatsThis::add( selenium, i18n("<img source=\"%1\"> <b>Black & White with Selenium Tone</b>:"
                                      "<p>This effect replicate traditional selenium chemical toning done "
                                      "in the darkroom.</p>").arg(previewEffectPic("selenium")));
    m_bwTools->insert(selenium, BWSelenium);

    QRadioButton *platinium = new QRadioButton(i18n("Platinum Tone"), m_bwTools);
    QWhatsThis::add( platinium, i18n("<img source=\"%1\"> <b>Black & White with Platinum Tone</b>:"
                                      "<p>This effect replicate traditional platinum chemical toning done "
                                    "in the darkroom.</p>").arg(previewEffectPic("platinum")));
    m_bwTools->insert(platinium, BWPlatinum);
    
    gridSettings->addMultiCellWidget(m_bwTools, 3, 3, 0, 4);

    // -------------------------------------------------------------
    
    QLabel *label3 = new QLabel(i18n("Contrast:"), gboxSettings);
    m_cInput       = new KDoubleNumInput(gboxSettings);
    m_cInput->setPrecision(2);
    m_cInput->setRange(-1.0, 1.0, 0.01, true);
    m_cInput->setValue(0.0);
    QWhatsThis::add( m_cInput, i18n("<p>Set here the contrast adjustment of the image."));
    gridSettings->addMultiCellWidget(label3, 4, 4, 0, 4);
    gridSettings->addMultiCellWidget(m_cInput, 5, 5, 0, 4);

    gridSettings->setRowStretch(6, 10);
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------
    
    // Reset all parameters to the default values.
    QTimer::singleShot(0, this, SLOT(slotDefault()));
    
    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_bwTools, SIGNAL(released(int)),
            this, SLOT(slotEffect()));

    connect(m_cInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                        
    
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

QString ImageEffect_BWSepia::previewEffectPic(QString name)
{
    KGlobal::dirs()->addResourceType(name.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return ( KGlobal::dirs()->findResourceDir(name.ascii(), name + ".png") + name + ".png" );
}

void ImageEffect_BWSepia::slotDefault()
{
    m_bwTools->blockSignals(true);
    m_cInput->blockSignals(true);
    m_bwTools->setButton( BWNeutral );
    m_cInput->setValue(0.0);
    m_cInput->blockSignals(false);
    m_bwTools->blockSignals(false);
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
    bool a                          = iface->previewHasAlpha();
    bool sb                         = iface->previewSixteenBit();

    blackAndWhiteConversion(m_destinationPreviewData, w, h, sb, m_bwTools->selectedId());

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
    Digikam::BCGModifier cmod;
    cmod.setContrast(m_cInput->value() + (double)(1.00));
    cmod.applyBCG(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
    
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_BWSepia::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();
    
    if (data) 
    {
       int type = m_bwTools->selectedId();
       blackAndWhiteConversion(data, w, h, sb, type);
       Digikam::DImg img(w, h, sb, a, data);
       Digikam::BCGModifier cmod;
       cmod.setContrast(m_cInput->value() + (double)(1.00));
       cmod.applyBCG(img);
       
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
          
       iface->putOriginalImage(name, img.bits());
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
    Digikam::DImgImageFilters filter;
    
    switch (type)
    {
       case BWNeutral:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.3, 0.59 , 0.11,                                // Red channel gains.
                   0.0, 1.0,   0.0,                                 // Green channel gains (not used).
                   0.0, 0.0,   1.0);                                // Blue channel gains (not used).
          break;
       
       case BWGreenFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.1, 0.7, 0.2,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWOrangeFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.78, 0.22, 0.0,                                 // Red channel gains.
                   0.0,  1.0,  0.0,                                 // Green channel gains (not used).
                   0.0,  0.0,  1.0);                                // Blue channel gains (not used).
          break;
       
       case BWRedFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.9, 0.1, 0.0,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWYellowFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.6, 0.28, 0.12,                                 // Red channel gains.
                   0.0, 1.0,  0.0,                                  // Green channel gains (not used).
                   0.0, 0.0,  1.0);                                 // Blue channel gains (not used).
          break;
       
       case BWSepia:
          filter.changeTonality(data, w, h, sb, 162*mul, 132*mul, 101*mul);
          break;
       
       case BWBrown:
          filter.changeTonality(data, w, h, sb, 129*mul, 115*mul, 104*mul);
          break;
       
       case BWCold:
          filter.changeTonality(data, w, h, sb, 102*mul, 109*mul, 128*mul);
          break;
       
       case BWSelenium:
          filter.changeTonality(data, w, h, sb, 122*mul, 115*mul, 122*mul);
          break;
       
       case BWPlatinum:
          filter.changeTonality(data, w, h, sb, 115*mul, 110*mul, 106*mul);
          break;
    }
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_bwsepia.moc"

