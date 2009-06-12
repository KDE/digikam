/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "adjustlevelstool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QButtonGroup>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "daboutdata.h"
#include "dgradientslider.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagelevels.h"
#include "imagewidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamAdjustLevelsImagesPlugin
{

class AdjustLevelsToolPriv
{
public:

    AdjustLevelsToolPriv()
    {
        destinationPreviewData    = 0;
        histoSegments             = 0;
        currentPreviewMode        = 0;
        pickerBox                 = 0;
        resetButton               = 0;
        autoButton                = 0;
        pickBlack                 = 0;
        pickGray                  = 0;
        pickWhite                 = 0;
        pickerColorButtonGroup    = 0;
        minInput                  = 0;
        maxInput                  = 0;
        minOutput                 = 0;
        maxOutput                 = 0;
        gammaInput                = 0;
        levelsHistogramWidget     = 0;
        inputLevels               = 0;
        outputLevels              = 0;
        previewWidget             = 0;
        levels                    = 0;
        originalImage             = 0;
        gboxSettings              = 0;
    }

    uchar*               destinationPreviewData;

    int                  histoSegments;
    int                  currentPreviewMode;

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

    ImageWidget*         previewWidget;

    ImageLevels*         levels;

    DImg*                originalImage;

    EditorToolSettings*  gboxSettings;
};

AdjustLevelsTool::AdjustLevelsTool(QObject* parent)
                : EditorTool(parent),
                  d(new AdjustLevelsToolPriv)
{
    setObjectName("adjustlevels");
    setToolName(i18n("Adjust Levels"));
    setToolIcon(SmallIcon("adjustlevels"));

    d->destinationPreviewData = 0;

    ImageIface iface(0, 0);
    d->originalImage = iface.getOriginalImg();

    d->histoSegments = d->originalImage->sixteenBit() ? 65535 : 255;
    d->levels        = new ImageLevels(d->originalImage->sixteenBit());

    // -------------------------------------------------------------

    d->previewWidget = new ImageWidget("adjustlevels Tool", 0,
            i18n("Here you can see the image's "
                 "level-adjustments preview. You can pick a spot on the image "
                 "to see the corresponding level in the histogram."));

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram,
                                            HistogramBox::LRGBA);

    // we don't need to use the Gradient widget in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);


    d->levelsHistogramWidget = new HistogramWidget(256, 140, d->originalImage->bits(),
                                                             d->originalImage->width(),
                                                             d->originalImage->height(),
                                                             d->originalImage->sixteenBit(),
                                                             d->gboxSettings->plainPage(), false);
    d->levelsHistogramWidget->setWhatsThis( i18n("This is the histogram drawing of the selected channel "
                                                 "from the original image."));

    // -------------------------------------------------------------

    d->inputLevels = new DGradientSlider();
    d->inputLevels->setWhatsThis( i18n("Select the input intensity of the histogram here."));
    d->inputLevels->setToolTip( i18n( "Input intensity." ) );
    d->inputLevels->installEventFilter(this);

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
    d->pickerColorButtonGroup->addButton(d->pickBlack, BlackTonal);
    d->pickerColorButtonGroup->addButton(d->pickGray, GrayTonal);
    d->pickerColorButtonGroup->addButton(d->pickWhite, WhiteTonal);

    QHBoxLayout *pickerBoxLayout = new QHBoxLayout;
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

    QLabel *space = new QLabel();
    space->setFixedWidth(d->gboxSettings->spacingHint());

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(d->pickerBox);
    l3->addWidget(d->autoButton);
    l3->addWidget(space);
    l3->addWidget(d->resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    mainLayout->addWidget(d->levelsHistogramWidget, 0, 1, 1, 5);
    mainLayout->addWidget(d->inputLevels,           1, 0, 1, 7);
    mainLayout->addWidget(d->minInput,              2, 1, 1, 1);
    mainLayout->addWidget(d->maxInput,              2, 5, 1, 1);
    mainLayout->addWidget(d->gammaInput,            3, 0, 1, 7);
    mainLayout->addWidget(d->outputLevels,          4, 0, 1, 7);
    mainLayout->addWidget(d->minOutput,             5, 1, 1, 1);
    mainLayout->addWidget(d->maxOutput,             5, 5, 1, 1);
    mainLayout->addLayout(l3,                       6, 0, 1, 7);
    mainLayout->setRowStretch(7, 10);
    mainLayout->setColumnStretch(2, 10);
    mainLayout->setColumnStretch(4, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    // Channels and scale selection slots.

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotSpotColorChanged(const Digikam::DColor&)));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // Color sliders and spinbox slots.

    connect(d->inputLevels, SIGNAL(leftValueChanged(double)),
            this, SLOT(slotAdjustMinInputSpinBox(double)));

    connect(d->minInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(d->gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotGammaInputchanged(double)));

    connect(d->inputLevels, SIGNAL(rightValueChanged(double)),
            this, SLOT(slotAdjustMaxInputSpinBox(double)));

    connect(d->maxInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(d->outputLevels, SIGNAL(leftValueChanged(double)),
            this, SLOT(slotAdjustMinOutputSpinBox(double)));

    connect(d->minOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(d->outputLevels, SIGNAL(rightValueChanged(double)),
            this, SLOT(slotAdjustMaxOutputSpinBox(double)));

    connect(d->maxOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(d->autoButton, SIGNAL(clicked()),
            this, SLOT(slotAutoLevels()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->pickerColorButtonGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotPickerColorButtonActived()));
}

AdjustLevelsTool::~AdjustLevelsTool()
{
    delete [] d->destinationPreviewData;
    delete d;
}

void AdjustLevelsTool::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    d->currentPreviewMode = d->previewWidget->getRenderingPreviewMode();
    d->previewWidget->setRenderingPreviewMode(ImageGuideWidget::PreviewOriginalImage);
}

void AdjustLevelsTool::slotSpotColorChanged(const DColor& color)
{
    if ( d->pickBlack->isChecked() )
    {
       // Black tonal levels point.
       d->levels->levelsBlackToneAdjustByColors(d->gboxSettings->histogramBox()->channel(), color);
       d->pickBlack->setChecked(false);
    }
    else if ( d->pickGray->isChecked() )
    {
       // Gray tonal levels point.
       d->levels->levelsGrayToneAdjustByColors(d->gboxSettings->histogramBox()->channel(), color);
       d->pickGray->setChecked(false);
    }
    else if ( d->pickWhite->isChecked() )
    {
       // White tonal levels point.
       d->levels->levelsWhiteToneAdjustByColors(d->gboxSettings->histogramBox()->channel(), color);
       d->pickWhite->setChecked(false);
    }
    else
    {
       d->levelsHistogramWidget->setHistogramGuideByColor(color);
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged();

    // restore previous rendering mode.
    d->previewWidget->setRenderingPreviewMode(d->currentPreviewMode);

    slotEffect();
}

void AdjustLevelsTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotGammaInputchanged(double val)
{
    blockSignals(true);
    d->levels->setLevelGammaValue(d->gboxSettings->histogramBox()->channel(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMinInputSpinBox(double val)
{
    d->minInput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->minInput->setValue(newVal);
    d->levels->setLevelLowInputValue(d->gboxSettings->histogramBox()->channel(), newVal);
    d->minInput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMaxInputSpinBox(double val)
{
    d->maxInput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->maxInput->setValue(newVal);
    d->levels->setLevelHighInputValue(d->gboxSettings->histogramBox()->channel(), newVal);
    d->maxInput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMinOutputSpinBox(double val)
{
    d->minOutput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->minOutput->setValue(newVal);
    d->levels->setLevelLowOutputValue(d->gboxSettings->histogramBox()->channel(), newVal);
    d->minOutput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMaxOutputSpinBox(double val)
{
    d->maxOutput->blockSignals(true);
    int newVal = (int)(val*d->histoSegments);
    d->maxOutput->setValue(newVal);
    d->levels->setLevelHighOutputValue(d->gboxSettings->histogramBox()->channel(), newVal);
    d->maxOutput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustSliders()
{
    adjustSliders(d->minInput->value(), d->gammaInput->value(),
                  d->maxInput->value(), d->minOutput->value(),
                  d->maxOutput->value());
}

void AdjustLevelsTool::adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut)
{
    d->inputLevels->setLeftValue((double)minIn/(double)d->histoSegments);
    d->inputLevels->setRightValue((double)maxIn/(double)d->histoSegments);
    d->gammaInput->setValue(gamIn);
    d->outputLevels->setLeftValue((double)minOut/(double)d->histoSegments);
    d->outputLevels->setRightValue((double)maxOut/(double)d->histoSegments);
}

void AdjustLevelsTool::slotResetCurrentChannel()
{
    d->levels->levelsChannelReset(d->gboxSettings->histogramBox()->channel());

    // Refresh the current levels config.
    slotChannelChanged();
    d->levelsHistogramWidget->reset();

    slotEffect();
    d->gboxSettings->histogramBox()->histogram()->reset();
}

void AdjustLevelsTool::slotAutoLevels()
{
    // Calculate Auto levels.
    d->levels->levelsAuto(d->levelsHistogramWidget->m_imageHistogram);

    // Refresh the current levels config.
    slotChannelChanged();

    slotEffect();
}

void AdjustLevelsTool::slotEffect()
{
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *orgData    = iface->getPreviewImage();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sb           = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    d->destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    d->levels->levelsLutSetup(ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    d->levels->levelsLutProcess(orgData, d->destinationPreviewData, w, h);

    iface->putPreviewImage(d->destinationPreviewData);
    d->previewWidget->updatePreview();

    // Update histogram.
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);

    delete [] orgData;
}

void AdjustLevelsTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *orgData    = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    // Create the new empty destination image data space.
    uchar* desData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    d->levels->levelsLutSetup(ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    d->levels->levelsLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Level"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
}

void AdjustLevelsTool::slotChannelChanged()
{
    int channel = d->gboxSettings->histogramBox()->channel();
    switch (channel)
    {
        case EditorToolSettings::LuminosityChannel:
            d->levelsHistogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            d->inputLevels->setColors(QColor("black"), QColor("white"));
            d->inputLevels->setColors(QColor("black"), QColor("white"));
            d->outputLevels->setColors(QColor("black"), QColor("white"));
            d->outputLevels->setColors(QColor("black"), QColor("white"));
            break;

        case EditorToolSettings::RedChannel:
            d->levelsHistogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            d->inputLevels->setColors(QColor("black"), QColor("red"));
            d->inputLevels->setColors(QColor("black"), QColor("red"));
            d->outputLevels->setColors(QColor("black"), QColor("red"));
            d->outputLevels->setColors(QColor("black"), QColor("red"));
            break;

        case EditorToolSettings::GreenChannel:
            d->levelsHistogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            d->inputLevels->setColors(QColor("black"), QColor("green"));
            d->inputLevels->setColors(QColor("black"), QColor("green"));
            d->outputLevels->setColors(QColor("black"), QColor("green"));
            d->outputLevels->setColors(QColor("black"), QColor("green"));
            break;

        case EditorToolSettings::BlueChannel:
            d->levelsHistogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            d->inputLevels->setColors(QColor("black"), QColor("blue"));
            d->inputLevels->setColors(QColor("black"), QColor("blue"));
            d->outputLevels->setColors(QColor("black"), QColor("blue"));
            d->outputLevels->setColors(QColor("black"), QColor("blue"));
            break;

        case EditorToolSettings::AlphaChannel:
            d->levelsHistogramWidget->m_channelType = HistogramWidget::AlphaChannelHistogram;
            d->inputLevels->setColors(QColor("black"), QColor("white"));
            d->inputLevels->setColors(QColor("black"), QColor("white"));
            d->outputLevels->setColors(QColor("black"), QColor("white"));
            d->outputLevels->setColors(QColor("black"), QColor("white"));
            break;
    }

    adjustSliders(d->levels->getLevelLowInputValue(channel),
                  d->levels->getLevelGammaValue(channel),
                  d->levels->getLevelHighInputValue(channel),
                  d->levels->getLevelLowOutputValue(channel),
                  d->levels->getLevelHighOutputValue(channel));

    d->levelsHistogramWidget->repaint();
    d->gboxSettings->histogramBox()->slotChannelChanged();
}

void AdjustLevelsTool::slotScaleChanged()
{
    d->levelsHistogramWidget->m_scaleType = d->gboxSettings->histogramBox()->scale();
    d->levelsHistogramWidget->repaint();
}

void AdjustLevelsTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("adjustlevels Tool");

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
            gamma      = group.readEntry(QString("GammaChannel%1").arg(i), 1.0);
            lowInput   = group.readEntry(QString("LowInputChannel%1").arg(i), 0);
            lowOutput  = group.readEntry(QString("LowOutputChannel%1").arg(i), 0);
            highInput  = group.readEntry(QString("HighInputChannel%1").arg(i), max);
            highOutput = group.readEntry(QString("HighOutputChannel%1").arg(i), max);

            d->levels->setLevelGammaValue(i, gamma);
            d->levels->setLevelLowInputValue(i, sb ? lowInput*255 : lowInput);
            d->levels->setLevelHighInputValue(i, sb ? highInput*255 : highInput);
            d->levels->setLevelLowOutputValue(i, sb ? lowOutput*255 : lowOutput);
            d->levels->setLevelHighOutputValue(i, sb ? highOutput*255 : highOutput);
        }
    }

    d->levelsHistogramWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();

    d->gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                    (int)EditorToolSettings::LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                    (int)HistogramWidget::LogScaleHistogram));

    // This is mandatory here to set spinbox values because slot connections
    // can be not set completely at plugin startup.
    d->minInput->setValue(d->levels->getLevelLowInputValue(d->gboxSettings->histogramBox()->channel()));
    d->minOutput->setValue(d->levels->getLevelLowOutputValue(d->gboxSettings->histogramBox()->channel()));
    d->maxInput->setValue(d->levels->getLevelHighInputValue(d->gboxSettings->histogramBox()->channel()));
    d->maxOutput->setValue(d->levels->getLevelHighOutputValue(d->gboxSettings->histogramBox()->channel()));
    slotAdjustSliders();
}

void AdjustLevelsTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("adjustlevels Tool");
    group.writeEntry("Histogram Channel", d->gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", d->gboxSettings->histogramBox()->scale());

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

            group.writeEntry(QString("GammaChannel%1").arg(i), gamma);
            group.writeEntry(QString("LowInputChannel%1").arg(i), sb ? lowInput/255 : lowInput);
            group.writeEntry(QString("LowOutputChannel%1").arg(i), sb ? lowOutput/255 : lowOutput);
            group.writeEntry(QString("HighInputChannel%1").arg(i), sb ? highInput/255 : highInput);
            group.writeEntry(QString("HighOutputChannel%1").arg(i), sb ? highOutput/255 : highOutput);
        }
    }

    d->previewWidget->writeSettings();

    config->sync();
}

void AdjustLevelsTool::slotResetSettings()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
       d->levels->levelsChannelReset(channel);

    // Refresh the current levels config.
    slotChannelChanged();
    d->levelsHistogramWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();
}

void AdjustLevelsTool::slotLoadSettings()
{
    KUrl loadLevelsFile;

    loadLevelsFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Select Gimp Levels File to Load")) );
    if( loadLevelsFile.isEmpty() )
       return;

    if ( d->levels->loadLevelsFromGimpLevelsFile( loadLevelsFile ) == false )
    {
       KMessageBox::error(kapp->activeWindow(),
                          i18n("Cannot load from the Gimp levels text file."));
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged();
}

void AdjustLevelsTool::slotSaveAsSettings()
{
    KUrl saveLevelsFile;

    saveLevelsFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Gimp Levels File to Save")) );
    if( saveLevelsFile.isEmpty() )
       return;

    if ( d->levels->saveLevelsToGimpLevelsFile( saveLevelsFile ) == false )
    {
       KMessageBox::error(kapp->activeWindow(),
                          i18n("Cannot save to the Gimp levels text file."));
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged();
}

// See B.K.O #146636: use event filter with all level slider to display a
// guide over level histogram.
bool AdjustLevelsTool::eventFilter(QObject *obj, QEvent *ev)
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
        return EditorTool::eventFilter(obj, ev);
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

}  // namespace DigikamAdjustLevelsImagesPlugin
