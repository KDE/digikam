/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2008-08-04
 * Description : Raw import dialog
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
#include <qpushbutton.h>
#include <qvbox.h>
#include <qsplitter.h>

// KDE includes.

#include <knuminput.h>
#include <kcursor.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/dcrawsettingswidget.h>

// Local includes.

#include "ddebug.h"
#include "drawdecoding.h"
#include "imagedialog.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "rawpreview.h"
#include "rawimportdlg.h"
#include "rawimportdlg.moc"

namespace Digikam
{

class RawImportDlgPriv
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

    RawImportDlgPriv()
    {
        previewWidget       = 0;
        decodingSettingsBox = 0;
        channelCB           = 0;
        colorsCB            = 0;
        effectType          = 0;
        scaleBG             = 0;
        hGradient           = 0;
        histogramWidget     = 0;
        infoBox             = 0;
        advExposureBox      = 0;
        splitter            = 0;
    }

    QWidget                          *advExposureBox;

    QComboBox                        *channelCB;
    QComboBox                        *effectType;
    QComboBox                        *colorsCB;

    QLabel                           *gammaLabel;

    QSplitter                        *splitter;

    QHButtonGroup                    *scaleBG;

    KDoubleNumInput                  *gammaSpinBox;

    KURL                              url;

    ColorGradientWidget              *hGradient;

    HistogramWidget                  *histogramWidget;


    ImageDialogPreview               *infoBox;

    RawPreview                       *previewWidget;

    KDcrawIface::DcrawSettingsWidget *decodingSettingsBox;
};

