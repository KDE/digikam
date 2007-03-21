/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-02-26
 * Description : 
 * 
 * Copyright 2005-2007 by Gilles Caulier 
 *
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

#include <qcolor.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h> 
#include <qlabel.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qtooltip.h>

// KDE includes.

#include <kconfig.h>
#include <kcursor.h>
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

// Local includes.

#include "version.h"
#include "dimg.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimgimagefilters.h"
#include "channelmixer.h"
#include "channelmixer.moc"

namespace DigikamChannelMixerImagesPlugin
{

ChannelMixerDialog::ChannelMixerDialog(QWidget* parent)
                  : Digikam::ImageDlgBase(parent, i18n("Color Channel Mixer"), 
                    "channelmixer", true, false)
{
    m_destinationPreviewData = 0L;

    // About data and help button.

    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Color Channel Mixer"),
                                       digikam_version,
                                       I18N_NOOP("An image color channel mixer plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2005-2007, Gilles Caulier",
                                       0,
                                       "http://www.digikam.org");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    setAboutData(about);

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("channelmixer Tool Dialog", plainPage(),
                          i18n("<p>You can see here the image's color channels' "
                               "gains adjustments preview. You can pick color on image "
                               "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* grid = new QGridLayout( gboxSettings, 9, 4, spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gboxSettings );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->setCurrentText( i18n("Red") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the color channel to mix:<p>"
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

    grid->addMultiCellLayout(l1, 0, 0, 0, 4);

    // -------------------------------------------------------------

    QVBox *histoBox   = new QVBox(gboxSettings);
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing "
                                             "of the selected image channel. This one is re-computed at any "
                                             "mixer settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
    
    grid->addMultiCellWidget(histoBox, 1, 2, 0, 4);
    
    // -------------------------------------------------------------
        
    QLabel *redLabel = new QLabel(i18n("Red:"), gboxSettings);
    m_redGain = new KDoubleNumInput(gboxSettings);
    m_redGain->setPrecision(0);
    m_redGain->setRange(-200.0, 200.0, 1, true);
    QWhatsThis::add( m_redGain, i18n("<p>Select here the red color gain in percent for the current channel."));
    
    QLabel *blueLabel = new QLabel(i18n("Blue:"), gboxSettings);
    m_greenGain = new KDoubleNumInput(gboxSettings);
    m_greenGain->setPrecision(0);
    m_greenGain->setRange(-200.0, 200.0, 1, true);
    QWhatsThis::add( m_greenGain, i18n("<p>Select here the green color gain in percent for the current channel."));
    
    QLabel *greenLabel = new QLabel(i18n("Green:"), gboxSettings);
    m_blueGain = new KDoubleNumInput(gboxSettings);
    m_blueGain->setPrecision(0);
    m_blueGain->setRange(-200.0, 200.0, 1, true);
    QWhatsThis::add( m_blueGain, i18n("<p>Select here the blue color gain in percent for the current channel."));

    m_resetButton = new QPushButton(i18n("&Reset"), gboxSettings);
    QWhatsThis::add( m_resetButton, i18n("Reset color channels' gains settings from the currently selected channel."));

    grid->addMultiCellWidget(redLabel, 3, 3, 0, 0);
    grid->addMultiCellWidget(greenLabel, 4, 4, 0, 0);
    grid->addMultiCellWidget(blueLabel, 5, 5, 0, 0);
    grid->addMultiCellWidget(m_redGain, 3, 3, 1, 4);
    grid->addMultiCellWidget(m_greenGain, 4, 4, 1, 4);
    grid->addMultiCellWidget(m_blueGain, 5, 5, 1, 4);
    grid->addMultiCellWidget(m_resetButton, 6, 6, 0, 1);

    // -------------------------------------------------------------
    
    m_monochrome = new QCheckBox( i18n("Monochrome"), gboxSettings);
    QWhatsThis::add( m_monochrome, i18n("<p>Enable this option if you want the image rendered in monochrome mode. "
                                        "In this mode, the histogram will display only luminosity values."));
    
    m_preserveLuminosity = new QCheckBox( i18n("Preserve luminosity"), gboxSettings);
    QWhatsThis::add( m_preserveLuminosity, i18n("<p>Enable this option is you want preserve the image luminosity."));
    
    grid->addMultiCellWidget(m_monochrome, 7, 7, 0, 4);
    grid->addMultiCellWidget(m_preserveLuminosity, 8, 8, 0, 4);
    grid->setRowStretch(9, 10);
    
    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));
                        
    connect(m_previewWidget, SIGNAL(signalResized()),
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
            this, SLOT(slotResetCurrentChannel()));
}

ChannelMixerDialog::~ChannelMixerDialog()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
}

void ChannelMixerDialog::slotResetCurrentChannel()
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

