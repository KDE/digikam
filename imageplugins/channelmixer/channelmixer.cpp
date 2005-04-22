/* ============================================================
 * File  : channelmixer.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-26
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
 * Load and save mixer gains methods inspired from 
 * Gimp 2.2.3 and copyrighted 2002 by Martin Guldahl 
 * <mguldahl at xmission dot com>.
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

// C++ includes.

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes.

#include <qlayout.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qfile.h>

// KDE includes.

#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kselect.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "channelmixer.h"

namespace DigikamChannelMixerImagesPlugin
{

ChannelMixerDialog::ChannelMixerDialog(QWidget* parent, uint *imageData, uint width, uint height)
                  : KDialogBase(Plain, i18n("Color Channel Mixer"), Help|User1|Ok|Cancel, Ok,
                                parent, 0, true, true, i18n("&Reset Values"))
{
    m_destinationPreviewData = 0L;
    parentWidget()->setCursor( KCursor::waitCursor() );
    
    setButtonWhatsThis ( User1, i18n("<p>Reset color channels' gains settings from the current selected channel.") );

    // About data and help button.

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Channel Mixer"),
                                       digikamimageplugins_version,
                                       I18N_NOOP("An image color channel mixer plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Channel Mixer Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );

    // -------------------------------------------------------------

    QGridLayout* topLayout = new QGridLayout( plainPage(), 3, 2 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Image Color Channel Mixer"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");

    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );

    // -------------------------------------------------------------

    QGroupBox *gbox = new QGroupBox(plainPage());
    gbox->setFlat(false);
    gbox->setTitle(i18n("Settings"));
    QGridLayout* grid = new QGridLayout( gbox, 6, 5, 20, spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), gbox);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gbox );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->setCurrentText( i18n("Red") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the color channel to mix:<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    QLabel *label2 = new QLabel(i18n("Scale:"), gbox);
    label2->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_scaleCB = new QComboBox( false, gbox );
    m_scaleCB->insertItem( i18n("Linear") );
    m_scaleCB->insertItem( i18n("Logarithmic") );
    m_scaleCB->setCurrentText( i18n("Logarithmic") );
    QWhatsThis::add( m_scaleCB, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(m_channelCB, 0, 0, 1, 1);
    grid->addMultiCellWidget(label2, 0, 0, 3, 3);
    grid->addMultiCellWidget(m_scaleCB, 0, 0, 4, 4);

    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, imageData, width, height, frame, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing of the "
                                             "selected image channel. This one is re-computed at any mixer "
                                             "settings changes."));
    l->addWidget(m_histogramWidget, 0);
    grid->addMultiCellWidget(frame, 1, 1, 0, 4);
    
    m_hGradient = new Digikam::ColorGradientWidget( KSelector::Horizontal, 20, gbox );
    m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );

    QLabel *redLabel = new QLabel(i18n("Red:"), gbox);
    m_redGain = new KDoubleNumInput(gbox);
    m_redGain->setPrecision(0);
    m_redGain->setRange(-200.0, 200.0, 1, true);
    QWhatsThis::add( m_redGain, i18n("<p>Select here the red color gain in percent for the current channel."));
    
    QLabel *blueLabel = new QLabel(i18n("Blue:"), gbox);
    m_greenGain = new KDoubleNumInput(gbox);
    m_greenGain->setPrecision(0);
    m_greenGain->setRange(-200.0, 200.0, 1, true);
    QWhatsThis::add( m_greenGain, i18n("<p>Select here the green color gain in percent for the current channel."));
    
    QLabel *greenLabel = new QLabel(i18n("Green:"), gbox);
    m_blueGain = new KDoubleNumInput(gbox);
    m_blueGain->setPrecision(0);
    m_blueGain->setRange(-200.0, 200.0, 1, true);
    QWhatsThis::add( m_blueGain, i18n("<p>Select here the blue color gain in percent for the current channel."));

    grid->addMultiCellWidget(m_hGradient, 2, 2, 0, 4);
    grid->addMultiCellWidget(redLabel, 3, 3, 0, 0);
    grid->addMultiCellWidget(greenLabel, 4, 4, 0, 0);
    grid->addMultiCellWidget(blueLabel, 5, 5, 0, 0);
    grid->addMultiCellWidget(m_redGain, 3, 3, 1, 4);
    grid->addMultiCellWidget(m_greenGain, 4, 4, 1, 4);
    grid->addMultiCellWidget(m_blueGain, 5, 5, 1, 4);

    m_monochrome = new QCheckBox( i18n("Monochrome"), gbox);
    QWhatsThis::add( m_monochrome, i18n("<p>Enable this option if you want rendering the image in monochrome mode. "
                                        "In this mode, histogram will display only luminosity values."));
    
    m_preserveLuminosity = new QCheckBox( i18n("Preserve luminosity"), gbox);
    QWhatsThis::add( m_preserveLuminosity, i18n("<p>Enable this option is you want preserve the image luminosity."));
    
    grid->addMultiCellWidget(m_monochrome, 6, 6, 0, 1);
    grid->addMultiCellWidget(m_preserveLuminosity, 6, 6, 3, 4);
    
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);

    // -------------------------------------------------------------

    QHGroupBox *gbox3 = new QHGroupBox(i18n("All Channels' Gains"), plainPage());
    m_loadButton = new QPushButton(i18n("&Load..."), gbox3);
    QWhatsThis::add( m_loadButton, i18n("<p>Load gains settings from a Gimp gains text file."));
    m_saveButton = new QPushButton(i18n("&Save As..."), gbox3);
    QWhatsThis::add( m_saveButton, i18n("<p>Save gains settings to a Gimp gains text file."));
    m_resetButton = new QPushButton(i18n("&Reset All"), gbox3);
    QWhatsThis::add( m_resetButton, i18n("<p>Reset all color channels' gains settings."));

    topLayout->addMultiCellWidget(gbox3, 3, 3, 0, 0);

    // -------------------------------------------------------------

    QVGroupBox *gbox4 = new QVGroupBox(i18n("Preview"), plainPage());

    QFrame *frame2 = new QFrame(gbox4);
    frame2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2  = new QVBoxLayout(frame2, 5, 0);
    m_previewOriginalWidget = new Digikam::ImageWidget(300, 200, frame2);
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image."));
    l2->addWidget(m_previewOriginalWidget, 0, Qt::AlignCenter);

    QFrame *frame3 = new QFrame(gbox4);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3  = new QVBoxLayout(frame3, 5, 0);
    m_previewTargetWidget = new Digikam::ImageGuideWidget(300, 200, frame3, true, 
                                                          Digikam::ImageGuideWidget::PickColorMode);
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the image's color channels' "
                                                 "gains adjustments preview. You can pick color on image "
                                                 "to see the color level corresponding on histogram."));
    l3->addWidget(m_previewTargetWidget, 0, Qt::AlignCenter);

    m_overExposureIndicatorBox = new QCheckBox(i18n("Over exposure indicator"), gbox4);
    QWhatsThis::add( m_overExposureIndicatorBox, i18n("<p>If you enable this option, over-exposed pixels from the target image preview "
                                                      "will be over-colored. This will not have an effect on the final rendering."));
    
    topLayout->addMultiCellWidget(gbox4, 1, 3, 1, 1);

    adjustSize();
    disableResize();

    parentWidget()->setCursor( KCursor::arrowCursor()  );

    QTimer::singleShot(0, this, SLOT(slotResetAllGains())); // Reset all parameters to the default values.
                    
    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewTargetWidget, SIGNAL(spotPositionChanged( const QColor &, bool, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const QColor & ))); 
                        
    connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));              
    
    // -------------------------------------------------------------
    // Gains settings slots.

    connect(m_redGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));  

    connect(m_greenGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));  
    
    connect(m_blueGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));  

    connect(m_preserveLuminosity, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));             
    
    connect(m_monochrome, SIGNAL(toggled (bool)),
            this, SLOT(slotMonochromeActived(bool)));             

    // -------------------------------------------------------------
    // Bouttons slots.

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetAllGains()));

    connect(m_loadButton, SIGNAL(clicked()),
            this, SLOT(slotLoadGains()));

    connect(m_saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveGains()));
}

ChannelMixerDialog::~ChannelMixerDialog()
{
}

void ChannelMixerDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("channelmixer", "digikamimageplugins");
}

void ChannelMixerDialog::closeEvent(QCloseEvent *e)
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    e->accept();
}

void ChannelMixerDialog::slotResetAllGains()
{
    m_redRedGain = 1.0; 
    m_redGreenGain = 0.0; 
    m_redBlueGain = 0.0; 
    
    m_greenRedGain = 0.0;
    m_greenGreenGain = 1.0; 
    m_greenBlueGain = 0.0;
    
    m_blueRedGain = 0.0;
    m_blueGreenGain = 0.0;
    m_blueBlueGain = 1.0;

    m_blackRedGain = 1.0; 
    m_blackGreenGain = 0.0; 
    m_blackBlueGain = 0.0; 
    
    m_monochrome->blockSignals(true);
    m_preserveLuminosity->blockSignals(true);
    
    m_monochrome->setChecked( false );
    m_preserveLuminosity->setChecked( false );
    adjustSliders();
    
    m_monochrome->blockSignals(false);
    m_preserveLuminosity->blockSignals(false);
    m_channelCB->setEnabled(true);
    
    m_histogramWidget->reset();

    slotEffect();
}

void ChannelMixerDialog::slotUser1()
{
    switch( m_channelCB->currentItem() )
       {
       case GreenChannelGains:           // Green.
          m_greenRedGain   = 0.0;
          m_greenGreenGain = 1.0;
          m_greenBlueGain  = 0.0;
          break;

       case BlueChannelGains:            // Blue.
          m_blueRedGain   = 0.0;
          m_blueGreenGain = 0.0;
          m_blueBlueGain  = 1.0;
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
             {
             m_blackRedGain   = 1.0;
             m_blackGreenGain = 0.0;
             m_blackBlueGain  = 0.0;
             }
          else
             {
             m_redRedGain   = 1.0;
             m_redGreenGain = 0.0;
             m_redBlueGain  = 0.0;
             }
          break;
       }
    
    adjustSliders();
    slotEffect();
    m_histogramWidget->reset();
}

void ChannelMixerDialog::slotColorSelectedFromTarget( const QColor &color )
{
    m_histogramWidget->setHistogramGuide(color);
}

void ChannelMixerDialog::slotGainsChanged()
{
    switch( m_channelCB->currentItem() )
       {
       case GreenChannelGains:           // Green.
          m_greenRedGain   = m_redGain->value() / 100.0;
          m_greenGreenGain = m_greenGain->value() / 100.0;
          m_greenBlueGain  = m_blueGain->value() / 100.0;
          break;

       case BlueChannelGains:            // Blue.
          m_blueRedGain   = m_redGain->value() / 100.0;
          m_blueGreenGain = m_greenGain->value() / 100.0;
          m_blueBlueGain  = m_blueGain->value() / 100.0;
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
             {
             m_blackRedGain   = m_redGain->value() / 100.0;
             m_blackGreenGain = m_greenGain->value() / 100.0;
             m_blackBlueGain  = m_blueGain->value() / 100.0;
             }
          else
             {
             m_redRedGain   = m_redGain->value() / 100.0;
             m_redGreenGain = m_greenGain->value() / 100.0;
             m_redBlueGain  = m_blueGain->value() / 100.0;
             }
          break;
       }

    slotEffect();
}

void ChannelMixerDialog::adjustSliders(void)
{
    m_redGain->blockSignals(true);
    m_greenGain->blockSignals(true);
    m_blueGain->blockSignals(true);

    switch( m_channelCB->currentItem() )
       {
       case GreenChannelGains:           // Green.
          m_redGain->setValue(m_greenRedGain * 100.0);
          m_greenGain->setValue(m_greenGreenGain * 100.0);
          m_blueGain->setValue(m_greenBlueGain * 100.0);
          break;

       case BlueChannelGains:            // Blue.
          m_redGain->setValue(m_blueRedGain * 100.0);
          m_greenGain->setValue(m_blueGreenGain * 100.0);
          m_blueGain->setValue(m_blueBlueGain * 100.0);
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
             {
             m_redGain->setValue(m_blackRedGain * 100.0);
             m_greenGain->setValue(m_blackGreenGain * 100.0);
             m_blueGain->setValue(m_blackBlueGain * 100.0);
             }
          else
             {
             m_redGain->setValue(m_redRedGain * 100.0);
             m_greenGain->setValue(m_redGreenGain * 100.0);
             m_blueGain->setValue(m_redBlueGain * 100.0);
             }
          break;
       }
    
    m_redGain->blockSignals(false);
    m_greenGain->blockSignals(false);
    m_blueGain->blockSignals(false);
}

void ChannelMixerDialog::slotMonochromeActived(bool mono)
{
    m_channelCB->setEnabled(!mono);
    m_channelCB->setCurrentItem(RedChannelGains); // Red for monochrome.
    slotChannelChanged(RedChannelGains);          // Monochrome => display luminosity histogram value.
}

void ChannelMixerDialog::slotEffect()
{
    Digikam::ImageIface* ifaceOrg = m_previewOriginalWidget->imageIface();

    Digikam::ImageIface* ifaceDest = m_previewTargetWidget->imageIface();

    uint* orgData = ifaceOrg->getPreviewData();
    int   w       = ifaceOrg->previewWidth();
    int   h       = ifaceOrg->previewHeight();
    bool  l       = m_preserveLuminosity->isChecked();
    bool  m       = m_monochrome->isChecked();

    // Create the new empty destination image data space.
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
    
    m_destinationPreviewData = new uint[w*h];

    memcpy (m_destinationPreviewData, orgData, w*h*4);  

    if (m)
       {
       Digikam::ImageFilters::channelMixerImage(m_destinationPreviewData, w, h,    // Image data.
                l,                                                                 // Preserve luminosity.    
                m,                                                                 // Monochrome.
                m_blackRedGain, m_blackGreenGain, m_blackBlueGain,                 // Red channel gains.
                0.0,            1.0,              0.0,                             // Green channel gains (not used).
                0.0,            0.0,              1.0,                             // Blue channel gains (not used).
                m_overExposureIndicatorBox->isChecked());
       }
    else
       {
       Digikam::ImageFilters::channelMixerImage(m_destinationPreviewData, w, h,    // Image data.
                l,                                                                 // Preserve luminosity.    
                m,                                                                 // Monochrome.
                m_redRedGain,   m_redGreenGain,   m_redBlueGain,                   // Red channel gains.
                m_greenRedGain, m_greenGreenGain, m_greenBlueGain,                 // Green channel gains.
                m_blueRedGain,  m_blueGreenGain,  m_blueBlueGain,                  // Blue channel gains.
                m_overExposureIndicatorBox->isChecked());
       }
    
    ifaceDest->putPreviewData(m_destinationPreviewData);
    m_previewTargetWidget->update();
    
    // Update histogram.
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, 0, 0, 0, false); 
    
    delete [] orgData;
}

void ChannelMixerDialog::slotOk()
{
    Digikam::ImageIface* ifaceOrg = m_previewOriginalWidget->imageIface();

    Digikam::ImageIface* ifaceDest = m_previewTargetWidget->imageIface();

    uint* orgData = ifaceOrg->getOriginalData();
    int   w       = ifaceOrg->originalWidth();
    int   h       = ifaceOrg->originalHeight();
    bool  l       = m_preserveLuminosity->isChecked();
    bool  m       = m_monochrome->isChecked();

    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];

    memcpy (desData, orgData, w*h*4);  

    if (m)
       {
       Digikam::ImageFilters::channelMixerImage(desData, w, h,      // Image data.
                l,                                                  // Preserve luminosity.    
                m,                                                  // Monochrome.
                m_blackRedGain, m_blackGreenGain, m_blackBlueGain,  // Red channel gains.
                0.0,            1.0,              0.0,              // Green channel gains (not used).
                0.0,            0.0,              1.0);             // Blue channel gains (not used).
       }
    else
       {
       Digikam::ImageFilters::channelMixerImage(desData, w, h,      // Image data.
                l,                                                  // Preserve luminosity.    
                m,                                                  // Monochrome.
                m_redRedGain,   m_redGreenGain,   m_redBlueGain,    // Red channel gains.
                m_greenRedGain, m_greenGreenGain, m_greenBlueGain,  // Green channel gains.
                m_blueRedGain,  m_blueGreenGain,  m_blueBlueGain);  // Blue channel gains.
       }
    
    ifaceDest->putOriginalData(i18n("Channel Mixer"), desData);

    delete [] orgData;
    delete [] desData;
    accept();
}

void ChannelMixerDialog::slotChannelChanged(int channel)
{
    switch(channel)
       {
       case GreenChannelGains:           // Green.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
          break;

       case BlueChannelGains:            // Blue.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
             {
             m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
             m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );          
             }
          else
             {
             m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
             m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );          
             }
          break;
       }

    m_histogramWidget->repaint(false);
    adjustSliders();
    slotEffect();
}

void ChannelMixerDialog::slotScaleChanged(int scale)
{
    switch(scale)
       {
       case Linear:
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LinScaleHistogram;
          break;
       
       case Logarithmic:
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LogScaleHistogram;
          break;
       }

    m_histogramWidget->repaint(false);
}

void ChannelMixerDialog::slotLoadGains()
{
    KURL loadGainsFileUrl;
    FILE *fp = 0L;
    int currentOutputChannel;
    bool monochrome;
    
    loadGainsFileUrl = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Select Gimp Gains Mixer File to Load")) );
    if( loadGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(loadGainsFileUrl.path()), "r");   
    
    if ( fp )
        {
        char buf1[1024];
        char buf2[1024];
        char buf3[1024];

        buf1[0] = '\0';

        fgets(buf1, 1023, fp);

        fscanf (fp, "%*s %s", buf1);
        
        // Get the current output channel in dialog.
        
        if (strcmp (buf1, "RED") == 0)
            currentOutputChannel = RedChannelGains;
        else if (strcmp (buf1, "GREEN") == 0)
            currentOutputChannel = GreenChannelGains; 
        else if (strcmp (buf1, "BLUE") == 0)
            currentOutputChannel = BlueChannelGains;  

        fscanf (fp, "%*s %s", buf1); // preview flag, preserved for compatibility 

        fscanf (fp, "%*s %s", buf1);
          
        if (strcmp (buf1, "TRUE") == 0)
            monochrome = true;
        else
            monochrome = false;

        fscanf (fp, "%*s %s", buf1);
        
        if (strcmp (buf1, "TRUE") == 0)
            m_preserveLuminosity->setChecked(true);
        else
            m_preserveLuminosity->setChecked(false);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_redRedGain = atof(buf1);
        m_redGreenGain = atof(buf2);
        m_redBlueGain = atof(buf3);
        
        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_greenRedGain = atof(buf1);
        m_greenGreenGain = atof(buf2);
        m_greenBlueGain = atof(buf3);
        
        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_blueRedGain = atof(buf1);
        m_blueGreenGain = atof(buf2);
        m_blueBlueGain = atof(buf3);
        
        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_blackRedGain = atof(buf1);
        m_blackGreenGain = atof(buf2);
        m_blackBlueGain = atof(buf3);
        
        fclose(fp);

        // Refresh settings.
        m_monochrome->setChecked(monochrome);
        m_channelCB->setCurrentItem(currentOutputChannel);
        slotChannelChanged(currentOutputChannel);
        }
    else
        {
        KMessageBox::error(this, i18n("Cannot load settings from the Gains Mixer text file."));
        return;
        }
}

void ChannelMixerDialog::slotSaveGains()
{
    KURL saveGainsFileUrl;
    FILE *fp = 0L;
    
    saveGainsFileUrl = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                               QString( "*" ), this,
                                               QString( i18n("Gimp Gains Mixer File to Save")) );
    if( saveGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(saveGainsFileUrl.path()), "w");   
    
    if ( fp )
        {       
        const char *str = 0L;
        char        buf1[256];
        char        buf2[256];
        char        buf3[256];

        switch ( m_channelCB->currentItem() )
           {
           case RedChannelGains:
              str = "RED";
              break;
           case GreenChannelGains:
              str = "GREEN";
              break;
           case BlueChannelGains:
              str = "BLUE";
              break;
           default:
              kdWarning() <<  "Unknown Color channel gains" << endl;;
              break;
           }

        fprintf (fp, "# Channel Mixer Configuration File\n");

        fprintf (fp, "CHANNEL: %s\n", str);
        fprintf (fp, "PREVIEW: %s\n", "TRUE"); // preserved for compatibility 
        fprintf (fp, "MONOCHROME: %s\n",
                 m_monochrome->isChecked() ? "TRUE" : "FALSE");
        fprintf (fp, "PRESERVE_LUMINOSITY: %s\n",
                 m_preserveLuminosity->isChecked() ? "TRUE" : "FALSE");

        sprintf (buf1, "%5.3f", m_redRedGain);                 
        sprintf (buf2, "%5.3f", m_redGreenGain);
        sprintf (buf3, "%5.3f", m_redBlueGain);
        fprintf (fp, "RED: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", m_greenRedGain);                 
        sprintf (buf2, "%5.3f", m_greenGreenGain);
        sprintf (buf3, "%5.3f", m_greenBlueGain);
        fprintf (fp, "GREEN: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", m_blueRedGain);
        sprintf (buf2, "%5.3f", m_blueGreenGain);
        sprintf (buf3, "%5.3f", m_blueBlueGain);
        fprintf (fp, "BLUE: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", m_blackRedGain);
        sprintf (buf2, "%5.3f", m_blackGreenGain);
        sprintf (buf3, "%5.3f", m_blackBlueGain);
        fprintf (fp, "BLACK: %s %s %s\n", buf1, buf2,buf3);
        
        fclose (fp);
        }
    else
        {
        KMessageBox::error(this, i18n("Cannot save settings to the Gains Mixer text file."));
        return;
        }
}

}  // NameSpace DigikamChannelMixerImagesPlugin

#include "channelmixer.moc"
