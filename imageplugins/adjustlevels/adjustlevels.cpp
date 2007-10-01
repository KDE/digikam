/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QButtonGroup> 
#include <QPixmap>
#include <QHBoxLayout>
#include <QGridLayout>

// KDE includes.

#include <kconfig.h>
#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kselector.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "imagelevels.h"
#include "histogramwidget.h"
#include "dimgimagefilters.h"
#include "adjustlevels.h"
#include "adjustlevels.moc"

namespace DigikamAdjustLevelsImagesPlugin
{

AdjustLevelDialog::AdjustLevelDialog(QWidget* parent)
                 : Digikam::ImageDlgBase(parent, i18n("Adjust Color Levels"), 
                   "adjustlevels", true, false)
{
    m_destinationPreviewData = 0L;

    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    m_originalImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;

    m_histoSegments = m_originalImage.sixteenBit() ? 65535 : 255;
    m_levels = new Digikam::ImageLevels(m_originalImage.sixteenBit());

    // About data and help button.

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Adjust Color Levels"),
                                       digikam_version,
                                       ki18n("An image-histogram-levels adjustment plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2007, Gilles Caulier"),
                                       KLocalizedString(),
                                       "http://www.digikam.org");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    setAboutData(about);

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("adjustlevels Tool Dialog", mainWidget(),
                                               i18n("<p>Here you can see the image's "
                                                    "level-adjustments preview. You can pick a spot on the image "
                                                    "to see the corresponding level in the histogram."));
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(mainWidget());
    QGridLayout* grid     = new QGridLayout(gboxSettings);

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( gboxSettings );
    m_channelCB->addItem( i18n("Luminosity") );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->addItem( i18n("Alpha") );
    m_channelCB->setCurrentIndex( 0 );
    m_channelCB->setWhatsThis( i18n("<p>Select here the histogram channel to display:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"
                                    "<b>Alpha</b>: display the alpha image-channel values. "
                                    "This channel corresponds to the transparency value and "
                                    "is supported by some image formats, such as PNG or TIF."));

    // -------------------------------------------------------------