void ChannelMixerDialog::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ChannelMixerDialog::slotGainsChanged()
{
    switch( m_channelCB->currentItem() )
    {
       case GreenChannelGains:           // Green.
          m_greenRedGain   = m_redGain->value()   / 100.0;
          m_greenGreenGain = m_greenGain->value() / 100.0;
          m_greenBlueGain  = m_blueGain->value()  / 100.0;
          break;

       case BlueChannelGains:            // Blue.
          m_blueRedGain   = m_redGain->value()   / 100.0;
          m_blueGreenGain = m_greenGain->value() / 100.0;
          m_blueBlueGain  = m_blueGain->value()  / 100.0;
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
          {
             m_blackRedGain   = m_redGain->value()   / 100.0;
             m_blackGreenGain = m_greenGain->value() / 100.0;
             m_blackBlueGain  = m_blueGain->value()  / 100.0;
          }
          else
          {
             m_redRedGain   = m_redGain->value()   / 100.0;
             m_redGreenGain = m_greenGain->value() / 100.0;
             m_redBlueGain  = m_blueGain->value()  / 100.0;
          }
          break;
    }

    slotTimer();
}

void ChannelMixerDialog::adjustSliders(void)
{
    m_redGain->blockSignals(true);
    m_greenGain->blockSignals(true);
    m_blueGain->blockSignals(true);

    switch( m_channelCB->currentItem() )
    {
       case GreenChannelGains:           // Green.
          m_redGain->setValue(m_greenRedGain     * 100.0);
          m_greenGain->setValue(m_greenGreenGain * 100.0);
          m_blueGain->setValue(m_greenBlueGain   * 100.0);
          break;

       case BlueChannelGains:            // Blue.
          m_redGain->setValue(m_blueRedGain     * 100.0);
          m_greenGain->setValue(m_blueGreenGain * 100.0);
          m_blueGain->setValue(m_blueBlueGain   * 100.0);
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
          {
             m_redGain->setValue(m_blackRedGain     * 100.0);
             m_greenGain->setValue(m_blackGreenGain * 100.0);
             m_blueGain->setValue(m_blackBlueGain   * 100.0);
          }
          else
          {
             m_redGain->setValue(m_redRedGain     * 100.0);
             m_greenGain->setValue(m_redGreenGain * 100.0);
             m_blueGain->setValue(m_redBlueGain   * 100.0);
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
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();
    
    // Create the new empty destination image data space.
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
    
    m_destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];
    Digikam::DImgImageFilters filter;

    if (m_monochrome->isChecked())
    {
       filter.channelMixerImage(data, w, h, sb,                                 // Image data.
                m_preserveLuminosity->isChecked(),                              // Preserve luminosity.
                m_monochrome->isChecked(),                                      // Monochrome.
                m_blackRedGain, m_blackGreenGain, m_blackBlueGain,              // Red channel gains.
                0.0,            1.0,              0.0,                          // Green channel gains (not used).
                0.0,            0.0,              1.0);                         // Blue channel gains (not used).
    }
    else
    {
       filter.channelMixerImage(data, w, h, sb,                                 // Image data.
                m_preserveLuminosity->isChecked(),                              // Preserve luminosity.
                m_monochrome->isChecked(),                                      // Monochrome.
                m_redRedGain,   m_redGreenGain,   m_redBlueGain,                // Red channel gains.
                m_greenRedGain, m_greenGreenGain, m_greenBlueGain,              // Green channel gains.
                m_blueRedGain,  m_blueGreenGain,  m_blueBlueGain);              // Blue channel gains.
    }
    
    iface->putPreviewImage(data);
    m_previewWidget->updatePreview();
    
    // Update histogram.
    memcpy (m_destinationPreviewData, data, w*h*(sb ? 8 : 4));
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] data;
}

