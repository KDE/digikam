/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "adjustlevelstool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QButtonGroup>
#include <QColor>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dgradientslider.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "levelsfilter.h"
#include "imagelevels.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;

namespace DigikamColorImagePlugin
{

class AdjustLevelsTool::AdjustLevelsToolPriv
{

public:

    enum ColorPicker
    {
        NoPicker   = -1,
        BlackTonal = 0,
        GrayTonal,
        WhiteTonal
    };

public:

    AdjustLevelsToolPriv() :
        destinationPreviewData(0),
        histoSegments(0),
        pickerBox(0),
        resetButton(0),
        autoButton(0),
        pickBlack(0),
        pickGray(0),
        pickWhite(0),
        pickerColorButtonGroup(0),
        minInput(0),
        maxInput(0),
        minOutput(0),
        maxOutput(0),
        gammaInput(0),
        levelsHistogramWidget(0),
        inputLevels(0),
        outputLevels(0),
        previewWidget(0),
        levels(0),
        originalImage(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configGammaChannelEntry;
    static const QString configLowInputChannelEntry;
    static const QString configLowOutputChannelEntry;
    static const QString configHighInputChannelEntry;
    static const QString configHighOutputChannelEntry;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    int                  histoSegments;

    QWidget*             pickerBox;

    QPushButton*         resetButton;
    QToolButton*         autoButton;
    QToolButton*         pickBlack;
    QToolButton*         pickGray;
    QToolButton*         pickWhite;

    QButtonGroup*        pickerColorButtonGroup;

    RIntNumInput*        minInput;
    RIntNumInput*        maxInput;
    RIntNumInput*        minOutput;
    RIntNumInput*        maxOutput;

    RDoubleNumInput*     gammaInput;

    HistogramWidget*     levelsHistogramWidget;

    DGradientSlider*     inputLevels;
    DGradientSlider*     outputLevels;

    ImageRegionWidget*   previewWidget;

    ImageLevels*         levels;

    DImg*                originalImage;

    EditorToolSettings*  gboxSettings;
};
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configGroupName("adjustlevels Tool");
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configGammaChannelEntry("GammaChannel%1");
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configLowInputChannelEntry("LowInputChannel%1");
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configLowOutputChannelEntry("LowOutputChannel%1");
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configHighInputChannelEntry("HighInputChannel%1");
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configHighOutputChannelEntry("HighOutputChannel%1");
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString AdjustLevelsTool::AdjustLevelsToolPriv::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

AdjustLevelsTool::AdjustLevelsTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new AdjustLevelsToolPriv)
{
    setObjectName("adjustlevels");
    setToolName(i18n("Adjust Levels"));
    setToolIcon(SmallIcon("adjustlevels"));

    ImageIface iface(0, 0);
    d->originalImage = iface.getOriginalImg();

    d->histoSegments = d->originalImage->sixteenBit() ? 65535 : 255;
    d->levels        = new ImageLevels(d->originalImage->sixteenBit());

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(Digikam::LRGBAC);

    // we don't need to use the Gradient widget in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);

    // -------------------------------------------------------------

    d->levelsHistogramWidget = new HistogramWidget(256, 140, d->originalImage->bits(),
            d->originalImage->width(),
            d->originalImage->height(),
            d->originalImage->sixteenBit(),
            d->gboxSettings->plainPage(), false);
    d->levelsHistogramWidget->setWhatsThis(i18n("This is the histogram drawing of the selected channel "
                                           "from the original image."));
    QHBoxLayout* inputLevelsLayout = new QHBoxLayout;
    inputLevelsLayout->addWidget(d->levelsHistogramWidget);

    // -------------------------------------------------------------

    d->inputLevels = new DGradientSlider();
    d->inputLevels->setWhatsThis( i18n("Select the input intensity of the histogram here."));
    d->inputLevels->setToolTip( i18n( "Input intensity." ) );
    d->inputLevels->installEventFilter(this);
    d->gboxSettings->histogramBox()->setHistogramMargin(d->inputLevels->gradientOffset());
    inputLevelsLayout->setContentsMargins(d->inputLevels->gradientOffset(), 0,
                                          d->inputLevels->gradientOffset(), 0);

    d->outputLevels = new DGradientSlider();
    d->outputLevels->setWhatsThis( i18n("Select the output intensity of the histogram here."));
    d->outputLevels->setToolTip( i18n( "Output intensity." ) );
    d->outputLevels->installEventFilter(this);

    d->minInput = new RIntNumInput();
    d->minInput->setRange(0, d->histoSegments, 1);
    d->minInput->setSliderEnabled(false);
    d->minInput->setDefaultValue(0);
    d->minInput->setWhatsThis( i18n("Select the minimal input intensity value of the histogram here."));
    d->minInput->setToolTip( i18n( "Minimal input intensity." ) );

    d->gammaInput = new RDoubleNumInput();
    d->gammaInput->setDecimals(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setToolTip( i18n( "Gamma input value." ) );
    d->gammaInput->setWhatsThis( i18n("Select the gamma input value here."));

    d->maxInput = new RIntNumInput();
    d->maxInput->setRange(0, d->histoSegments, 1);
    d->maxInput->setSliderEnabled(false);
    d->maxInput->setDefaultValue(d->histoSegments);
    d->maxInput->setToolTip( i18n( "Maximal input intensity." ) );
    d->maxInput->setWhatsThis( i18n("Select the maximal input intensity value of the histogram here."));

    d->minOutput = new RIntNumInput();
    d->minOutput->setRange(0, d->histoSegments, 1);
    d->minOutput->setSliderEnabled(false);
    d->minOutput->setDefaultValue(0);
    d->minOutput->setToolTip( i18n( "Minimal output intensity." ) );
    d->minOutput->setWhatsThis( i18n("Select the minimal output intensity value of the histogram here."));

    d->maxOutput = new RIntNumInput();
    d->maxOutput->setRange(0, d->histoSegments, 1);
    d->maxOutput->setSliderEnabled(false);
    d->maxOutput->setDefaultValue(d->histoSegments);
    d->maxOutput->setToolTip( i18n( "Maximal output intensity." ) );
    d->maxOutput->setWhatsThis( i18n("Select the maximal output intensity value of the histogram here."));

    // -------------------------------------------------------------

    d->pickerBox = new QWidget();

    d->pickBlack = new QToolButton();
    d->pickBlack->setIcon(KIcon("color-picker-black"));
    d->pickBlack->setCheckable(true);
    d->pickBlack->setToolTip( i18n( "All channels shadow tone color picker" ) );
    d->pickBlack->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "input levels on the Red, Green, Blue, and Luminosity channels."));