    QWidget *scaleBox = new QWidget(gboxSettings);
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    m_scaleBG         = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select here the histogram scale.<p>"
                                "If the image's maximal counts are small, you can use the linear scale.<p>"
                                "Logarithmic scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph."));
    
    QPushButton *linHistoButton = new QPushButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/histogram-lin.png")));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    
    QPushButton *logHistoButton = new QPushButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/histogram-log.png")));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    
    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(linHistoButton);
    hlay->addWidget(logHistoButton);

    m_scaleBG->setExclusive(true);
    logHistoButton->setChecked(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(scaleBox);

    // -------------------------------------------------------------

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, gboxSettings, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram "
                                          "drawing of the selected image channel. This one is "
                                          "re-computed at any levels settings changes."));
    
    m_levelsHistogramWidget = new Digikam::HistogramWidget(256, 140, m_originalImage.bits(),
                                                           m_originalImage.width(), m_originalImage.height(),
                                                           m_originalImage.sixteenBit(), gboxSettings, false);
    m_levelsHistogramWidget->setWhatsThis( i18n("<p>This is the histogram drawing of the selected channel "
                                                "from original image"));

    // -------------------------------------------------------------
    
    m_hGradientMinInput = new KGradientSelector( Qt::Horizontal, gboxSettings );
    m_hGradientMinInput->setIndent(false);
    m_hGradientMinInput->setFixedHeight( 16 );
    m_hGradientMinInput->setMinimum(0);
    m_hGradientMinInput->setMaximum(m_histoSegments);
    m_hGradientMinInput->setWhatsThis( i18n("<p>Select here the minimal intensity "
                                            "input value of the histogram."));
    m_hGradientMinInput->setToolTip( i18n( "Minimal intensity input." ) );
    m_hGradientMinInput->setColors( QColor( "black" ), QColor( "white" ) );
    m_hGradientMinInput->installEventFilter(this);

    m_hGradientMaxInput = new KGradientSelector( Qt::Horizontal, gboxSettings );
    m_hGradientMaxInput->setIndent(false);
    m_hGradientMaxInput->setFixedHeight( 16 );
    m_hGradientMaxInput->setMinimum(0);
    m_hGradientMaxInput->setMaximum(m_histoSegments);
    m_hGradientMaxInput->setWhatsThis( i18n("<p>Select here the maximal intensity input "
                                            "value of the histogram."));
    m_hGradientMaxInput->setToolTip( i18n( "Maximal intensity input." ) );
    m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "white" ) );
    m_hGradientMaxInput->installEventFilter(this);

    m_minInput = new QSpinBox(gboxSettings);
    m_minInput->setRange(0, m_histoSegments);
    m_minInput->setSingleStep(1);
    m_minInput->setValue(0);
    m_minInput->setWhatsThis( i18n("<p>Select here the minimal intensity input "
                                   "value of the histogram."));
    m_minInput->setToolTip( i18n( "Minimal intensity input." ) );

    m_gammaInput = new KDoubleNumInput(gboxSettings);
    m_gammaInput->setPrecision(2);
    m_gammaInput->setRange(0.1, 3.0, 0.01);
    m_gammaInput->setValue(1.0);
    m_gammaInput->setToolTip( i18n( "Gamma input value." ) );
    m_gammaInput->setWhatsThis( i18n("<p>Select here the gamma input value."));

    m_maxInput = new QSpinBox(gboxSettings);
    m_maxInput->setRange(0, m_histoSegments);
    m_maxInput->setSingleStep(1);
    m_maxInput->setValue(m_histoSegments);
    m_maxInput->setToolTip( i18n( "Maximal intensity input." ) );
    m_maxInput->setWhatsThis( i18n("<p>Select here the maximal intensity input "
                                   "value of the histogram."));

    m_hGradientMinOutput = new KGradientSelector( Qt::Horizontal, gboxSettings );
    m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "white" ) );
    m_hGradientMinOutput->setWhatsThis(i18n("<p>Select here the minimal intensity output "
                                             "value of the histogram."));    
    m_hGradientMinOutput->setToolTip( i18n( "Minimal intensity output." ) );
    m_hGradientMinOutput->setIndent(false);
    m_hGradientMinOutput->setFixedHeight( 16 );
    m_hGradientMinOutput->setMinimum(0);
    m_hGradientMinOutput->setMaximum(m_histoSegments);
    m_hGradientMinOutput->installEventFilter(this);

    m_hGradientMaxOutput = new KGradientSelector( Qt::Horizontal, gboxSettings );
    m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "white" ) );
    m_hGradientMaxOutput->setWhatsThis(i18n("<p>Select here the maximal intensity output "
                                            "value of the histogram."));
    m_hGradientMaxOutput->setToolTip( i18n( "Maximal intensity output." ) );
    m_hGradientMaxOutput->setIndent(false);
    m_hGradientMaxOutput->setFixedHeight( 16 );
    m_hGradientMaxOutput->setMinimum(0);
    m_hGradientMaxOutput->setMaximum(m_histoSegments);
    m_hGradientMaxOutput->installEventFilter(this);

    m_minOutput = new QSpinBox(gboxSettings);
    m_minOutput->setRange(0, m_histoSegments);
    m_minOutput->setSingleStep(1);
    m_minOutput->setValue(0);
    m_minOutput->setToolTip( i18n( "Minimal intensity output." ) );
    m_minOutput->setWhatsThis( i18n("<p>Select here the minimal intensity output "
                                    "value of the histogram."));

    m_maxOutput = new QSpinBox(gboxSettings);
    m_maxOutput->setRange(0, m_histoSegments);
    m_maxOutput->setSingleStep(1);
    m_maxOutput->setValue(m_histoSegments);
    m_maxOutput->setToolTip( i18n( "Maximal intensity output." ) );
    m_maxOutput->setWhatsThis( i18n("<p>Select here the maximal intensity output "
                                    "value of the histogram."));

    // -------------------------------------------------------------

    m_pickerBox              = new QWidget(gboxSettings);
    QHBoxLayout *hlay3       = new QHBoxLayout(m_pickerBox);
    m_pickerColorButtonGroup = new QButtonGroup(m_pickerBox);

    m_pickBlack = new QPushButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickBlack, BlackTonal);
    m_pickBlack->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/color-picker-black.png")));
    m_pickBlack->setCheckable(true);
    m_pickBlack->setToolTip( i18n( "All channels shadow tone color picker" ) );
    m_pickBlack->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "levels input on Red, Green, Blue, and Luminosity channels."));

    m_pickGray  = new QPushButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickGray, GrayTonal);
    m_pickGray->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/color-picker-grey.png")));
    m_pickGray->setCheckable(true);
    m_pickGray->setToolTip( i18n( "All channels middle tone color picker" ) );
    m_pickGray->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                   "image used to set <b>Middle Tone</b> "
                                   "levels input on Red, Green, Blue, and Luminosity channels."));

    m_pickWhite = new QPushButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickWhite, WhiteTonal);
    m_pickWhite->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/color-picker-white.png")));
    m_pickWhite->setCheckable(true);
    m_pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    m_pickWhite->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                    "image used to set <b>Highlight Tone</b> "
                                    "levels input on Red, Green, Blue, and Luminosity channels."));

    hlay3->setMargin(0);
    hlay3->setSpacing(0);
    hlay3->addWidget(m_pickBlack);
    hlay3->addWidget(m_pickGray);
    hlay3->addWidget(m_pickWhite);

    m_pickerColorButtonGroup->setExclusive(true);

    // -------------------------------------------------------------

    m_autoButton = new QPushButton(gboxSettings);
    m_autoButton->setIcon(KIconLoader::global()->loadIcon("gear", KIconLoader::Toolbar));    
    m_autoButton->setToolTip( i18n( "Adjust all levels automatically." ) );
    m_autoButton->setWhatsThis( i18n("<p>If you press this button, all channel levels will be adjusted "
                                     "automatically."));

    m_resetButton = new QPushButton(i18n("&Reset"), gboxSettings);
    m_resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));     
    m_resetButton->setToolTip( i18n( "Reset current channel levels' values." ) );
    m_resetButton->setWhatsThis( i18n("<p>If you press this button, all levels' values "
                                      "from the current selected channel "
                                      "will be reset to the default values."));

    QLabel *space = new QLabel(gboxSettings);
    space->setFixedWidth(spacingHint());    

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(m_pickerBox);
    l3->addWidget(m_autoButton);
    l3->addWidget(space);
    l3->addWidget(m_resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------
    
    grid->addLayout(l1, 0, 0, 1, 6+1);
    grid->setRowMinimumHeight(1, spacingHint());
    grid->addWidget(m_histogramWidget, 2, 1, 1, 5);
    grid->setRowMinimumHeight(3, spacingHint());
    grid->addWidget(m_levelsHistogramWidget, 4, 1, 1, 5);
    grid->addWidget(m_hGradientMinInput, 5, 0, 1, 6+1);
    grid->addWidget(m_minInput, 5, 8, 1, 1);
    grid->setRowMinimumHeight(6, spacingHint());
    grid->addWidget(m_hGradientMaxInput, 7, 0, 1, 6+1);
    grid->addWidget(m_maxInput, 7, 8, 1, 1);
    grid->setRowMinimumHeight(8, spacingHint());
    grid->addWidget(m_gammaInput, 9, 0, 1, 8+1);
    grid->setRowMinimumHeight(10, spacingHint());
    grid->addWidget(m_hGradientMinOutput, 11, 0, 1, 6+1);
    grid->addWidget(m_minOutput, 11, 8, 1, 1);
    grid->setRowMinimumHeight(12, spacingHint());
    grid->addWidget(m_hGradientMaxOutput, 13, 0, 1, 6+1);
    grid->addWidget(m_maxOutput, 13, 8, 1, 1);
    grid->setRowMinimumHeight(14, spacingHint());
    grid->addLayout(l3, 15, 0, 1, 8+1);
    grid->setRowStretch(16, 10);
    grid->setColumnStretch(2, 10);
    grid->setColumnMinimumWidth(0, 5);
    grid->setColumnMinimumWidth(6, 5);
    grid->setColumnMinimumWidth(7, spacingHint());
    grid->setMargin(spacingHint());
    grid->setSpacing(0);
    
    setUserAreaWidget(gboxSettings);    
                    
    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));
            
    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const Digikam::DColor & )));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));
            
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));                                                            
                        
    // -------------------------------------------------------------
    // Color sliders and spinbox slots.

    connect(m_hGradientMinInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMinInputSpinBox(int)));

    connect(m_minInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_gammaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotGammaInputchanged(double)));

    connect(m_hGradientMaxInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMaxInputSpinBox(int)));

    connect(m_maxInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_hGradientMinOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMinOutputSpinBox(int)));

    connect(m_minOutput, SIGNAL(valueChanged (int)),
            this, SLOT(slotAdjustSliders()));

    connect(m_hGradientMaxOutput, SIGNAL(valueChanged(int)),
            this, SLOT(slotAdjustMaxOutputSpinBox(int)));

    connect(m_maxOutput, SIGNAL(valueChanged (int)),
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

AdjustLevelDialog::~AdjustLevelDialog()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_levelsHistogramWidget;
    delete m_levels;
}

