/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-11
 * Description : a digiKam image editor plugin to correct 
 *               image white balance 
 * 
 * Copyright 2005-2007 by Gilles Caulier 
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
 
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qhbuttongroup.h> 
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qvbox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <kseparator.h>
#include <kconfig.h>
#include <kactivelabel.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "dcolor.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "whitebalance.h"
#include "colorgradientwidget.h"
#include "histogramwidget.h"
#include "dimgimagefilters.h"
#include "imageeffect_whitebalance.h"
#include "imageeffect_whitebalance.moc"

namespace DigikamWhiteBalanceImagesPlugin
{

ImageEffect_WhiteBalance::ImageEffect_WhiteBalance(QWidget* parent)
                        : Digikam::ImageDlgBase(parent, i18n("White Color Balance Correction"), 
                                                "whitebalance", true, false)
{
    QString whatsThis;

    Digikam::ImageIface iface(0, 0);

    m_destinationPreviewData = 0L;

    // About data and help button.

    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("White Color Balance Correction"), 
                                       digikam_version,
                                       I18N_NOOP("A digiKam image plugin to correct white color balance."),
                                       KAboutData::License_GPL,
                                       "(c) 2005-2007, Gilles Caulier", 
                                       0,
                                       "http://wwww.digikam.org");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor("Pawel T. Jochym", I18N_NOOP("White color balance correction algorithm"),
                     "jochym at ifj edu pl");

