/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor plugin to correct
 *               image white balance
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Guillaume Castagnino <casta at xwing dot info>
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

#include <QButtonGroup>
#include <QComboBox>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QTextStream>
#include <QTimer>
#include <QToolButton>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes.

#include <k3activelabel.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

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

using namespace KDcrawIface;

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

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("White Color Balance Correction"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A digiKam image plugin to correct white color balance."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2008, Gilles Caulier"),
                                       KLocalizedString(),
                                       "http://wwww.digikam.org");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Pawel T. Jochym"), ki18n("White color balance correction algorithm"),
                     "jochym at ifj edu pl");

    setAboutData(about);

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("whitebalance Tool Dialog", mainWidget(),
                                               i18n("<p>You can see here the image's white-balance "
                                                    "adjustments preview. You can pick color on image to "
                                                    "see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(mainWidget());
    QVBoxLayout* layout2  = new QVBoxLayout(gboxSettings);
    QGridLayout *grid     = new QGridLayout();

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( gboxSettings );
    m_channelCB->addItem( i18n("Luminosity") );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->setWhatsThis( i18n("<p>Select the histogram channel to display here:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"));

    QWidget *scaleBox  = new QWidget(gboxSettings);
    QHBoxLayout *hlay1 = new QHBoxLayout(scaleBox);
    m_scaleBG          = new QButtonGroup(scaleBox);
    m_scaleBG->setExclusive(true);
    scaleBox->setWhatsThis( i18n("<p>Select the histogram scale here.<p>"
                                 "If the image's maximal counts are small, you can use the linear scale.<p>"
                                 "Logarithmic scale can be used when the maximal counts are big; "
                                 "if it is used, all values (small and large) will be visible on the "
                                 "graph."));

    QToolButton *linHistoButton = new QToolButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    m_scaleBG->addButton(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    hlay1->addWidget(linHistoButton);

    QToolButton *logHistoButton = new QToolButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    m_scaleBG->addButton(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    hlay1->addWidget(logHistoButton);
    hlay1->setMargin(0);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(scaleBox);

    // -------------------------------------------------------------

    KVBox *histoBox   = new KVBox(gboxSettings);
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram "
                                          "drawing of the selected image channel. This one is "
                                          "re-computed at any filter settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());
    grid->addLayout(l1, 0, 0, 1, 5 );
    grid->addWidget(histoBox, 1, 0, 2, 5 );

    // -------------------------------------------------------------

    QGridLayout *grid2 = new QGridLayout();

    m_temperatureLabel    = new K3ActiveLabel(i18n("<qt><a href='http://en.wikipedia.org/wiki/Color_temperature'>Color Temperature</a> "
                                                  " (K): </qt>"), gboxSettings);
    m_adjTemperatureLabel = new QLabel(i18n("Adjustment:"), gboxSettings);
    m_temperatureInput    = new RDoubleNumInput(gboxSettings);
    m_temperatureInput->setDecimals(1);
    m_temperatureInput->input()->setRange(2000.0, 12000.0, 10.0);
    m_temperatureInput->setDefaultValue(6500.0);
    m_temperatureInput->setWhatsThis( i18n("<p>Set here the white balance color temperature in Kelvin."));

    m_temperaturePresetLabel = new QLabel(i18n("Preset:"), gboxSettings);
    m_temperaturePresetCB    = new RComboBox(gboxSettings);
    m_temperaturePresetCB->addItem( i18n("Candle") );
    m_temperaturePresetCB->addItem( i18n("40W Lamp") );
    m_temperaturePresetCB->addItem( i18n("100W Lamp") );
    m_temperaturePresetCB->addItem( i18n("200W Lamp") );
    m_temperaturePresetCB->addItem( i18n("Sunrise") );
    m_temperaturePresetCB->addItem( i18n("Studio Lamp") );
    m_temperaturePresetCB->addItem( i18n("Moonlight") );
    m_temperaturePresetCB->addItem( i18n("Neutral") );
    m_temperaturePresetCB->addItem( i18n("Daylight D50") );
    m_temperaturePresetCB->addItem( i18n("Photo Flash") );
    m_temperaturePresetCB->addItem( i18n("Sun") );
    m_temperaturePresetCB->addItem( i18n("Xenon Lamp") );
    m_temperaturePresetCB->addItem( i18n("Daylight D65") );
    m_temperaturePresetCB->addItem( i18n("None") );
    m_temperaturePresetCB->setDefaultIndex(DaylightD65);
    m_temperaturePresetCB->setWhatsThis( i18n("<p>Select the white balance color temperature "
                                              "preset to use here:<p>"
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
    m_pickTemperature = new QToolButton(gboxSettings);
    m_pickTemperature->setIcon(KIcon("color-picker-grey"));
    m_pickTemperature->setCheckable(true);
    m_pickTemperature->setToolTip( i18n( "Temperature tone color picker." ) );
    m_pickTemperature->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                          "image used to set white color balance temperature and "
                                          "green component."));

    KSeparator *line = new KSeparator(Qt::Horizontal, gboxSettings);

    // -------------------------------------------------------------

    m_blackLabel = new QLabel(i18n("Black point:"), gboxSettings);
    m_blackInput = new RDoubleNumInput(gboxSettings);
    m_blackInput->setDecimals(2);
    m_blackInput->input()->setRange(0.0, 0.05, 0.01, true);
    m_blackInput->setWhatsThis( i18n("<p>Set here the black level value."));
    m_blackInput->setDefaultValue(0.0);

    m_darkLabel = new QLabel(i18n("Shadows:"), gboxSettings);
    m_darkInput = new RDoubleNumInput(gboxSettings);
    m_darkInput->setDecimals(2);
    m_darkInput->input()->setRange(0.0, 1.0, 0.01, true);
    m_darkInput->setDefaultValue(0.5);
    m_darkInput->setWhatsThis( i18n("<p>Set here the shadows noise suppression level."));

    m_saturationLabel = new QLabel(i18n("Saturation:"), gboxSettings);
    m_saturationInput = new RDoubleNumInput(gboxSettings);
    m_saturationInput->setDecimals(2);
    m_saturationInput->input()->setRange(0.0, 2.0, 0.01, true);
    m_saturationInput->setDefaultValue(1.0);
    m_saturationInput->setWhatsThis( i18n("<p>Set here the saturation value."));

    m_gammaLabel = new QLabel(i18n("Gamma:"), gboxSettings);
    m_gammaInput = new RDoubleNumInput(gboxSettings);
    m_gammaInput->setDecimals(2);
    m_gammaInput->input()->setRange(0.1, 3.0, 0.01, true);
    m_gammaInput->setDefaultValue(1.0);
    m_gammaInput->setWhatsThis( i18n("<p>Set here the gamma correction value."));

    m_greenLabel = new QLabel(i18n("Green:"), gboxSettings);
    m_greenInput = new RDoubleNumInput(gboxSettings);
    m_greenInput->setDecimals(2);
    m_greenInput->input()->setRange(0.2, 2.5, 0.01, true);
    m_greenInput->setDefaultValue(1.0);
    m_greenInput->setWhatsThis( i18n("<p>Set here the green component to set magenta color "
                                     "cast removal level."));

    KSeparator *line2 = new KSeparator(Qt::Horizontal, gboxSettings);

    // -------------------------------------------------------------

    m_exposureLabel      = new K3ActiveLabel(i18n("<qt><a href='http://en.wikipedia.org/wiki/Exposure_value'>Exposure Compensation</a> "
                                                 " (E.V): </qt>"), gboxSettings);
    m_mainExposureLabel  = new QLabel(i18n("Main:"), gboxSettings);
    m_autoAdjustExposure = new QToolButton(gboxSettings);
    m_autoAdjustExposure->setIcon(KIconLoader::global()->loadIcon("system-run", KIconLoader::Toolbar));
    m_autoAdjustExposure->setToolTip( i18n( "Auto exposure adjustments" ) );
    m_autoAdjustExposure->setWhatsThis( i18n("<p>With this button, you can automatically adjust Exposure "
                                             "and Black Point values."));
    m_mainExposureInput = new RDoubleNumInput(gboxSettings);
    m_mainExposureInput->setDecimals(2);
    m_mainExposureInput->input()->setRange(-6.0, 8.0, 0.1, true);
    m_mainExposureInput->setDefaultValue(0.0);
    m_mainExposureInput->setWhatsThis( i18n("<p>Set here the main exposure compensation value in E.V."));

    m_fineExposureLabel = new QLabel(i18n("Fine:"), gboxSettings);
    m_fineExposureInput = new RDoubleNumInput(gboxSettings);
    m_fineExposureInput->setDecimals(2);
    m_fineExposureInput->input()->setRange(-0.5, 0.5, 0.01, true);
    m_fineExposureInput->setDefaultValue(0.0);
    m_fineExposureInput->setWhatsThis( i18n("<p>This value in E.V will be added to main exposure "
                                            "compensation value to set fine exposure adjustment."));

    // -------------------------------------------------------------

    layout2->setMargin(0);
    layout2->setSpacing(spacingHint());
    layout2->addLayout(grid);
    layout2->addLayout(grid2);

    grid2->setMargin(spacingHint());
    grid2->setSpacing(spacingHint());
    grid2->addWidget(m_temperatureLabel, 0, 0, 1, 5+1);
    grid2->addWidget(m_adjTemperatureLabel, 1, 0, 1, 1);
    grid2->addWidget(m_pickTemperature, 1, 1, 1, 1);
    grid2->addWidget(m_temperatureInput, 1, 2, 1, 5-2+1);
    grid2->addWidget(m_temperaturePresetLabel, 2, 0, 1, 1);
    grid2->addWidget(m_temperaturePresetCB, 2, 2, 1, 5-2+1);
    grid2->addWidget(line, 3, 0, 1, 5+1);
    grid2->addWidget(m_blackLabel, 4, 0, 1, 1);
    grid2->addWidget(m_blackInput, 4, 1, 1, 5);
    grid2->addWidget(m_darkLabel, 5, 0, 1, 1);
    grid2->addWidget(m_darkInput, 5, 1, 1, 5);
    grid2->addWidget(m_saturationLabel, 6, 0, 1, 1);
    grid2->addWidget(m_saturationInput, 6, 1, 1, 5);
    grid2->addWidget(m_gammaLabel, 7, 0, 1, 1);
    grid2->addWidget(m_gammaInput, 7, 1, 1, 5);
    grid2->addWidget(m_greenLabel, 8, 0, 1, 1);
    grid2->addWidget(m_greenInput, 8, 1, 1, 5);
    grid2->addWidget(line2, 9, 0, 1, 5+1);
    grid2->addWidget(m_exposureLabel, 10, 0, 1, 5+1);
    grid2->addWidget(m_mainExposureLabel, 11, 0, 1, 1);
    grid2->addWidget(m_autoAdjustExposure, 11, 1, 1, 1);
    grid2->addWidget(m_mainExposureInput, 11, 2, 1, 5- 2+1);
    grid2->addWidget(m_fineExposureLabel, 12, 0, 1, 2 );
    grid2->addWidget(m_fineExposureInput, 12, 2, 1, 5- 2+1);
    grid2->setRowStretch(13, 10);

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
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
          m_temperaturePresetCB->setCurrentIndex(Candle);
          break;

       case 2680:
          m_temperaturePresetCB->setCurrentIndex(Lamp40W);
          break;

       case 2800:
          m_temperaturePresetCB->setCurrentIndex(Lamp100W);
          break;

       case 3000:
          m_temperaturePresetCB->setCurrentIndex(Lamp200W);
          break;

       case 3200:
          m_temperaturePresetCB->setCurrentIndex(Sunrise);
          break;

       case 3400:
          m_temperaturePresetCB->setCurrentIndex(StudioLamp);
          break;

       case 4100:
          m_temperaturePresetCB->setCurrentIndex(MoonLight);
          break;

       case 4750:
          m_temperaturePresetCB->setCurrentIndex(Neutral);
          break;

       case 5000:
          m_temperaturePresetCB->setCurrentIndex(DaylightD50);
          break;

       case 5500:
          m_temperaturePresetCB->setCurrentIndex(Flash);
          break;

       case 5770:
          m_temperaturePresetCB->setCurrentIndex(Sun);
          break;

       case 6420:
          m_temperaturePresetCB->setCurrentIndex(XeonLamp);
          break;

       case 6500:
          m_temperaturePresetCB->setCurrentIndex(DaylightD65);
          break;

       default:
          m_temperaturePresetCB->setCurrentIndex(None);
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
    if ( m_pickTemperature->isChecked() )
    {
        Digikam::DColor dc = color;
        QColor tc          = dc.getQColor();
        double temperatureLevel, greenLevel;

        Digikam::WhiteBalance::autoWBAdjustementFromColor(tc, temperatureLevel, greenLevel);

        m_temperatureInput->setValue(temperatureLevel);
        m_greenInput->setValue(greenLevel);
        m_pickTemperature->setChecked(false);
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
    m_histogramWidget->repaint();
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

    m_histogramWidget->repaint();
}

void ImageEffect_WhiteBalance::slotAutoAdjustExposure()
{
    parentWidget()->setCursor( Qt::WaitCursor );

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

    double temperature  = m_temperatureInput->value();
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
    kapp->setOverrideCursor( Qt::WaitCursor );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    double temperature  = m_temperatureInput->value();
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
    m_blackInput->blockSignals(true);
    m_darkInput->blockSignals(true);
    m_fineExposureInput->blockSignals(true);
    m_gammaInput->blockSignals(true);
    m_greenInput->blockSignals(true);
    m_mainExposureInput->blockSignals(true);
    m_saturationInput->blockSignals(true);
    m_temperatureInput->blockSignals(true);
    m_temperaturePresetCB->blockSignals(true);

    // Neutral color temperature settings is D65.
    m_blackInput->slotReset();
    m_darkInput->slotReset();
    m_fineExposureInput->slotReset();
    m_gammaInput->slotReset();
    m_greenInput->slotReset();
    m_mainExposureInput->slotReset();
    m_saturationInput->slotReset();
    m_temperaturePresetCB->slotReset();
    slotTemperaturePresetChanged(m_temperaturePresetCB->defaultIndex());
    m_temperatureInput->slotReset();

    m_previewWidget->resetSpotPosition();
    m_channelCB->setCurrentIndex(LuminosityChannel);
    slotChannelChanged(LuminosityChannel);

    m_histogramWidget->reset();

    m_blackInput->blockSignals(false);
    m_darkInput->blockSignals(false);
    m_fineExposureInput->blockSignals(false);
    m_gammaInput->blockSignals(false);
    m_greenInput->blockSignals(false);
    m_mainExposureInput->blockSignals(false);
    m_saturationInput->blockSignals(false);
    m_temperatureInput->blockSignals(false);
    m_temperaturePresetCB->blockSignals(false);

    slotEffect();
}

void ImageEffect_WhiteBalance::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("whitebalance Tool Dialog");
    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    m_blackInput->setValue(group.readEntry("Black", m_blackInput->defaultValue()));
    m_mainExposureInput->setValue(group.readEntry("MainExposure", m_mainExposureInput->defaultValue()));
    m_fineExposureInput->setValue(group.readEntry("FineExposure", m_fineExposureInput->defaultValue()));
    m_gammaInput->setValue(group.readEntry("Gamma", m_gammaInput->defaultValue()));
    m_saturationInput->setValue(group.readEntry("Saturation", m_saturationInput->defaultValue()));
    m_greenInput->setValue(group.readEntry("Green", m_greenInput->defaultValue()));
    m_temperatureInput->setValue(group.readEntry("Temperature", m_temperatureInput->defaultValue()));

    slotTemperatureChanged(m_temperatureInput->value());
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ImageEffect_WhiteBalance::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("whitebalance Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());

    group.writeEntry("Dark", m_darkInput->value());
    group.writeEntry("Black", m_blackInput->value());
    group.writeEntry("MainExposure", m_mainExposureInput->value());
    group.writeEntry("FineExposure", m_fineExposureInput->value());
    group.writeEntry("Gamma", m_gammaInput->value());
    group.writeEntry("Saturation", m_saturationInput->value());
    group.writeEntry("Green", m_greenInput->value());
    group.writeEntry("Temperature", m_temperatureInput->value());
    config->sync();
}

// Load all settings.
void ImageEffect_WhiteBalance::slotUser3()
{
    KUrl loadWhiteBalanceFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("White Color Balance Settings File to Load")) );
    if( loadWhiteBalanceFile.isEmpty() )
       return;

    QFile file(loadWhiteBalanceFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );

        if ( stream.readLine() != "# White Color Balance Configuration File V2" )
        {
           KMessageBox::error(this,
                        i18n("\"%1\" is not a White Color Balance settings text file.",
                             loadWhiteBalanceFile.fileName()));
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
    KUrl saveWhiteBalanceFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("White Color Balance Settings File to Save")) );
    if( saveWhiteBalanceFile.isEmpty() )
       return;

    QFile file(saveWhiteBalanceFile.path());

    if ( file.open(QIODevice::WriteOnly) )
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