RawImportDlg::RawImportDlg(const KURL& url, QWidget *parent)
            : KDialogBase(parent, 0, false, QString(),
                          Help|Default|User1|User2|Ok|Cancel, Cancel, true)
{
    d = new RawImportDlgPriv;
    d->url = url;

    setHelp("rawimport.anchor", "digikam");
    setCaption(i18n("Raw Import - %1").arg(d->url.fileName()));

    setButtonText(User1, i18n("&Preview"));
    setButtonTip(User1, i18n("<p>Generate a Raw image preview using current settings."));

    setButtonText(User2, i18n("&Abort"));
    setButtonTip(User2, i18n("<p>Abort the current Raw image preview"));

    setButtonText(Ok, i18n("&Import"));
    setButtonTip(Ok, i18n("<p>Import image to editor using current settings."));

    setButtonText(Cancel, i18n("&Use Default"));
    setButtonTip(Cancel, i18n("<p>Use general Raw decoding settings to load this image in editor."));

    setButtonText(Default, i18n("&Reset"));
    setButtonTip(Default, i18n("<p>Reset these settings to default values."));

    QHBox *page    = new QHBox(this);
    setMainWidget(page);
    d->splitter      = new QSplitter(page);
    d->previewWidget = new RawPreview(d->splitter);

    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    d->previewWidget->setSizePolicy(rightSzPolicy);

    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);

    // ---------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(d->splitter);
    QGridLayout* gridSettings = new QGridLayout(gboxSettings, 5, 4);

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->channelCB   = new QComboBox( false, gboxSettings );
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

    d->scaleBG = new QHButtonGroup(gboxSettings);
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

    QLabel *label10 = new QLabel(i18n("Colors:"), gboxSettings);
    label10->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->colorsCB = new QComboBox( false, gboxSettings );
    d->colorsCB->insertItem( i18n("Red") );
    d->colorsCB->insertItem( i18n("Green") );
    d->colorsCB->insertItem( i18n("Blue") );
    d->colorsCB->setEnabled( false );
    QWhatsThis::add( d->colorsCB, i18n("<p>Select the main color displayed with Colors Channel mode here:<p>"
                                       "<b>Red</b>: Draw the red image channel in the foreground.<p>"
                                       "<b>Green</b>: Draw the green image channel in the foreground.<p>"
                                       "<b>Blue</b>: Draw the blue image channel in the foreground.<p>"));

    // ---------------------------------------------------------------

    QVBox *histoBox    = new QVBox(gboxSettings);
    d->histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add(d->histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing "
                                             "of the selected image channel. This one is re-computed at any "
                                             "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    d->hGradient  = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histoBox );
    d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // ---------------------------------------------------------------

    d->decodingSettingsBox = new KDcrawIface::DcrawSettingsWidget(gboxSettings, true, true, true);
    d->infoBox             = new ImageDialogPreview(d->decodingSettingsBox);
    d->infoBox->showPreview(d->url);

    d->advExposureBox              = new QWidget(d->decodingSettingsBox);
    QGridLayout* advExposureLayout = new QGridLayout(d->advExposureBox, 2, 2);

    d->gammaLabel   = new QLabel(i18n("Gamma:"), d->advExposureBox);
    d->gammaSpinBox = new KDoubleNumInput(d->advExposureBox);
    d->gammaSpinBox->setPrecision(2);
    d->gammaSpinBox->setRange(0.1, 3.0, 0.01, true);
    d->gammaSpinBox->setValue(1.0);
    QWhatsThis::add(d->gammaSpinBox, i18n("<p><b>Gamma</b><p>"
                    "Set here the gamma adjustement of the image"));

    advExposureLayout->addMultiCellWidget(d->gammaLabel,   0, 0, 0, 0);
    advExposureLayout->addMultiCellWidget(d->gammaSpinBox, 0, 0, 1, 2);
    advExposureLayout->setRowStretch(2, 10);
    advExposureLayout->setSpacing(KDialog::spacingHint());
    advExposureLayout->setMargin(KDialog::spacingHint());

#if KDCRAW_VERSION >= 0x000105
    d->decodingSettingsBox->addItem(d->advExposureBox, i18n("Exposure"));
    d->decodingSettingsBox->addItem(d->infoBox, i18n("Infos"));
#else
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
    gridSettings->setSpacing(spacingHint());
    gridSettings->setMargin(0);

    // ---------------------------------------------------------------

/*    mainLayout->addMultiCellWidget(d->previewWidget, 0, 1, 0, 0);
    mainLayout->addMultiCellWidget(gboxSettings,     0, 0, 1, 1);
    mainLayout->setColStretch(0, 10);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setSpacing(spacingHint());
    mainLayout->setMargin(0);
*/
    // ---------------------------------------------------------------


    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(d->scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalImageLoaded(const DImg&)),
            this, SLOT(slotImageLoaded(const DImg&)));

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalLoadingFailed()),
            this, SLOT(slotLoadingFailed()));

    connect(d->previewWidget, SIGNAL(signalLoadingProgress(float)),
            this, SLOT(slotLoadingProgress(float)));

    connect(d->decodingSettingsBox, SIGNAL(signalSixteenBitsImageToggled(bool)),
            this, SLOT(slotSixteenBitsImageToggled(bool)));

    // ---------------------------------------------------------------

    busy(false);
    readSettings();
    d->previewWidget->setUrl(d->url);
    slotUser1();
}

RawImportDlg::~RawImportDlg()
{
    delete d;
}

void RawImportDlg::closeEvent(QCloseEvent *e)
{
    if (!e) return;
    saveSettings();
    e->accept();
}

void RawImportDlg::slotClose()
{
    saveSettings();
    KDialogBase::slotClose();
}

void RawImportDlg::slotDefault()
{
    d->decodingSettingsBox->setDefaultSettings();
    d->gammaSpinBox->setValue(1.0);
}

void RawImportDlg::slotOk()
{
    saveSettings();
    KDialogBase::slotOk();
}

void RawImportDlg::slotSixteenBitsImageToggled(bool)
{
#if KDCRAW_VERSION >= 0x000105
    // Dcraw do not provide a way to set brigness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.
    d->decodingSettingsBox->setEnabledBrightnessSettings(true);
#endif
}

void RawImportDlg::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");

    d->channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", RawImportDlgPriv::LuminosityChannel));
    d->scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    d->colorsCB->setCurrentItem(config->readNumEntry("Histogram Color", RawImportDlgPriv::AllColorsRed));

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

    d->gammaSpinBox->setValue(config->readDoubleNumEntry("Gamma", 1.0));

    resize(configDialogSize(*config, QString("RAW Import Dialog")));

    slotChannelChanged(d->channelCB->currentItem());
    slotScaleChanged(d->scaleBG->selectedId());
    slotColorsChanged(d->colorsCB->currentItem());
}