void AdjustLevelDialog::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    m_currentPreviewMode = m_previewWidget->getRenderingPreviewMode();
    m_previewWidget->setRenderingPreviewMode(Digikam::ImageGuideWidget::PreviewOriginalImage);
}

void AdjustLevelDialog::slotSpotColorChanged(const Digikam::DColor &color)
{
    if ( m_pickBlack->isChecked() )
    {
       // Black tonal levels point.
       m_levels->levelsBlackToneAdjustByColors(m_channelCB->currentIndex(), color);      
       m_pickBlack->setChecked(false);
    }
    else if ( m_pickGray->isChecked() )
    {
       // Gray tonal levels point.
       m_levels->levelsGrayToneAdjustByColors(m_channelCB->currentIndex(), color);      
       m_pickGray->setChecked(false);
    }
    else if ( m_pickWhite->isChecked() )
    {
       // White tonal levels point.
       m_levels->levelsWhiteToneAdjustByColors(m_channelCB->currentIndex(), color);      
       m_pickWhite->setChecked(false);
    }
    else
    {
       m_levelsHistogramWidget->setHistogramGuideByColor(color);
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentIndex());
       
    // restore previous rendering mode.
    m_previewWidget->setRenderingPreviewMode(m_currentPreviewMode);
              
    slotEffect();                
}

