/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
 *
 * Copyright 2004-2007 by Gilles Caulier
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

#include <qspinbox.h>
#include <qslider.h>
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

#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kstandarddirs.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "colormodifier.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_rgb.h"
#include "imageeffect_rgb.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_RGB::ImageEffect_RGB(QWidget* parent)
               : Digikam::ImageDlgBase(parent, i18n("Color Balance"), "colorbalance", false)
{
    m_destinationPreviewData = 0L;
    setHelp("colorbalancetool.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget("colorbalance Tool Dialog", plainPage(),
                                               i18n("<p>Here you can see the image "
                                                    "color-balance adjustments preview. "
                                                    "You can pick color on image "
                                                    "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);
    
    // -------------------------------------------------------------
                
    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 7, 4, marginHint(), spacingHint());

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
    l1->addStretch(10);
    l1->addWidget(m_scaleBG);
    
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
    
    QLabel *labelLeft = new QLabel(i18n("Cyan"), gboxSettings);
    labelLeft->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_rSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, gboxSettings, "m_rSlider");
    m_rSlider->setTickmarks(QSlider::Below);
    m_rSlider->setTickInterval(20);
    QWhatsThis::add( m_rSlider, i18n("<p>Set here the cyan/red color adjustment of the image."));
    QLabel *labelRight = new QLabel(i18n("Red"), gboxSettings);
    labelRight->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
    m_rInput = new QSpinBox(-100, 100, 1, gboxSettings, "m_rInput");
    
    gridSettings->addMultiCellWidget(labelLeft, 3, 3, 0, 0);
    gridSettings->addMultiCellWidget(m_rSlider, 3, 3, 1, 1);
    gridSettings->addMultiCellWidget(labelRight, 3, 3, 2, 2);
    gridSettings->addMultiCellWidget(m_rInput, 3, 3, 3, 3);
    
    // -------------------------------------------------------------
        
    labelLeft = new QLabel(i18n("Magenta"), gboxSettings);
    labelLeft->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_gSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, gboxSettings, "m_gSlider");
    m_gSlider->setTickmarks(QSlider::Below);
    m_gSlider->setTickInterval(20);
    QWhatsThis::add( m_gSlider, i18n("<p>Set here the magenta/green color adjustment of the image."));
    labelRight = new QLabel(i18n("Green"), gboxSettings);
    labelRight->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
    m_gInput = new QSpinBox(-100, 100, 1, gboxSettings, "m_gInput");

    gridSettings->addMultiCellWidget(labelLeft, 4, 4, 0, 0);
    gridSettings->addMultiCellWidget(m_gSlider, 4, 4, 1, 1);
    gridSettings->addMultiCellWidget(labelRight, 4, 4, 2, 2);
    gridSettings->addMultiCellWidget(m_gInput, 4, 4, 3, 3);
    
    // -------------------------------------------------------------
        
    labelLeft = new QLabel(i18n("Yellow"), gboxSettings);
    labelLeft->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_bSlider = new QSlider(-100, 100, 1, 0, Qt::Horizontal, gboxSettings, "m_bSlider");
    m_bSlider->setTickmarks(QSlider::Below);
    m_bSlider->setTickInterval(20);
    QWhatsThis::add( m_bSlider, i18n("<p>Set here the yellow/blue color adjustment of the image."));
    labelRight = new QLabel(i18n("Blue"), gboxSettings);
    labelRight->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
    m_bInput = new QSpinBox(-100, 100, 1, gboxSettings, "m_bInput");
    
    gridSettings->addMultiCellWidget(labelLeft, 5, 5, 0, 0);
    gridSettings->addMultiCellWidget(m_bSlider, 5, 5, 1, 1);
    gridSettings->addMultiCellWidget(labelRight, 5, 5, 2, 2);
    gridSettings->addMultiCellWidget(m_bInput, 5, 5, 3, 3);

    m_rInput->setValue(0);
    m_gInput->setValue(0);
    m_bInput->setValue(0);
    
    gridSettings->setRowStretch(6, 10);
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------
        
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));
        
    connect(m_rSlider, SIGNAL(valueChanged(int)),
            m_rInput, SLOT(setValue(int)));
    connect(m_rInput, SIGNAL(valueChanged (int)),
            m_rSlider, SLOT(setValue(int)));
    connect(m_rInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_gSlider, SIGNAL(valueChanged(int)),
            m_gInput, SLOT(setValue(int)));
    connect(m_gInput, SIGNAL(valueChanged (int)),
            m_gSlider, SLOT(setValue(int)));
    connect(m_gInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_bSlider, SIGNAL(valueChanged(int)),
            m_bInput, SLOT(setValue(int)));
    connect(m_bInput, SIGNAL(valueChanged (int)),
            m_bSlider, SLOT(setValue(int)));
    connect(m_bInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));     
                        
    // -------------------------------------------------------------
                
    enableButtonOK( false );
}

