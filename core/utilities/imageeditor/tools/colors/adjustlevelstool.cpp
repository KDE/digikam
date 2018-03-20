/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "adjustlevelstool.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QButtonGroup>
#include <QColor>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QIcon>
#include <QStandardPaths>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dnuminput.h"
#include "dgradientslider.h"
#include "dfiledialog.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "levelsfilter.h"
#include "imagelevels.h"
#include "imageregionwidget.h"

namespace Digikam
{

class AdjustLevelsTool::Private
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

    Private() :
        histoSegments(0),
        pickerBox(0),
        resetButton(0),
        autoButton(0),
        pickBlack(0),
        pickGray(0),
        pickWhite(0),
        pickerType(0),
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

    int                  histoSegments;

    QWidget*             pickerBox;

    QPushButton*         resetButton;
    QToolButton*         autoButton;
    QToolButton*         pickBlack;
    QToolButton*         pickGray;
    QToolButton*         pickWhite;

    QButtonGroup*        pickerType;

    DIntNumInput*        minInput;
    DIntNumInput*        maxInput;
    DIntNumInput*        minOutput;
    DIntNumInput*        maxOutput;

    DDoubleNumInput*     gammaInput;

    HistogramWidget*     levelsHistogramWidget;

    DGradientSlider*     inputLevels;
    DGradientSlider*     outputLevels;

    ImageRegionWidget*   previewWidget;

    ImageLevels*         levels;

    DImg*                originalImage;

    EditorToolSettings*  gboxSettings;
};

const QString AdjustLevelsTool::Private::configGroupName(QLatin1String("adjustlevels Tool"));
const QString AdjustLevelsTool::Private::configGammaChannelEntry(QLatin1String("GammaChannel%1"));
const QString AdjustLevelsTool::Private::configLowInputChannelEntry(QLatin1String("LowInputChannel%1"));
const QString AdjustLevelsTool::Private::configLowOutputChannelEntry(QLatin1String("LowOutputChannel%1"));
const QString AdjustLevelsTool::Private::configHighInputChannelEntry(QLatin1String("HighInputChannel%1"));
const QString AdjustLevelsTool::Private::configHighOutputChannelEntry(QLatin1String("HighOutputChannel%1"));
const QString AdjustLevelsTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString AdjustLevelsTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