    setAboutData(about);

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("whitebalance Tool Dialog", plainPage(),
                                               i18n("<p>You can see here the image's white-balance "
                                                    "adjustments preview. You can pick color on image to "
                                                    "see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(plainPage());
    QVBoxLayout* layout2  = new QVBoxLayout( gboxSettings, spacingHint() );
    QGridLayout *grid     = new QGridLayout( layout2, 2, 4, spacingHint());

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

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(m_scaleBG);

    grid->addMultiCellLayout(l1, 0, 0, 0, 4);

    // -------------------------------------------------------------

    QVBox *histoBox   = new QVBox(gboxSettings);
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram "
                                             "drawing of the selected image channel. This one is "
                                             "re-computed at any filter settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    grid->addMultiCellWidget(histoBox, 1, 2, 0, 4);
    
    // -------------------------------------------------------------

    QGridLayout *grid2 = new QGridLayout(layout2, 13, 5, spacingHint());

    m_temperatureLabel    = new KActiveLabel(i18n("<qt><a href='http://en.wikipedia.org/wiki/Color_temperature'>Color Temperature</a> "
                                                  " (K): </qt>"), gboxSettings);
    m_adjTemperatureLabel = new QLabel(i18n("Adjustment:"), gboxSettings);
    m_temperatureInput    = new KDoubleNumInput(gboxSettings);
    m_temperatureInput->setPrecision(1);
    m_temperatureInput->setRange(2200.0, 7000.0, 10.0, true);
    QWhatsThis::add( m_temperatureInput, i18n("<p>Set here the white balance color temperature in Kelvin."));
    
    m_temperaturePresetLabel = new QLabel(i18n("Preset:"), gboxSettings);
    m_temperaturePresetCB = new QComboBox( false, gboxSettings );
    m_temperaturePresetCB->insertItem( i18n("Candle") );
    m_temperaturePresetCB->insertItem( i18n("40W Lamp") );
    m_temperaturePresetCB->insertItem( i18n("100W Lamp") );
    m_temperaturePresetCB->insertItem( i18n("200W Lamp") );
    m_temperaturePresetCB->insertItem( i18n("Sunrise") );
    m_temperaturePresetCB->insertItem( i18n("Studio Lamp") );
    m_temperaturePresetCB->insertItem( i18n("Moonlight") );
    m_temperaturePresetCB->insertItem( i18n("Neutral") );
    m_temperaturePresetCB->insertItem( i18n("Daylight D50") );
    m_temperaturePresetCB->insertItem( i18n("Photo Flash") );
    m_temperaturePresetCB->insertItem( i18n("Sun") );
    m_temperaturePresetCB->insertItem( i18n("Xenon Lamp") );
    m_temperaturePresetCB->insertItem( i18n("Daylight D65") );
    m_temperaturePresetCB->insertItem( i18n("None") );
    QWhatsThis::add( m_temperaturePresetCB, i18n("<p>Select here the white balance color temperature "
                                                 "preset to use:<p>"
                                                 "<b>Candle</b>: candle light (1850K).<p>"
                                                 "<b>40W Lamp</b>: 40 Watt incandescent lamp (2680K).<p>"
                                                 "<b>100W Lamp</b>: 100 Watt incandescent lamp (2800K).<p>"
                                                 "<b>200W Lamp</b>: 200 Watt incandescent lamp (3000K).<p>"
                                                 "<b>Sunrise</b>: sunrise or sunset light (3200K).<p>"
                                                 "<b>Studio Lamp</b>: tungsten lamp used in photo studio "
                                                 "or light at 1 hour from dusk/dawn (3400K).<p>"
                                                 "<b>Moonlight</b>: moon light (4100K).<p>"
                                                 "<b>Neutral</b>: neutral color temperature (4750K).<p>"
                                                 "<b>Daylight D50</b>: sunny daylight around noon (5000K).<p>"
                                                 "<b>Photo Flash</b>: electronic photo flash (5500K).<p>"
                                                 "<b>Sun</b>: effective sun temperature (5770K).<p>"
                                                 "<b>Xenon Lamp</b>: xenon lamp or light arc (6420K).<p>"
                                                 "<b>Daylight D65</b>: overcast sky light (6500K).<p>"
                                                 "<b>None</b>: no preset value."));
    m_pickTemperature = new QPushButton(gboxSettings);
    KGlobal::dirs()->addResourceType("color-picker-grey", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-grey", "color-picker-grey.png");
    m_pickTemperature->setPixmap( QPixmap( directory + "color-picker-grey.png" ) );
    m_pickTemperature->setToggleButton(true);
    QToolTip::add( m_pickTemperature, i18n( "Temperature tone color picker." ) );
    QWhatsThis::add( m_pickTemperature, i18n("<p>With this button, you can pick the color from original "
                                             "image used to set white color balance temperature and "
                                             "green component."));

    KSeparator *line = new KSeparator (Horizontal, gboxSettings);

    // -------------------------------------------------------------
    
    m_blackLabel = new QLabel(i18n("Black point:"), gboxSettings);
    m_blackInput = new KDoubleNumInput(gboxSettings);
    m_blackInput->setPrecision(2);
    m_blackInput->setRange(0.0, 0.05, 0.01, true);
    QWhatsThis::add( m_blackInput, i18n("<p>Set here the black level value."));
        
    m_darkLabel = new QLabel(i18n("Shadows:"), gboxSettings);
    m_darkInput = new KDoubleNumInput(gboxSettings);
    m_darkInput->setPrecision(2);
    m_darkInput->setRange(0.0, 1.0, 0.01, true);
    QWhatsThis::add( m_darkInput, i18n("<p>Set here the shadows noise suppresion level."));

    m_saturationLabel = new QLabel(i18n("Saturation:"), gboxSettings);
    m_saturationInput = new KDoubleNumInput(gboxSettings);
    m_saturationInput->setPrecision(2);
    m_saturationInput->setRange(0.0, 2.0, 0.01, true);
    QWhatsThis::add( m_saturationInput, i18n("<p>Set here the saturation value."));
        
    m_gammaLabel = new QLabel(i18n("Gamma:"), gboxSettings);
    m_gammaInput = new KDoubleNumInput(gboxSettings);
    m_gammaInput->setPrecision(2);
    m_gammaInput->setRange(0.1, 3.0, 0.01, true);
    QWhatsThis::add( m_gammaInput, i18n("<p>Set here the gamma correction value."));

    m_greenLabel = new QLabel(i18n("Green:"), gboxSettings);
    m_greenInput = new KDoubleNumInput(gboxSettings);
    m_greenInput->setPrecision(2);
    m_greenInput->setRange(1.0, 2.5, 0.01, true);
    QWhatsThis::add(m_greenInput, i18n("<p>Set here the green component to set magenta color "
                                       "cast removal level."));

    KSeparator *line2 = new KSeparator (Horizontal, gboxSettings);

    // -------------------------------------------------------------

    m_exposureLabel      = new KActiveLabel(i18n("<qt><a href='http://en.wikipedia.org/wiki/Exposure_value'>Exposure Compensation</a> "
                                                 " (E.V): </qt>"), gboxSettings);
    m_mainExposureLabel  = new QLabel(i18n("Main:"), gboxSettings);
    m_autoAdjustExposure = new QPushButton(gboxSettings);
    m_autoAdjustExposure->setPixmap(kapp->iconLoader()->loadIcon("run", (KIcon::Group)KIcon::Toolbar));
    QToolTip::add( m_autoAdjustExposure, i18n( "Auto exposure adjustments" ) );
    QWhatsThis::add( m_autoAdjustExposure, i18n("<p>With this button, you can automatically adjust Exposure "
                                                "and Black Point values."));
    m_mainExposureInput = new KDoubleNumInput(gboxSettings);
    m_mainExposureInput->setPrecision(2);
    m_mainExposureInput->setRange(-6.0, 8.0, 0.1, true);
    QWhatsThis::add( m_mainExposureInput, i18n("<p>Set here the main exposure compensation value in E.V."));

    m_fineExposureLabel = new QLabel(i18n("Fine:"), gboxSettings);
    m_fineExposureInput = new KDoubleNumInput(gboxSettings);
    m_fineExposureInput->setPrecision(2);
    m_fineExposureInput->setRange(-0.5, 0.5, 0.01, true);
    QWhatsThis::add( m_fineExposureInput, i18n("<p>This value in E.V will be added to main exposure "
                                               "compensation value to set fine exposure adjustment."));

    // -------------------------------------------------------------

    grid2->addMultiCellWidget(m_temperatureLabel, 0, 0, 0, 5);
    grid2->addMultiCellWidget(m_adjTemperatureLabel, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_pickTemperature, 1, 1, 1, 1);
    grid2->addMultiCellWidget(m_temperatureInput, 1, 1, 2, 5);
    grid2->addMultiCellWidget(m_temperaturePresetLabel, 2, 2, 0, 0);
    grid2->addMultiCellWidget(m_temperaturePresetCB, 2, 2, 2, 5);
    
    grid2->addMultiCellWidget(line, 3, 3, 0, 5);
    
    grid2->addMultiCellWidget(m_blackLabel, 4, 4, 0, 0);
    grid2->addMultiCellWidget(m_blackInput, 4, 4, 1, 5);
    grid2->addMultiCellWidget(m_darkLabel, 5, 5, 0, 0);
    grid2->addMultiCellWidget(m_darkInput, 5, 5, 1, 5);
    grid2->addMultiCellWidget(m_saturationLabel, 6, 6, 0, 0);
    grid2->addMultiCellWidget(m_saturationInput, 6, 6, 1, 5);
    grid2->addMultiCellWidget(m_gammaLabel, 7, 7, 0, 0);
    grid2->addMultiCellWidget(m_gammaInput, 7, 7, 1, 5);
    grid2->addMultiCellWidget(m_greenLabel, 8, 8, 0, 0);
    grid2->addMultiCellWidget(m_greenInput, 8, 8, 1, 5);

    grid2->addMultiCellWidget(line2, 9, 9, 0, 5);

    grid2->addMultiCellWidget(m_exposureLabel, 10, 10, 0, 5);
    grid2->addMultiCellWidget(m_mainExposureLabel, 11, 11, 0, 0);
    grid2->addMultiCellWidget(m_autoAdjustExposure, 11, 11, 1, 1);
    grid2->addMultiCellWidget(m_mainExposureInput, 11, 11, 2, 5);
    grid2->addMultiCellWidget(m_fineExposureLabel, 12, 12, 0, 1);
    grid2->addMultiCellWidget(m_fineExposureInput, 12, 12, 2, 5);
    grid2->setRowStretch(13, 10);
            
    setUserAreaWidget(gboxSettings);
            
    // -------------------------------------------------------------
 
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromOriginal( const Digikam::DColor & )));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));
                                    
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));                                        

    // -------------------------------------------------------------                
    // Correction Filter Slider controls.
                        
    connect(m_temperaturePresetCB, SIGNAL(activated(int)),
            this, SLOT(slotTemperaturePresetChanged(int)));
    
    connect(m_temperatureInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTemperatureChanged(double)));                       
            
    connect(m_darkInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                       
    
    connect(m_blackInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                       
    
    connect(m_mainExposureInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                       

    connect(m_fineExposureInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                       
    
    connect(m_gammaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                       

    connect(m_saturationInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                         

    connect(m_greenInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    // -------------------------------------------------------------
    // Bouttons slots.
    
    connect(m_autoAdjustExposure, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustExposure()));

    connect(m_pickTemperature, SIGNAL(released()),
            this, SLOT(slotPickerColorButtonActived()));            
}

ImageEffect_WhiteBalance::~ImageEffect_WhiteBalance()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
}

void ImageEffect_WhiteBalance::slotTemperatureChanged(double temperature)
{
   switch((uint)temperature)
   {
       case 1850:
          m_temperaturePresetCB->setCurrentItem(Candle);
          break;

       case 2680:
          m_temperaturePresetCB->setCurrentItem(Lamp40W);
          break;

       case 2800:
          m_temperaturePresetCB->setCurrentItem(Lamp100W);
          break;
       
       case 3000:
          m_temperaturePresetCB->setCurrentItem(Lamp200W);
          break;
       
       case 3200:
          m_temperaturePresetCB->setCurrentItem(Sunrise);
          break;
       
       case 3400:
          m_temperaturePresetCB->setCurrentItem(StudioLamp);
          break;

       case 4100:
          m_temperaturePresetCB->setCurrentItem(MoonLight);
          break;

       case 4750:
          m_temperaturePresetCB->setCurrentItem(Neutral);
          break;
                 
       case 5000:
          m_temperaturePresetCB->setCurrentItem(DaylightD50);
          break;
       
       case 5500:
          m_temperaturePresetCB->setCurrentItem(Flash);
          break;
       
       case 5770:
          m_temperaturePresetCB->setCurrentItem(Sun);
          break;
       
       case 6420:
          m_temperaturePresetCB->setCurrentItem(XeonLamp);
          break;

       case 6500:
          m_temperaturePresetCB->setCurrentItem(DaylightD65);
          break;
          
       default:
          m_temperaturePresetCB->setCurrentItem(None);
          break;
    }

    slotTimer();  
}

void ImageEffect_WhiteBalance::slotTemperaturePresetChanged(int tempPreset)
{
   switch(tempPreset)
   {
       case Candle:
          m_temperatureInput->setValue(1850.0);
          break;

       case Lamp40W:
          m_temperatureInput->setValue(2680.0);
          break;

       case Lamp100W:
          m_temperatureInput->setValue(2800.0);
          break;
       
       case Lamp200W:
          m_temperatureInput->setValue(3000.0);
          break;
       
       case Sunrise:
          m_temperatureInput->setValue(3200.0);
          break;
       
       case StudioLamp:
          m_temperatureInput->setValue(3400.0);
          break;

       case MoonLight:
          m_temperatureInput->setValue(4100.0);
          break;

       case Neutral:
          m_temperatureInput->setValue(4750.0);
          break;
                 
       case DaylightD50:
          m_temperatureInput->setValue(5000.0);
          break;
       
       case Flash:
          m_temperatureInput->setValue(5500.0);
          break;
       
       case Sun:
          m_temperatureInput->setValue(5770.0);
          break;
       
       case XeonLamp:
          m_temperatureInput->setValue(6420.0);
          break;

       case DaylightD65:
          m_temperatureInput->setValue(6500.0);
          break;
       
       default:    // None.
          break;
    }

    slotEffect();  
}

void ImageEffect_WhiteBalance::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    m_currentPreviewMode = m_previewWidget->getRenderingPreviewMode();
    m_previewWidget->setRenderingPreviewMode(Digikam::ImageGuideWidget::PreviewOriginalImage);
}

void ImageEffect_WhiteBalance::slotColorSelectedFromOriginal(const Digikam::DColor &color)
{
    if ( m_pickTemperature->isOn() )
    {
        Digikam::DColor dc = color;
        QColor tc = dc.getQColor();
        double temperatureLevel, greenLevel;
    
        Digikam::WhiteBalance::autoWBAdjustementFromColor(tc, temperatureLevel, greenLevel);
            
        m_temperatureInput->setValue(temperatureLevel);
        m_greenInput->setValue(greenLevel);
        m_pickTemperature->setOn(false);
    }
    else
        return;
       
    // restore previous rendering mode.
    m_previewWidget->setRenderingPreviewMode(m_currentPreviewMode);
    
    slotEffect();  
}

void ImageEffect_WhiteBalance::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_WhiteBalance::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_WhiteBalance::slotChannelChanged(int channel)
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

void ImageEffect_WhiteBalance::slotAutoAdjustExposure()
{
    parentWidget()->setCursor( KCursor::waitCursor() );

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int width                  = iface->originalWidth();
    int height                 = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();
    
    double blackLevel;
    double exposureLevel;

    Digikam::WhiteBalance::autoExposureAdjustement(data, width, height, sb, blackLevel, exposureLevel);
    delete [] data;        

    m_blackInput->setValue(blackLevel);
    m_mainExposureInput->setValue(exposureLevel);
    m_fineExposureInput->setValue(0.0);
    
    parentWidget()->unsetCursor();
    slotEffect();  
}

void ImageEffect_WhiteBalance::slotEffect()
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

    double temperature  = m_temperatureInput->value()/1000.0;
    double dark         = m_darkInput->value();
    double black        = m_blackInput->value();
    double mainExposure = m_mainExposureInput->value();
    double fineExposure = m_fineExposureInput->value();
    double gamma        = m_gammaInput->value();
    double saturation   = m_saturationInput->value();
    double green        = m_greenInput->value();
            
    Digikam::WhiteBalance wbFilter(sb);
    wbFilter.whiteBalance(data, w, h, sb, 
                          black, mainExposure + fineExposure,
                          temperature, green, dark, 
                          gamma, saturation);
     
    iface->putPreviewImage(data);
    m_previewWidget->updatePreview();
    
    // Update histogram.
    memcpy (m_destinationPreviewData, data, w*h*(sb ? 8 : 4));
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] data;
}

void ImageEffect_WhiteBalance::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    double temperature  = m_temperatureInput->value()/1000.0;
    double dark         = m_darkInput->value();
    double black        = m_blackInput->value();
    double mainExposure = m_mainExposureInput->value();
    double fineExposure = m_fineExposureInput->value();
    double gamma        = m_gammaInput->value();
    double saturation   = m_saturationInput->value();
    double green        = m_greenInput->value();
            
    Digikam::WhiteBalance wbFilter(sb);
    wbFilter.whiteBalance(data, w, h, sb, 
                          black, mainExposure + fineExposure,
                          temperature, green, dark, 
                          gamma, saturation);

    iface->putOriginalImage(i18n("White Balance"), data);
    delete [] data;
    kapp->restoreOverrideCursor();
    accept();       
}

