/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor plugin to correct
 *               image white balance
 *
 * Copyright (C) 2008-2009 by Guillaume Castagnino <casta at xwing dot info>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "whitebalancetool.h"
#include "whitebalancetool.moc"

// Qt includes

#include <QButtonGroup>
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
#include <QVBoxLayout>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
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

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "dcolor.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"
#include "whitebalance.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamWhiteBalanceImagesPlugin
{

enum TemperaturePreset
{
    Candle=0,
    Lamp40W,
    Lamp100W,
    Lamp200W,
    Sunrise,
    StudioLamp,
    MoonLight,
    Neutral,
    DaylightD50,
    Flash,
    Sun,
    XeonLamp,
    DaylightD65,
    None
};

class WhiteBalanceToolPriv
{
public:

    WhiteBalanceToolPriv() :
        configGroupName("whitebalance Tool"),
        configDarkInputEntry("Dark"),
        configBlackInputEntry("Black"),
        configMainExposureEntry("MainExposure"),
        configFineExposureEntry("FineExposure"),
        configGammaInputEntry("Gamma"),
        configSaturationInputEntry("Saturation"),
        configGreenInputEntry("Green"),
        configTemeratureInputEntry("Temperature"),
        configHistogramChannelEntry("Histogram Chanel"),
        configHistogramScaleEntry("Histogram Scale"),

        destinationPreviewData(0),
        currentPreviewMode(0),
        pickTemperature(0),
        autoAdjustExposure(0),
        adjTemperatureLabel(0),
        temperaturePresetLabel(0),
        darkLabel(0), blackLabel(0),
        mainExposureLabel(0),
        fineExposureLabel(0),
        gammaLabel(0),
        saturationLabel(0),
        greenLabel(0),
        exposureLabel(0),
        temperatureLabel(0),
        temperaturePresetCB(0),
        temperatureInput(0),
        darkInput(0),
        blackInput(0),
        mainExposureInput(0),
        fineExposureInput(0),
        gammaInput(0),
        saturationInput(0),
        greenInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configDarkInputEntry;
    const QString       configBlackInputEntry;
    const QString       configMainExposureEntry;
    const QString       configFineExposureEntry;
    const QString       configGammaInputEntry;
    const QString       configSaturationInputEntry;
    const QString       configGreenInputEntry;
    const QString       configTemeratureInputEntry;
    const QString       configHistogramChannelEntry;
    const QString       configHistogramScaleEntry;

    uchar*              destinationPreviewData;

    int                 currentPreviewMode;

    QToolButton*        pickTemperature;
    QToolButton*        autoAdjustExposure;

    QLabel*             adjTemperatureLabel;
    QLabel*             temperaturePresetLabel;
    QLabel*             darkLabel;
    QLabel*             blackLabel;
    QLabel*             mainExposureLabel;
    QLabel*             fineExposureLabel;
    QLabel*             gammaLabel;
    QLabel*             saturationLabel;
    QLabel*             greenLabel;
    QLabel*             exposureLabel;
    QLabel*             temperatureLabel;

    RComboBox*          temperaturePresetCB;

    RDoubleNumInput*    temperatureInput;
    RDoubleNumInput*    darkInput;
    RDoubleNumInput*    blackInput;
    RDoubleNumInput*    mainExposureInput;
    RDoubleNumInput*    fineExposureInput;
    RDoubleNumInput*    gammaInput;
    RDoubleNumInput*    saturationInput;
    RDoubleNumInput*    greenInput;

    ImageWidget*        previewWidget;

    EditorToolSettings* gboxSettings;
};

WhiteBalanceTool::WhiteBalanceTool(QObject* parent)
                : EditorTool(parent), d(new WhiteBalanceToolPriv)
{
    setObjectName("whitebalance");
    setToolName(i18n("White Balance"));
    setToolIcon(SmallIcon("whitebalance"));

    d->destinationPreviewData = 0;

    // -------------------------------------------------------------

    d->previewWidget = new ImageWidget("whitebalance Tool", 0,
                                      i18n("The image's white-balance adjustments preview "
                                           "is shown here.  Pick a color on the image to "
                                           "see the corresponding color level on the histogram."));
    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->gboxSettings->setTools(EditorToolSettings::Histogram);

    // -------------------------------------------------------------


    d->temperatureLabel = new QLabel(i18n("<a href='http://en.wikipedia.org/wiki/Color_temperature'>"
                                          "Color Temperature</a> (K): "));
    d->temperatureLabel->setOpenExternalLinks(true);

    d->adjTemperatureLabel = new QLabel(i18n("Adjustment:"));
    d->temperatureInput    = new RDoubleNumInput;
    d->temperatureInput->setDecimals(1);
    d->temperatureInput->input()->setRange(1750.0, 12000.0, 10.0);
    d->temperatureInput->setDefaultValue(6500.0);
    d->temperatureInput->setWhatsThis( i18n("Set here the white balance color temperature in Kelvin."));

    d->temperaturePresetLabel = new QLabel(i18n("Preset:"));
    d->temperaturePresetCB    = new RComboBox;
    d->temperaturePresetCB->addItem(i18n("Candle"));
    d->temperaturePresetCB->addItem(i18n("40W Lamp"));
    d->temperaturePresetCB->addItem(i18n("100W Lamp"));
    d->temperaturePresetCB->addItem(i18n("200W Lamp"));
    d->temperaturePresetCB->addItem(i18n("Sunrise"));
    d->temperaturePresetCB->addItem(i18n("Studio Lamp"));
    d->temperaturePresetCB->addItem(i18n("Moonlight"));
    d->temperaturePresetCB->addItem(i18n("Neutral"));
    d->temperaturePresetCB->addItem(i18n("Daylight D50"));
    d->temperaturePresetCB->addItem(i18n("Photo Flash"));
    d->temperaturePresetCB->addItem(i18n("Sun"));
    d->temperaturePresetCB->addItem(i18n("Xenon Lamp"));
    d->temperaturePresetCB->addItem(i18n("Daylight D65"));
    d->temperaturePresetCB->addItem(i18nc("no temperature preset", "None"));
    d->temperaturePresetCB->setDefaultIndex(DaylightD65);
    d->temperaturePresetCB->setWhatsThis(i18n("<p>Select the white balance color temperature "
                                              "preset to use here:</p>"
                                              "<p><b>Candle</b>: candle light (1850K).</p>"
                                              "<p><b>40W Lamp</b>: 40 Watt incandescent lamp (2680K).</p>"
                                              "<p><b>100W Lamp</b>: 100 Watt incandescent lamp (2800K).</p>"
                                              "<p><b>200W Lamp</b>: 200 Watt incandescent lamp (3000K).</p>"
                                              "<p><b>Sunrise</b>: sunrise or sunset light (3200K).</p>"
                                              "<p><b>Studio Lamp</b>: tungsten lamp used in photo studio or "
                                              "light at 1 hour from dusk/dawn (3400K).</p>"
                                              "<p><b>Moonlight</b>: moon light (4100K).</p>"
                                              "<p><b>Neutral</b>: neutral color temperature (4750K).</p>"
                                              "<p><b>Daylight D50</b>: sunny daylight around noon (5000K).</p>"
                                              "<p><b>Photo Flash</b>: electronic photo flash (5500K).</p>"
                                              "<p><b>Sun</b>: effective sun temperature (5770K).</p>"
                                              "<p><b>Xenon Lamp</b>: xenon lamp or light arc (6420K).</p>"
                                              "<p><b>Daylight D65</b>: overcast sky light (6500K).</p>"
                                              "<p><b>None</b>: no preset value.</p>"));
    d->pickTemperature = new QToolButton;
    d->pickTemperature->setIcon(KIcon("color-picker-grey"));
    d->pickTemperature->setCheckable(true);
    d->pickTemperature->setToolTip( i18n( "Temperature tone color picker." ) );
    d->pickTemperature->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                          "image used to set the white color balance temperature and "
                                          "green component."));

    KSeparator *line = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------

    d->blackLabel = new QLabel(i18n("Black point:"));
    d->blackInput = new RDoubleNumInput;
    d->blackInput->setDecimals(2);
    d->blackInput->input()->setRange(0.0, 0.05, 0.01, true);
    d->blackInput->setWhatsThis( i18n("Set here the black level value."));
    d->blackInput->setDefaultValue(0.0);

    d->darkLabel = new QLabel(i18n("Shadows:"));
    d->darkInput = new RDoubleNumInput;
    d->darkInput->setDecimals(2);
    d->darkInput->input()->setRange(0.0, 1.0, 0.01, true);
    d->darkInput->setDefaultValue(0.5);
    d->darkInput->setWhatsThis( i18n("Set here the shadow noise suppression level."));

    d->saturationLabel = new QLabel(i18n("Saturation:"));
    d->saturationInput = new RDoubleNumInput;
    d->saturationInput->setDecimals(2);
    d->saturationInput->input()->setRange(0.0, 2.0, 0.01, true);
    d->saturationInput->setDefaultValue(1.0);
    d->saturationInput->setWhatsThis( i18n("Set here the saturation value."));

    d->gammaLabel = new QLabel(i18n("Gamma:"));
    d->gammaInput = new RDoubleNumInput;
    d->gammaInput->setDecimals(2);
    d->gammaInput->input()->setRange(0.1, 3.0, 0.01, true);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setWhatsThis( i18n("Set here the gamma correction value."));

    d->greenLabel = new QLabel(i18n("Green:"));
    d->greenInput = new RDoubleNumInput;
    d->greenInput->setDecimals(2);
    d->greenInput->input()->setRange(0.2, 2.5, 0.01, true);
    d->greenInput->setDefaultValue(1.0);
    d->greenInput->setWhatsThis(i18n("Set here the green component to control the magenta color "
                                     "cast removal level."));

    KSeparator *line2 = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------

    d->exposureLabel = new QLabel(i18n("<a href='http://en.wikipedia.org/wiki/Exposure_value'>"
                                       "Exposure Compensation</a> (E.V): "));
    d->exposureLabel->setOpenExternalLinks(true);

    d->mainExposureLabel  = new QLabel(i18nc("main exposure value", "Main:"));
    d->autoAdjustExposure = new QToolButton;
    d->autoAdjustExposure->setIcon(KIconLoader::global()->loadIcon("system-run", KIconLoader::Toolbar));
    d->autoAdjustExposure->setToolTip( i18n( "Auto exposure adjustments" ) );
    d->autoAdjustExposure->setWhatsThis(i18n("With this button, you can automatically adjust Exposure "
                                             "and Black Point values."));
    d->mainExposureInput = new RDoubleNumInput;
    d->mainExposureInput->setDecimals(2);
    d->mainExposureInput->input()->setRange(-6.0, 8.0, 0.1, true);
    d->mainExposureInput->setDefaultValue(0.0);
    d->mainExposureInput->setWhatsThis( i18n("Set here the main exposure compensation value in E.V."));

    d->fineExposureLabel = new QLabel(i18nc("fine exposure adjustment", "Fine:"));
    d->fineExposureInput = new RDoubleNumInput;
    d->fineExposureInput->setDecimals(2);
    d->fineExposureInput->input()->setRange(-0.5, 0.5, 0.01, true);
    d->fineExposureInput->setDefaultValue(0.0);
    d->fineExposureInput->setWhatsThis(i18n("This value in E.V will be added to main exposure "
                                            "compensation value to set fine exposure adjustment."));

    // -------------------------------------------------------------

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(d->temperatureLabel,        0, 0, 1, 6);
    mainLayout->addWidget(d->adjTemperatureLabel,     1, 0, 1, 1);
    mainLayout->addWidget(d->pickTemperature,         1, 1, 1, 1);
    mainLayout->addWidget(d->temperatureInput,        1, 2, 1, 4);
    mainLayout->addWidget(d->temperaturePresetLabel,  2, 0, 1, 1);
    mainLayout->addWidget(d->temperaturePresetCB,     2, 2, 1, 4);
    mainLayout->addWidget(line,                       3, 0, 1, 6);
    mainLayout->addWidget(d->blackLabel,              4, 0, 1, 1);
    mainLayout->addWidget(d->blackInput,              4, 1, 1, 5);
    mainLayout->addWidget(d->darkLabel,               5, 0, 1, 1);
    mainLayout->addWidget(d->darkInput,               5, 1, 1, 5);
    mainLayout->addWidget(d->saturationLabel,         6, 0, 1, 1);
    mainLayout->addWidget(d->saturationInput,         6, 1, 1, 5);
    mainLayout->addWidget(d->gammaLabel,              7, 0, 1, 1);
    mainLayout->addWidget(d->gammaInput,              7, 1, 1, 5);
    mainLayout->addWidget(d->greenLabel,              8, 0, 1, 1);
    mainLayout->addWidget(d->greenInput,              8, 1, 1, 5);
    mainLayout->addWidget(line2,                      9, 0, 1, 6);
    mainLayout->addWidget(d->exposureLabel,          10, 0, 1, 6);
    mainLayout->addWidget(d->mainExposureLabel,      11, 0, 1, 1);
    mainLayout->addWidget(d->autoAdjustExposure,     11, 1, 1, 1);
    mainLayout->addWidget(d->mainExposureInput,      11, 2, 1, 4);
    mainLayout->addWidget(d->fineExposureLabel,      12, 0, 1, 2);
    mainLayout->addWidget(d->fineExposureInput,      12, 2, 1, 4);
    mainLayout->setRowStretch(13, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromOriginal( const Digikam::DColor & )));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // Correction Filter Slider controls.

    connect(d->temperaturePresetCB, SIGNAL(activated(int)),
            this, SLOT(slotTemperaturePresetChanged(int)));

    connect(d->temperatureInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTemperatureChanged(double)));

    connect(d->darkInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->blackInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->mainExposureInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->fineExposureInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->gammaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->saturationInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->greenInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(d->autoAdjustExposure, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustExposure()));

    connect(d->pickTemperature, SIGNAL(released()),
            this, SLOT(slotPickerColorButtonActived()));
}

