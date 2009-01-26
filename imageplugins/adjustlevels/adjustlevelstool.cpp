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

// C++ includes.

#include <cmath>

// Qt includes.

#include <qcolor.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kselect.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "imagelevels.h"
#include "histogramwidget.h"
#include "dimgimagefilters.h"
#include "adjustlevelstool.h"
#include "adjustlevelstool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamAdjustLevelsImagesPlugin
{

AdjustLevelsTool::AdjustLevelsTool(QObject* parent)
                : EditorTool(parent)
{
    m_destinationPreviewData = 0;

    ImageIface iface(0, 0);
    m_originalImage = iface.getOriginalImg();

    m_histoSegments = m_originalImage->sixteenBit() ? 65535 : 255;
    m_levels        = new ImageLevels(m_originalImage->sixteenBit());

    setName("adjustlevels");
    setToolName(i18n("Adjust Levels"));
    setToolIcon(SmallIcon("adjustlevels"));

    // -------------------------------------------------------------

    m_previewWidget = new ImageWidget("adjustlevels Tool", 0,
                                      i18n("<p>Here you can see the image's "
                                           "level-adjustments preview. You can pick a spot on the image "
                                           "to see the corresponding level in the histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 20, 6);

    QLabel *label1 = new QLabel(i18n("Channel:"), m_gboxSettings->plainPage());
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, m_gboxSettings->plainPage() );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->insertItem( i18n("Alpha") );
    m_channelCB->setCurrentText( i18n("Luminosity") );
    QWhatsThis::add( m_channelCB, i18n("<p>Here select the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"
                                       "<b>Alpha</b>: display the alpha image-channel values. "
                                       "This channel corresponds to the transparency value and "
                                       "is supported by some image formats, such as PNG or TIF."));

    m_scaleBG = new QHButtonGroup(m_gboxSettings->plainPage());
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Here select the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "The Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    m_scaleBG->insert(linHistoButton, HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    m_scaleBG->insert(logHistoButton, HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(m_scaleBG);

    // -------------------------------------------------------------

    m_histogramWidget = new HistogramWidget(256, 140, m_gboxSettings->plainPage(), false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing of the "
                                             "selected image channel. This one is re-computed at any levels "
                                             "settings changes."));

    m_levelsHistogramWidget = new HistogramWidget(256, 140, m_originalImage->bits(), m_originalImage->width(),
                                                  m_originalImage->height(), m_originalImage->sixteenBit(), m_gboxSettings, false);
    QWhatsThis::add( m_levelsHistogramWidget, i18n("<p>This is the histogram drawing of the selected channel "
                                                   "from original image"));

    // -------------------------------------------------------------

    m_hGradientMinInput = new KGradientSelector( KSelector::Horizontal, m_gboxSettings->plainPage() );
    m_hGradientMinInput->setFixedHeight( 20 );
    m_hGradientMinInput->setMinValue(0);
    m_hGradientMinInput->setMaxValue(m_histoSegments);
    QWhatsThis::add( m_hGradientMinInput, i18n("<p>Select the minimal intensity input value of the histogram."));
    QToolTip::add( m_hGradientMinInput, i18n( "Minimal intensity input." ) );
    m_hGradientMinInput->setColors( QColor( "black" ), QColor( "white" ) );
    m_hGradientMinInput->installEventFilter(this);

    m_hGradientMaxInput = new KGradientSelector( KSelector::Horizontal, m_gboxSettings->plainPage() );
    m_hGradientMaxInput->setFixedHeight( 20 );
    m_hGradientMaxInput->setMinValue(0);
    m_hGradientMaxInput->setMaxValue(m_histoSegments);
    QWhatsThis::add( m_hGradientMaxInput, i18n("<p>Select the maximal intensity input value of the histogram."));
    QToolTip::add( m_hGradientMaxInput, i18n( "Maximal intensity input." ) );
    m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "white" ) );
    m_hGradientMaxInput->installEventFilter(this);

    m_minInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_minInput->input()->setRange(0, m_histoSegments, 1, false);
    m_minInput->setDefaultValue(0);
    QWhatsThis::add( m_minInput, i18n("<p>Select the minimal intensity input value of the histogram."));
    QToolTip::add( m_minInput, i18n( "Minimal intensity input." ) );

    m_gammaInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_gammaInput->setPrecision(2);
    m_gammaInput->setRange(0.1, 3.0, 0.01, true);
    m_gammaInput->setDefaultValue(1.0);
    QToolTip::add( m_gammaInput, i18n( "Gamma input value." ) );
    QWhatsThis::add( m_gammaInput, i18n("<p>Select the gamma input value."));

    m_maxInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_maxInput->input()->setRange(0, m_histoSegments, 1, false);
    m_maxInput->setDefaultValue(m_histoSegments);
    QToolTip::add( m_maxInput, i18n( "Maximal intensity input." ) );
    QWhatsThis::add( m_maxInput, i18n("<p>Select the maximal intensity input value of the histogram."));

    m_hGradientMinOutput = new KGradientSelector( KSelector::Horizontal, m_gboxSettings->plainPage() );
    m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "white" ) );
    QWhatsThis::add( m_hGradientMinOutput, i18n("<p>Select the minimal intensity output value of the histogram."));
    QToolTip::add( m_hGradientMinOutput, i18n( "Minimal intensity output." ) );
    m_hGradientMinOutput->setFixedHeight( 20 );
    m_hGradientMinOutput->setMinValue(0);
    m_hGradientMinOutput->setMaxValue(m_histoSegments);
    m_hGradientMinOutput->installEventFilter(this);

    m_hGradientMaxOutput = new KGradientSelector( KSelector::Horizontal, m_gboxSettings->plainPage() );
    m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "white" ) );
    QWhatsThis::add( m_hGradientMaxOutput, i18n("<p>Select the maximal intensity output value of the histogram."));
    QToolTip::add( m_hGradientMaxOutput, i18n( "Maximal intensity output." ) );
    m_hGradientMaxOutput->setFixedHeight( 20 );
    m_hGradientMaxOutput->setMinValue(0);
    m_hGradientMaxOutput->setMaxValue(m_histoSegments);
    m_hGradientMaxOutput->installEventFilter(this);

    m_minOutput = new RIntNumInput(m_gboxSettings->plainPage());
    m_minOutput->input()->setRange(0, m_histoSegments, 1, false);
    m_minOutput->setDefaultValue(0);
    QToolTip::add( m_minOutput, i18n( "Minimal intensity output." ) );
    QWhatsThis::add( m_minOutput, i18n("<p>Select the minimal intensity output value of the histogram."));

    m_maxOutput = new RIntNumInput(m_gboxSettings->plainPage());
    m_maxOutput->input()->setRange(0, m_histoSegments, 1, false);
    m_maxOutput->setDefaultValue(m_histoSegments);
    QToolTip::add( m_maxOutput, i18n( "Maximal intensity output." ) );
    QWhatsThis::add( m_maxOutput, i18n("<p>Select the maximal intensity output value of the histogram."));

    // -------------------------------------------------------------

    m_pickerColorButtonGroup = new QHButtonGroup(m_gboxSettings->plainPage());
    m_pickBlack              = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickBlack, BlackTonal);
    KGlobal::dirs()->addResourceType("color-picker-black", KGlobal::dirs()->kde_default("data") +
                                     "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-black", "color-picker-black.png");
    m_pickBlack->setPixmap( QPixmap( directory + "color-picker-black.png" ) );
    m_pickBlack->setToggleButton(true);
    QToolTip::add( m_pickBlack, i18n( "All channels shadow tone color picker" ) );
    QWhatsThis::add( m_pickBlack, i18n("<p>With this button, you can pick the color from original image used to set <b>Shadow Tone</b> "
                                       "levels input on Red, Green, Blue, and Luminosity channels."));
    m_pickGray  = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickGray, GrayTonal);
    KGlobal::dirs()->addResourceType("color-picker-gray", KGlobal::dirs()->kde_default("data") +
                                     "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-grey", "color-picker-grey.png");
    m_pickGray->setPixmap( QPixmap( directory + "color-picker-grey.png" ) );
    m_pickGray->setToggleButton(true);
    QToolTip::add( m_pickGray, i18n( "All channels middle tone color picker" ) );
    QWhatsThis::add( m_pickGray, i18n("<p>With this button, you can pick the color from original image used to set <b>Middle Tone</b> "
                                      "levels input on Red, Green, Blue, and Luminosity channels."));
    m_pickWhite = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickWhite, WhiteTonal);
    KGlobal::dirs()->addResourceType("color-picker-white", KGlobal::dirs()->kde_default("data") +
                                     "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-white", "color-picker-white.png");
    m_pickWhite->setPixmap( QPixmap( directory + "color-picker-white.png" ) );
    m_pickWhite->setToggleButton(true);
    QToolTip::add( m_pickWhite, i18n( "All channels highlight tone color picker" ) );
    QWhatsThis::add( m_pickWhite, i18n("<p>With this button, you can pick the color from original image used to set <b>Highlight Tone</b> "
                                       "levels input on Red, Green, Blue, and Luminosity channels."));
    m_pickerColorButtonGroup->setExclusive(true);
    m_pickerColorButtonGroup->setFrameShape(QFrame::NoFrame);

    m_autoButton = new QPushButton(m_gboxSettings->plainPage());
    m_autoButton->setPixmap(kapp->iconLoader()->loadIcon("run", (KIcon::Group)KIcon::Toolbar));    QToolTip::add( m_autoButton, i18n( "Adjust all levels automatically." ) );
    QWhatsThis::add( m_autoButton, i18n("<p>If you press this button, all channel levels will be adjusted "
                                        "automatically."));

    m_resetButton = new QPushButton(i18n("&Reset"), m_gboxSettings->plainPage());
    m_resetButton->setPixmap(kapp->iconLoader()->loadIcon("reload_page", (KIcon::Group)KIcon::Toolbar));
    QToolTip::add( m_resetButton, i18n( "Reset current channel levels' values." ) );
    QWhatsThis::add( m_resetButton, i18n("<p>If you press this button, all levels' values "
                                         "from the current selected channel "
                                         "will be reset to the default values."));

    QLabel *space = new QLabel(m_gboxSettings->plainPage());
    space->setFixedWidth(m_gboxSettings->spacingHint());

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(m_pickerColorButtonGroup);
    l3->addWidget(m_autoButton);
    l3->addWidget(space);
    l3->addWidget(m_resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    grid->addMultiCellLayout(l1,                      0, 0, 0, 6);
    grid->addMultiCellWidget(m_histogramWidget,       2, 2, 1, 5);
    grid->addMultiCellWidget(m_levelsHistogramWidget, 4, 4, 1, 5);
    grid->addMultiCellWidget(m_hGradientMinInput,     5, 5, 0, 6);
    grid->addMultiCellWidget(m_hGradientMaxInput,     7, 7, 0, 6);
    grid->addMultiCellWidget(m_minInput,              9, 9, 1, 1);
    grid->addMultiCellWidget(m_maxInput,              9, 9, 5, 5);
    grid->addMultiCellWidget(m_gammaInput,            11, 11, 0, 6);
    grid->addMultiCellWidget(m_hGradientMinOutput,    13, 13, 0, 6);
    grid->addMultiCellWidget(m_hGradientMaxOutput,    15, 15, 0, 6);
    grid->addMultiCellWidget(m_minOutput,             17, 17, 1, 1);
    grid->addMultiCellWidget(m_maxOutput,             17, 17, 5, 5);
    grid->addMultiCellLayout(l3,                      19, 19, 0, 6);
    grid->setRowStretch(20, 10);
    grid->setColStretch(3, 10);
    grid->setMargin(0);
    grid->setSpacing(5);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotSpotColorChanged(const Digikam::DColor&)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // Color sliders and spinbox slots.

    connect(m_hGradientMinInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMinInputSpinBox(int)));

    connect(m_minInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotGammaInputchanged(double)));

    connect(m_hGradientMaxInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMaxInputSpinBox(int)));

    connect(m_maxInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_hGradientMinOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMinOutputSpinBox(int)));

    connect(m_minOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_hGradientMaxOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMaxOutputSpinBox(int)));

    connect(m_maxOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustSliders()));

    // -------------------------------------------------------------
    // Bouttons slots.

    connect(m_autoButton, SIGNAL(clicked()),
            this, SLOT(slotAutoLevels()));

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(m_pickerColorButtonGroup, SIGNAL(released(int)),
            this, SLOT(slotPickerColorButtonActived()));
}