void RawImportDlg::saveSettings()
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
#endif
    config->writeEntry("NR Threshold",               d->decodingSettingsBox->NRThreshold());
    config->writeEntry("EnableCACorrection",         d->decodingSettingsBox->useCACorrection());
    config->writeEntry("caRedMultiplier",            d->decodingSettingsBox->caRedMultiplier());
    config->writeEntry("caBlueMultiplier",           d->decodingSettingsBox->caBlueMultiplier());
    config->writeEntry("Decoding Quality",           (int)d->decodingSettingsBox->quality());
    config->writeEntry("Output Color Space",         (int)d->decodingSettingsBox->outputColorSpace());
    config->writeEntry("Gamma",                      d->gammaSpinBox->value());

    saveDialogSize(*config, QString("RAW Import Dialog"));
    config->sync();
}

DRawDecoding RawImportDlg::rawDecodingSettings()
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
#endif
    settings.NRThreshold             = d->decodingSettingsBox->NRThreshold();
    settings.enableCACorrection      = d->decodingSettingsBox->useCACorrection();
    settings.caMultiplier[0]         = d->decodingSettingsBox->caRedMultiplier();
    settings.caMultiplier[1]         = d->decodingSettingsBox->caBlueMultiplier();
    settings.RAWQuality              = d->decodingSettingsBox->quality();
    settings.outputColorSpace        = d->decodingSettingsBox->outputColorSpace();
    settings.gamma                   = d->gammaSpinBox->value();

    return settings;
}

// 'Preview' dialog button.
void RawImportDlg::slotUser1()
{
    DRawDecoding settings = rawDecodingSettings();

    // We will load an half size image to speed up preview computing.
    settings.halfSizeColorImage = true;

    d->previewWidget->setDecodingSettings(settings);
}

// 'Abort' dialog button.
void RawImportDlg::slotUser2()
{
    d->previewWidget->cancelLoading();
    d->histogramWidget->stopHistogramComputation();
    busy(false);
}

void RawImportDlg::busy(bool val)
{
    if (val) d->previewWidget->setCursor(KCursor::waitCursor());
    else d->previewWidget->unsetCursor();
    d->decodingSettingsBox->setEnabled(!val);
    enableButton (User1, !val);
    enableButton (User2, val);
    enableButton (Close, !val);
}

void RawImportDlg::slotLoadingStarted()
{
    d->histogramWidget->setDataLoading();
    busy(true);
}

void RawImportDlg::slotLoadingProgress(float /*progress*/)
{
}

void RawImportDlg::slotImageLoaded(const DImg& img)
{
    d->histogramWidget->stopHistogramComputation();
    d->histogramWidget->updateData(img.bits(), img.width(), img.height(), img.sixteenBit(), 0, 0, 0, true);
    busy(false);
}

void RawImportDlg::slotLoadingFailed()
{
    d->histogramWidget->setLoadingFailed();
    busy(false);
}

void RawImportDlg::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case RawImportDlgPriv::LuminosityChannel:
            d->histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawImportDlgPriv::RedChannel:
            d->histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawImportDlgPriv::GreenChannel:
            d->histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawImportDlgPriv::BlueChannel:
            d->histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawImportDlgPriv::ColorChannels:
            d->histogramWidget->m_channelType = HistogramWidget::ColorChannelsHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            d->colorsCB->setEnabled(true);
            break;
    }

    d->histogramWidget->repaint(false);
}

void RawImportDlg::slotScaleChanged(int scale)
{
    d->histogramWidget->m_scaleType = scale;
    d->histogramWidget->repaint(false);
}

void RawImportDlg::slotColorsChanged(int color)
{
    switch(color)
    {
        case RawImportDlgPriv::AllColorsGreen:
            d->histogramWidget->m_colorType = HistogramWidget::GreenColor;
            break;

        case RawImportDlgPriv::AllColorsBlue:
            d->histogramWidget->m_colorType = HistogramWidget::BlueColor;
            break;

        default:          // Red.
            d->histogramWidget->m_colorType = HistogramWidget::RedColor;
            break;
    }

    d->histogramWidget->repaint(false);
}

} // NameSpace Digikam