    d->pickGray  = new QToolButton();
    d->pickGray->setIcon(KIcon("color-picker-grey"));
    d->pickGray->setCheckable(true);
    d->pickGray->setToolTip( i18n( "All channels middle tone color picker" ) );
    d->pickGray->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                   "image used to set <b>Middle Tone</b> "
                                   "input levels on the Red, Green, Blue, and Luminosity channels."));

    d->pickWhite = new QToolButton();
    d->pickWhite->setIcon(KIcon("color-picker-white"));
    d->pickWhite->setCheckable(true);
    d->pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    d->pickWhite->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                    "image used to set <b>Highlight Tone</b> "
                                    "input levels on the Red, Green, Blue, and Luminosity channels."));

    d->pickerColorButtonGroup = new QButtonGroup(d->pickerBox);
    d->pickerColorButtonGroup->addButton(d->pickBlack, AdjustLevelsToolPriv::BlackTonal);
    d->pickerColorButtonGroup->addButton(d->pickGray,  AdjustLevelsToolPriv::GrayTonal);
    d->pickerColorButtonGroup->addButton(d->pickWhite, AdjustLevelsToolPriv::WhiteTonal);

    QHBoxLayout* pickerBoxLayout = new QHBoxLayout;
    pickerBoxLayout->setMargin(0);
    pickerBoxLayout->setSpacing(0);
    pickerBoxLayout->addWidget(d->pickBlack);
    pickerBoxLayout->addWidget(d->pickGray);
    pickerBoxLayout->addWidget(d->pickWhite);
    d->pickerBox->setLayout(pickerBoxLayout);

    d->pickerColorButtonGroup->setExclusive(true);

    // -------------------------------------------------------------

    d->autoButton = new QToolButton();
    d->autoButton->setIcon(KIconLoader::global()->loadIcon("system-run", KIconLoader::Toolbar));
    d->autoButton->setToolTip( i18n( "Adjust all levels automatically." ) );
    d->autoButton->setWhatsThis(i18n("If you press this button, all channel levels will be adjusted "
                                     "automatically."));

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    d->resetButton->setToolTip( i18n( "Reset current channel levels' values." ) );
    d->resetButton->setWhatsThis(i18n("If you press this button, all levels' values "
                                      "from the currently selected channel "
                                      "will be reset to the default values."));

    QLabel* space = new QLabel();
    space->setFixedWidth(d->gboxSettings->spacingHint());

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(d->pickerBox);
    l3->addWidget(d->autoButton);
    l3->addWidget(space);
    l3->addWidget(d->resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    QGridLayout* grid = new QGridLayout();
    grid->addLayout(inputLevelsLayout, 0, 0, 1, 7);
    grid->addWidget(d->inputLevels,    1, 0, 1, 7);
    grid->addWidget(d->minInput,       2, 1, 1, 1);
    grid->addWidget(d->maxInput,       2, 5, 1, 1);
    grid->addWidget(d->gammaInput,     3, 0, 1, 7);
    grid->addWidget(d->outputLevels,   4, 0, 1, 7);
    grid->addWidget(d->minOutput,      5, 1, 1, 1);
    grid->addWidget(d->maxOutput,      5, 5, 1, 1);
    grid->addLayout(l3,                6, 0, 1, 7);
    grid->setRowStretch(7, 10);
    grid->setColumnStretch(2, 10);
    grid->setColumnStretch(4, 10);
    grid->setMargin(0);
    grid->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            this, SLOT(slotSpotColorChanged(Digikam::DColor)));
    /*
        connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
                this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));
    */

    // -------------------------------------------------------------
    // Color sliders and spinbox slots.

    connect(d->inputLevels, SIGNAL(leftValueChanged(double)),
            this, SLOT(slotAdjustMinInputSpinBox(double)));

    connect(d->inputLevels, SIGNAL(rightValueChanged(double)),
            this, SLOT(slotAdjustMaxInputSpinBox(double)));

    connect(d->outputLevels, SIGNAL(leftValueChanged(double)),
            this, SLOT(slotAdjustMinOutputSpinBox(double)));

    connect(d->outputLevels, SIGNAL(rightValueChanged(double)),
            this, SLOT(slotAdjustMaxOutputSpinBox(double)));

    connect(d->minInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(d->maxInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(d->minOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(d->maxOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(d->gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotGammaInputchanged(double)));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(d->autoButton, SIGNAL(clicked()),
            this, SLOT(slotAutoLevels()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->pickerColorButtonGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotPickerColorButtonActived(int)));

    slotTimer();
}

AdjustLevelsTool::~AdjustLevelsTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d->levels;
    delete d;
}

// See B.K.O #146636: use event filter with all level slider to display a
// guide over level histogram.
bool AdjustLevelsTool::eventFilter(QObject* obj, QEvent* ev)
{
    if ( obj == d->inputLevels )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(d->inputLevels, SIGNAL(leftValueChanged(double)),
                    this, SLOT(slotShowInputHistogramGuide(double)));

            connect(d->inputLevels, SIGNAL(rightValueChanged(double)),
                    this, SLOT(slotShowInputHistogramGuide(double)));

            return false;
        }

        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(d->inputLevels, SIGNAL(leftValueChanged(double)),
                       this, SLOT(slotShowInputHistogramGuide(double)));

            disconnect(d->inputLevels, SIGNAL(rightValueChanged(double)),
                       this, SLOT(slotShowInputHistogramGuide(double)));

            d->levelsHistogramWidget->reset();
            return false;
        }
        else
        {
            return false;
        }
    }

    if ( obj == d->outputLevels )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(d->outputLevels, SIGNAL(leftValueChanged(double)),
                    this, SLOT(slotShowOutputHistogramGuide(double)));

            connect(d->outputLevels, SIGNAL(rightValueChanged(double)),
                    this, SLOT(slotShowOutputHistogramGuide(double)));

            return false;
        }

        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(d->outputLevels, SIGNAL(leftValueChanged(double)),
                       this, SLOT(slotShowOutputHistogramGuide(double)));

            disconnect(d->outputLevels, SIGNAL(rightValueChanged(double)),
                       this, SLOT(slotShowOutputHistogramGuide(double)));

            d->gboxSettings->histogramBox()->histogram()->reset();
            return false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // pass the event on to the parent class
        return EditorToolThreaded::eventFilter(obj, ev);
    }
}