void ChannelMixerDialog::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    Digikam::DImgImageFilters filter;

    if (m_monochrome->isChecked())
    {
       filter.channelMixerImage(data, w, h, sb,                     // Image data.
                m_preserveLuminosity->isChecked(),                  // Preserve luminosity.
                m_monochrome->isChecked(),                          // Monochrome.
                m_blackRedGain, m_blackGreenGain, m_blackBlueGain,  // Red channel gains.
                0.0,            1.0,              0.0,              // Green channel gains (not used).
                0.0,            0.0,              1.0);             // Blue channel gains (not used).
    }
    else
    {
       filter.channelMixerImage(data, w, h, sb,                     // Image data.
                m_preserveLuminosity->isChecked(),                  // Preserve luminosity.
                m_monochrome->isChecked(),                          // Monochrome.
                m_redRedGain,   m_redGreenGain,   m_redBlueGain,    // Red channel gains.
                m_greenRedGain, m_greenGreenGain, m_greenBlueGain,  // Green channel gains.
                m_blueRedGain,  m_blueGreenGain,  m_blueBlueGain);  // Blue channel gains.
    }

    iface->putOriginalImage(i18n("Channel Mixer"), data);
    delete [] data;
    kapp->restoreOverrideCursor();
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
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ChannelMixerDialog::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("channelmixer Tool Dialog");

    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LogScaleHistogram));

    m_monochrome->setChecked(config->readBoolEntry("Monochrome", false));
    m_preserveLuminosity->setChecked(config->readNumEntry("PreserveLuminosity", false));

    m_redRedGain     = config->readDoubleNumEntry("RedRedGain", 1.0); 
    m_redGreenGain   = config->readDoubleNumEntry("RedGreenGain", 0.0); 
    m_redBlueGain    = config->readDoubleNumEntry("RedBlueGain", 0.0); 
    
    m_greenRedGain   = config->readDoubleNumEntry("GreenRedGain", 0.0);
    m_greenGreenGain = config->readDoubleNumEntry("GreenGreenGain", 1.0); 
    m_greenBlueGain  = config->readDoubleNumEntry("GreenBlueGain", 0.0);
    
    m_blueRedGain    = config->readDoubleNumEntry("BlueRedGain", 0.0);
    m_blueGreenGain  = config->readDoubleNumEntry("BlueGreenGain", 0.0);
    m_blueBlueGain   = config->readDoubleNumEntry("BlueBlueGain", 1.0);

    m_blackRedGain   = config->readDoubleNumEntry("BlackRedGain", 1.0); 
    m_blackGreenGain = config->readDoubleNumEntry("BlackGreenGain", 0.0); 
    m_blackBlueGain  = config->readDoubleNumEntry("BlackBlueGain", 0.0); 

    adjustSliders();
    m_histogramWidget->reset();

    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void ChannelMixerDialog::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("channelmixer Tool Dialog");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());

    config->writeEntry("Monochrome", m_monochrome->isChecked());
    config->writeEntry("PreserveLuminosity", m_preserveLuminosity->isChecked());

    config->writeEntry("RedRedGain", m_redRedGain);
    config->writeEntry("RedGreenGain", m_redGreenGain);
    config->writeEntry("RedBlueGain", m_redBlueGain);

    config->writeEntry("GreenRedGain", m_greenRedGain);
    config->writeEntry("GreenGreenGain", m_greenGreenGain);
    config->writeEntry("GreenBlueGain", m_greenBlueGain);

    config->writeEntry("BlueRedGain", m_blueRedGain);
    config->writeEntry("BlueGreenGain", m_blueGreenGain);
    config->writeEntry("BlueBlueGain", m_blueBlueGain);

    config->writeEntry("BlackRedGain", m_blackRedGain);
    config->writeEntry("BlackGreenGain", m_blackGreenGain);
    config->writeEntry("BlackBlueGain", m_blackBlueGain);

    config->sync();
}

void ChannelMixerDialog::resetValues()
{
    m_monochrome->blockSignals(true);
    m_preserveLuminosity->blockSignals(true);
    
    m_redRedGain     = 1.0; 
    m_redGreenGain   = 0.0; 
    m_redBlueGain    = 0.0; 
    
    m_greenRedGain   = 0.0;
    m_greenGreenGain = 1.0; 
    m_greenBlueGain  = 0.0;
    
    m_blueRedGain    = 0.0;
    m_blueGreenGain  = 0.0;
    m_blueBlueGain   = 1.0;

    m_blackRedGain   = 1.0; 
    m_blackGreenGain = 0.0; 
    m_blackBlueGain  = 0.0; 

    adjustSliders();
    
    m_monochrome->blockSignals(false);
    m_preserveLuminosity->blockSignals(false);
    m_channelCB->setEnabled(true);
    
    m_histogramWidget->reset();

    slotChannelChanged(RedChannelGains);
}

// Load all gains.
void ChannelMixerDialog::slotUser3()
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
          
        if (strcmp (buf1, "true") == 0)
            monochrome = true;
        else
            monochrome = false;

        fscanf (fp, "%*s %s", buf1);
        
        if (strcmp (buf1, "true") == 0)
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

// Save all gains.
void ChannelMixerDialog::slotUser2()
{
    KURL saveGainsFileUrl;
    FILE *fp = 0L;
    
    saveGainsFileUrl = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
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
              DWarning() <<  "Unknown Color channel gains" << endl;
              break;
        }

        fprintf (fp, "# Channel Mixer Configuration File\n");

        fprintf (fp, "CHANNEL: %s\n", str);
        fprintf (fp, "PREVIEW: %s\n", "true"); // preserved for compatibility 
        fprintf (fp, "MONOCHROME: %s\n",
                 m_monochrome->isChecked() ? "true" : "false");
        fprintf (fp, "PRESERVE_LUMINOSITY: %s\n",
                 m_preserveLuminosity->isChecked() ? "true" : "false");

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

