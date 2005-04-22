/* ============================================================
 * File  : adjustlevels.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright 2004-2005 by Gilles Caulier
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
#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>
#include <qhbuttongroup.h> 
#include <qpixmap.h>
#include <qcheckbox.h>

// KDE includes.

#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kselect.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "adjustlevels.h"

namespace DigikamAdjustLevelsImagesPlugin
{

AdjustLevelDialog::AdjustLevelDialog(QWidget* parent, uint *imageData, uint width, uint height)
                 : KDialogBase(Plain, i18n("Adjust Color Levels"), Help|User1|Ok|Cancel, Ok,
                               parent, 0, true, true, i18n("&Reset Values"))
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    
    // Create an empty instance of levels to use.
    m_levels = new Digikam::ImageLevels();

    setButtonWhatsThis ( User1, i18n("<p>Reset levels values from the current selected channel.") );

    // About data and help button.

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Adjust Levels"),
                                       digikamimageplugins_version,
                                       I18N_NOOP("An image-histogram-levels adjustment plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Adjust Levels Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );

    // -------------------------------------------------------------

    QGridLayout* topLayout = new QGridLayout( plainPage(), 3, 2 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Adjust Color Levels"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");

    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );

    // -------------------------------------------------------------

    QGroupBox *gbox = new QGroupBox(plainPage());
    gbox->setFlat(false);
    gbox->setTitle(i18n("Settings"));
    QGridLayout* grid = new QGridLayout( gbox, 6, 5, 20, spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), gbox);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gbox );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->insertItem( i18n("Alpha") );
    m_channelCB->setCurrentText( i18n("Luminosity") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"
                                       "<b>Alpha</b>: display the alpha image-channel values. "
                                       "This channel corresponds to the transparency value and "
                                       "is supported by some image formats, such as PNG or GIF."));

    QLabel *label2 = new QLabel(i18n("Scale:"), gbox);
    label2->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_scaleCB = new QComboBox( false, gbox );
    m_scaleCB->insertItem( i18n("Linear") );
    m_scaleCB->insertItem( i18n("Logarithmic") );
    m_scaleCB->setCurrentText( i18n("Logarithmic") );
    QWhatsThis::add( m_scaleCB, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(m_channelCB, 0, 0, 1, 1);
    grid->addMultiCellWidget(label2, 0, 0, 3, 3);
    grid->addMultiCellWidget(m_scaleCB, 0, 0, 4, 4);

    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, imageData, width, height, frame, false);
    QWhatsThis::add( m_histogramWidget, i18n("<p>This is the histogram drawing of the selected image channel"));
    l->addWidget(m_histogramWidget, 0);
    grid->addMultiCellWidget(frame, 1, 1, 0, 4);

    m_hGradientMinInput = new KGradientSelector( KSelector::Horizontal, gbox );
    m_hGradientMinInput->setFixedHeight( 20 );
    m_hGradientMinInput->setMinValue(0);
    m_hGradientMinInput->setMaxValue(255);
    QWhatsThis::add( m_hGradientMinInput, i18n("<p>Select here the minimal intensity input value of the histogram."));
    QToolTip::add( m_hGradientMinInput, i18n( "Minimal intensity input." ) );
    m_hGradientMinInput->setColors( QColor( "black" ), QColor( "white" ) );
    grid->addMultiCellWidget(m_hGradientMinInput, 2, 2, 0, 4);

    m_hGradientMaxInput = new KGradientSelector( KSelector::Horizontal, gbox );
    m_hGradientMaxInput->setFixedHeight( 20 );
    m_hGradientMaxInput->setMinValue(0);
    m_hGradientMaxInput->setMaxValue(255);
    QWhatsThis::add( m_hGradientMaxInput, i18n("<p>Select here the maximal intensity input value of the histogram."));
    QToolTip::add( m_hGradientMaxInput, i18n( "Maximal intensity input." ) );
    m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "white" ) );
    grid->addMultiCellWidget(m_hGradientMaxInput, 3, 3, 0, 4);

    m_minInput = new QSpinBox(0, 255, 1, gbox);
    m_minInput->setValue(0);
    QWhatsThis::add( m_minInput, i18n("<p>Select here the minimal intensity input value of the histogram."));
    QToolTip::add( m_minInput, i18n( "Minimal intensity input." ) );
    m_gammaInput = new KDoubleNumInput(gbox);
    m_gammaInput->setPrecision(2);
    m_gammaInput->setRange(0.1, 10.0, 0.1);
    m_gammaInput->setValue(1.0);
    QToolTip::add( m_gammaInput, i18n( "Gamma input value." ) );
    QWhatsThis::add( m_gammaInput, i18n("<p>Select here the gamma input value."));
    m_maxInput = new QSpinBox(0, 255, 1, gbox);
    m_maxInput->setValue(255);
    QToolTip::add( m_maxInput, i18n( "Maximal intensity input." ) );
    QWhatsThis::add( m_maxInput, i18n("<p>Select here the maximal intensity input value of the histogram."));
    grid->addMultiCellWidget(m_minInput, 2, 2, 5, 5);
    grid->addMultiCellWidget(m_maxInput, 3, 3, 5, 5);
    grid->addMultiCellWidget(m_gammaInput, 4, 4, 0, 5);

    m_hGradientMinOutput = new KGradientSelector( KSelector::Horizontal, gbox );
    m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "white" ) );
    QWhatsThis::add( m_hGradientMinOutput, i18n("<p>Select here the minimal intensity output value of the histogram."));    
    QToolTip::add( m_hGradientMinOutput, i18n( "Minimal intensity output." ) );
    m_hGradientMinOutput->setFixedHeight( 20 );
    m_hGradientMinOutput->setMinValue(0);
    m_hGradientMinOutput->setMaxValue(255);
    grid->addMultiCellWidget(m_hGradientMinOutput, 5, 5, 0, 4);

    m_hGradientMaxOutput = new KGradientSelector( KSelector::Horizontal, gbox );
    m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "white" ) );
    QWhatsThis::add( m_hGradientMaxOutput, i18n("<p>Select here the maximal intensity output value of the histogram."));
    QToolTip::add( m_hGradientMaxOutput, i18n( "Maximal intensity output." ) );
    m_hGradientMaxOutput->setFixedHeight( 20 );
    m_hGradientMaxOutput->setMinValue(0);
    m_hGradientMaxOutput->setMaxValue(255);
    grid->addMultiCellWidget(m_hGradientMaxOutput, 6, 6, 0, 4);

    m_minOutput = new QSpinBox(0, 255, 1, gbox);
    m_minOutput->setValue(0);
    QToolTip::add( m_minOutput, i18n( "Minimal intensity output." ) );
    QWhatsThis::add( m_minOutput, i18n("<p>Select here the minimal intensity output value of the histogram."));
    m_maxOutput = new QSpinBox(0, 255, 1, gbox);
    m_maxOutput->setValue(255);
    QToolTip::add( m_maxOutput, i18n( "Maximal intensity output." ) );
    QWhatsThis::add( m_maxOutput, i18n("<p>Select here the maximal intensity output value of the histogram."));
    grid->addMultiCellWidget(m_minOutput, 5, 5, 5, 5);
    grid->addMultiCellWidget(m_maxOutput, 6, 6, 5, 5);

    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);

    // -------------------------------------------------------------

    QHGroupBox *gbox3 = new QHGroupBox(i18n("All Channels' Levels"), plainPage());
    m_loadButton = new QPushButton(i18n("&Load..."), gbox3);
    QWhatsThis::add( m_loadButton, i18n("<p>Load levels settings from a Gimp levels text file."));
    m_saveButton = new QPushButton(i18n("&Save As..."), gbox3);
    QWhatsThis::add( m_saveButton, i18n("<p>Save levels settings to a Gimp levels text file."));
    m_autoButton = new QPushButton(i18n("&Auto"), gbox3);
    QWhatsThis::add( m_autoButton, i18n("<p>Adjust levels automatically."));
    m_resetButton = new QPushButton(i18n("&Reset All"), gbox3);
    QWhatsThis::add( m_resetButton, i18n("<p>Reset all channels' level values."));

    m_pickerColorButtonGroup = new QHButtonGroup(gbox3);
    m_pickBlack = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickBlack, BlackTonal);
    KGlobal::dirs()->addResourceType("color-picker-black", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-black", "color-picker-black.png");
    m_pickBlack->setPixmap( QPixmap( directory + "color-picker-black.png" ) );
    m_pickBlack->setToggleButton(true);
    QToolTip::add( m_pickBlack, i18n( "All Channels shadow tone color picker" ) );
    QWhatsThis::add( m_pickBlack, i18n("<p>With this button, you can pick the color from original image used to set <b>Shadow Tone</b> "
                                       "levels input on Red, Green, Blue, and Luminosity channels."));
    m_pickGray  = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickGray, GrayTonal);
    KGlobal::dirs()->addResourceType("color-picker-gray", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-gray", "color-picker-gray.png");
    m_pickGray->setPixmap( QPixmap( directory + "color-picker-gray.png" ) );
    m_pickGray->setToggleButton(true);
    QToolTip::add( m_pickGray, i18n( "All Channels middle tone color picker" ) );
    QWhatsThis::add( m_pickGray, i18n("<p>With this button, you can pick the color from original image used to set <b>Middle Tone</b> "
                                      "levels input on Red, Green, Blue, and Luminosity channels."));
    m_pickWhite = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickWhite, WhiteTonal);
    KGlobal::dirs()->addResourceType("color-picker-white", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-white", "color-picker-white.png");
    m_pickWhite->setPixmap( QPixmap( directory + "color-picker-white.png" ) );
    m_pickWhite->setToggleButton(true);
    QToolTip::add( m_pickWhite, i18n( "All Channels highlight tone color picker" ) );
    QWhatsThis::add( m_pickWhite, i18n("<p>With this button, you can pick the color from original image used to set <b>Highlight Tone</b> "
                                       "levels input on Red, Green, Blue, and Luminosity channels."));
    m_pickerColorButtonGroup->setExclusive(true);
    m_pickerColorButtonGroup->setFrameShape(QFrame::NoFrame);    
    
    topLayout->addMultiCellWidget(gbox3, 3, 3, 0, 0);

    // -------------------------------------------------------------

    QVGroupBox *gbox4 = new QVGroupBox(i18n("Preview"), plainPage());

    QFrame *frame2 = new QFrame(gbox4);
    frame2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2  = new QVBoxLayout(frame2, 5, 0);
    m_previewOriginalWidget = new Digikam::ImageGuideWidget(300, 200, frame2, true, 
                                                            Digikam::ImageGuideWidget::PickColorMode);
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image. You can pick a color on the image using the color "
                                                   "picker tools to select shadow, middle, and highlight tones to adjust the level points in the Red, "
                                                   "Green, Blue, and Luminosity Channels."));
    l2->addWidget(m_previewOriginalWidget, 0, Qt::AlignCenter);

    QFrame *frame3 = new QFrame(gbox4);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3  = new QVBoxLayout(frame3, 5, 0);
    m_previewTargetWidget = new Digikam::ImageWidget(300, 200, frame3);
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the image's level-adjustments preview."));
    l3->addWidget(m_previewTargetWidget, 0, Qt::AlignCenter);
    
    m_overExposureIndicatorBox = new QCheckBox(i18n("Over exposure indicator"), gbox4);
    QWhatsThis::add( m_overExposureIndicatorBox, i18n("<p>If you enable this option, over-exposed pixels from the target image preview "
                                                      "will be over-colored. This will not have an effect on the final rendering."));
                                                      
    topLayout->addMultiCellWidget(gbox4, 1, 3, 1, 1);

    adjustSize();
    disableResize();

    QTimer::singleShot(0, this, SLOT(slotResetAllChannels())); // Reset all parameters to the default values.
    parentWidget()->setCursor( KCursor::arrowCursor()  );
                    
    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));
            
    connect(m_previewOriginalWidget, SIGNAL(spotPositionChanged(  const QColor &, bool, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const QColor &, bool )));             

    connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
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
            this, SLOT(slotResetAllChannels()));

    connect(m_loadButton, SIGNAL(clicked()),
            this, SLOT(slotLoadLevels()));

    connect(m_saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveLevels()));
}

AdjustLevelDialog::~AdjustLevelDialog()
{
}

void AdjustLevelDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("adjustlevels", "digikamimageplugins");
}

void AdjustLevelDialog::closeEvent(QCloseEvent *e)
{
    delete m_histogramWidget;
    delete m_levels;
    e->accept();
}

void AdjustLevelDialog::slotSpotColorChanged(const QColor &color, bool release)
{
    if ( m_pickBlack->isOn() )
       {
       // Black tonal levels point.
       m_levels->levelsBlackToneAdjustByColors(m_channelCB->currentItem(), color);      
       m_pickBlack->setOn(!release);
       }
    else if ( m_pickGray->isOn() )
       {
       // Gray tonal levels point.
       m_levels->levelsGrayToneAdjustByColors(m_channelCB->currentItem(), color);      
       m_pickGray->setOn(!release);
       }
    else if ( m_pickWhite->isOn() )
       {
       // White tonal levels point.
       m_levels->levelsWhiteToneAdjustByColors(m_channelCB->currentItem(), color);      
       m_pickWhite->setOn(!release);
       }
    else
       m_histogramWidget->setHistogramGuide(color);

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());
       
    slotEffect();                
}

void AdjustLevelDialog::slotGammaInputchanged(double val)
{
    blockSignals(true);
    m_levels->setLevelGammaValue(m_channelCB->currentItem(), val);
    blockSignals(false);
    slotEffect();
}

void AdjustLevelDialog::slotAdjustMinInputSpinBox(int val)
{
    blockSignals(true);

    if ( val < m_hGradientMaxInput->value() )
       val = m_hGradientMaxInput->value();

    m_minInput->setValue(255 - val);
    m_hGradientMinInput->setValue( val );
    m_levels->setLevelLowInputValue(m_channelCB->currentItem(), 255 - val);
    blockSignals(false);
    slotEffect();
}

void AdjustLevelDialog::slotAdjustMaxInputSpinBox(int val)
{
    blockSignals(true);

    if ( val > m_hGradientMinInput->value() )
       val = m_hGradientMinInput->value();

    m_maxInput->setValue(255 - val);
    m_hGradientMaxInput->setValue( val );
    m_levels->setLevelHighInputValue(m_channelCB->currentItem(), 255 - val);
    blockSignals(false);
    slotEffect();
}

void AdjustLevelDialog::slotAdjustMinOutputSpinBox(int val)
{
    blockSignals(true);

    if ( val < m_hGradientMaxOutput->value() )
       val = m_hGradientMaxOutput->value();

    m_minOutput->setValue(255 - val);
    m_hGradientMinOutput->setValue( val );
    m_levels->setLevelLowOutputValue(m_channelCB->currentItem(), 255 - val);
    blockSignals(false);
    slotEffect();
}

void AdjustLevelDialog::slotAdjustMaxOutputSpinBox(int val)
{
    blockSignals(true);

    if ( val > m_hGradientMinOutput->value() )
       val = m_hGradientMinOutput->value();

    m_maxOutput->setValue(255 - val);
    m_hGradientMaxOutput->setValue( val );
    m_levels->setLevelHighOutputValue(m_channelCB->currentItem(), 255 - val);
    blockSignals(false);
    slotEffect();
}

void AdjustLevelDialog::slotAdjustSliders()
{
    adjustSliders(m_minInput->value(), m_gammaInput->value(),
                  m_maxInput->value(), m_minOutput->value(),
                  m_maxOutput->value());
}

void AdjustLevelDialog::adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut)
{
    m_hGradientMinInput->setValue(255 - minIn);
    m_hGradientMaxInput->setValue(255 - maxIn);
    m_gammaInput->setValue(gamIn);
    m_hGradientMinOutput->setValue(255 - minOut);
    m_hGradientMaxOutput->setValue(255 - maxOut);
}

void AdjustLevelDialog::slotResetAllChannels()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
       m_levels->levelsChannelReset(channel);

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());
    m_histogramWidget->reset();
    
    slotEffect();
}

void AdjustLevelDialog::slotUser1()
{
    m_levels->levelsChannelReset(m_channelCB->currentItem());

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());
    m_histogramWidget->reset();

    slotEffect();
}

void AdjustLevelDialog::slotAutoLevels()
{
    // Calculate Auto levels.
    m_levels->levelsAuto(m_histogramWidget->m_imageHistogram);

    // Refresh the current levels config.
    slotChannelChanged(m_channelCB->currentItem());

    slotEffect();
}

void AdjustLevelDialog::slotEffect()
{
    Digikam::ImageIface* ifaceOrg = m_previewOriginalWidget->imageIface();

    Digikam::ImageIface* ifaceDest = m_previewTargetWidget->imageIface();

    uint* orgData = ifaceOrg->getPreviewData();
    int   w       = ifaceOrg->previewWidth();
    int   h       = ifaceOrg->previewHeight();

    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];

    // Calculate the LUT to apply on the image.
    m_levels->levelsLutSetup(Digikam::ImageHistogram::AlphaChannel, m_overExposureIndicatorBox->isChecked());

    // Apply the lut to the image.
    m_levels->levelsLutProcess(orgData, desData, w, h);

    ifaceDest->putPreviewData(desData);
    m_previewTargetWidget->update();

    delete [] orgData;
    delete [] desData;
}

void AdjustLevelDialog::slotOk()
{
    Digikam::ImageIface* ifaceOrg = m_previewOriginalWidget->imageIface();

    Digikam::ImageIface* ifaceDest = m_previewTargetWidget->imageIface();

    uint* orgData = ifaceOrg->getOriginalData();
    int   w       = ifaceOrg->originalWidth();
    int   h       = ifaceOrg->originalHeight();

    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];

    // Calculate the LUT to apply on the image.
    m_levels->levelsLutSetup(Digikam::ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_levels->levelsLutProcess(orgData, desData, w, h);

    ifaceDest->putOriginalData(i18n("Adjust Level"), desData);

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
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "white" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "white" ) );
          break;
       
       case RedChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "red" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "red" ) );
          break;

       case GreenChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "green" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "green" ) );
          break;

       case BlueChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradientMinInput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMaxInput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMinOutput->setColors( QColor( "black" ), QColor( "blue" ) );
          m_hGradientMaxOutput->setColors( QColor( "black" ), QColor( "blue" ) );
          break;

       case AlphaChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
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

    m_histogramWidget->repaint(false);
}

void AdjustLevelDialog::slotScaleChanged(int scale)
{
    switch(scale)
       {
       case Linear: 
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LinScaleHistogram;
          break;
       
       case Logarithmic:
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LogScaleHistogram;
          break;
       }

    m_histogramWidget->repaint(false);
}

void AdjustLevelDialog::slotLoadLevels()
{
    KURL loadLevelsFile;

    loadLevelsFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
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
    slotChannelChanged(m_channelCB->currentItem());
}

void AdjustLevelDialog::slotSaveLevels()
{
    KURL saveLevelsFile;

    saveLevelsFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
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
    slotChannelChanged(m_channelCB->currentItem());
}

}  // NameSpace DigikamAdjustLevelsImagesPlugin

#include "adjustlevels.moc"