WhiteBalanceTool::~WhiteBalanceTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void WhiteBalanceTool::slotTemperatureChanged(double temperature)
{
   switch ((uint) temperature)
    {
        case 1850:
            d->temperaturePresetCB->setCurrentIndex(Candle);
            break;

        case 2680:
            d->temperaturePresetCB->setCurrentIndex(Lamp40W);
            break;

        case 2800:
            d->temperaturePresetCB->setCurrentIndex(Lamp100W);
            break;

        case 3000:
            d->temperaturePresetCB->setCurrentIndex(Lamp200W);
            break;

        case 3200:
            d->temperaturePresetCB->setCurrentIndex(Sunrise);
            break;

        case 3400:
            d->temperaturePresetCB->setCurrentIndex(StudioLamp);
            break;

        case 4100:
            d->temperaturePresetCB->setCurrentIndex(MoonLight);
            break;

        case 4750:
            d->temperaturePresetCB->setCurrentIndex(Neutral);
            break;

        case 5000:
            d->temperaturePresetCB->setCurrentIndex(DaylightD50);
            break;

        case 5500:
            d->temperaturePresetCB->setCurrentIndex(Flash);
            break;

        case 5770:
            d->temperaturePresetCB->setCurrentIndex(Sun);
            break;

        case 6420:
            d->temperaturePresetCB->setCurrentIndex(XeonLamp);
            break;

        case 6500:
            d->temperaturePresetCB->setCurrentIndex(DaylightD65);
            break;

        default:
            d->temperaturePresetCB->setCurrentIndex(None);
            break;
    }

    slotTimer();
}