void AdjustLevelsTool::slotShowInputHistogramGuide(double v)
{
    int val = (int)(v * d->histoSegments);
    DColor color(val, val, val, val, d->originalImage->sixteenBit());
    d->levelsHistogramWidget->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotShowOutputHistogramGuide(double v)
{
    int val = (int)(v * d->histoSegments);
    DColor color(val, val, val, val, d->originalImage->sixteenBit());
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotPickerColorButtonActived(int type)
{
    if (type == AdjustLevelsToolPriv::NoPicker)
    {
        return;
    }

    d->previewWidget->setCapturePointMode(true);
}

void AdjustLevelsTool::slotSpotColorChanged(const DColor& color)
{
	ChannelType channel = d->gboxSettings->histogramBox()->channel();

    if ( d->pickBlack->isChecked() )
    {
    	if (channel != ColorChannels) {
			// Black tonal levels point.
			d->levels->levelsBlackToneAdjustByColors(channel, color);
    	} else {
        	for (int i = RedChannel; i <= BlueChannel; i++)
        		d->levels->levelsBlackToneAdjustByColors(i, color);
    	}
		d->pickBlack->setChecked(false);
    }
    else if ( d->pickGray->isChecked() )
    {
    	if (channel != ColorChannels) {
    		// Gray tonal levels point.
    		d->levels->levelsGrayToneAdjustByColors(channel, color);
    	}
		d->pickGray->setChecked(false);
    }
    else if ( d->pickWhite->isChecked() )
    {
    	if (channel != ColorChannels) {
    		// White tonal levels point.
    		d->levels->levelsWhiteToneAdjustByColors(channel, color);
    	} else {
    		for (int i = RedChannel; i <= BlueChannel; i++)
    			d->levels->levelsWhiteToneAdjustByColors(i, color);
    	}
		d->pickWhite->setChecked(false);
    }
    else
    {
        d->levelsHistogramWidget->setHistogramGuideByColor(color);
        return;
    }

    // Refresh the current levels config.
    slotChannelChanged();

    d->previewWidget->setCapturePointMode(false);
    slotEffect();
}

void AdjustLevelsTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotGammaInputchanged(double val)
{
	ChannelType channel = d->gboxSettings->histogramBox()->channel();

	if (channel == ColorChannels)
		channel = LuminosityChannel;

    blockSignals(true);
    d->levels->setLevelGammaValue(channel, val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMinInputSpinBox(double val)
{
    d->minInput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->minInput->setValue(newVal);
    d->minInput->blockSignals(false);
    slotAdjustSliders();
}

void AdjustLevelsTool::slotAdjustMaxInputSpinBox(double val)
{
    d->maxInput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->maxInput->setValue(newVal);
    d->maxInput->blockSignals(false);
    slotAdjustSliders();
}

void AdjustLevelsTool::slotAdjustMinOutputSpinBox(double val)
{
    d->minOutput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->minOutput->setValue(newVal);
    d->minOutput->blockSignals(false);
    slotAdjustSliders();
}

void AdjustLevelsTool::slotAdjustMaxOutputSpinBox(double val)
{
    d->maxOutput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->maxOutput->setValue(newVal);
    d->maxOutput->blockSignals(false);
    slotAdjustSliders();
}

void AdjustLevelsTool::slotAdjustSliders()
{
    adjustSliders(d->minInput->value(), d->gammaInput->value(),
                  d->maxInput->value(), d->minOutput->value(),
                  d->maxOutput->value());

    slotTimer();
}

void AdjustLevelsTool::adjustSlidersAndSpinboxes(int minIn, double gamIn, int maxIn, int minOut, int maxOut)
{
    d->minInput->blockSignals(true);
    d->maxInput->blockSignals(true);
    d->minOutput->blockSignals(true);
    d->maxOutput->blockSignals(true);

    d->minInput->setValue(minIn);
    d->maxInput->setValue(maxIn);
    d->minOutput->setValue(minOut);
    d->maxOutput->setValue(maxOut);

    d->minInput->blockSignals(false);
    d->maxInput->blockSignals(false);
    d->minOutput->blockSignals(false);
    d->maxOutput->blockSignals(false);

    adjustSliders(minIn, gamIn, maxIn, minOut, maxOut);
}

void AdjustLevelsTool::adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut)
{
	ChannelType channel = d->gboxSettings->histogramBox()->channel();

	if (channel == ColorChannels)
		channel = LuminosityChannel;

	d->inputLevels->blockSignals(true);
    d->gammaInput->blockSignals(true);
    d->outputLevels->blockSignals(true);

    d->inputLevels->setLeftValue((double)minIn/(double)d->histoSegments);
    d->inputLevels->setRightValue((double)maxIn/(double)d->histoSegments);
    d->gammaInput->setValue(gamIn);
    d->outputLevels->setLeftValue((double)minOut/(double)d->histoSegments);
    d->outputLevels->setRightValue((double)maxOut/(double)d->histoSegments);

    d->levels->setLevelLowInputValue(channel, minIn);
    d->levels->setLevelHighInputValue(channel, maxIn);
    d->levels->setLevelLowOutputValue(channel, minOut);
    d->levels->setLevelHighOutputValue(channel, maxOut);

    d->inputLevels->blockSignals(false);
    d->gammaInput->blockSignals(false);
    d->outputLevels->blockSignals(false);
}

void AdjustLevelsTool::slotResetCurrentChannel()
{
	ChannelType channel = d->gboxSettings->histogramBox()->channel();

	if (channel == ColorChannels)
		channel = LuminosityChannel;

	d->levels->levelsChannelReset(channel);

    // Refresh the current levels config.
    slotChannelChanged();
    d->levelsHistogramWidget->reset();

    slotEffect();
}

void AdjustLevelsTool::slotAutoLevels()
{
    // Calculate Auto levels.
    d->levels->levelsAuto(d->levelsHistogramWidget->currentHistogram());

    // Refresh the current levels config.
    slotChannelChanged();

    slotEffect();
}

void AdjustLevelsTool::slotChannelChanged()
{
    ChannelType channel = d->gboxSettings->histogramBox()->channel();
    d->levelsHistogramWidget->setChannelType(channel);

    if (channel == ColorChannels)
    	channel = LuminosityChannel;

    switch (channel)
    {
        case RedChannel:
            d->inputLevels->setColors(QColor("black"), QColor("red"));
            d->outputLevels->setColors(QColor("black"), QColor("red"));
            break;

        case GreenChannel:
            d->inputLevels->setColors(QColor("black"), QColor("green"));
            d->outputLevels->setColors(QColor("black"), QColor("green"));
            break;

        case BlueChannel:
            d->inputLevels->setColors(QColor("black"), QColor("blue"));
            d->outputLevels->setColors(QColor("black"), QColor("blue"));
            break;

        default:
            d->inputLevels->setColors(QColor("black"), QColor("white"));
            d->outputLevels->setColors(QColor("black"), QColor("white"));
            break;
    }

    adjustSlidersAndSpinboxes(d->levels->getLevelLowInputValue(channel),
                              d->levels->getLevelGammaValue(channel),
                              d->levels->getLevelHighInputValue(channel),
                              d->levels->getLevelLowOutputValue(channel),
                              d->levels->getLevelHighOutputValue(channel));

}

void AdjustLevelsTool::slotScaleChanged()
{
    d->levelsHistogramWidget->setScaleType(d->gboxSettings->histogramBox()->scale());
}

void AdjustLevelsTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    {
        bool sb        = d->originalImage->sixteenBit();
        int max        = sb ? 65535 : 255;
        double gamma   = 0.0;
        int lowInput   = 0;
        int lowOutput  = 0;
        int highInput  = 0;
        int highOutput = 0;

        for (int i = 0 ; i < 5 ; ++i)
        {
            gamma      = group.readEntry(d->configGammaChannelEntry.arg(i), 1.0);
            lowInput   = group.readEntry(d->configLowInputChannelEntry.arg(i), 0);
            lowOutput  = group.readEntry(d->configLowOutputChannelEntry.arg(i), 0);
            highInput  = group.readEntry(d->configHighInputChannelEntry.arg(i), max);
            highOutput = group.readEntry(d->configHighOutputChannelEntry.arg(i), max);

            d->levels->setLevelGammaValue(i, gamma);
            d->levels->setLevelLowInputValue(i, sb ? lowInput*255 : lowInput);
            d->levels->setLevelHighInputValue(i, sb ? highInput*255 : highInput);
            d->levels->setLevelLowOutputValue(i, sb ? lowOutput*255 : lowOutput);
            d->levels->setLevelHighOutputValue(i, sb ? highOutput*255 : highOutput);
        }
    }

    d->levelsHistogramWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    // This is mandatory here to set spinbox values because slot connections
    // can be not set completely at plugin startup.
    d->minInput->setValue(d->levels->getLevelLowInputValue(d->gboxSettings->histogramBox()->channel()));
    d->minOutput->setValue(d->levels->getLevelLowOutputValue(d->gboxSettings->histogramBox()->channel()));
    d->maxInput->setValue(d->levels->getLevelHighInputValue(d->gboxSettings->histogramBox()->channel()));
    d->maxOutput->setValue(d->levels->getLevelHighOutputValue(d->gboxSettings->histogramBox()->channel()));
    slotChannelChanged();
    slotScaleChanged();
}

void AdjustLevelsTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    {
        bool sb        = d->originalImage->sixteenBit();
        double gamma   = 0.0;
        int lowInput   = 0;
        int lowOutput  = 0;
        int highInput  = 0;
        int highOutput = 0;

        for (int i = 0 ; i < 5 ; ++i)
        {
            gamma      = d->levels->getLevelGammaValue(i);
            lowInput   = d->levels->getLevelLowInputValue(i);
            lowOutput  = d->levels->getLevelLowOutputValue(i);
            highInput  = d->levels->getLevelHighInputValue(i);
            highOutput = d->levels->getLevelHighOutputValue(i);

            group.writeEntry(d->configGammaChannelEntry.arg(i), gamma);
            group.writeEntry(d->configLowInputChannelEntry.arg(i), sb ? lowInput/255 : lowInput);
            group.writeEntry(d->configLowOutputChannelEntry.arg(i), sb ? lowOutput/255 : lowOutput);
            group.writeEntry(d->configHighInputChannelEntry.arg(i), sb ? highInput/255 : highInput);
            group.writeEntry(d->configHighOutputChannelEntry.arg(i), sb ? highOutput/255 : highOutput);
        }
    }

    config->sync();
}

void AdjustLevelsTool::slotResetSettings()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
    {
        d->levels->levelsChannelReset(channel);
    }

    // Refresh the current levels config.
    slotChannelChanged();
    d->levelsHistogramWidget->reset();

    slotEffect();
}