void ImageEffect_WhiteBalance::resetValues()
{
    m_darkInput->blockSignals(true);
    m_blackInput->blockSignals(true);
    m_mainExposureInput->blockSignals(true);
    m_fineExposureInput->blockSignals(true);
    m_gammaInput->blockSignals(true);
    m_saturationInput->blockSignals(true);  
    m_greenInput->blockSignals(true);
    m_temperaturePresetCB->blockSignals(true);
    
    // Neutral color temperature settings.
    m_darkInput->setValue(0.5);
    m_blackInput->setValue(0.0);
    m_mainExposureInput->setValue(0.0);
    m_fineExposureInput->setValue(0.0);
    m_gammaInput->setValue(1.0);  
    m_saturationInput->setValue(1.0);  
    m_greenInput->setValue(1.2);  
    m_temperaturePresetCB->setCurrentItem(Neutral);
    slotTemperaturePresetChanged(Neutral);
    
    m_previewWidget->resetSpotPosition();    
    m_channelCB->setCurrentItem(LuminosityChannel);
    slotChannelChanged(LuminosityChannel);
    
    m_histogramWidget->reset();
    
    m_darkInput->blockSignals(false);
    m_blackInput->blockSignals(false);
    m_mainExposureInput->blockSignals(false);
    m_fineExposureInput->blockSignals(false);
    m_gammaInput->blockSignals(false);
    m_saturationInput->blockSignals(false);  
    m_greenInput->blockSignals(false);
    m_temperaturePresetCB->blockSignals(false);
    slotEffect();  
} 