void WhiteBalanceTool::slotTemperaturePresetChanged(int tempPreset)
{
   switch (tempPreset)
    {
        case Candle:
            d->temperatureInput->setValue(1850.0);
            break;

        case Lamp40W:
            d->temperatureInput->setValue(2680.0);
            break;

        case Lamp100W:
            d->temperatureInput->setValue(2800.0);
            break;

        case Lamp200W:
            d->temperatureInput->setValue(3000.0);
            break;

        case Sunrise:
            d->temperatureInput->setValue(3200.0);
            break;

        case StudioLamp:
            d->temperatureInput->setValue(3400.0);
            break;

        case MoonLight:
            d->temperatureInput->setValue(4100.0);
            break;

        case Neutral:
            d->temperatureInput->setValue(4750.0);
            break;

        case DaylightD50:
            d->temperatureInput->setValue(5000.0);
            break;

        case Flash:
            d->temperatureInput->setValue(5500.0);
            break;

        case Sun:
            d->temperatureInput->setValue(5770.0);
            break;

        case XeonLamp:
            d->temperatureInput->setValue(6420.0);
            break;

        case DaylightD65:
            d->temperatureInput->setValue(6500.0);
            break;

        default: // None.
            break;
    }

    slotEffect();
}

void WhiteBalanceTool::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    d->currentPreviewMode = d->previewWidget->getRenderingPreviewMode();
    d->previewWidget->setRenderingPreviewMode(ImageGuideWidget::PreviewOriginalImage);
}