void AdjustLevelsTool::prepareEffect()
{
    LevelsContainer settings;

    for (int i=0 ; i<5 ; ++i)
    {
        settings.lInput[i]  = d->levels->getLevelLowInputValue(i);
        settings.hInput[i]  = d->levels->getLevelHighInputValue(i);
        settings.lOutput[i] = d->levels->getLevelLowOutputValue(i);
        settings.hOutput[i] = d->levels->getLevelHighOutputValue(i);
        settings.gamma[i]   = d->levels->getLevelGammaValue(i);
    }

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new LevelsFilter(&preview, this, settings));
}

void AdjustLevelsTool::putPreviewData()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    d->destinationPreviewData = preview.copyBits();
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
            preview.width(), preview.height(), preview.sixteenBit(),
            0, 0, 0, false);
}

void AdjustLevelsTool::prepareFinal()
{
    LevelsContainer settings;

    for (int i=0 ; i<5 ; ++i)
    {
        settings.lInput[i]  = d->levels->getLevelLowInputValue(i);
        settings.hInput[i]  = d->levels->getLevelHighInputValue(i);
        settings.lOutput[i] = d->levels->getLevelLowOutputValue(i);
        settings.hOutput[i] = d->levels->getLevelHighOutputValue(i);
        settings.gamma[i]   = d->levels->getLevelGammaValue(i);
    }

    ImageIface iface(0, 0);
    setFilter(new LevelsFilter(iface.getOriginalImg(), this, settings));
}

void AdjustLevelsTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Adjust Levels"), filter()->filterAction(), filter()->getTargetImage().bits());
}

void AdjustLevelsTool::slotLoadSettings()
{
    KUrl loadLevelsFile;

    loadLevelsFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                     QString( "*" ), kapp->activeWindow(),
                     QString( i18n("Select Gimp Levels File to Load")) );

    if ( loadLevelsFile.isEmpty() )
    {
        return;
    }

    if ( d->levels->loadLevelsFromGimpLevelsFile( loadLevelsFile ) == false )
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load from the Gimp levels text file."));
        return;
    }

    // Refresh the current levels config.
    slotChannelChanged();

    slotEffect();
}

void AdjustLevelsTool::slotSaveAsSettings()
{
    KUrl saveLevelsFile;

    saveLevelsFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                     QString( "*" ), kapp->activeWindow(),
                     QString( i18n("Gimp Levels File to Save")) );

    if ( saveLevelsFile.isEmpty() )
    {
        return;
    }

    if ( d->levels->saveLevelsToGimpLevelsFile( saveLevelsFile ) == false )
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save to the Gimp levels text file."));
        return;
    }

    // Refresh the current levels config.
    slotChannelChanged();
}

}  // namespace DigikamColorImagePlugin