AdjustLevelsTool::~AdjustLevelsTool()
{
    delete [] m_destinationPreviewData;
    delete m_levels;
}

void AdjustLevelsTool::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    m_currentPreviewMode = m_previewWidget->getRenderingPreviewMode();
    m_previewWidget->setRenderingPreviewMode(ImageGuideWidget::PreviewOriginalImage);
}

void AdjustLevelsTool::slotSpotColorChanged(const DColor& color)
{
    if ( m_pickBlack->isOn() )
    {
       // Black tonal levels point.
       m_levels->levelsBlackToneAdjustByColors(m_channelCB->currentItem(), color);
       m_pickBlack->setOn(false);
    }
    else if ( m_pickGray->isOn() )
    {
       // Gray tonal levels point.
       m_levels->levelsGrayToneAdjustByColors(m_channelCB->currentItem(), color);
       m_pickGray->setOn(false);
    }
    else if ( m_pickWhite->isOn() )
    {
       // White tonal levels point.
       m_levels->levelsWhiteToneAdjustByColors(m_channelCB->currentItem(), color);
       m_pickWhite->setOn(false);
    }
    else
    {
       m_levelsHistogramWidget->setHistogramGuideByColor(color);
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());

    // restore previous rendering mode.
    m_previewWidget->setRenderingPreviewMode(m_currentPreviewMode);

    slotEffect();
}

