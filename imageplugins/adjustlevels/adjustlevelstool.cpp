/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "version.h"
#include "daboutdata.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "imagelevels.h"
#include "dgradientslider.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamAdjustLevelsImagesPlugin
{

AdjustLevelsTool::AdjustLevelsTool(QObject* parent)
                : EditorTool(parent)
{
    setObjectName("adjustlevels");
    setToolName(i18n("Adjust Levels"));
    setToolIcon(SmallIcon("adjustlevels"));

    m_destinationPreviewData = 0;

    ImageIface iface(0, 0);
    m_originalImage = iface.getOriginalImg();

    m_histoSegments = m_originalImage->sixteenBit() ? 65535 : 255;
    m_levels        = new ImageLevels(m_originalImage->sixteenBit());


    // -------------------------------------------------------------

    m_previewWidget = new ImageWidget("adjustlevels Tool", 0,
            i18n("Here you can see the image's "
                 "level-adjustments preview. You can pick a spot on the image "
                 "to see the corresponding level in the histogram."));

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram,
                                            HistogramBox::LRGBA);

    // we don't need to use the Gradient widget in this tool
    m_gboxSettings->histogramBox()->setGradientVisible(false);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    m_levelsHistogramWidget = new HistogramWidget(256, 140, m_originalImage->bits(),
                                                            m_originalImage->width(),
                                                            m_originalImage->height(),
                                                            m_originalImage->sixteenBit(),
                                                            m_gboxSettings->plainPage(), false);
    m_levelsHistogramWidget->setWhatsThis( i18n("This is the histogram drawing of the selected channel "
                                                "from the original image."));

    // -------------------------------------------------------------

    m_inputLevels = new DGradientSlider(m_gboxSettings->plainPage());
    m_inputLevels->setWhatsThis( i18n("Select the input intensity of the histogram here."));
    m_inputLevels->setToolTip( i18n( "Input intensity." ) );
    m_inputLevels->installEventFilter(this);

    m_outputLevels = new DGradientSlider(m_gboxSettings->plainPage());
    m_outputLevels->setWhatsThis( i18n("Select the output intensity of the histogram here."));
    m_outputLevels->setToolTip( i18n( "Output intensity." ) );
    m_outputLevels->installEventFilter(this);

    m_minInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_minInput->setRange(0, m_histoSegments, 1);
    m_minInput->setSliderEnabled(false);
    m_minInput->setDefaultValue(0);
    m_minInput->setWhatsThis( i18n("Select the minimal input intensity value of the histogram here."));
    m_minInput->setToolTip( i18n( "Minimal input intensity." ) );

    m_gammaInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_gammaInput->setDecimals(2);
    m_gammaInput->setRange(0.1, 3.0, 0.01);
    m_gammaInput->setDefaultValue(1.0);
    m_gammaInput->setToolTip( i18n( "Gamma input value." ) );
    m_gammaInput->setWhatsThis( i18n("Select the gamma input value here."));

    m_maxInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_maxInput->setRange(0, m_histoSegments, 1);
    m_maxInput->setSliderEnabled(false);
    m_maxInput->setDefaultValue(m_histoSegments);
    m_maxInput->setToolTip( i18n( "Maximal input intensity." ) );
    m_maxInput->setWhatsThis( i18n("Select the maximal input intensity value of the histogram here."));

    m_minOutput = new RIntNumInput(m_gboxSettings->plainPage());
    m_minOutput->setRange(0, m_histoSegments, 1);
    m_minOutput->setSliderEnabled(false);
    m_minOutput->setDefaultValue(0);
    m_minOutput->setToolTip( i18n( "Minimal output intensity." ) );
    m_minOutput->setWhatsThis( i18n("Select the minimal output intensity value of the histogram here."));

    m_maxOutput = new RIntNumInput(m_gboxSettings->plainPage());
    m_maxOutput->setRange(0, m_histoSegments, 1);
    m_maxOutput->setSliderEnabled(false);
    m_maxOutput->setDefaultValue(m_histoSegments);
    m_maxOutput->setToolTip( i18n( "Maximal output intensity." ) );
    m_maxOutput->setWhatsThis( i18n("Select the maximal output intensity value of the histogram here."));

    // -------------------------------------------------------------

    m_pickerBox              = new QWidget(m_gboxSettings->plainPage());
    QHBoxLayout *hlay3       = new QHBoxLayout(m_pickerBox);
    m_pickerColorButtonGroup = new QButtonGroup(m_pickerBox);

    m_pickBlack = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickBlack, BlackTonal);
    m_pickBlack->setIcon(KIcon("color-picker-black"));
    m_pickBlack->setCheckable(true);
    m_pickBlack->setToolTip( i18n( "All channels shadow tone color picker" ) );
    m_pickBlack->setWhatsThis( i18n("With this button, you can pick the color from the original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "input levels on the Red, Green, Blue, and Luminosity channels."));

    m_pickGray  = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickGray, GrayTonal);
    m_pickGray->setIcon(KIcon("color-picker-grey"));
    m_pickGray->setCheckable(true);
    m_pickGray->setToolTip( i18n( "All channels middle tone color picker" ) );
    m_pickGray->setWhatsThis( i18n("With this button, you can pick the color from the original "
                                   "image used to set <b>Middle Tone</b> "
                                   "input levels on the Red, Green, Blue, and Luminosity channels."));

    m_pickWhite = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickWhite, WhiteTonal);
    m_pickWhite->setIcon(KIcon("color-picker-white"));
    m_pickWhite->setCheckable(true);
    m_pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    m_pickWhite->setWhatsThis( i18n("With this button, you can pick the color from the original "
                                    "image used to set <b>Highlight Tone</b> "
                                    "input levels on the Red, Green, Blue, and Luminosity channels."));

    hlay3->setMargin(0);
    hlay3->setSpacing(0);
    hlay3->addWidget(m_pickBlack);
    hlay3->addWidget(m_pickGray);
    hlay3->addWidget(m_pickWhite);

    m_pickerColorButtonGroup->setExclusive(true);

    // -------------------------------------------------------------

    m_autoButton = new QToolButton(m_gboxSettings->plainPage());
    m_autoButton->setIcon(KIconLoader::global()->loadIcon("system-run", KIconLoader::Toolbar));
    m_autoButton->setToolTip( i18n( "Adjust all levels automatically." ) );
    m_autoButton->setWhatsThis( i18n("If you press this button, all channel levels will be adjusted "
                                     "automatically."));

    m_resetButton = new QPushButton(i18n("&Reset"), m_gboxSettings->plainPage());
    m_resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    m_resetButton->setToolTip( i18n( "Reset current channel levels' values." ) );
    m_resetButton->setWhatsThis( i18n("If you press this button, all levels' values "
                                      "from the currently selected channel "
                                      "will be reset to the default values."));

    QLabel *space = new QLabel(m_gboxSettings->plainPage());
    space->setFixedWidth(m_gboxSettings->spacingHint());

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(m_pickerBox);
    l3->addWidget(m_autoButton);
    l3->addWidget(space);
    l3->addWidget(m_resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    grid->setMargin(0);
    grid->setSpacing(m_gboxSettings->spacingHint());
    grid->addWidget(m_levelsHistogramWidget, 0, 1, 1, 5);
    grid->addWidget(m_inputLevels,           1, 0, 1, 7);
    grid->addWidget(m_minInput,              2, 1, 1, 1);
    grid->addWidget(m_maxInput,              2, 5, 1, 1);
    grid->addWidget(m_gammaInput,            3, 0, 1, 7);
    grid->addWidget(m_outputLevels,          4, 0, 1, 7);
    grid->addWidget(m_minOutput,             5, 1, 1, 1);
    grid->addWidget(m_maxOutput,             5, 5, 1, 1);
    grid->addLayout(l3,                      6, 0, 1, 7);
    grid->setRowStretch(7, 10);
    grid->setColumnStretch(2, 10);
    grid->setColumnStretch(4, 10);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    // Channels and scale selection slots.

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotSpotColorChanged(const Digikam::DColor&)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // Color sliders and spinbox slots.

    connect(m_inputLevels, SIGNAL(leftValueChanged(double)),
            this, SLOT(slotAdjustMinInputSpinBox(double)));

    connect(m_minInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotGammaInputchanged(double)));

    connect(m_inputLevels, SIGNAL(rightValueChanged(double)),
            this, SLOT(slotAdjustMaxInputSpinBox(double)));

    connect(m_maxInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_outputLevels, SIGNAL(leftValueChanged(double)),
            this, SLOT(slotAdjustMinOutputSpinBox(double)));

    connect(m_minOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_outputLevels, SIGNAL(rightValueChanged(double)),
            this, SLOT(slotAdjustMaxOutputSpinBox(double)));

    connect(m_maxOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(m_autoButton, SIGNAL(clicked()),
            this, SLOT(slotAutoLevels()));

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(m_pickerColorButtonGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotPickerColorButtonActived()));
}