void ImageEffect_WhiteBalance::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("whitebalance Tool Dialog");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LogScaleHistogram));

    m_darkInput->setValue(config->readDoubleNumEntry("Dark", 0.5));
    m_blackInput->setValue(config->readDoubleNumEntry("Black", 0.0));
    m_mainExposureInput->setValue(config->readDoubleNumEntry("MainExposure", 0.0));
    m_fineExposureInput->setValue(config->readDoubleNumEntry("FineExposure", 0.0));
    m_gammaInput->setValue(config->readDoubleNumEntry("Gamma", 1.0));  
    m_saturationInput->setValue(config->readDoubleNumEntry("Saturation", 1.0));  
    m_greenInput->setValue(config->readDoubleNumEntry("Green", 1.2));  
    m_temperatureInput->setValue(config->readDoubleNumEntry("Temperature", 4750.0));
    slotTemperatureChanged(m_temperatureInput->value());
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void ImageEffect_WhiteBalance::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("whitebalance Tool Dialog");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());

    config->writeEntry("Dark", m_darkInput->value());
    config->writeEntry("Black", m_blackInput->value());
    config->writeEntry("MainExposure", m_mainExposureInput->value());
    config->writeEntry("FineExposure", m_fineExposureInput->value());
    config->writeEntry("Gamma", m_gammaInput->value());
    config->writeEntry("Saturation", m_saturationInput->value());
    config->writeEntry("Green", m_greenInput->value());
    config->writeEntry("Temperature", m_temperatureInput->value()); 
    config->sync();
}

