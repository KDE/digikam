/* ============================================================
 * File  : histogramviewer.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-21
 * Description : an image histogram viewer dialog.
 * 
 * Copyright 2004 by Gilles Caulier
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
 
#include <qlayout.h>
#include <qcolor.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kselect.h>

// Local includes.

#include <imagehistogram.h>
#include <histogramwidget.h>
#include <colorgradientwidget.h> 
#include "histogramviewer.h"


// Constructor using local image file name.
HistogramViewer::HistogramViewer(QWidget *parent, QString imageFile)
               : KDialogBase(Plain, i18n("Histogram"), Help|Ok, Ok,
                             parent, 0, true, true)
{
    m_histogramWidget = 0L;
    m_hGradient       = 0L;
    m_image.load(imageFile);
    
    if (m_image.isNull() == false)
       {
       if(m_image.depth() < 32)           // we works always with 32bpp.
          m_image = m_image.convertDepth(32);
       
       m_image.setAlphaBuffer(true);
       setupGui((uint *)m_image.bits(),
                m_image.width(), 
                m_image.height());
       }
}

// Constructor using QImage instance.
HistogramViewer::HistogramViewer(QWidget *parent, QImage image)
               : KDialogBase(Plain, i18n("Histogram"), Help|Ok, Ok,
                             parent, 0, true, true)
{                             
    m_histogramWidget = 0L;
    m_hGradient       = 0L;
    
    if (image.isNull() == true)
       return;
       
    m_image = image;
    
    if(m_image.depth() < 32)           // we works always with 32bpp.
       m_image = m_image.convertDepth(32);

    m_image.setAlphaBuffer(true);
    setupGui((uint *)m_image.bits(),
             m_image.width(), 
             m_image.height());
}                            

// Constructor using image RAW data 32 bits (RGBA).                          
HistogramViewer::HistogramViewer(QWidget* parent, uint *imageData, uint width, uint height)
               : KDialogBase(Plain, i18n("Histogram"), Help|Ok, Ok,
                             parent, 0, true, true)
{
    m_histogramWidget = 0L;
    m_hGradient       = 0L;
    setupGui(imageData, width, height);
}

void HistogramViewer::setupGui(uint *imageData, uint width, uint height)
{
    setHelp("imageviewer.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    // -------------------------------------------------------------
                                              
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Channel:"), plainPage());
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, plainPage() );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->insertItem( i18n("Alpha") );
    m_channelCB->insertItem( i18n("Colors") );
    m_channelCB->setCurrentText( i18n("Luminosity") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display.<p>"
                                       "<b>Luminosity</b>: drawing the image luminosity values.<p>"
                                       "<b>Red</b>: drawing the red image channel values.<p>"
                                       "<b>Green</b>: drawing the green image channel values.<p>"
                                       "<b>Blue</b>: drawing the blue image channel values.<p>"
                                       "<b>Alpha</b>: drawing the alpha image channel values. " 
                                       "This channel corresponding to the transparency value and "
                                       "is supported by some image formats like PNG or GIF.<p>"
                                       "<b>Colors</b>: drawing all color channels values at the same time."));
    
    QLabel *label2 = new QLabel(i18n("Scale:"), plainPage());
    label2->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_scaleCB = new QComboBox( false, plainPage() );
    m_scaleCB->insertItem( i18n("Linear") );
    m_scaleCB->insertItem( i18n("Logarithmic") );
    m_scaleCB->setCurrentText( i18n("Logarithmic") );
    QWhatsThis::add( m_scaleCB, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image maximal counts is small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts is big. "
                                     "Like this all values (small and big) will be visible on the graph."));
    
    hlay->addWidget(label1);
    hlay->addWidget(m_channelCB);
    hlay->addWidget(label2);
    hlay->addWidget(m_scaleCB);
    
    // -------------------------------------------------------------
    
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, 
                                                     imageData,
                                                     width,
                                                     height,
                                                     frame);
    QWhatsThis::add( m_histogramWidget, i18n("<p>This is the histogram drawing of the selected image channel"));
    l->addWidget(m_histogramWidget, 0);
        
    m_hGradient = new Digikam::ColorGradientWidget( KSelector::Horizontal, 20, plainPage() );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    topLayout->addWidget(frame, 4);
    topLayout->addWidget(m_hGradient, 0);

    // -------------------------------------------------------------

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Intensity range:"), plainPage());
    label3->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_minInterv = new QSpinBox(0, 255, 1, plainPage());
    m_minInterv->setValue(0);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the minimal intensity value "
                                       "of the histogram selection."));
    m_maxInterv = new QSpinBox(0, 255, 1, plainPage());
    m_maxInterv->setValue(255);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the maximal intensity value "
                                       "of the histogram selection."));
    hlay2->addWidget(label3);
    hlay2->addWidget(m_minInterv);
    hlay2->addWidget(m_maxInterv);
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox = new QGroupBox(4, Qt::Horizontal, i18n("Statistics"), plainPage());
    QWhatsThis::add( gbox, i18n("<p>You can see here the statistic results calculated with the "
                                "selected histogram part. This values are available for all channels "
                                "excepted when you displayed all color channels at the same time."));
    
    QLabel *label4 = new QLabel(i18n("Mean:"), gbox);
    label4->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelMeanValue = new QLabel(gbox);
    m_labelMeanValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label5 = new QLabel(i18n("Pixels:"), gbox);
    label5->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelPixelsValue = new QLabel(gbox);
    m_labelMeanValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label6 = new QLabel(i18n("Std Dev.:"), gbox);
    label6->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelStdDevValue = new QLabel(gbox);
    m_labelStdDevValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label7 = new QLabel(i18n("Count:"), gbox);
    label7->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelCountValue = new QLabel(gbox);
    m_labelCountValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label8 = new QLabel(i18n("Median:"), gbox);
    label8->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelMedianValue = new QLabel(gbox);
    m_labelMedianValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label9 = new QLabel(i18n("Percentile:"), gbox);
    label9->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelPercentileValue = new QLabel(gbox);
    m_labelPercentileValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    topLayout->addWidget(gbox);
    topLayout->addStretch();
    
    // -------------------------------------------------------------
    
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(m_scaleCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));
 
    connect(m_histogramWidget, SIGNAL(signalMousePressed( int )),
            this, SLOT(slotUpdateMinInterv(int)));
       
    connect(m_histogramWidget, SIGNAL(signalMouseReleased( int )),
            this, SLOT(slotUpdateMaxInterv(int)));

    connect(m_minInterv, SIGNAL(valueChanged (int)),
            m_histogramWidget, SLOT(slotMinValueChanged(int)));

    connect(m_minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotIntervChanged(int)));

    connect(m_maxInterv, SIGNAL(valueChanged (int)),
            m_histogramWidget, SLOT(slotMaxValueChanged(int)));

    connect(m_maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotIntervChanged(int)));
                                                           
    adjustSize();
    updateInformations();
}

HistogramViewer::~HistogramViewer()
{
}

void HistogramViewer::closeEvent(QCloseEvent *e)
{
    if ( m_histogramWidget )
       delete m_histogramWidget;
    
    if ( m_hGradient )        
       delete m_hGradient;
    
    e->accept();    
}

void HistogramViewer::slotChannelChanged(int channel)
{
    switch(channel)
       {
       case 1:           // Red.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
          break;
       
       case 2:           // Green.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
          break;
          
       case 3:           // Blue.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
          break;

       case 4:           // Alpha.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          break;

       case 5:           // All color channels.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::ColorChannelsHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          break;
                              
       default:          // Luminosity.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          break;
       }
   
    m_histogramWidget->repaint(false);
    updateInformations();
}

void HistogramViewer::slotScaleChanged(int scale)
{
    switch(scale)
       {
       case 1:           // Log.
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LogScaleHistogram;
          break;
          
       default:          // Lin.
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LinScaleHistogram;
          break;
       }
   
    m_histogramWidget->repaint(false);
}

void HistogramViewer::slotUpdateMinInterv(int min)
{
    m_minInterv->setValue(min);
}


void HistogramViewer::slotUpdateMaxInterv(int max)
{
    m_maxInterv->setValue(max);
    updateInformations();
}

void HistogramViewer::slotIntervChanged(int)
{
    m_maxInterv->setMinValue(m_minInterv->value());
    m_minInterv->setMaxValue(m_maxInterv->value());
    updateInformations();
}

void HistogramViewer::updateInformations()
{
    QString value;
    int min = m_minInterv->value();
    int max = m_maxInterv->value();
    int channel = m_channelCB->currentItem();
    
    if ( channel != Digikam::HistogramWidget::ColorChannelsHistogram )
       {
       double mean = m_histogramWidget->m_imageHistogram->getMean(channel, min, max);
       m_labelMeanValue->setText(value.sprintf("%3.1f", mean));
    
       double pixels = m_histogramWidget->m_imageHistogram->getPixels();
       m_labelPixelsValue->setText(value.sprintf("%8d", (int)pixels));
    
       double stddev = m_histogramWidget->m_imageHistogram->getStdDev(channel, min, max);
       m_labelStdDevValue->setText(value.sprintf("%3.1f", stddev));
      
       double counts = m_histogramWidget->m_imageHistogram->getCount(channel, min, max);
       m_labelCountValue->setText(value.sprintf("%8d", (int)counts));
    
       double median = m_histogramWidget->m_imageHistogram->getMedian(channel, min, max);
       m_labelMedianValue->setText(value.sprintf("%3.1f", median));

       double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
       m_labelPercentileValue->setText(value.sprintf("%4.1f", percentile));
       }
    else
       {
       m_labelMeanValue->setText("");
       m_labelPixelsValue->setText("");
       m_labelStdDevValue->setText("");
       m_labelCountValue->setText("");
       m_labelMedianValue->setText("");
       m_labelPercentileValue->setText("");
       }
}

#include "histogramviewer.moc"