AdjustLevelsTool::AdjustLevelsTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("adjustlevels"));
    setToolName(i18n("Adjust Levels"));
    setToolIcon(QIcon::fromTheme(QLatin1String("adjustlevels")));
    setInitPreview(true);

    ImageIface iface;
    d->originalImage = iface.original();

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

    d->levelsHistogramWidget = new HistogramWidget(256, 140, d->gboxSettings->plainPage(), false);
    d->levelsHistogramWidget->updateData(*d->originalImage);
    d->levelsHistogramWidget->setWhatsThis(i18n("This is the histogram drawing of the selected channel "
                                           "from the original image."));
    QHBoxLayout* const inputLevelsLayout = new QHBoxLayout;
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

    d->minInput = new DIntNumInput();
    d->minInput->setRange(0, d->histoSegments, 1);
    d->minInput->setDefaultValue(0);
    d->minInput->setWhatsThis( i18n("Select the minimal input intensity value of the histogram here."));
    d->minInput->setToolTip( i18n( "Minimal input intensity." ) );

    d->gammaInput = new DDoubleNumInput();
    d->gammaInput->setDecimals(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setToolTip( i18n( "Gamma input value." ) );
    d->gammaInput->setWhatsThis( i18n("Select the gamma input value here."));

    d->maxInput = new DIntNumInput();
    d->maxInput->setRange(0, d->histoSegments, 1);
    d->maxInput->setDefaultValue(d->histoSegments);
    d->maxInput->setToolTip( i18n( "Maximal input intensity." ) );
    d->maxInput->setWhatsThis( i18n("Select the maximal input intensity value of the histogram here."));

    d->minOutput = new DIntNumInput();
    d->minOutput->setRange(0, d->histoSegments, 1);
    d->minOutput->setDefaultValue(0);
    d->minOutput->setToolTip( i18n( "Minimal output intensity." ) );
    d->minOutput->setWhatsThis( i18n("Select the minimal output intensity value of the histogram here."));

    d->maxOutput = new DIntNumInput();
    d->maxOutput->setRange(0, d->histoSegments, 1);
    d->maxOutput->setDefaultValue(d->histoSegments);
    d->maxOutput->setToolTip( i18n( "Maximal output intensity." ) );
    d->maxOutput->setWhatsThis( i18n("Select the maximal output intensity value of the histogram here."));

    // -------------------------------------------------------------

    d->pickerBox = new QWidget();

    d->pickBlack = new QToolButton();
    d->pickBlack->setIcon(QIcon::fromTheme(QLatin1String("color-picker-black")));
    d->pickBlack->setCheckable(true);
    d->pickBlack->setToolTip( i18n( "All channels shadow tone color picker" ) );
    d->pickBlack->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "input levels on the Red, Green, Blue, and Luminosity channels."));

    d->pickGray  = new QToolButton();
    d->pickGray->setIcon(QIcon::fromTheme(QLatin1String("color-picker-grey")));
    d->pickGray->setCheckable(true);
    d->pickGray->setToolTip( i18n( "All channels middle tone color picker" ) );
    d->pickGray->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                   "image used to set <b>Middle Tone</b> "
                                   "input levels on the Red, Green, Blue, and Luminosity channels."));

    d->pickWhite = new QToolButton();
    d->pickWhite->setIcon(QIcon::fromTheme(QLatin1String("color-picker-white")));
    d->pickWhite->setCheckable(true);
    d->pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    d->pickWhite->setWhatsThis(i18n("With this button, you can pick the color from the original "
                                    "image used to set <b>Highlight Tone</b> "
                                    "input levels on the Red, Green, Blue, and Luminosity channels."));

    d->pickerType = new QButtonGroup(d->pickerBox);
    d->pickerType->addButton(d->pickBlack, Private::BlackTonal);
    d->pickerType->addButton(d->pickGray,  Private::GrayTonal);
    d->pickerType->addButton(d->pickWhite, Private::WhiteTonal);

    QHBoxLayout* pickerBoxLayout = new QHBoxLayout;
    pickerBoxLayout->setContentsMargins(QMargins());
    pickerBoxLayout->setSpacing(0);
    pickerBoxLayout->addWidget(d->pickBlack);
    pickerBoxLayout->addWidget(d->pickGray);
    pickerBoxLayout->addWidget(d->pickWhite);
    d->pickerBox->setLayout(pickerBoxLayout);

    d->pickerType->setExclusive(true);

    // -------------------------------------------------------------

    d->autoButton = new QToolButton();
    d->autoButton->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    d->autoButton->setToolTip( i18n( "Adjust all levels automatically." ) );
    d->autoButton->setWhatsThis(i18n("If you press this button, all channel levels will be adjusted "
                                     "automatically."));

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip( i18n( "Reset current channel levels' values." ) );
    d->resetButton->setWhatsThis(i18n("If you press this button, all levels' values "
                                      "from the currently selected channel "
                                      "will be reset to the default values."));

    QLabel* const space   = new QLabel();
    space->setFixedWidth(d->gboxSettings->spacingHint());

    QHBoxLayout* const l3 = new QHBoxLayout();
    l3->addWidget(d->pickerBox);
    l3->addWidget(d->autoButton);
    l3->addWidget(space);
    l3->addWidget(d->resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    QGridLayout* const grid = new QGridLayout();
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
    grid->setContentsMargins(QMargins());
    grid->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------
    // Channels and scale selection slots.

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

    connect(d->pickerType, SIGNAL(buttonReleased(int)),
            this, SLOT(slotPickerColorButtonActived(int)));
}

AdjustLevelsTool::~AdjustLevelsTool()
{
    delete d->levels;
    delete d;
}

// See bug #146636: use event filter with all level slider to display a
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
    if (type == Private::NoPicker)
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
        if (channel != ColorChannels)
        {
            // Black tonal levels point.
            d->levels->levelsBlackToneAdjustByColors(channel, color);
        }
        else
        {
            for (int i = RedChannel; i <= BlueChannel; i++)
                d->levels->levelsBlackToneAdjustByColors(i, color);
        }
    }
    else if ( d->pickGray->isChecked() )
    {
        if (channel != ColorChannels)
        {
            // Gray tonal levels point.
            d->levels->levelsGrayToneAdjustByColors(channel, color);
        }
    }
    else if ( d->pickWhite->isChecked() )
    {
        if (channel != ColorChannels)
        {
            // White tonal levels point.
            d->levels->levelsWhiteToneAdjustByColors(channel, color);
        }
        else
        {
            for (int i = RedChannel; i <= BlueChannel; i++)
                d->levels->levelsWhiteToneAdjustByColors(i, color);
        }
    }
    else
    {
        d->levelsHistogramWidget->setHistogramGuideByColor(color);
        return;
    }

    d->pickerType->setExclusive(false);
    d->pickBlack->setChecked(false);
    d->pickGray->setChecked(false);
    d->pickWhite->setChecked(false);
    d->pickerType->setExclusive(true);

    // Refresh the current levels config.
    slotChannelChanged();

    d->previewWidget->setCapturePointMode(false);
    slotPreview();
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

    slotPreview();
}

