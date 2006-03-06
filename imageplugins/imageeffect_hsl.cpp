/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-07-16
 * Description : digiKam image editor Hue/Saturation/Lightness 
 *               correction tool
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

#include <knuminput.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kstandarddirs.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "hslmodifier.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_hsl.h"

namespace DigikamImagesPluginCore
{

ImageEffect_HSL::ImageEffect_HSL(QWidget* parent)
               : Digikam::ImageDlgBase(parent, i18n("Hue/Saturation/Lightness"), "hdladjust", false)
{
    m_destinationPreviewData = 0L;
    setHelp("hsladjusttool.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget(plainPage(),
                                               i18n("<p>Here you can see the image "
                                                    "Hue/Saturation/Lightness adjustments preview. "
                                                    "You can pick color on image "
                                                    "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------
    
    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 10, 4, marginHint(), spacingHint());

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

    QLabel *label2 = new QLabel(i18n("Hue:"), gboxSettings);
    m_hInput       = new KDoubleNumInput(gboxSettings);
    m_hInput->setPrecision(0);
    m_hInput->setRange(-180.0, 180.0, 1.0, true);
    m_hInput->setValue(0.0);
    QWhatsThis::add( m_hInput, i18n("<p>Set here the hue adjustment of the image."));
    gridSettings->addMultiCellWidget(label2, 3, 3, 0, 4);
    gridSettings->addMultiCellWidget(m_hInput, 4, 4, 0, 4);

    QLabel *label3 = new QLabel(i18n("Saturation:"), gboxSettings);
    m_sInput       = new KDoubleNumInput(gboxSettings);
    m_sInput->setPrecision(2);
    m_sInput->setRange(-100.0, 100.0, 0.01, true);
    m_sInput->setValue(0.0);
    QWhatsThis::add( m_sInput, i18n("<p>Set here the saturation adjustment of the image."));
    gridSettings->addMultiCellWidget(label3, 5, 5, 0, 4);
    gridSettings->addMultiCellWidget(m_sInput, 6, 6, 0, 4);

    QLabel *label4 = new QLabel(i18n("Lightness:"), gboxSettings);
    m_lInput       = new KDoubleNumInput(gboxSettings);
    m_lInput->setPrecision(2);
    m_lInput->setRange(-100.0, 100.0, 0.01, true);
    m_lInput->setValue(0.0);
    QWhatsThis::add( m_lInput, i18n("<p>Set here the lightness adjustment of the image."));    
    gridSettings->addMultiCellWidget(label4, 7, 7, 0, 4);
    gridSettings->addMultiCellWidget(m_lInput, 8, 8, 0, 4);

    m_overExposureIndicatorBox = new QCheckBox(i18n("Over exposure indicator"), gboxSettings);
    QWhatsThis::add( m_overExposureIndicatorBox, i18n("<p>If you enable this option, over-exposed pixels "
            "from the target image preview will be over-colored. "
                    "This will not have an effect on the final rendering."));
    gridSettings->addMultiCellWidget(m_overExposureIndicatorBox, 9, 9, 0, 4);

    gridSettings->setRowStretch(10, 10);
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_hInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
            
    connect(m_sInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
            
    connect(m_lInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
    
    connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));              
            
    // -------------------------------------------------------------            

    enableButtonOK( false );
}

ImageEffect_HSL::~ImageEffect_HSL()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_HSL::slotChannelChanged(int channel)
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

void ImageEffect_HSL::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_HSL::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_HSL::slotDefault()
{
    m_hInput->blockSignals(true);	
    m_sInput->blockSignals(true);	
    m_lInput->blockSignals(true);	
    m_hInput->setValue(0.0);
    m_sInput->setValue(0.0);
    m_lInput->setValue(0.0);
    m_hInput->blockSignals(false);	
    m_sInput->blockSignals(false);	
    m_lInput->blockSignals(false);	
    slotEffect();
} 

void ImageEffect_HSL::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    double hu  = m_hInput->value();
    double sa  = m_sInput->value();    
    double lu  = m_lInput->value();
    bool   o = m_overExposureIndicatorBox->isChecked();
    
    enableButtonOK( hu != 0.0 || sa != 0.0 || lu != 0.0);
    
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
    Digikam::HSLModifier cmod;
    cmod.setOverIndicator(o);
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
   
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_HSL::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    double hu  = m_hInput->value();
    double sa  = m_sInput->value();    
    double lu  = m_lInput->value();

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();
    Digikam::DImg original(w, h, sb, a, data);
    delete [] data;

    Digikam::HSLModifier cmod;
    cmod.setHue(hu);
    cmod.setSaturation(sa);
    cmod.setLightness(lu);
    cmod.applyHSL(original);

    iface->putOriginalImage(i18n("HSL Adjustments"), original.bits());
    kapp->restoreOverrideCursor();
    accept();
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_hsl.moc"
