/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-05
 * Description : digiKam image editor to adjust Brightness, 
                 Contrast, and Gamma of picture.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QCheckBox>
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
#include <kglobal.h>
#include <kvbox.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "bcgmodifier.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_bcg.h"
#include "imageeffect_bcg.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_BCG::ImageEffect_BCG(QWidget* parent)
               : Digikam::ImageDlgBase(parent, i18n("Brightness Contrast Gamma Adjustments"), 
                                       "bcgadjust", false)
{
    m_destinationPreviewData = 0L;
    setHelp("bcgadjusttool.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget("bcgadjust Tool Dialog", mainWidget(),
                                               i18n("<p>Here you can see the image "
                                               "brightness-contrast-gamma adjustments preview. "
                                               "You can pick color on image "
                                               "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------
                
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
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    
    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Brightness:"), gboxSettings);
    m_bInput       = new KIntNumInput(gboxSettings);
    m_bInput->setRange(-100, 100, 1);
    m_bInput->setSliderEnabled(true);
    m_bInput->setValue(0);
    m_bInput->setWhatsThis( i18n("<p>Set here the brightness adjustment of the image."));

    QLabel *label3 = new QLabel(i18n("Contrast:"), gboxSettings);
    m_cInput       = new KIntNumInput(gboxSettings);
    m_cInput->setRange(-100, 100, 1);
    m_cInput->setSliderEnabled(true);
    m_cInput->setValue(0);
    m_cInput->setWhatsThis( i18n("<p>Set here the contrast adjustment of the image."));

    QLabel *label4 = new QLabel(i18n("Gamma:"), gboxSettings);
    m_gInput = new KDoubleNumInput(gboxSettings);
    m_gInput->setDecimals(2);
    m_gInput->setRange(0.1, 3.0, 0.01, true);
    m_gInput->setValue(1.0);
    m_gInput->setWhatsThis( i18n("<p>Set here the gamma adjustment of the image."));

    // -------------------------------------------------------------

    gridSettings->addLayout(l1, 0, 0, 1, 5 );
    gridSettings->addWidget(histoBox, 1, 0, 2, 5 );
    gridSettings->addWidget(label2, 3, 0, 1, 5 );
    gridSettings->addWidget(m_bInput, 4, 0, 1, 5 );
    gridSettings->addWidget(label3, 5, 0, 1, 5 );
    gridSettings->addWidget(m_cInput, 6, 0, 1, 5 );
    gridSettings->addWidget(label4, 7, 0, 1, 5 );
    gridSettings->addWidget(m_gInput, 8, 0, 1, 5 );
    gridSettings->setRowStretch(9, 10);    
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

    connect(m_bInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
            
    connect(m_cInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
            
    connect(m_gInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));    
            
    // -------------------------------------------------------------
                
    enableButtonOk( false );
}

ImageEffect_BCG::~ImageEffect_BCG()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_BCG::slotChannelChanged(int channel)
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

void ImageEffect_BCG::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ImageEffect_BCG::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_BCG::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("bcgadjust Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale", 
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    m_bInput->setValue(group.readEntry("BrightnessAjustment", 0));
    m_cInput->setValue(group.readEntry("ContrastAjustment", 0));
    m_gInput->setValue(group.readEntry("GammaAjustment", 1.0));
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ImageEffect_BCG::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("bcgadjust Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());
    group.writeEntry("BrightnessAjustment", m_bInput->value());
    group.writeEntry("ContrastAjustment", m_cInput->value());
    group.writeEntry("GammaAjustment", m_gInput->value());
    config->sync();
}

void ImageEffect_BCG::resetValues()
{
    m_bInput->blockSignals(true);	
    m_cInput->blockSignals(true);	
    m_gInput->blockSignals(true);	
    m_bInput->setValue(0);
    m_cInput->setValue(0);
    m_gInput->setValue(1.0);
    m_bInput->blockSignals(false);	
    m_cInput->blockSignals(false);	
    m_gInput->blockSignals(false);	
} 

void ImageEffect_BCG::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    double b = (double)m_bInput->value()/250.0;
    double c = (double)(m_cInput->value()/100.0) + 1.00;    
    double g = m_gInput->value();

    enableButtonOk( b != 0.0 || c != 1.0 || g != 1.0 );
    
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
    Digikam::BCGModifier cmod;
    cmod.setGamma(g);
    cmod.setBrightness(b);
    cmod.setContrast(c);
    cmod.applyBCG(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
   
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_BCG::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    double b = (double)m_bInput->value()/250.0;
    double c = (double)(m_cInput->value()/100.0) + 1.00;    
    double g = m_gInput->value();

    iface->setOriginalBCG(b, c, g);
    kapp->restoreOverrideCursor();
    accept();
}

}  // NameSpace DigikamImagesPluginCore