void AdjustLevelsTool::slotAutoLevels()
{
    // Calculate Auto levels.
    d->levels->levelsAuto(d->levelsHistogramWidget->currentHistogram());

    // Refresh the current levels config.
    slotChannelChanged();

    slotPreview();
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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    {
        bool sb        = d->originalImage->sixteenBit();
        double gamma   = 0.0;
        int lowInput   = 0;
        int lowOutput  = 0;
        int highInput  = 0;
        int highOutput = 0;

        for (int i = 0 ; i < 5 ; ++i)
        {
            gamma      = group.readEntry(d->configGammaChannelEntry.arg(i),      1.0);
            lowInput   = group.readEntry(d->configLowInputChannelEntry.arg(i),   0);
            lowOutput  = group.readEntry(d->configLowOutputChannelEntry.arg(i),  0);
            highInput  = group.readEntry(d->configHighInputChannelEntry.arg(i),  65535);
            highOutput = group.readEntry(d->configHighOutputChannelEntry.arg(i), 65535);

            d->levels->setLevelGammaValue(i, gamma);
            d->levels->setLevelLowInputValue(i,   sb ? lowInput   : lowInput   / 256);
            d->levels->setLevelHighInputValue(i,  sb ? highInput  : highInput  / 256);
            d->levels->setLevelLowOutputValue(i,  sb ? lowOutput  : lowOutput  / 256);
            d->levels->setLevelHighOutputValue(i, sb ? highOutput : highOutput / 256);
        }
    }

    d->levelsHistogramWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();

    ChannelType ch = (ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel);

    // restore the previous channel
    d->gboxSettings->histogramBox()->setChannel(ch);

    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    // if ColorChannels was set, make sure to take values from LuminosityChannel
    if (ch == ColorChannels)
        ch = LuminosityChannel;

    // This is mandatory here to set spinbox values because slot connections
    // can be not set completely at tool startup.
    d->minInput->setValue(d->levels->getLevelLowInputValue(ch));
    d->minOutput->setValue(d->levels->getLevelLowOutputValue(ch));
    d->maxInput->setValue(d->levels->getLevelHighInputValue(ch));
    d->maxOutput->setValue(d->levels->getLevelHighOutputValue(ch));
    slotChannelChanged();
    slotScaleChanged();
}

void AdjustLevelsTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
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
            group.writeEntry(d->configLowInputChannelEntry.arg(i),   sb ? lowInput   : lowInput   * 256);
            group.writeEntry(d->configLowOutputChannelEntry.arg(i),  sb ? lowOutput  : lowOutput  * 256);
            group.writeEntry(d->configHighInputChannelEntry.arg(i),  sb ? highInput  : highInput  * 256);
            group.writeEntry(d->configHighOutputChannelEntry.arg(i), sb ? highOutput : highOutput * 256);
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

    slotPreview();
}

void AdjustLevelsTool::preparePreview()
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

void AdjustLevelsTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
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

    ImageIface iface;
    setFilter(new LevelsFilter(iface.original(), this, settings));
}

void AdjustLevelsTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Adjust Levels"), filter()->filterAction(), filter()->getTargetImage());
}

void AdjustLevelsTool::slotLoadSettings()
{
    QUrl loadLevelsFile;

    loadLevelsFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18n("Select Gimp Levels File to Load"),
                                                 QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                 QLatin1String("*"));

    if ( loadLevelsFile.isEmpty() )
    {
        return;
    }

    if ( d->levels->loadLevelsFromGimpLevelsFile( loadLevelsFile ) == false )
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load from the Gimp levels text file."));
        return;
    }

    // Refresh the current levels config.
    slotChannelChanged();

    slotPreview();
}

void AdjustLevelsTool::slotSaveAsSettings()
{
    QUrl saveLevelsFile;

    saveLevelsFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18n("Gimp Levels File to Save"),
                                                 QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                 QLatin1String("*"));

    if ( saveLevelsFile.isEmpty() )
    {
        return;
    }

    if ( d->levels->saveLevelsToGimpLevelsFile( saveLevelsFile ) == false )
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save to the Gimp levels text file."));
        return;
    }

    // Refresh the current levels config.
    slotChannelChanged();
}

}  // namespace Digikam