// Load all settings.
void ImageEffect_WhiteBalance::slotUser3()
{
    KURL loadWhiteBalanceFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("White Color Balance Settings File to Load")) );
    if( loadWhiteBalanceFile.isEmpty() )
       return;

    QFile file(loadWhiteBalanceFile.path());
    
    if ( file.open(IO_ReadOnly) )   
    {
        QTextStream stream( &file );

        if ( stream.readLine() != "# White Color Balance Configuration File V2" )
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a White Color Balance settings text file.")
                        .arg(loadWhiteBalanceFile.fileName()));
           file.close();            
           return;
        }
        
        blockSignals(true);
        m_temperatureInput->setValue( stream.readLine().toDouble() );
        m_darkInput->setValue( stream.readLine().toDouble() );
        m_blackInput->setValue( stream.readLine().toDouble() );
        m_mainExposureInput->setValue( stream.readLine().toDouble() );
        m_fineExposureInput->setValue( stream.readLine().toDouble() );
        m_gammaInput->setValue( stream.readLine().toDouble() );
        m_saturationInput->setValue( stream.readLine().toDouble() );
        m_greenInput->setValue( stream.readLine().toDouble() );
        m_histogramWidget->reset();
        blockSignals(false);
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the White Color Balance text file."));

    file.close();            
}

// Save all settings.
void ImageEffect_WhiteBalance::slotUser2()
{
    KURL saveWhiteBalanceFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("White Color Balance Settings File to Save")) );
    if( saveWhiteBalanceFile.isEmpty() )
       return;

    QFile file(saveWhiteBalanceFile.path());
    
    if ( file.open(IO_WriteOnly) )   
    {
        QTextStream stream( &file );        
        stream << "# White Color Balance Configuration File V2\n";    
        stream << m_temperatureInput->value() << "\n";    
        stream << m_darkInput->value() << "\n";    
        stream << m_blackInput->value() << "\n";    
        stream << m_mainExposureInput->value() << "\n";    
        stream << m_fineExposureInput->value() << "\n";    
        stream << m_gammaInput->value() << "\n";    
        stream << m_saturationInput->value() << "\n";    
        stream << m_greenInput->value() << "\n";    
    }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the White Color Balance text file."));
    
    file.close();        
}

}  // NameSpace DigikamWhiteBalanceImagesPlugin

