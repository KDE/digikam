/* ============================================================
 * File  : adjustcurves.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : image histogram adjust curves.
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
#include "adjustcurves.h"

namespace DigikamAdjustCurvesImagesPlugin
{

AdjustCurveDialog::AdjustCurveDialog(QWidget* parent, uint *imageData, uint width, uint height)
                 : KDialogBase(Plain, i18n("Adjust Color Curves"), Help|User1|Ok|Cancel, Ok,
                               parent, 0, true, true, i18n("&Reset Values"))
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    
    // Create an empty instance of curves to use.
    m_curves = new Digikam::ImageCurves();

    setButtonWhatsThis ( User1, i18n("<p>Reset curves' values from the current selected channel.") );

    // About data and help button.

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Adjust Curves"),
                                       digikamimageplugins_version,
                                       I18N_NOOP("An image-histogram-curves adjustment plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Adjust Curves Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Adjust Color Curves"), headerFrame, "labelTitle" );
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
    QGridLayout* grid = new QGridLayout( gbox, 4, 6, 20, spacingHint());

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
                                     "if it is used, all values (small and large) will be visible on the "
                                     "graph."));

    grid->addMultiCellWidget(label1, 0, 0, 1, 1);
    grid->addMultiCellWidget(m_channelCB, 0, 0, 2, 2);
    grid->addMultiCellWidget(label2, 0, 0, 4, 4);
    grid->addMultiCellWidget(m_scaleCB, 0, 0, 5, 5);
    
    QLabel *label5 = new QLabel(i18n("Type:"), gbox);
    label5->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_typeCB = new QComboBox( false, gbox );
    m_typeCB->insertItem( i18n("Smooth") );
    m_typeCB->insertItem( i18n("Free") );
    m_typeCB->setCurrentText( i18n("Smooth") );
    QWhatsThis::add( m_typeCB, i18n("<p>Select here the curve type to draw for the current channel.<p>"
                                    "<b>Smooth</b>: this mode constrains the curve type to a smooth line with tension.<p>"
                                    "<b>Free</b>: with this mode, you can draw your curve free-hand with the mouse."));

    m_labelPos = new QLabel(gbox);
    m_labelPos->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
                                     
    grid->addMultiCellWidget(label5, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_typeCB, 1, 1, 2, 2);
    grid->addMultiCellWidget(m_labelPos, 1, 1, 5, 5);
    
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);

    m_vGradient = new Digikam::ColorGradientWidget( KSelector::Vertical, 20, gbox );
    m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
    grid->addMultiCellWidget(m_vGradient, 2, 2, 0, 0);

    m_curvesWidget = new Digikam::CurvesWidget(256, 256, imageData, width, height, m_curves, frame);
    QWhatsThis::add( m_curvesWidget, i18n("<p>This is the curve drawing of the selected image "
                                          "histogram channel"));
    l->addWidget(m_curvesWidget, 0);
    grid->addMultiCellWidget(frame, 2, 2, 1, 5);
    
    m_hGradient = new Digikam::ColorGradientWidget( KSelector::Horizontal, 20, gbox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    grid->addMultiCellWidget(m_hGradient, 3, 3, 1, 5);
    
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);

    // -------------------------------------------------------------

    QHGroupBox *gbox3 = new QHGroupBox(i18n("All Channels' Curves"), plainPage());
    m_loadButton = new QPushButton(i18n("&Load..."), gbox3);
    QWhatsThis::add( m_loadButton, i18n("<p>Load curve settings from a Gimp curves text file."));
    m_saveButton = new QPushButton(i18n("&Save As..."), gbox3);
    QWhatsThis::add( m_saveButton, i18n("<p>Save curve settings to a Gimp curves text file."));
    m_resetButton = new QPushButton(i18n("&Reset All"), gbox3);
    QWhatsThis::add( m_resetButton, i18n("<p>Reset all channels' curve values."));
    
    m_pickerColorButtonGroup = new QHButtonGroup(gbox3);
    m_pickBlack = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickBlack, BlackTonal);
    KGlobal::dirs()->addResourceType("color-picker-black", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-black", "color-picker-black.png");
    m_pickBlack->setPixmap( QPixmap( directory + "color-picker-black.png" ) );
    m_pickBlack->setToggleButton(true);
    QToolTip::add( m_pickBlack, i18n( "All channels shadow tone color picker" ) );
    QWhatsThis::add( m_pickBlack, i18n("<p>With this button, you can pick the color from original image used to set <b>Shadow Tone</b> "
                                       "smooth curves point on Red, Green, Blue, and Luminosity channels."));
    m_pickGray  = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickGray, GrayTonal);
    KGlobal::dirs()->addResourceType("color-picker-gray", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-gray", "color-picker-gray.png");
    m_pickGray->setPixmap( QPixmap( directory + "color-picker-gray.png" ) );
    m_pickGray->setToggleButton(true);
    QToolTip::add( m_pickGray, i18n( "All channels middle tone color picker" ) );
    QWhatsThis::add( m_pickGray, i18n("<p>With this button, you can pick the color from original image used to set <b>Middle Tone</b> "
                                      "smooth curves point on Red, Green, Blue, and Luminosity channels."));
    m_pickWhite = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickWhite, WhiteTonal);
    KGlobal::dirs()->addResourceType("color-picker-white", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-white", "color-picker-white.png");
    m_pickWhite->setPixmap( QPixmap( directory + "color-picker-white.png" ) );
    m_pickWhite->setToggleButton(true);
    QToolTip::add( m_pickWhite, i18n( "All channels highlight tone color picker" ) );
    QWhatsThis::add( m_pickWhite, i18n("<p>With this button, you can pick the color from original image used to set <b>Highlight Tone</b> "
                                       "smooth curves point on Red, Green, Blue, and Luminosity channels."));
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
                                                   "picker tools to select shadow, middle, and highlight tones to adjust the curves' points in the Red, "
                                                   "Green, Blue, and Luminosity Channels."));
    l2->addWidget(m_previewOriginalWidget, 0, Qt::AlignCenter);

    QFrame *frame3 = new QFrame(gbox4);
    frame3->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l3  = new QVBoxLayout(frame3, 5, 0);
    m_previewTargetWidget = new Digikam::ImageWidget(300, 200, frame3);
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the image's curve-adjustments preview."));
    l3->addWidget(m_previewTargetWidget, 0, Qt::AlignCenter);

    m_overExposureIndicatorBox = new QCheckBox(i18n("Over exposure indicator"), gbox4);
    QWhatsThis::add( m_overExposureIndicatorBox, i18n("<p>If you enable this option, over-exposed pixels from the target image preview "
                                                      "will be over-colored. This will not have an effect on the final rendering."));
                                                          
    topLayout->addMultiCellWidget(gbox4, 1, 3, 1, 1);

    // -------------------------------------------------------------
    
    adjustSize();
    disableResize();

    QTimer::singleShot(0, this, SLOT(slotResetAllChannels())); // Reset all parameters to the default values.
    parentWidget()->setCursor( KCursor::arrowCursor()  );

    // -------------------------------------------------------------
    
    connect(m_curvesWidget, SIGNAL(signalMouseMoved(int, int)),
            this, SLOT(slotPositionChanged(int, int)));
                        
    connect(m_curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_previewOriginalWidget, SIGNAL(spotPositionChanged(  const QColor &, bool, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const QColor &, bool ))); 
            
    connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));              
    
    // -------------------------------------------------------------
    // ComboBox slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));
            
    connect(m_typeCB, SIGNAL(activated(int)),
            this, SLOT(slotCurveTypeChanged(int)));
    
    // -------------------------------------------------------------
    // Bouttons slots.

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetAllChannels()));

    connect(m_loadButton, SIGNAL(clicked()),
            this, SLOT(slotLoadCurves()));

    connect(m_saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveCurves()));
}

AdjustCurveDialog::~AdjustCurveDialog()
{
}

void AdjustCurveDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("adjustcurves", "digikamimageplugins");
}

void AdjustCurveDialog::closeEvent(QCloseEvent *e)
{
    delete m_curvesWidget;
    delete m_curves;
    e->accept();
}

void AdjustCurveDialog::slotSpotColorChanged(const QColor &color, bool release)
{
    if ( m_pickBlack->isOn() )
       {
       // Black tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 1, 
                               QPoint::QPoint(QMAX(QMAX(color.red(), color.green()), color.blue()), 42));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 1, QPoint::QPoint(color.red(), 42));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 1, QPoint::QPoint(color.green(), 42));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 1, QPoint::QPoint(color.blue(), 42));      
       m_pickBlack->setOn(!release);
       }
    else if ( m_pickGray->isOn() )
       {
       // Gray tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8, 
                               QPoint::QPoint(QMAX(QMAX(color.red(), color.green()), color.blue()), 128));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 8, QPoint::QPoint(color.red(), 128));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 8, QPoint::QPoint(color.green(), 128));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 8, QPoint::QPoint(color.blue(), 128));      
       m_pickGray->setOn(!release);
       }
    else if ( m_pickWhite->isOn() )
       {
       // White tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 15, 
                               QPoint::QPoint(QMAX(QMAX(color.red(), color.green()), color.blue()), 213));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 15, QPoint::QPoint(color.red(), 213));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 15, QPoint::QPoint(color.green(), 213));      
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 15, QPoint::QPoint(color.blue(), 213));      
       m_pickWhite->setOn(!release);
       }
    else
       m_curvesWidget->setCurveGuide(color);

    // Calculate Red, green, blue curves.
    
    for (int i = Digikam::ImageHistogram::ValueChannel ; i <= Digikam::ImageHistogram::BlueChannel ; i++)
       m_curves->curvesCalculateCurve(i);
    
    m_curvesWidget->repaint(false);
       
    slotEffect();  
}

void AdjustCurveDialog::slotResetAllChannels()
{
    for (int channel = 0 ; channel < 5 ; channel++)
       m_curves->curvesChannelReset(channel);

    m_curvesWidget->reset();
    slotEffect();
}

void AdjustCurveDialog::slotUser1()
{
    m_curves->curvesChannelReset(m_channelCB->currentItem());

    m_curvesWidget->reset();
    slotEffect();
}

void AdjustCurveDialog::slotPositionChanged(int x, int y)
{
    if ( x == -1 && y == -1)
       m_labelPos->clear();
    else
       m_labelPos->setText(i18n("x:%1   y:%2").arg(x).arg(y));
}

void AdjustCurveDialog::slotEffect()
{
    Digikam::ImageIface* ifaceOrg = m_previewOriginalWidget->imageIface();

    Digikam::ImageIface* ifaceDest = m_previewTargetWidget->imageIface();

    uint* orgData = ifaceOrg->getPreviewData();
    int   w       = ifaceOrg->previewWidth();
    int   h       = ifaceOrg->previewHeight();

    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];

    // Calculate the LUT to apply on the image.
    m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel, m_overExposureIndicatorBox->isChecked());

    // Apply the lut to the image.
    m_curves->curvesLutProcess(orgData, desData, w, h);

    ifaceDest->putPreviewData(desData);
    m_previewTargetWidget->update();

    delete [] orgData;
    delete [] desData;
}

void AdjustCurveDialog::slotOk()
{
    Digikam::ImageIface* ifaceOrg = m_previewOriginalWidget->imageIface();

    Digikam::ImageIface* ifaceDest = m_previewTargetWidget->imageIface();

    uint* orgData = ifaceOrg->getOriginalData();
    int   w       = ifaceOrg->originalWidth();
    int   h       = ifaceOrg->originalHeight();

    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];

    // Calculate the LUT to apply on the image.
    m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_curves->curvesLutProcess(orgData, desData, w, h);

    ifaceDest->putOriginalData(i18n("Adjust Curve"), desData);

    delete [] orgData;
    delete [] desData;
    accept();
}

void AdjustCurveDialog::slotChannelChanged(int channel)
{
    switch(channel)
       {
       case LuminosityChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::ValueHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;
       
       case RedChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::RedChannelHistogram;
          m_vGradient->setColors( QColor( "red" ), QColor( "black" ) );
          break;

       case GreenChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::GreenChannelHistogram;
          m_vGradient->setColors( QColor( "green" ), QColor( "black" ) );
          break;

       case BlueChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::BlueChannelHistogram;
          m_vGradient->setColors( QColor( "blue" ), QColor( "black" ) );
          break;

       case AlphaChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::AlphaChannelHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;
       }

    m_typeCB->setCurrentItem(m_curves->getCurveType(channel));  
                
    m_curvesWidget->repaint(false);
}

void AdjustCurveDialog::slotScaleChanged(int scale)
{
    switch(scale)
       {
       case Linear:
          m_curvesWidget->m_scaleType = Digikam::CurvesWidget::LinScaleHistogram;
          break;
       
       case Logarithmic:
          m_curvesWidget->m_scaleType = Digikam::CurvesWidget::LogScaleHistogram;
          break;
       }

    m_curvesWidget->repaint(false);
}

void AdjustCurveDialog::slotCurveTypeChanged(int type)
{
    switch(type)
       {
       case SmoothDrawing:          
          {
          m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_SMOOTH);
          m_pickerColorButtonGroup->setEnabled(true);
          break;
          }
       
       case FreeDrawing:          
          {
          m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_FREE);
          m_pickerColorButtonGroup->setEnabled(false);
          break;
          }
       }
    
    m_curvesWidget->curveTypeChanged();
}

void AdjustCurveDialog::slotLoadCurves()
{
    KURL loadCurvesFile;

    loadCurvesFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Select Gimp Curves File to Load")) );
    if( loadCurvesFile.isEmpty() )
       return;

    if ( m_curves->loadCurvesFromGimpCurvesFile( loadCurvesFile ) == false )
       {
       KMessageBox::error(this, i18n("Cannot load from the Gimp curves text file."));
       return;
       }

    // Refresh the current curves config.
    slotChannelChanged(m_channelCB->currentItem());
    slotEffect();
}

void AdjustCurveDialog::slotSaveCurves()
{
    KURL saveCurvesFile;

    saveCurvesFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Gimp Curves File to Save")) );
    if( saveCurvesFile.isEmpty() )
       return;

    if ( m_curves->saveCurvesToGimpCurvesFile( saveCurvesFile ) == false )
       {
       KMessageBox::error(this, i18n("Cannot save to the Gimp curves text file."));
       return;
       }

    // Refresh the current curves config.
    slotChannelChanged(m_channelCB->currentItem());
}

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#include "adjustcurves.moc"