void AdjustLevelsTool::slotColorSelectedFromTarget(const DColor& color)
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotGammaInputchanged(double val)
{
    blockSignals(true);
    m_levels->setLevelGammaValue(m_channelCB->currentItem(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMinInputSpinBox(int val)
{
    blockSignals(true);

    if ( val < m_hGradientMaxInput->value() )
       val = m_hGradientMaxInput->value();

    m_minInput->setValue(m_histoSegments - val);
    m_hGradientMinInput->setValue( val );
    m_levels->setLevelLowInputValue(m_channelCB->currentItem(), m_histoSegments - val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMaxInputSpinBox(int val)
{
    blockSignals(true);

    if ( val > m_hGradientMinInput->value() )
       val = m_hGradientMinInput->value();

    m_maxInput->setValue(m_histoSegments - val);
    m_hGradientMaxInput->setValue( val );
    m_levels->setLevelHighInputValue(m_channelCB->currentItem(), m_histoSegments - val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMinOutputSpinBox(int val)
{
    blockSignals(true);

    if ( val < m_hGradientMaxOutput->value() )
       val = m_hGradientMaxOutput->value();

    m_minOutput->setValue(m_histoSegments - val);
    m_hGradientMinOutput->setValue( val );
    m_levels->setLevelLowOutputValue(m_channelCB->currentItem(), m_histoSegments - val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelsTool::slotAdjustMaxOutputSpinBox(int val)
{
    blockSignals(true);

    if ( val > m_hGradientMinOutput->value() )
       val = m_hGradientMinOutput->value();

    m_maxOutput->setValue(m_histoSegments - val);
    m_hGradientMaxOutput->setValue( val );
    m_levels->setLevelHighOutputValue(m_channelCB->currentItem(), m_histoSegments - val);
    blockSignals(false);
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
    m_hGradientMinInput->setValue(m_histoSegments - minIn);
    m_hGradientMaxInput->setValue(m_histoSegments - maxIn);
    m_gammaInput->setValue(gamIn);
    m_hGradientMinOutput->setValue(m_histoSegments - minOut);
    m_hGradientMaxOutput->setValue(m_histoSegments - maxOut);
}

void AdjustLevelsTool::slotResetCurrentChannel()
{
    m_levels->levelsChannelReset(m_channelCB->currentItem());

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());
    m_levelsHistogramWidget->reset();

    slotEffect();
    m_histogramWidget->reset();
}

void AdjustLevelsTool::slotAutoLevels()
{
    // Calculate Auto levels.
    m_levels->levelsAuto(m_levelsHistogramWidget->m_imageHistogram);

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());

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
    m_histogramWidget->stopHistogramComputation();

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
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    delete [] orgData;
}

void AdjustLevelsTool::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
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

void AdjustLevelsTool::slotChannelChanged(int channel)
{
    switch(channel)
    {
       case LuminosityChannel:
          m_histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
          m_levelsHistogramWidget->m_channelType = HistogramWidget::ValueHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "white" ) );
          break;

       case RedChannel:
          m_histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
          m_levelsHistogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "red" ) );
          break;

       case GreenChannel:
          m_histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
          m_levelsHistogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "green" ) );
          break;

       case BlueChannel:
          m_histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
          m_levelsHistogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "blue" ) );
          break;

       case AlphaChannel:
          m_histogramWidget->m_channelType = HistogramWidget::AlphaChannelHistogram;
          m_levelsHistogramWidget->m_channelType = HistogramWidget::AlphaChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "white" ) );
          break;
    }

    adjustSliders(m_levels->getLevelLowInputValue(channel),
                  m_levels->getLevelGammaValue(channel),
                  m_levels->getLevelHighInputValue(channel),
                  m_levels->getLevelLowOutputValue(channel),
                  m_levels->getLevelHighOutputValue(channel));

    m_levelsHistogramWidget->repaint(false);
    m_histogramWidget->repaint(false);
}