void WhiteBalanceTool::slotColorSelectedFromOriginal(const DColor& color)
{
    if ( d->pickTemperature->isChecked() )
    {
        DColor dc = color;
        QColor tc          = dc.getQColor();
        double temperatureLevel, greenLevel;

        WhiteBalance::autoWBAdjustementFromColor(tc, temperatureLevel, greenLevel);

        d->temperatureInput->setValue(temperatureLevel);
        d->greenInput->setValue(greenLevel);
        d->pickTemperature->setChecked(false);
    }
    else
        return;

    // restore previous rendering mode.
    d->previewWidget->setRenderingPreviewMode(d->currentPreviewMode);

    slotEffect();
}

void WhiteBalanceTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void WhiteBalanceTool::slotAutoAdjustExposure()
{
    kapp->activeWindow()->setCursor( Qt::WaitCursor );

    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int width                  = iface->originalWidth();
    int height                 = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    double blackLevel;
    double exposureLevel;

    WhiteBalance::autoExposureAdjustement(data, width, height, sb, blackLevel, exposureLevel);
    delete [] data;

    d->blackInput->setValue(blackLevel);
    d->mainExposureInput->setValue(exposureLevel);
    d->fineExposureInput->setValue(0.0);

    kapp->activeWindow()->unsetCursor();
    slotEffect();
}