void AdjustLevelDialog::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void AdjustLevelDialog::slotGammaInputchanged(double val)
{
    blockSignals(true);
    m_levels->setLevelGammaValue(m_channelCB->currentIndex(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelDialog::slotAdjustMinInputSpinBox(int val)
{
    blockSignals(true);

    if ( val > m_hGradientMaxInput->value() )
       val = m_hGradientMaxInput->value();

    m_minInput->setValue(val);
    m_hGradientMinInput->setValue( val );
    m_levels->setLevelLowInputValue(m_channelCB->currentIndex(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelDialog::slotAdjustMaxInputSpinBox(int val)
{
    blockSignals(true);

    if ( val < m_hGradientMinInput->value() )
       val = m_hGradientMinInput->value();

    m_maxInput->setValue(val);
    m_hGradientMaxInput->setValue( val );
    m_levels->setLevelHighInputValue(m_channelCB->currentIndex(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelDialog::slotAdjustMinOutputSpinBox(int val)
{
    blockSignals(true);

    if ( val > m_hGradientMaxOutput->value() )
       val = m_hGradientMaxOutput->value();

    m_minOutput->setValue(val);
    m_hGradientMinOutput->setValue( val );
    m_levels->setLevelLowOutputValue(m_channelCB->currentIndex(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelDialog::slotAdjustMaxOutputSpinBox(int val)
{
    blockSignals(true);

    if ( val < m_hGradientMinOutput->value() )
       val = m_hGradientMinOutput->value();

    m_maxOutput->setValue(val);
    m_hGradientMaxOutput->setValue( val );
    m_levels->setLevelHighOutputValue(m_channelCB->currentIndex(), val);
    blockSignals(false);
    slotTimer();
}

void AdjustLevelDialog::slotAdjustSliders()
{
    adjustSliders(m_minInput->value(), m_gammaInput->value(),
                  m_maxInput->value(), m_minOutput->value(),
                  m_maxOutput->value());
}

void AdjustLevelDialog::adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut)
{
    m_hGradientMinInput->setValue(minIn);
    m_hGradientMaxInput->setValue(maxIn);
    m_gammaInput->setValue(gamIn);
    m_hGradientMinOutput->setValue(minOut);
    m_hGradientMaxOutput->setValue(maxOut);
}

void AdjustLevelDialog::slotResetCurrentChannel()
{
    m_levels->levelsChannelReset(m_channelCB->currentIndex());

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentIndex());
    m_levelsHistogramWidget->reset();

    slotEffect();
    m_histogramWidget->reset();
}

void AdjustLevelDialog::slotAutoLevels()
{
    // Calculate Auto levels.
    m_levels->levelsAuto(m_levelsHistogramWidget->m_imageHistogram);

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentIndex());

    slotEffect();
}

void AdjustLevelDialog::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData             = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();
    
    // Create the new empty destination image data space.
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
    
    m_destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_levels->levelsLutSetup(Digikam::ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_levels->levelsLutProcess(orgData, m_destinationPreviewData, w, h);

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    
    delete [] orgData;
}

void AdjustLevelDialog::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData             = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    // Create the new empty destination image data space.
    uchar* desData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_levels->levelsLutSetup(Digikam::ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_levels->levelsLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Level"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
    accept();
}

void AdjustLevelDialog::slotChannelChanged(int channel)
{
    switch(channel)
    {
       case LuminosityChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
          m_levelsHistogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "white" ) );
          break;
       
       case RedChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;       
          m_levelsHistogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "red" ) );
          break;

       case GreenChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;       
          m_levelsHistogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "green" ) );
          break;

       case BlueChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_levelsHistogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "blue" ) );
          break;

       case AlphaChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;       
          m_levelsHistogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
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

    m_levelsHistogramWidget->repaint();
    m_histogramWidget->repaint();    
}

void AdjustLevelDialog::slotScaleChanged(int scale)
{
    m_levelsHistogramWidget->m_scaleType = scale;
    m_histogramWidget->m_scaleType       = scale;
    m_histogramWidget->repaint();
    m_levelsHistogramWidget->repaint();
}

void AdjustLevelDialog::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustlevels Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale", 
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    for (int i = 0 ; i < 5 ; i++)
    {
        bool sb        = m_originalImage.sixteenBit();
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
    m_histogramWidget->reset();
    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());

    // This is mandatory here to set spinbox values because slot connections 
    // can be not set completely at plugin startup.
    m_minInput->setValue(m_levels->getLevelLowInputValue(m_channelCB->currentIndex()));
    m_minOutput->setValue(m_levels->getLevelLowOutputValue(m_channelCB->currentIndex()));
    m_maxInput->setValue(m_levels->getLevelHighInputValue(m_channelCB->currentIndex()));
    m_maxOutput->setValue(m_levels->getLevelHighOutputValue(m_channelCB->currentIndex()));
}

void AdjustLevelDialog::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustlevels Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());

    for (int i = 0 ; i < 5 ; i++)
    {
        bool sb        = m_originalImage.sixteenBit();
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

    config->sync();
}