void AdjustLevelsTool::slotScaleChanged(int scale)
{
    m_levelsHistogramWidget->m_scaleType = scale;
    m_histogramWidget->m_scaleType       = scale;
    m_histogramWidget->repaint(false);
    m_levelsHistogramWidget->repaint(false);
}

void AdjustLevelsTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("adjustlevels Tool");

    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));

    for (int i = 0 ; i < 5 ; i++)
    {
        bool sb        = m_originalImage->sixteenBit();
        int max        = sb ? 65535 : 255;
        double gamma   = config->readDoubleNumEntry(QString("GammaChannel%1").arg(i), 1.0);
        int lowInput   = config->readNumEntry(QString("LowInputChannel%1").arg(i), 0);
        int lowOutput  = config->readNumEntry(QString("LowOutputChannel%1").arg(i), 0);
        int highInput  = config->readNumEntry(QString("HighInputChannel%1").arg(i), max);
        int highOutput = config->readNumEntry(QString("HighOutputChannel%1").arg(i), max);

        m_levels->setLevelGammaValue(i, gamma);
        m_levels->setLevelLowInputValue(i, sb ? lowInput*255 : lowInput);
        m_levels->setLevelHighInputValue(i, sb ? highInput*255 : highInput);
        m_levels->setLevelLowOutputValue(i, sb ? lowOutput*255 : lowOutput);
        m_levels->setLevelHighOutputValue(i, sb ? highOutput*255 : highOutput);
    }

    m_levelsHistogramWidget->reset();
    m_histogramWidget->reset();
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());

    // This is mandatory here to set spinbox values because slot connections
    // can be not set completely at plugin startup.
    m_minInput->setValue(m_levels->getLevelLowInputValue(m_channelCB->currentItem()));
    m_minOutput->setValue(m_levels->getLevelLowOutputValue(m_channelCB->currentItem()));
    m_maxInput->setValue(m_levels->getLevelHighInputValue(m_channelCB->currentItem()));
    m_maxOutput->setValue(m_levels->getLevelHighOutputValue(m_channelCB->currentItem()));
}

void AdjustLevelsTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("adjustlevels Tool");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());

    for (int i = 0 ; i < 5 ; i++)
    {
        bool sb        = m_originalImage->sixteenBit();
        double gamma   = m_levels->getLevelGammaValue(i);
        int lowInput   = m_levels->getLevelLowInputValue(i);
        int lowOutput  = m_levels->getLevelLowOutputValue(i);
        int highInput  = m_levels->getLevelHighInputValue(i);
        int highOutput = m_levels->getLevelHighOutputValue(i);

        config->writeEntry(QString("GammaChannel%1").arg(i), gamma);
        config->writeEntry(QString("LowInputChannel%1").arg(i), sb ? lowInput/255 : lowInput);
        config->writeEntry(QString("LowOutputChannel%1").arg(i), sb ? lowOutput/255 : lowOutput);
        config->writeEntry(QString("HighInputChannel%1").arg(i), sb ? highInput/255 : highInput);
        config->writeEntry(QString("HighOutputChannel%1").arg(i), sb ? highOutput/255 : highOutput);
    }

    m_previewWidget->writeSettings();
    config->sync();
}

void AdjustLevelsTool::slotResetSettings()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
       m_levels->levelsChannelReset(channel);

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());
    m_levelsHistogramWidget->reset();
    slotEffect();
    m_histogramWidget->reset();
}

