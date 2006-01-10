/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
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

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt Includes.
 
#include <qlayout.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h> 
#include <qpushbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ktabwidget.h>

// Local includes.

#include "dimg.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "navigatebarwidget.h"
#include "loadsavethread.h"
#include "imagepropertiescolorstab.h"

#include LCMS_HEADER

namespace Digikam
{

ImagePropertiesColorsTab::ImagePropertiesColorsTab(QWidget* parent, QRect* selectionArea, bool navBar)
                           : QWidget(parent)
{
    m_imageLoaderThreaded = 0;
    m_selectionArea       = selectionArea;
    
    QVBoxLayout *vLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    m_navigateBar        = new NavigateBarWidget(this, navBar);
    KTabWidget *tab      = new KTabWidget(this);
    vLayout->addWidget(m_navigateBar);
    vLayout->addWidget(tab);
       
    // Histogram tab area -----------------------------------------------------
       
    QWidget* histogramPage = new QWidget( tab );
    QGridLayout *topLayout = new QGridLayout(histogramPage, 8, 3, KDialog::marginHint(), KDialog::spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), histogramPage);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, histogramPage );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->insertItem( i18n("Alpha") );
    m_channelCB->insertItem( i18n("Colors") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: drawing the image luminosity values.<p>"
                                       "<b>Red</b>: drawing the red image channel values.<p>"
                                       "<b>Green</b>: drawing the green image channel values.<p>"
                                       "<b>Blue</b>: drawing the blue image channel values.<p>"
                                       "<b>Alpha</b>: drawing the alpha image channel values. " 
                                       "This channel corresponding to the transparency value and "
                                       "is supported by some image formats such as PNG or GIF.<p>"
                                       "<b>Colors</b>: drawing all color channels values at the same time."));
    
    m_scaleBG = new QHButtonGroup(histogramPage);
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the "
                                     "graph."));
    
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
    
    QLabel *label10 = new QLabel(i18n("Colors:"), histogramPage);
    label10->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_colorsCB = new QComboBox( false, histogramPage );
    m_colorsCB->insertItem( i18n("Red") );
    m_colorsCB->insertItem( i18n("Green") );
    m_colorsCB->insertItem( i18n("Blue") );
    m_colorsCB->setEnabled( false );
    QWhatsThis::add( m_colorsCB, i18n("<p>Select here the main color displayed with Colors Channel mode:<p>"
                                      "<b>Red</b>: drawing the red image channel on the foreground.<p>"
                                      "<b>Green</b>: drawing the green image channel on the foreground.<p>"
                                      "<b>Blue</b>: drawing the blue image channel on the foreground.<p>"));
                                       
    m_regionBG = new QHButtonGroup(histogramPage);
    m_regionBG->setExclusive(true);
    m_regionBG->setFrameShape(QFrame::NoFrame);
    m_regionBG->setInsideMargin( 0 );
    m_regionBG->hide();
    QWhatsThis::add( m_regionBG, i18n("<p>Select here the histogram region computation:<p>"
                                      "<b>Full Image</b>: drawing histogram using the full image.<p>"
                                      "<b>Selection</b>: drawing histogram using the current image "
                                      "selection."));
    
    QPushButton *fullImageButton = new QPushButton( m_regionBG );
    QToolTip::add( fullImageButton, i18n( "<p>Full Image" ) );
    m_regionBG->insert(fullImageButton, Digikam::HistogramWidget::FullImageHistogram);
    KGlobal::dirs()->addResourceType("image-full", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-full", "image-full.png");
    fullImageButton->setPixmap( QPixmap( directory + "image-full.png" ) );
    fullImageButton->setToggleButton(true);
    
    QPushButton *SelectionImageButton = new QPushButton( m_regionBG );
    QToolTip::add( SelectionImageButton, i18n( "<p>Selection" ) );
    m_regionBG->insert(SelectionImageButton, Digikam::HistogramWidget::ImageSelectionHistogram);
    KGlobal::dirs()->addResourceType("image-selection", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-selection", "image-selection.png");
    SelectionImageButton->setPixmap( QPixmap( directory + "image-selection.png" ) );
    SelectionImageButton->setToggleButton(true);       
                                                                        
    topLayout->addMultiCellWidget(label1, 1, 1, 0, 0);
    topLayout->addMultiCellWidget(m_channelCB, 1, 1, 1, 1);
    topLayout->addMultiCellWidget(m_scaleBG, 1, 1, 2, 2);
    topLayout->addMultiCellWidget(label10, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(m_colorsCB, 2, 2, 1, 1);
    topLayout->addMultiCellWidget(m_regionBG, 2, 2, 2, 2);
    topLayout->setColStretch(3, 10);
    
    // -------------------------------------------------------------
    
    m_histogramWidget = new HistogramWidget(256, 140, histogramPage);
    QWhatsThis::add( m_histogramWidget, i18n("<p>This is the histogram drawing of the "
                                             "selected image channel"));
        
    m_hGradient = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histogramPage );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    topLayout->addMultiCellWidget(m_histogramWidget, 3, 3, 0, 3);
    topLayout->addMultiCellWidget(m_hGradient, 4, 4, 0, 3);

    // -------------------------------------------------------------

    QHBoxLayout *hlay2 = new QHBoxLayout(KDialog::spacingHint());
    QLabel *label3 = new QLabel(i18n("Range:"), histogramPage);
    label3->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_minInterv = new QSpinBox(0, 255, 1, histogramPage);
    m_minInterv->setValue(0);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the minimal intensity "
                                       "value of the histogram selection."));    
    m_maxInterv = new QSpinBox(0, 255, 1, histogramPage);
    m_maxInterv->setValue(255);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the maximal intensity value "
                                       "of the histogram selection."));
    hlay2->addWidget(label3);
    hlay2->addWidget(m_minInterv);
    hlay2->addWidget(m_maxInterv);
    topLayout->addMultiCellLayout(hlay2, 5, 5, 0, 3);
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox = new QGroupBox(2, Qt::Horizontal, i18n("Statistics"), histogramPage);
    QWhatsThis::add( gbox, i18n("<p>Here you can see the statistic results calculated with the "
                                "selected histogram part. These values are available for all "
                                "channels."));
                                
    QLabel *label4 = new QLabel(i18n("Mean:"), gbox);
    label4->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelMeanValue = new QLabel(gbox);
    m_labelMeanValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label5 = new QLabel(i18n("Pixels:"), gbox);
    label5->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelPixelsValue = new QLabel(gbox);
    m_labelPixelsValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label6 = new QLabel(i18n("Standard deviation:"), gbox);
    label6->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelStdDevValue = new QLabel(gbox);
    m_labelStdDevValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label7 = new QLabel(i18n("Count:"), gbox);
    label7->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelCountValue = new QLabel(gbox);
    m_labelCountValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label8 = new QLabel(i18n("Median:"), gbox);
    label8->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelMedianValue = new QLabel(gbox);
    m_labelMedianValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label9 = new QLabel(i18n("Percentile:"), gbox);
    label9->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelPercentileValue = new QLabel(gbox);
    m_labelPercentileValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    topLayout->addMultiCellWidget(gbox, 6, 6, 0, 3);

    // -------------------------------------------------------------

    QGroupBox *gbox2 = new QGroupBox(2, Qt::Horizontal, histogramPage);
    gbox2->setFrameStyle( QFrame::NoFrame );

    QLabel *label11     = new QLabel(i18n("Color depth:"), gbox2);
    label11->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelColorDepth   = new QLabel(gbox2);
    m_labelColorDepth->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *label12     = new QLabel(i18n("Alpha Channel:"), gbox2);
    label12->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_labelAlphaChannel = new QLabel(gbox2);
    m_labelAlphaChannel->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    topLayout->addMultiCellWidget(gbox2, 7, 7, 0, 3);

    topLayout->setRowStretch(8, 10);
    tab->addTab(histogramPage, i18n("Histogram") );

    // ICC Profiles tab area ---------------------------------------
    
    QWidget* iccprofilePage = new QWidget( tab );
    QGridLayout *iccLayout = new QGridLayout(iccprofilePage, 8, 3, KDialog::marginHint(), KDialog::spacingHint());
    
    QGroupBox *iccbox = new QGroupBox(2, Qt::Vertical, iccprofilePage);
    iccbox->setFrameStyle (QFrame::NoFrame);

    m_infoHeader = new QLabel(0, iccbox);

    QGroupBox *iccdetail = new QGroupBox(2, Qt::Horizontal, iccprofilePage);

    QLabel *labelName = new QLabel(i18n("Name: "), iccdetail);
    m_labelICCName = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelDescription = new QLabel(i18n("Description: "), iccdetail);
    m_labelICCDescription = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelCright = new QLabel(i18n("Copyright: "), iccdetail);
    m_labelICCCopyright = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelIntent = new QLabel(i18n("Rendering Intent: "), iccdetail);
    m_labelICCIntent = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelColor = new QLabel(i18n("Color Space: "), iccdetail);
    m_labelICCColorSpace = new KSqueezedTextLabel(0, iccdetail);

    iccLayout->addMultiCellWidget(iccbox, 0, 0, 0, 2);
    iccLayout->addMultiCellWidget(iccdetail, 2, 7, 0, 5);
    iccLayout->setRowStretch(8, 10);

    tab->addTab(iccprofilePage, i18n("ICC profile") );

    // -------------------------------------------------------------

    connect(m_navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(m_navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(m_navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(m_navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
                            
    // -------------------------------------------------------------
                                
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));
    
    connect(m_colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));     
                   
    connect(m_regionBG, SIGNAL(released(int)),
            this, SLOT(slotRenderingChanged(int)));       
             
    connect(m_histogramWidget, SIGNAL(signalIntervalChanged( int, int )),
            this, SLOT(slotUpdateInterval(int, int)));
       
    connect(m_histogramWidget, SIGNAL(signalMaximumValueChanged( int )),
            this, SLOT(slotUpdateIntervRange(int)));

    connect(m_histogramWidget, SIGNAL(signalHistogramComputationDone(bool)),
            this, SLOT(slotRefreshOptions(bool)));

    connect(m_histogramWidget, SIGNAL(signalHistogramComputationFailed(void)),
            this, SLOT(slotHistogramComputationFailed(void)));
                        
    connect(m_minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMinValueChanged(int)));

    connect(m_maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMaxValueChanged(int)));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LogScaleHistogram));
    m_colorsCB->setCurrentItem(config->readNumEntry("Histogram Color", 0));       // Red.
    m_regionBG->setButton(config->readNumEntry("Histogram Rendering", Digikam::HistogramWidget::FullImageHistogram));
}