void AdjustLevelDialog::resetValues()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
       m_levels->levelsChannelReset(channel);

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentIndex());
    m_levelsHistogramWidget->reset();
    m_histogramWidget->reset();
}

// Load all settings.
void AdjustLevelDialog::slotUser3()
{
    KUrl loadLevelsFile;

    loadLevelsFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Select Gimp Levels File to Load")) );
    if( loadLevelsFile.isEmpty() )
       return;

    if ( m_levels->loadLevelsFromGimpLevelsFile( loadLevelsFile ) == false )
    {
       KMessageBox::error(this, i18n("Cannot load from the Gimp levels text file."));
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentIndex());
}

// Save all settings.
void AdjustLevelDialog::slotUser2()
{
    KUrl saveLevelsFile;

    saveLevelsFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Gimp Levels File to Save")) );
    if( saveLevelsFile.isEmpty() )
       return;

    if ( m_levels->saveLevelsToGimpLevelsFile( saveLevelsFile ) == false )
    {
       KMessageBox::error(this, i18n("Cannot save to the Gimp levels text file."));
       return;
    }

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentIndex());
}

// See B.K.O #146636: use event filter with all level slider to display a
// guide over level histogram.
bool AdjustLevelDialog::eventFilter(QObject *obj, QEvent *ev)
{
    if ( obj == m_hGradientMinInput )
    {
        if ( ev->type() == QEvent::MouseButtonPress)
        {
            connect(m_minInput, SIGNAL(valueChanged(int)),
                    this, SLOT(slotShowHistogramGuide(int)));
            
            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_minInput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowHistogramGuide(int)));

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
                    this, SLOT(slotShowHistogramGuide(int)));
            
            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_maxInput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowHistogramGuide(int)));

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
                    this, SLOT(slotShowHistogramGuide(int)));
            
            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_minOutput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowHistogramGuide(int)));

            m_levelsHistogramWidget->reset();
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
                    this, SLOT(slotShowHistogramGuide(int)));
            
            return false;
        }
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            disconnect(m_maxOutput, SIGNAL(valueChanged(int)),
                       this, SLOT(slotShowHistogramGuide(int)));

            m_levelsHistogramWidget->reset();
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
        return KDialog::eventFilter(obj, ev);
    }
}

void AdjustLevelDialog::slotShowHistogramGuide(int v)
{
    Digikam::DColor color(v, v, v, v, m_originalImage.sixteenBit());
    m_levelsHistogramWidget->setHistogramGuideByColor(color);
}

}  // NameSpace DigikamAdjustLevelsImagesPlugin