void AdjustLevelsTool::slotLoadSettings()
{
    KURL loadLevelsFile;

    loadLevelsFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Select Gimp Levels File to Load")) );
    if( loadLevelsFile.isEmpty() )
       return;

    if ( m_levels->loadLevelsFromGimpLevelsFile( loadLevelsFile ) == false )
    {
       KMessageBox::error(kapp->activeWindow(), i18n("Cannot load from the Gimp levels text file."));
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());
}

void AdjustLevelsTool::slotSaveAsSettings()
{
    KURL saveLevelsFile;

    saveLevelsFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Gimp Levels File to Save")) );
    if( saveLevelsFile.isEmpty() )
       return;

    if ( m_levels->saveLevelsToGimpLevelsFile( saveLevelsFile ) == false )
    {
       KMessageBox::error(kapp->activeWindow(), i18n("Cannot save to the Gimp levels text file."));
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());
}

// See B.K.O #146636: use event filter with all level slider to display a
// guide over level histogram.
bool AdjustLevelsTool::eventFilter(QObject *obj, QEvent *ev)
{
    if ( obj == m_hGradientMinInput )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(m_minInput, SIGNAL(valueChanged(int)),
                    this, SLOT(slotShowInputHistogramGuide(int)));

            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_minInput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowInputHistogramGuide(int)));

            m_levelsHistogramWidget->reset();
            return false;
        }
        else
        {
            return false;
        }
    }
    if ( obj == m_hGradientMaxInput )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(m_maxInput, SIGNAL(valueChanged(int)),
                    this, SLOT(slotShowInputHistogramGuide(int)));

            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_maxInput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowInputHistogramGuide(int)));

            m_levelsHistogramWidget->reset();
            return false;
        }
        else
        {
            return false;
        }
    }
    if ( obj == m_hGradientMinOutput )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(m_minOutput, SIGNAL(valueChanged(int)),
                    this, SLOT(slotShowOutputHistogramGuide(int)));

            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_minOutput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowOutputHistogramGuide(int)));

            m_histogramWidget->reset();
            return false;
        }
        else
        {
            return false;
        }
    }
    if ( obj == m_hGradientMaxOutput )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(m_maxOutput, SIGNAL(valueChanged(int)),
                    this, SLOT(slotShowOutputHistogramGuide(int)));

            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_maxOutput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowOutputHistogramGuide(int)));

            m_histogramWidget->reset();
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

void AdjustLevelsTool::slotShowInputHistogramGuide(int v)
{
    DColor color(v, v, v, v, m_originalImage->sixteenBit());
    m_levelsHistogramWidget->setHistogramGuideByColor(color);
}

void AdjustLevelsTool::slotShowOutputHistogramGuide(int v)
{
    DColor color(v, v, v, v, m_originalImage->sixteenBit());
    m_histogramWidget->setHistogramGuideByColor(color);
}

}  // NameSpace DigikamAdjustLevelsImagesPlugin
