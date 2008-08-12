/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-11
 * Description : Raw import settings box
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes.

#include <qstring.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qhbuttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>

// KDE includes.

#include <kapplication.h>
#include <kdialog.h>
#include <knuminput.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/dcrawsettingswidget.h>

// Local includes.

#include "ddebug.h"
#include "imagedialog.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "curveswidget.h"
#include "colorgradientwidget.h"
#include "rawsettingsbox.h"
#include "rawsettingsbox.moc"

namespace Digikam
{

class RawSettingsBoxPriv
{
public:

    enum ColorChannel
    {
        LuminosityChannel=0,
        RedChannel,
        GreenChannel,
        BlueChannel,
        ColorChannels
    };

    enum AllColorsColorType
    {
        AllColorsRed=0,
        AllColorsGreen,
        AllColorsBlue
    };

public:

    RawSettingsBoxPriv()
    {
        channelCB           = 0;
        colorsCB            = 0;
        scaleBG             = 0;
        hGradient           = 0;
        histogramWidget     = 0;
        infoBox             = 0;
        advExposureBox      = 0;
        gammaLabel          = 0;
        gammaSpinBox        = 0;
        saturationLabel     = 0;
        saturationInput     = 0;
        fineExposureLabel   = 0;
        fineExposureInput   = 0;
        contrastSpinBox     = 0;
        contrastLabel       = 0;
        curveBox            = 0;
        curveWidget         = 0;
        curves              = 0;
        resetCurveBtn       = 0;
        decodingSettingsBox = 0;
    }

    QWidget                          *advExposureBox;
    QWidget                          *curveBox;

    QComboBox                        *channelCB;
    QComboBox                        *colorsCB;

    QLabel                           *contrastLabel;
    QLabel                           *gammaLabel;
    QLabel                           *saturationLabel;
    QLabel                           *fineExposureLabel;

    QHButtonGroup                    *scaleBG;

    QToolButton                      *resetCurveBtn;

    KIntNumInput                     *contrastSpinBox;

    KDoubleNumInput                  *gammaSpinBox;
    KDoubleNumInput                  *saturationInput;
    KDoubleNumInput                  *fineExposureInput;

    ColorGradientWidget              *hGradient;

    ImageCurves                      *curves;
    CurvesWidget                     *curveWidget;

    HistogramWidget                  *histogramWidget;

    ImageDialogPreview               *infoBox;

    DImg                              image;