ImagePropertiesColorsTab::~ImagePropertiesColorsTab()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the m_image data are deleted automaticly!
    m_histogramWidget->stopHistogramComputation();

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("Histogram Color", m_colorsCB->currentItem());
    config->writeEntry("Histogram Rendering", m_regionBG->selectedId());

    if (m_imageLoaderThreaded)
       delete m_imageLoaderThreaded;
    
    if ( m_histogramWidget )
       delete m_histogramWidget;
    
    if ( m_hGradient )        
       delete m_hGradient;
}

void ImagePropertiesColorsTab::setData(const KURL& url, QRect *selectionArea,
                                       DImg *img, int itemType)
{
    // This is necessary to stop computation because m_image.bits() is currently used by
    // threaded histogram algorithm.
    
    m_histogramWidget->stopHistogramComputation();
    
    if (url.isEmpty())
    {
       m_navigateBar->setFileName("");
       m_labelMeanValue->clear();
       m_labelPixelsValue->clear();
       m_labelStdDevValue->clear();
       m_labelCountValue->clear();
       m_labelMedianValue->clear();
       m_labelPercentileValue->clear();
       m_labelColorDepth->clear();
       m_labelAlphaChannel->clear();       
       setEnabled(false);
       return;
    }

    setEnabled(true);
    
    m_navigateBar->setFileName(url.filename());
    m_navigateBar->setButtonsState(itemType);
    m_selectionArea = selectionArea;
    m_image.reset();
                
    if (!img)
    {
        loadImageFromUrl(url);
    }
    else 
    {
        m_image = img->copy();

        if ( !m_image.isNull() )
        {
            getICCData();

            // If a selection area is done in Image Editor and if the current image is the same 
            // in Image Editor, then compute too the histogram for this selection.
            if (m_selectionArea)
            {
                m_imageSelection = m_image.copy(*m_selectionArea);
                m_histogramWidget->updateData(m_image.bits(), m_image.width(), m_image.height(), m_image.sixteenBit(),
                                              m_imageSelection.bits(), m_imageSelection.width(),
                                              m_imageSelection.height());
                m_regionBG->show();
                updateInformations();
            }
            else 
            {
                m_histogramWidget->updateData(m_image.bits(), m_image.width(), m_image.height(), m_image.sixteenBit());
                m_regionBG->hide();
                updateInformations();
            }
        }
        else 
        {
            slotHistogramComputationFailed();
        }
    }
}