void WhiteBalanceTool::slotEffect()
{
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data                = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    d->destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];

    double temperature  = d->temperatureInput->value();
    double dark         = d->darkInput->value();
    double black        = d->blackInput->value();
    double mainExposure = d->mainExposureInput->value();
    double fineExposure = d->fineExposureInput->value();
    double gamma        = d->gammaInput->value();
    double saturation   = d->saturationInput->value();
    double green        = d->greenInput->value();

    WhiteBalance wbFilter(sb);
    wbFilter.whiteBalance(data, w, h, sb,
                          black, mainExposure + fineExposure,
                          temperature, green, dark,
                          gamma, saturation);

    iface->putPreviewImage(data);
    d->previewWidget->updatePreview();

    // Update histogram.
    memcpy (d->destinationPreviewData, data, w*h*(sb ? 8 : 4));
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] data;
}

void WhiteBalanceTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data       = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    double temperature  = d->temperatureInput->value();
    double dark         = d->darkInput->value();
    double black        = d->blackInput->value();
    double mainExposure = d->mainExposureInput->value();
    double fineExposure = d->fineExposureInput->value();
    double gamma        = d->gammaInput->value();
    double saturation   = d->saturationInput->value();
    double green        = d->greenInput->value();

    WhiteBalance wbFilter(sb);
    wbFilter.whiteBalance(data, w, h, sb,
                          black, mainExposure + fineExposure,
                          temperature, green, dark,
                          gamma, saturation);

    iface->putOriginalImage(i18n("White Balance"), data);
    delete [] data;
    kapp->restoreOverrideCursor();
}

void WhiteBalanceTool::slotResetSettings()
{
    blockWidgetSignals(true);

    // Neutral color temperature settings is D65.
    d->blackInput->slotReset();
    d->darkInput->slotReset();
    d->fineExposureInput->slotReset();
    d->gammaInput->slotReset();
    d->greenInput->slotReset();
    d->mainExposureInput->slotReset();
    d->saturationInput->slotReset();
    d->temperaturePresetCB->slotReset();
    slotTemperaturePresetChanged(d->temperaturePresetCB->defaultIndex());
    d->temperatureInput->slotReset();

    d->previewWidget->resetSpotPosition();
    d->gboxSettings->histogramBox()->setChannel(LuminosityChannel);
    d->gboxSettings->histogramBox()->histogram()->reset();

    blockWidgetSignals(false);

    slotEffect();
}

void WhiteBalanceTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel(group.readEntry(d->configHistogramChannelEntry,
                        (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry(d->configHistogramScaleEntry,
                        (int)LogScaleHistogram));

    d->blackInput->setValue(group.readEntry(d->configDarkInputEntry,             d->darkInput->defaultValue()));
    d->blackInput->setValue(group.readEntry(d->configBlackInputEntry,            d->blackInput->defaultValue()));
    d->mainExposureInput->setValue(group.readEntry(d->configMainExposureEntry,   d->mainExposureInput->defaultValue()));
    d->fineExposureInput->setValue(group.readEntry(d->configFineExposureEntry,   d->fineExposureInput->defaultValue()));
    d->gammaInput->setValue(group.readEntry(d->configGammaInputEntry,            d->gammaInput->defaultValue()));
    d->saturationInput->setValue(group.readEntry(d->configSaturationInputEntry,  d->saturationInput->defaultValue()));
    d->greenInput->setValue(group.readEntry(d->configGreenInputEntry,            d->greenInput->defaultValue()));
    d->temperatureInput->setValue(group.readEntry(d->configTemeratureInputEntry, d->temperatureInput->defaultValue()));

    slotTemperatureChanged(d->temperatureInput->value());
}

void WhiteBalanceTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->configGroupName);
    group.writeEntry(d->configHistogramChannelEntry, d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   d->gboxSettings->histogramBox()->scale());

    group.writeEntry(d->configDarkInputEntry,       d->darkInput->value());
    group.writeEntry(d->configBlackInputEntry,      d->blackInput->value());
    group.writeEntry(d->configMainExposureEntry,    d->mainExposureInput->value());
    group.writeEntry(d->configFineExposureEntry,    d->fineExposureInput->value());
    group.writeEntry(d->configGammaInputEntry,      d->gammaInput->value());
    group.writeEntry(d->configSaturationInputEntry, d->saturationInput->value());
    group.writeEntry(d->configGreenInputEntry,      d->greenInput->value());
    group.writeEntry(d->configTemeratureInputEntry, d->temperatureInput->value());
    d->previewWidget->writeSettings();
    config->sync();
}

// Load all settings.
void WhiteBalanceTool::slotLoadSettings()
{
    KUrl loadWhiteBalanceFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString(i18n("White Color Balance Settings File to Load")));
    if (loadWhiteBalanceFile.isEmpty())
        return;

    QFile file(loadWhiteBalanceFile.toLocalFile());

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        if (stream.readLine() != "# White Color Balance Configuration File V2")
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a White Color Balance settings text file.",
                               loadWhiteBalanceFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
        d->temperatureInput->setValue(stream.readLine().toDouble());
        d->darkInput->setValue(stream.readLine().toDouble());
        d->blackInput->setValue(stream.readLine().toDouble());
        d->mainExposureInput->setValue(stream.readLine().toDouble());
        d->fineExposureInput->setValue(stream.readLine().toDouble());
        d->gammaInput->setValue(stream.readLine().toDouble());
        d->saturationInput->setValue(stream.readLine().toDouble());
        d->greenInput->setValue(stream.readLine().toDouble());
        d->gboxSettings->histogramBox()->histogram()->reset();
        blockSignals(false);
        slotEffect();
    }
    else
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the White Color Balance text file."));

    file.close();
}

// Save all settings.
void WhiteBalanceTool::slotSaveAsSettings()
{
    KUrl saveWhiteBalanceFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("White Color Balance Settings File to Save")));
    if ( saveWhiteBalanceFile.isEmpty() )
       return;

    QFile file(saveWhiteBalanceFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# White Color Balance Configuration File V2\n";
        stream << d->temperatureInput->value() << "\n";
        stream << d->darkInput->value() << "\n";
        stream << d->blackInput->value() << "\n";
        stream << d->mainExposureInput->value() << "\n";
        stream << d->fineExposureInput->value() << "\n";
        stream << d->gammaInput->value() << "\n";
        stream << d->saturationInput->value() << "\n";
        stream << d->greenInput->value() << "\n";
    }
    else
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the White Color Balance text file."));

    file.close();
}

void WhiteBalanceTool::blockWidgetSignals(bool b)
{
    d->blackInput->blockSignals(b);
    d->darkInput->blockSignals(b);
    d->fineExposureInput->blockSignals(b);
    d->gammaInput->blockSignals(b);
    d->greenInput->blockSignals(b);
    d->mainExposureInput->blockSignals(b);
    d->saturationInput->blockSignals(b);
    d->temperatureInput->blockSignals(b);
    d->temperaturePresetCB->blockSignals(b);
}

}  // namespace DigikamWhiteBalanceImagesPlugin