    KDcrawIface::DcrawSettingsWidget *decodingSettingsBox;
};

RawSettingsBox::RawSettingsBox(const KURL& url, QWidget *parent)
              : QWidget(parent)
{
    d = new RawSettingsBoxPriv;
    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(this, 5, 4);

    QLabel *label1 = new QLabel(i18n("Channel:"), this);
    label1->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->channelCB   = new QComboBox( false, this );
    d->channelCB->insertItem( i18n("Luminosity") );
    d->channelCB->insertItem( i18n("Red") );
    d->channelCB->insertItem( i18n("Green") );
    d->channelCB->insertItem( i18n("Blue") );
    d->channelCB->insertItem( i18n("Colors") );
    QWhatsThis::add(d->channelCB, i18n("<p>Select the histogram channel to display here:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"
                                       "<b>Colors</b>: Display all color channel values at the same time."));

    d->scaleBG = new QHButtonGroup(this);
    d->scaleBG->setExclusive(true);
    d->scaleBG->setFrameShape(QFrame::NoFrame);
    d->scaleBG->setInsideMargin( 0 );
    QWhatsThis::add(d->scaleBG, i18n("<p>Select the histogram scale here.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    d->scaleBG->insert(linHistoButton, HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    d->scaleBG->insert(logHistoButton, HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);

    QLabel *label10 = new QLabel(i18n("Colors:"), this);
    label10->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->colorsCB = new QComboBox( false, this );
    d->colorsCB->insertItem( i18n("Red") );
    d->colorsCB->insertItem( i18n("Green") );
    d->colorsCB->insertItem( i18n("Blue") );
    d->colorsCB->setEnabled( false );
    QWhatsThis::add( d->colorsCB, i18n("<p>Select the main color displayed with Colors Channel mode here:<p>"
                                       "<b>Red</b>: Draw the red image channel in the foreground.<p>"
                                       "<b>Green</b>: Draw the green image channel in the foreground.<p>"
                                       "<b>Blue</b>: Draw the blue image channel in the foreground.<p>"));

    // ---------------------------------------------------------------

    QVBox *histoBox    = new QVBox(this);
    d->histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add(d->histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing "
                                             "of the selected image channel. This one is re-computed at any "
                                             "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    d->hGradient  = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histoBox );
    d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // ---------------------------------------------------------------

    d->decodingSettingsBox = new KDcrawIface::DcrawSettingsWidget(this, true, true, false);
    d->infoBox             = new ImageDialogPreview(d->decodingSettingsBox);
    d->infoBox->showPreview(url);

    // ---------------------------------------------------------------

    d->advExposureBox              = new QWidget(d->decodingSettingsBox);
    QGridLayout* advExposureLayout = new QGridLayout(d->advExposureBox, 4, 2);

    d->contrastLabel   = new QLabel(i18n("Contrast:"), d->advExposureBox);
    d->contrastSpinBox = new KIntNumInput(d->advExposureBox);
    d->contrastSpinBox->setRange(-100, 100, 1, true);
    d->contrastSpinBox->setValue(0);
    QWhatsThis::add(d->contrastSpinBox, i18n("<p>Set here the contrast adjustment of the image."));

    d->gammaLabel   = new QLabel(i18n("Gamma:"), d->advExposureBox);
    d->gammaSpinBox = new KDoubleNumInput(d->advExposureBox);
    d->gammaSpinBox->setPrecision(2);
    d->gammaSpinBox->setRange(0.1, 3.0, 0.01, true);
    d->gammaSpinBox->setValue(1.0);
    QWhatsThis::add(d->gammaSpinBox, i18n("Set here the gamma adjustement of the image"));

    d->saturationLabel = new QLabel(i18n("Saturation:"), d->advExposureBox);
    d->saturationInput = new KDoubleNumInput(d->advExposureBox);
    d->saturationInput->setPrecision(2);
    d->saturationInput->setRange(0.0, 2.0, 0.01, true);
    QWhatsThis::add( d->saturationInput, i18n("<p>Set here the color saturation correction."));

    d->fineExposureLabel = new QLabel(i18n("Exposure:"), d->advExposureBox);
    d->fineExposureInput = new KDoubleNumInput(d->advExposureBox);
    d->fineExposureInput->setPrecision(2);
    d->fineExposureInput->setRange(-0.5, 0.5, 0.01, true);
    QWhatsThis::add(d->fineExposureInput, i18n("<p>This value in E.V will be used to perform "
                                               "an exposure compensation of the image."));

    advExposureLayout->addMultiCellWidget(d->contrastLabel,     0, 0, 0, 0);
    advExposureLayout->addMultiCellWidget(d->contrastSpinBox,   0, 0, 1, 2);
    advExposureLayout->addMultiCellWidget(d->gammaLabel,        1, 1, 0, 0);
    advExposureLayout->addMultiCellWidget(d->gammaSpinBox,      1, 1, 1, 2);
    advExposureLayout->addMultiCellWidget(d->saturationLabel,   2, 2, 0, 0);
    advExposureLayout->addMultiCellWidget(d->saturationInput,   2, 2, 1, 2);
    advExposureLayout->addMultiCellWidget(d->fineExposureLabel, 3, 3, 0, 0);
    advExposureLayout->addMultiCellWidget(d->fineExposureInput, 3, 3, 1, 2);
    advExposureLayout->setRowStretch(4, 10);
    advExposureLayout->setSpacing(0);
    advExposureLayout->setMargin(KDialog::spacingHint());

    // ---------------------------------------------------------------

    d->curveBox              = new QWidget(d->decodingSettingsBox);
    QGridLayout* curveLayout = new QGridLayout(d->curveBox, 3, 2);

    ColorGradientWidget* vGradient = new ColorGradientWidget(ColorGradientWidget::Vertical, 10, d->curveBox);
    vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    QLabel *spacev = new QLabel(d->curveBox);
    spacev->setFixedWidth(1);

    d->curves      = new ImageCurves(true);
    d->curveWidget = new CurvesWidget(256, 192, d->curves, d->curveBox);
    QWhatsThis::add(d->curveWidget, i18n("<p>This is the curve adjustment of the image luminosity"));

    d->resetCurveBtn = new QToolButton(d->curveBox);
    d->resetCurveBtn->setFixedSize(11, 11);
    d->resetCurveBtn->setIconSet(SmallIconSet("reload_page", 8));
    d->resetCurveBtn->setFocusPolicy(QWidget::NoFocus);
    d->resetCurveBtn->setAutoRaise(true);
    QToolTip::add(d->resetCurveBtn, i18n("Reset curve to linear"));

    QLabel *spaceh = new QLabel(d->curveBox);
    spaceh->setFixedHeight(1);

    ColorGradientWidget *hGradient = new ColorGradientWidget(ColorGradientWidget::Horizontal, 10, d->curveBox);
    hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    curveLayout->addMultiCellWidget(vGradient,        0, 0, 0, 0);
    curveLayout->addMultiCellWidget(spacev,           0, 0, 1, 1);
    curveLayout->addMultiCellWidget(d->curveWidget,   0, 0, 2, 2);
    curveLayout->addMultiCellWidget(spaceh,           1, 1, 2, 2);
    curveLayout->addMultiCellWidget(d->resetCurveBtn, 1, 2, 0, 1);
    curveLayout->addMultiCellWidget(hGradient,        2, 2, 2, 2);
    curveLayout->setRowStretch(3, 10);
    curveLayout->setSpacing(0);
    curveLayout->setMargin(KDialog::spacingHint());

    // ---------------------------------------------------------------

#if KDCRAW_VERSION >= 0x000105
    d->decodingSettingsBox->addItem(d->advExposureBox, i18n("Exposure"));
    d->decodingSettingsBox->addItem(d->curveBox, i18n("Curve"));
    d->decodingSettingsBox->addItem(d->infoBox, i18n("Infos"));
    d->decodingSettingsBox->setItemIconSet(0, SmallIconSet("kdcraw"));
    d->decodingSettingsBox->setItemIconSet(1, SmallIconSet("whitebalance"));
    d->decodingSettingsBox->setItemIconSet(2, SmallIconSet("lensdistortion"));
    d->decodingSettingsBox->setItemIconSet(3, SmallIconSet("colormanagement"));
    d->decodingSettingsBox->setItemIconSet(4, SmallIconSet("contrast"));
    d->decodingSettingsBox->setItemIconSet(5, SmallIconSet("adjustcurves"));
    d->decodingSettingsBox->setItemIconSet(6, SmallIconSet("exifinfo"));
    d->decodingSettingsBox->updateMinimumWidth();
#else
    d->decodingSettingsBox->insertTab(d->curveBox, i18n("Curve"));
    d->decodingSettingsBox->insertTab(d->advExposureBox, i18n("Exposure"));
    d->decodingSettingsBox->insertTab(d->infoBox, i18n("Infos"));
#endif

    // ---------------------------------------------------------------

    gridSettings->addMultiCellWidget(label1,                 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(d->channelCB,           0, 0, 1, 1);
    gridSettings->addMultiCellWidget(d->scaleBG,             0, 0, 4, 4);
    gridSettings->addMultiCellWidget(label10,                1, 1, 0, 0);
    gridSettings->addMultiCellWidget(d->colorsCB,            1, 1, 1, 1);
    gridSettings->addMultiCellWidget(histoBox,               2, 3, 0, 4);
    gridSettings->addMultiCellWidget(d->decodingSettingsBox, 4, 4, 0, 4);
    gridSettings->setRowStretch(5, 10);
    gridSettings->setColStretch(2, 10);
    gridSettings->setSpacing(KDialog::spacingHint());
    gridSettings->setMargin(0);

    // ---------------------------------------------------------------

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(d->scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));

    connect(d->decodingSettingsBox, SIGNAL(signalSixteenBitsImageToggled(bool)),
            this, SLOT(slotSixteenBitsImageToggled(bool)));

    connect(d->resetCurveBtn, SIGNAL(clicked()),
            this, SLOT(slotResetCurve()));
}

RawSettingsBox::~RawSettingsBox()
{
    delete d->curveWidget;
    delete d->curves;
    delete d;
}

void RawSettingsBox::setBusy(bool b)
{
    d->decodingSettingsBox->setEnabled(!b);
}

void RawSettingsBox::setImage(const DImg& img)
{
    d->image = img;
    d->histogramWidget->stopHistogramComputation();
    d->histogramWidget->updateData(d->image.bits(), d->image.width(), d->image.height(), d->image.sixteenBit());
    d->curveWidget->stopHistogramComputation();
    d->curveWidget->updateData(d->image.bits(), d->image.width(), d->image.height(), d->image.sixteenBit());
}

void RawSettingsBox::setDefaultSettings()
{
    d->decodingSettingsBox->setDefaultSettings();
    d->contrastSpinBox->setValue(0);
    d->gammaSpinBox->setValue(1.0);
    d->saturationInput->setValue(1.0);
    d->fineExposureInput->setValue(0.0);
    slotResetCurve();
}

void RawSettingsBox::slotResetCurve()
{
    d->curves->curvesReset();
    d->curveWidget->reset();
}

HistogramWidget* RawSettingsBox::histogram() const
{
   return d->histogramWidget;
}

CurvesWidget* RawSettingsBox::curve() const
{
    return d->curveWidget;
}

void RawSettingsBox::slotSixteenBitsImageToggled(bool)
{
#if KDCRAW_VERSION >= 0x000105
    // Dcraw do not provide a way to set brigness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.
    d->decodingSettingsBox->setEnabledBrightnessSettings(true);
#endif
}

void RawSettingsBox::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");

    d->channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", RawSettingsBoxPriv::LuminosityChannel));
    d->scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    d->colorsCB->setCurrentItem(config->readNumEntry("Histogram Color", RawSettingsBoxPriv::AllColorsRed));

    d->decodingSettingsBox->setSixteenBits(config->readBoolEntry("SixteenBitsImage", false));
    d->decodingSettingsBox->setWhiteBalance((DRawDecoding::WhiteBalance)
                                            config->readNumEntry("White Balance",
                                            DRawDecoding::CAMERA));
    d->decodingSettingsBox->setCustomWhiteBalance(config->readNumEntry("Custom White Balance", 6500));
    d->decodingSettingsBox->setCustomWhiteBalanceGreen(config->readDoubleNumEntry("Custom White Balance Green", 1.0));
    d->decodingSettingsBox->setFourColor(config->readBoolEntry("Four Color RGB", false));
    d->decodingSettingsBox->setUnclipColor(config->readNumEntry("Unclip Color", 0));
    d->decodingSettingsBox->setDontStretchPixels(config->readBoolEntry("Dont Stretch Pixels", false));
    d->decodingSettingsBox->setNoiseReduction(config->readBoolEntry("Use Noise Reduction", false));
    d->decodingSettingsBox->setBrightness(config->readDoubleNumEntry("Brightness Multiplier", 1.0));
    d->decodingSettingsBox->setUseBlackPoint(config->readBoolEntry("Use Black Point", false));
    d->decodingSettingsBox->setBlackPoint(config->readNumEntry("Black Point", 0));
#if KDCRAW_VERSION >= 0x000105
    d->decodingSettingsBox->setUseWhitePoint(config->readBoolEntry("Use White Point", false));
    d->decodingSettingsBox->setWhitePoint(config->readNumEntry("White Point", 0));
    d->decodingSettingsBox->setMedianFilterPasses(config->readNumEntry("Median Filter Passes", 0));
#endif
    d->decodingSettingsBox->setNRThreshold(config->readNumEntry("NR Threshold", 100));
    d->decodingSettingsBox->setUseCACorrection(config->readBoolEntry("EnableCACorrection", false));
    d->decodingSettingsBox->setcaRedMultiplier(config->readDoubleNumEntry("caRedMultiplier", 1.0));
    d->decodingSettingsBox->setcaBlueMultiplier(config->readDoubleNumEntry("caBlueMultiplier", 1.0));

    d->decodingSettingsBox->setQuality(
        (DRawDecoding::DecodingQuality)config->readNumEntry("Decoding Quality", 
            (int)(DRawDecoding::BILINEAR))); 

    d->decodingSettingsBox->setOutputColorSpace(
        (DRawDecoding::OutputColorSpace)config->readNumEntry("Output Color Space", 
            (int)(DRawDecoding::SRGB))); 

    d->contrastSpinBox->setValue(config->readNumEntry("Constrast", 0));
    d->gammaSpinBox->setValue(config->readDoubleNumEntry("Gamma", 1.0));
    d->saturationInput->setValue(config->readDoubleNumEntry("Saturation", 1.0));
    d->fineExposureInput->setValue(config->readDoubleNumEntry("FineExposure", 0.0));

    d->curves->curvesReset();
    d->curveWidget->reset();

    for (int j = 0 ; j <= 17 ; j++)
    {
        QPoint disable(-1, -1);
        QPoint p = config->readPointEntry(QString("CurveAjustmentPoint%1").arg(j), &disable);
        d->curves->setCurvePoint(ImageHistogram::ValueChannel, j, p);
    }
    d->curves->curvesCalculateCurve(ImageHistogram::ValueChannel);

    d->decodingSettingsBox->setCurrentIndex(config->readNumEntry("Settings Tab", 0));

    slotChannelChanged(d->channelCB->currentItem());
    slotScaleChanged(d->scaleBG->selectedId());
    slotColorsChanged(d->colorsCB->currentItem());
}

void RawSettingsBox::saveSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");

    config->writeEntry("Histogram Channel",          d->channelCB->currentItem());
    config->writeEntry("Histogram Scale",            d->scaleBG->selectedId());
    config->writeEntry("Histogram Color",            d->colorsCB->currentItem());

    config->writeEntry("SixteenBitsImage",           d->decodingSettingsBox->sixteenBits());
    config->writeEntry("White Balance",              d->decodingSettingsBox->whiteBalance());
    config->writeEntry("Custom White Balance",       d->decodingSettingsBox->customWhiteBalance());
    config->writeEntry("Custom White Balance Green", d->decodingSettingsBox->customWhiteBalanceGreen());
    config->writeEntry("Four Color RGB",             d->decodingSettingsBox->useFourColor());
    config->writeEntry("Unclip Color",               d->decodingSettingsBox->unclipColor());
    config->writeEntry("Dont Stretch Pixels",        d->decodingSettingsBox->useDontStretchPixels());
    config->writeEntry("Use Noise Reduction",        d->decodingSettingsBox->useNoiseReduction());
    config->writeEntry("Brightness Multiplier",      d->decodingSettingsBox->brightness());
    config->writeEntry("Use Black Point",            d->decodingSettingsBox->useBlackPoint());
    config->writeEntry("Black Point",                d->decodingSettingsBox->blackPoint());
#if KDCRAW_VERSION >= 0x000105
    config->writeEntry("Use White Point",            d->decodingSettingsBox->useWhitePoint());
    config->writeEntry("White Point",                d->decodingSettingsBox->whitePoint());
    config->writeEntry("MedianFilterPasses",         d->decodingSettingsBox->medianFilterPasses());
#endif
    config->writeEntry("NR Threshold",               d->decodingSettingsBox->NRThreshold());
    config->writeEntry("EnableCACorrection",         d->decodingSettingsBox->useCACorrection());
    config->writeEntry("caRedMultiplier",            d->decodingSettingsBox->caRedMultiplier());
    config->writeEntry("caBlueMultiplier",           d->decodingSettingsBox->caBlueMultiplier());
    config->writeEntry("Decoding Quality",           (int)d->decodingSettingsBox->quality());
    config->writeEntry("Output Color Space",         (int)d->decodingSettingsBox->outputColorSpace());

    config->writeEntry("Constrast",                  d->contrastSpinBox->value());
    config->writeEntry("Gamma",                      d->gammaSpinBox->value());
    config->writeEntry("Saturation",                 d->saturationInput->value());
    config->writeEntry("FineExposure",               d->fineExposureInput->value());

    for (int j = 0 ; j <= 17 ; j++)
    {
        QPoint p = d->curves->getCurvePoint(ImageHistogram::ValueChannel, j);
        config->writeEntry(QString("CurveAjustmentPoint%1").arg(j), p);
    }

    config->writeEntry("Settings Tab", d->decodingSettingsBox->currentIndex());
    config->sync();
}

DRawDecoding RawSettingsBox::settings()
{
    DRawDecoding settings;
    settings.sixteenBitsImage        = d->decodingSettingsBox->sixteenBits();
    settings.whiteBalance            = d->decodingSettingsBox->whiteBalance();
    settings.customWhiteBalance      = d->decodingSettingsBox->customWhiteBalance();
    settings.customWhiteBalanceGreen = d->decodingSettingsBox->customWhiteBalanceGreen();
    settings.RGBInterpolate4Colors   = d->decodingSettingsBox->useFourColor();
    settings.unclipColors            = d->decodingSettingsBox->unclipColor();
    settings.DontStretchPixels       = d->decodingSettingsBox->useDontStretchPixels();
    settings.enableNoiseReduction    = d->decodingSettingsBox->useNoiseReduction();
    settings.brightness              = d->decodingSettingsBox->brightness();
    settings.enableBlackPoint        = d->decodingSettingsBox->useBlackPoint();
    settings.blackPoint              = d->decodingSettingsBox->blackPoint();
#if KDCRAW_VERSION >= 0x000105
    settings.enableWhitePoint        = d->decodingSettingsBox->useWhitePoint();
    settings.whitePoint              = d->decodingSettingsBox->whitePoint();
    settings.medianFilterPasses      = d->decodingSettingsBox->medianFilterPasses();
#endif
    settings.NRThreshold             = d->decodingSettingsBox->NRThreshold();
    settings.enableCACorrection      = d->decodingSettingsBox->useCACorrection();
    settings.caMultiplier[0]         = d->decodingSettingsBox->caRedMultiplier();
    settings.caMultiplier[1]         = d->decodingSettingsBox->caBlueMultiplier();
    settings.RAWQuality              = d->decodingSettingsBox->quality();
    settings.outputColorSpace        = d->decodingSettingsBox->outputColorSpace();

    settings.contrast                = (double)(d->contrastSpinBox->value()/100.0) + 1.00;
    settings.gamma                   = d->gammaSpinBox->value();
    settings.saturation              = d->saturationInput->value();
    settings.exposureComp            = d->fineExposureInput->value();

    if (d->curves->isDirty())
        settings.curveAdjust         = d->curves->getCurvePoints(ImageHistogram::ValueChannel);

    return settings;
}

void RawSettingsBox::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case RawSettingsBoxPriv::LuminosityChannel:
            d->histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::RedChannel:
            d->histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::GreenChannel:
            d->histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::BlueChannel:
            d->histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::ColorChannels:
            d->histogramWidget->m_channelType = HistogramWidget::ColorChannelsHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            d->colorsCB->setEnabled(true);
            break;
    }

    d->histogramWidget->repaint(false);
}

void RawSettingsBox::slotScaleChanged(int scale)
{
    d->histogramWidget->m_scaleType = scale;
    d->histogramWidget->repaint(false);
}

void RawSettingsBox::slotColorsChanged(int color)
{
    switch(color)
    {
        case RawSettingsBoxPriv::AllColorsGreen:
            d->histogramWidget->m_colorType = HistogramWidget::GreenColor;
            break;

        case RawSettingsBoxPriv::AllColorsBlue:
            d->histogramWidget->m_colorType = HistogramWidget::BlueColor;
            break;

        default:          // Red.
            d->histogramWidget->m_colorType = HistogramWidget::RedColor;
            break;
    }

    d->histogramWidget->repaint(false);
}

} // NameSpace Digikam