AdjustLevelsTool::~AdjustLevelsTool()
{
    delete [] m_destinationPreviewData;
}

void AdjustLevelsTool::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    m_currentPreviewMode = m_previewWidget->getRenderingPreviewMode();
    m_previewWidget->setRenderingPreviewMode(ImageGuideWidget::PreviewOriginalImage);
}

void AdjustLevelsTool::slotSpotColorChanged(const DColor &color)
{
    if ( m_pickBlack->isChecked() )
    {
       // Black tonal levels point.
       m_levels->levelsBlackToneAdjustByColors(m_gboxSettings->histogramBox()->channel(), color);
       m_pickBlack->setChecked(false);
    }
    else if ( m_pickGray->isChecked() )
    {
       // Gray tonal levels point.
       m_levels->levelsGrayToneAdjustByColors(m_gboxSettings->histogramBox()->channel(), color);
       m_pickGray->setChecked(false);
    }
    else if ( m_pickWhite->isChecked() )
    {
       // White tonal levels point.
       m_levels->levelsWhiteToneAdjustByColors(m_gboxSettings->histogramBox()->channel(), color);
       m_pickWhite->setChecked(false);
    }
    else
    {
       m_levelsHistogramWidget->setHistogramGuideByColor(color);
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged();

    // restore previous rendering mode.
    m_previewWidget->setRenderingPreviewMode(m_currentPreviewMode);

    slotEffect();
}

void AdjustLevelsTool::slotColorSelectedFromTarget( const DColor &color )
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotGammaInputchanged(double val)
{
    blockSignals(true);
    m_levels->setLevelGammaValue(m_gboxSettings->histogramBox()->channel(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMinInputSpinBox(double val)
{
    m_minInput->blockSignals(true);
    int newVal = (int)(val*m_histoSegments);
    m_minInput->setValue(newVal);
    m_levels->setLevelLowInputValue(m_gboxSettings->histogramBox()->channel(), newVal);
    m_minInput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMaxInputSpinBox(double val)
{
    m_maxInput->blockSignals(true);
    int newVal = (int)(val*m_histoSegments);
    m_maxInput->setValue(newVal);
    m_levels->setLevelHighInputValue(m_gboxSettings->histogramBox()->channel(), newVal);
    m_maxInput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMinOutputSpinBox(double val)
{
    m_minOutput->blockSignals(true);
    int newVal = (int)(val*m_histoSegments);
    m_minOutput->setValue(newVal);
    m_levels->setLevelLowOutputValue(m_gboxSettings->histogramBox()->channel(), newVal);
    m_minOutput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMaxOutputSpinBox(double val)
{
    m_maxOutput->blockSignals(true);
    int newVal = (int)(val*m_histoSegments);
    m_maxOutput->setValue(newVal);
    m_levels->setLevelHighOutputValue(m_gboxSettings->histogramBox()->channel(), newVal);
    m_maxOutput->blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustSliders()
{
    adjustSliders(m_minInput->value(), m_gammaInput->value(),
                  m_maxInput->value(), m_minOutput->value(),
                  m_maxOutput->value());
}

void AdjustLevelsTool::adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut)
{
    m_inputLevels->setLeftValue((double)minIn/(double)m_histoSegments);
    m_inputLevels->setRightValue((double)maxIn/(double)m_histoSegments);
    m_gammaInput->setValue(gamIn);
    m_outputLevels->setLeftValue((double)minOut/(double)m_histoSegments);
    m_outputLevels->setRightValue((double)maxOut/(double)m_histoSegments);
}

void AdjustLevelsTool::slotResetCurrentChannel()
{
    m_levels->levelsChannelReset(m_gboxSettings->histogramBox()->channel());

    // Refresh the current levels config.
    slotChannelChanged();
    m_levelsHistogramWidget->reset();

    slotEffect();
    m_gboxSettings->histogramBox()->histogram()->reset();
}

void AdjustLevelsTool::slotAutoLevels()
{
    // Calculate Auto levels.
    m_levels->levelsAuto(m_levelsHistogramWidget->m_imageHistogram);

    // Refresh the current levels config.
    slotChannelChanged();

    slotEffect();
}

void AdjustLevelsTool::slotEffect()
{
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData    = iface->getPreviewImage();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sb           = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    m_gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    m_destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_levels->levelsLutSetup(ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_levels->levelsLutProcess(orgData, m_destinationPreviewData, w, h);

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.
    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    delete [] orgData;
}

void AdjustLevelsTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData    = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    // Create the new empty destination image data space.
    uchar* desData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_levels->levelsLutSetup(ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_levels->levelsLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Level"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
}

void AdjustLevelsTool::slotChannelChanged()
{
    int channel = m_gboxSettings->histogramBox()->channel();
    switch (channel)
    {
        case EditorToolSettings::LuminosityChannel:
            m_levelsHistogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            m_inputLevels->setColors(QColor("black"), QColor("white"));
            m_inputLevels->setColors(QColor("black"), QColor("white"));
            m_outputLevels->setColors(QColor("black"), QColor("white"));
            m_outputLevels->setColors(QColor("black"), QColor("white"));
            break;

        case EditorToolSettings::RedChannel:
            m_levelsHistogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            m_inputLevels->setColors(QColor("black"), QColor("red"));
            m_inputLevels->setColors(QColor("black"), QColor("red"));
            m_outputLevels->setColors(QColor("black"), QColor("red"));
            m_outputLevels->setColors(QColor("black"), QColor("red"));
            break;

        case EditorToolSettings::GreenChannel:
            m_levelsHistogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            m_inputLevels->setColors(QColor("black"), QColor("green"));
            m_inputLevels->setColors(QColor("black"), QColor("green"));
            m_outputLevels->setColors(QColor("black"), QColor("green"));
            m_outputLevels->setColors(QColor("black"), QColor("green"));
            break;

        case EditorToolSettings::BlueChannel:
            m_levelsHistogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            m_inputLevels->setColors(QColor("black"), QColor("blue"));
            m_inputLevels->setColors(QColor("black"), QColor("blue"));
            m_outputLevels->setColors(QColor("black"), QColor("blue"));
            m_outputLevels->setColors(QColor("black"), QColor("blue"));
            break;

        case EditorToolSettings::AlphaChannel:
            m_levelsHistogramWidget->m_channelType = HistogramWidget::AlphaChannelHistogram;
            m_inputLevels->setColors(QColor("black"), QColor("white"));
            m_inputLevels->setColors(QColor("black"), QColor("white"));
            m_outputLevels->setColors(QColor("black"), QColor("white"));
            m_outputLevels->setColors(QColor("black"), QColor("white"));
            break;
    }

    adjustSliders(m_levels->getLevelLowInputValue(channel),
                  m_levels->getLevelGammaValue(channel),
                  m_levels->getLevelHighInputValue(channel),
                  m_levels->getLevelLowOutputValue(channel),
                  m_levels->getLevelHighOutputValue(channel));

    m_levelsHistogramWidget->repaint();
    m_gboxSettings->histogramBox()->slotChannelChanged();
}

void AdjustLevelsTool::slotScaleChanged()
{
    m_levelsHistogramWidget->m_scaleType = m_gboxSettings->histogramBox()->scale();
    m_levelsHistogramWidget->repaint();
}

void AdjustLevelsTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("adjustlevels Tool");

    for (int i = 0 ; i < 5 ; ++i)
    {
        bool sb        = m_originalImage->sixteenBit();
        int max        = sb ? 65535 : 255;
        double gamma   = group.readEntry(QString("GammaChannel%1").arg(i), 1.0);
        int lowInput   = group.readEntry(QString("LowInputChannel%1").arg(i), 0);
        int lowOutput  = group.readEntry(QString("LowOutputChannel%1").arg(i), 0);
        int highInput  = group.readEntry(QString("HighInputChannel%1").arg(i), max);
        int highOutput = group.readEntry(QString("HighOutputChannel%1").arg(i), max);

        m_levels->setLevelGammaValue(i, gamma);
        m_levels->setLevelLowInputValue(i, sb ? lowInput*255 : lowInput);
        m_levels->setLevelHighInputValue(i, sb ? highInput*255 : highInput);
        m_levels->setLevelLowOutputValue(i, sb ? lowOutput*255 : lowOutput);
        m_levels->setLevelHighOutputValue(i, sb ? highOutput*255 : highOutput);
    }

    m_levelsHistogramWidget->reset();
    m_gboxSettings->histogramBox()->histogram()->reset();

    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                    (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                    (int)HistogramWidget::LogScaleHistogram));

    // This is mandatory here to set spinbox values because slot connections
    // can be not set completely at plugin startup.
    m_minInput->setValue(m_levels->getLevelLowInputValue(m_gboxSettings->histogramBox()->channel()));
    m_minOutput->setValue(m_levels->getLevelLowOutputValue(m_gboxSettings->histogramBox()->channel()));
    m_maxInput->setValue(m_levels->getLevelHighInputValue(m_gboxSettings->histogramBox()->channel()));
    m_maxOutput->setValue(m_levels->getLevelHighOutputValue(m_gboxSettings->histogramBox()->channel()));
    slotAdjustSliders();
}

void AdjustLevelsTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("adjustlevels Tool");
    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());

    for (int i = 0 ; i < 5 ; ++i)
    {
        bool sb        = m_originalImage->sixteenBit();
        double gamma   = m_levels->getLevelGammaValue(i);
        int lowInput   = m_levels->getLevelLowInputValue(i);
        int lowOutput  = m_levels->getLevelLowOutputValue(i);
        int highInput  = m_levels->getLevelHighInputValue(i);
        int highOutput = m_levels->getLevelHighOutputValue(i);

        group.writeEntry(QString("GammaChannel%1").arg(i), gamma);
        group.writeEntry(QString("LowInputChannel%1").arg(i), sb ? lowInput/255 : lowInput);
        group.writeEntry(QString("LowOutputChannel%1").arg(i), sb ? lowOutput/255 : lowOutput);
        group.writeEntry(QString("HighInputChannel%1").arg(i), sb ? highInput/255 : highInput);
        group.writeEntry(QString("HighOutputChannel%1").arg(i), sb ? highOutput/255 : highOutput);
    }

    m_previewWidget->writeSettings();

    config->sync();
}

void AdjustLevelsTool::slotResetSettings()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
       m_levels->levelsChannelReset(channel);

    // Refresh the current levels config.
    slotChannelChanged();
    m_levelsHistogramWidget->reset();
    m_gboxSettings->histogramBox()->histogram()->reset();
}

void AdjustLevelsTool::slotLoadSettings()
{
    KUrl loadLevelsFile;

    loadLevelsFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Select Gimp Levels File to Load")) );
    if( loadLevelsFile.isEmpty() )
       return;

    if ( m_levels->loadLevelsFromGimpLevelsFile( loadLevelsFile ) == false )
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

    if ( m_levels->saveLevelsToGimpLevelsFile( saveLevelsFile ) == false )
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
    if ( obj == m_inputLevels )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(m_inputLevels, SIGNAL(leftValueChanged(double)),
                    this, SLOT(slotShowInputHistogramGuide(double)));

            connect(m_inputLevels, SIGNAL(rightValueChanged(double)),
                    this, SLOT(slotShowInputHistogramGuide(double)));

            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_inputLevels, SIGNAL(leftValueChanged(double)),
                       this, SLOT(slotShowInputHistogramGuide(double)));

            disconnect(m_inputLevels, SIGNAL(rightValueChanged(double)),
                       this, SLOT(slotShowInputHistogramGuide(double)));

            m_levelsHistogramWidget->reset();
            return false;
        }
        else
        {
            return false;
        }
    }
    if ( obj == m_outputLevels )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(m_outputLevels, SIGNAL(leftValueChanged(double)),
                    this, SLOT(slotShowOutputHistogramGuide(double)));

            connect(m_outputLevels, SIGNAL(rightValueChanged(double)),
                    this, SLOT(slotShowOutputHistogramGuide(double)));

            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_outputLevels, SIGNAL(leftValueChanged(double)),
                       this, SLOT(slotShowOutputHistogramGuide(double)));

            disconnect(m_outputLevels, SIGNAL(rightValueChanged(double)),
                       this, SLOT(slotShowOutputHistogramGuide(double)));

            m_gboxSettings->histogramBox()->histogram()->reset();
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
    int val = (int)(v * m_histoSegments);
    DColor color(val, val, val, val, m_originalImage->sixteenBit());
    m_levelsHistogramWidget->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotShowOutputHistogramGuide(double v)
{
    int val = (int)(v * m_histoSegments);
    DColor color(val, val, val, val, m_originalImage->sixteenBit());
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

}  // namespace DigikamAdjustLevelsImagesPlugin