ImageEffect_RGB::~ImageEffect_RGB()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_RGB::slotChannelChanged(int channel)
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

void ImageEffect_RGB::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_RGB::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_RGB::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("colorbalance Tool Dialog");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LogScaleHistogram));
    int r = config->readNumEntry("RedAjustment", 0);
    int g = config->readNumEntry("GreenAjustment", 0);
    int b = config->readNumEntry("BlueAjustment", 0);
    adjustSliders(r, g, b);
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void ImageEffect_RGB::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("colorbalance Tool Dialog");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("RedAjustment", m_rSlider->value());
    config->writeEntry("GreenAjustment", m_gInput->value());
    config->writeEntry("BlueAjustment", m_bInput->value());
    config->sync();
}

void ImageEffect_RGB::resetValues()
{
    adjustSliders(0, 0, 0);
}

void ImageEffect_RGB::adjustSliders(int r, int g, int b)
{
    m_rSlider->blockSignals(true);
    m_gSlider->blockSignals(true);
    m_bSlider->blockSignals(true);
    m_rInput->blockSignals(true);
    m_gInput->blockSignals(true);
    m_bInput->blockSignals(true);

    m_rSlider->setValue(r);
    m_gSlider->setValue(g);
    m_bSlider->setValue(b);
    m_rInput->setValue(r);
    m_gInput->setValue(g);
    m_bInput->setValue(b);

    m_rSlider->blockSignals(false);
    m_gSlider->blockSignals(false);
    m_bSlider->blockSignals(false);
    m_rInput->blockSignals(false);
    m_gInput->blockSignals(false);
    m_bInput->blockSignals(false);

    slotEffect();
}

void ImageEffect_RGB::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    enableButtonOK(m_rInput->value() != 0 ||
                   m_gInput->value() != 0 ||
                   m_bInput->value() != 0);

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool alpha                 = iface->previewHasAlpha();
    bool sixteenBit            = iface->previewSixteenBit();

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
    double a = 1.0;

    Digikam::DImg preview(w, h, sixteenBit, alpha, m_destinationPreviewData);
    Digikam::ColorModifier cmod;
    cmod.applyColorModifier(preview, r, g, b, a);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();
    
    // Update histogram.
   
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sixteenBit, 0, 0, 0, false);
    
    kapp->restoreOverrideCursor();
}

void ImageEffect_RGB::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    double r = ((double)m_rInput->value() + 100.0)/100.0;
    double g = ((double)m_gInput->value() + 100.0)/100.0;
    double b = ((double)m_bInput->value() + 100.0)/100.0;
    double a = 1.0;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool alpha                 = iface->originalHasAlpha();
    bool sixteenBit            = iface->originalSixteenBit();
    Digikam::DImg original(w, h, sixteenBit, alpha, data);
    delete [] data;

    Digikam::ColorModifier cmod;
    cmod.applyColorModifier(original, r, g, b, a);

    iface->putOriginalImage(i18n("Color Balance"), original.bits());
    kapp->restoreOverrideCursor();
    accept();
}

}  // NameSpace DigikamImagesPluginCore