void ImagePropertiesColorsTab::loadImageFromUrl(const KURL& url)
{
    // create thread on demand
    if (!m_imageLoaderThreaded)
    {
        m_imageLoaderThreaded = new ManagedLoadSaveThread();

        connect(m_imageLoaderThreaded, SIGNAL(signalImageLoaded(const QString&, const DImg&)),
                this, SLOT(slotLoadImageFromUrlComplete(const QString&, const DImg&)));
        connect(m_imageLoaderThreaded, SIGNAL(signalLoadingProgress(const QString&, float)),
                this, SLOT(slotProgressInfo(const QString&, float)));
    }

    //TODO or not TODO: Network transparency?
    m_currentFilePath = url.path();
    m_imageLoaderThreaded->load(m_currentFilePath, ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
}

void ImagePropertiesColorsTab::slotLoadImageFromUrlComplete(const QString& filePath, const DImg& img)
{
    // Discard any leftover messages from previous, possibly aborted loads
    if ( filePath != m_currentFilePath )
        return;

    if ( !img.isNull() )
    {
        m_histogramWidget->updateData(img.bits(), img.width(), img.height(),
                                      img.sixteenBit());
        // As a safety precaution, this must be changed only after updateData is called,
        // which stops computation because m_image.bits() is currently used by threaded histogram algorithm.
        m_image = img;
        m_regionBG->hide();
        updateInformations();
        getICCData();
    }
    else
    {
        slotHistogramComputationFailed();
    }
}

void ImagePropertiesColorsTab::slotProgressInfo(const QString&, float progress)
{
    //kdDebug() << "Progress loading is " << progress << endl;
    //m_histogramWidget->setEnabled(true);
    m_histogramWidget->setDataLoadingProgress(progress);
}

void ImagePropertiesColorsTab::setSelection(QRect *selectionArea)
{
    // This is necessary to stop computation because m_image.bits() is currently used by
    // threaded histogram algorithm.
    
    m_histogramWidget->stopHistogramComputation();
    m_selectionArea = selectionArea;
        
    if (m_selectionArea)
    {
        m_imageSelection = m_image.copy(*m_selectionArea);
        m_histogramWidget->updateSelectionData(m_imageSelection.bits(), m_imageSelection.width(),
                                               m_imageSelection.height(), m_imageSelection.sixteenBit());
        m_regionBG->show();                                         
    }
    else 
    {
        m_regionBG->hide();
        slotRenderingChanged(Digikam::HistogramWidget::FullImageHistogram);
    }
}

void ImagePropertiesColorsTab::slotRefreshOptions(bool sixteenBit)
{
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
    slotColorsChanged(m_colorsCB->currentItem());
    
    if (m_selectionArea)
       slotRenderingChanged(m_regionBG->selectedId());
}

void ImagePropertiesColorsTab::slotHistogramComputationFailed()
{
    m_imageSelection.reset();
    m_image.reset();
}

void ImagePropertiesColorsTab::slotChannelChanged(int channel)
{
    switch(channel)
    {
    case 1:           // Red.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
        m_colorsCB->setEnabled(false);
        break;
       
    case 2:           // Green.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
        m_colorsCB->setEnabled(false);
        break;
          
    case 3:           // Blue.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
        m_colorsCB->setEnabled(false);
        break;

    case 4:           // Alpha.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        m_colorsCB->setEnabled(false);
        break;
          
    case 5:           // All color channels.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::ColorChannelsHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        m_colorsCB->setEnabled(true);
        break;
                              
    default:          // Luminosity.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        m_colorsCB->setEnabled(false);
        break;
    }
   
    m_histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImagePropertiesColorsTab::slotColorsChanged(int color)
{
    switch(color)
    {
    case 1:           // Green.
        m_histogramWidget->m_colorType = Digikam::HistogramWidget::GreenColor;
        break;
       
    case 2:           // Blue.
        m_histogramWidget->m_colorType = Digikam::HistogramWidget::BlueColor;
        break;

    default:          // Red.
        m_histogramWidget->m_colorType = Digikam::HistogramWidget::RedColor;
        break;
    }

    m_histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotRenderingChanged(int rendering)
{
    m_histogramWidget->m_renderingType = rendering;
    m_histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotMinValueChanged(int min)
{
    // Called when user changes values of spin box.
    // Communicate the change to histogram widget.

    // make the one control "push" the other
    if (min == m_maxInterv->value()+1)
        m_maxInterv->setValue(min);
    m_maxInterv->setMinValue(min-1);
    m_histogramWidget->slotMinValueChanged(min);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotMaxValueChanged(int max)
{
    if (max == m_minInterv->value()-1)
        m_minInterv->setValue(max);
    m_minInterv->setMaxValue(max+1);
    m_histogramWidget->slotMaxValueChanged(max);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotUpdateInterval(int min, int max)
{
    // Called when value is set from within histogram widget.
    // Block signals to prevent slotMinValueChanged and
    // slotMaxValueChanged being called. 
    m_minInterv->blockSignals(true);
    m_minInterv->setMaxValue(max+1);
    m_minInterv->setValue(min);
    m_minInterv->blockSignals(false);

    m_maxInterv->blockSignals(true);
    m_maxInterv->setMinValue(min-1);
    m_maxInterv->setValue(max);
    m_maxInterv->blockSignals(false);

    updateStatistiques();
}

void ImagePropertiesColorsTab::slotUpdateIntervRange(int range)
{
    m_maxInterv->setMaxValue( range );
}

void ImagePropertiesColorsTab::updateInformations()
{
    m_labelColorDepth->setText(m_image.sixteenBit() ? i18n("16 bits") : i18n("8 bits"));
    m_labelAlphaChannel->setText(m_image.hasAlpha() ? i18n("Yes") : i18n("No"));
}

void ImagePropertiesColorsTab::updateStatistiques()
{
    QString value;
    int min = m_minInterv->value();
    int max = m_maxInterv->value();
    int channel = m_channelCB->currentItem();

    if ( channel == Digikam::HistogramWidget::ColorChannelsHistogram )
        channel = m_colorsCB->currentItem()+1;
               
    double mean = m_histogramWidget->m_imageHistogram->getMean(channel, min, max);
    m_labelMeanValue->setText(value.setNum(mean, 'f', 1));
    
    double pixels = m_histogramWidget->m_imageHistogram->getPixels();
    m_labelPixelsValue->setText(value.setNum((float)pixels, 'f', 0));
    
    double stddev = m_histogramWidget->m_imageHistogram->getStdDev(channel, min, max);
    m_labelStdDevValue->setText(value.setNum(stddev, 'f', 1));
      
    double counts = m_histogramWidget->m_imageHistogram->getCount(channel, min, max);
    m_labelCountValue->setText(value.setNum((float)counts, 'f', 0));
    
    double median = m_histogramWidget->m_imageHistogram->getMedian(channel, min, max);
    m_labelMedianValue->setText(value.setNum(median, 'f', 1)); 

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    m_labelPercentileValue->setText(value.setNum(percentile, 'f', 1));
}

void ImagePropertiesColorsTab::getICCData()
{
    //TODO
    if (m_image.getICCProfil().isNull())
    {
        m_infoHeader->setText(i18n("This image has not\nembedded Color Profile."));

        m_labelICCName->setText(i18n("N.A."));
        m_labelICCDescription->setText(i18n("N.A."));
        m_labelICCCopyright->setText(i18n("N.A."));
        m_labelICCIntent->setText(i18n("N.A."));
        m_labelICCColorSpace->setText(i18n("N.A."));
    }
    else
    {
        cmsHPROFILE   embProfile=0;
        QString intent, colorSpace;
        m_infoHeader->setText(i18n("Image Color Profile Info:"));
        
        m_embedded_profile = m_image.getICCProfil();
        embProfile = cmsOpenProfileFromMem(m_embedded_profile.data(),
                                          (DWORD)m_embedded_profile.size());
        m_labelICCName->setText(QString(cmsTakeProductName(embProfile)));
        m_labelICCDescription->setText(QString(cmsTakeProductDesc(embProfile)));
        m_labelICCCopyright->setText(QString(cmsTakeCopyright(embProfile)));

        switch (cmsTakeRenderingIntent(embProfile))
        {
            case 0:
                intent = i18n("Perceptual");
                break;
            case 1:
                intent = i18n("Relative Colorimetric");
                break;
            case 2:
                intent = i18n("Saturation");
                break;
            case 3:
                intent = i18n("Absolute Colorimetric");
                break;
        }

        switch (cmsGetColorSpace(embProfile))
        {
            case icSigLabData:
                colorSpace = i18n("Lab");
                break;
            case icSigLuvData:
                colorSpace = i18n("Luv");
                break;
            case icSigRgbData:
                colorSpace = i18n("RGB");
                break;
            case icSigGrayData:
                colorSpace = i18n("GRAY");
                break;
            case icSigHsvData:
                colorSpace = i18n("HSV");
                break;
            case icSigHlsData:
                colorSpace = i18n("HLS");
                break;
            case icSigCmykData:
                colorSpace = i18n("CMYK");
                break;
            case icSigCmyData:
                colorSpace= i18n("CMY");
                break;
            default:
                colorSpace = i18n("Other");
                break;
        }

        m_labelICCIntent->setText(intent);

        m_labelICCColorSpace->setText(colorSpace);

        cmsCloseProfile(embProfile);
    }
}

}  // NameSpace Digikam

#include "imagepropertiescolorstab.moc"
