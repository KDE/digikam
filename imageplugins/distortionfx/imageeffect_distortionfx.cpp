/* ============================================================
 * File  : imageeffect_distortionfx.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-11
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Distortion algorithms copyrighted 2004-2005 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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

#define ANGLE_RATIO        0.017453292519943295769236907685   // Represents 1º 
 
// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qslider.h>
#include <qimage.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qdatetime.h> 
#include <qtimer.h>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kprogress.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_distortionfx.h"


namespace DigikamDistortionFXImagesPlugin
{

ImageEffect_DistortionFX::ImageEffect_DistortionFX(QWidget* parent)
                        : KDialogBase(Plain, i18n("Distortion Effects"),
                                      Help|User1|Ok|Cancel, Ok,
                                      parent, 0, true, true, i18n("&Reset Values")),
                          m_parent(parent)
{
    m_timer = 0;
    QString whatsThis;
    
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    m_cancel = false;
    m_dirty = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Distortion Effects"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply distortion effect to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Distortion algorithms"), 
                     "pieter_voloshyn at ame.com.br"); 
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Distortion Effects Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------
        
    QGridLayout* topLayout = new QGridLayout( plainPage(), 2, 2 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Apply Distortion Effect to Image"), headerFrame, "labelTitle" );
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
        
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the distortion effect.") );
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox2 = new QGroupBox(i18n("Settings"), plainPage());
    QGridLayout *gridBox2 = new QGridLayout( gbox2, 3, 2, marginHint(), spacingHint());
    
    m_effectTypeLabel = new QLabel(i18n("Type:"), gbox2);
    
    m_effectType = new QComboBox( false, gbox2 );
    m_effectType->insertItem( i18n("Fish Eyes") );
    m_effectType->insertItem( i18n("Twirl") );
    m_effectType->insertItem( i18n("Cilindrical Hor.") );
    m_effectType->insertItem( i18n("Cilindrical Vert.") );
    m_effectType->insertItem( i18n("Cilindrical H/V.") );
    m_effectType->insertItem( i18n("Caricature") );
    m_effectType->insertItem( i18n("Multiple Corners") );
    m_effectType->insertItem( i18n("Waves Hor.") );
    m_effectType->insertItem( i18n("Waves Vert.") );
    m_effectType->insertItem( i18n("Block Waves 1") );
    m_effectType->insertItem( i18n("Block Waves 2") );
    m_effectType->insertItem( i18n("Circular Waves 1") );
    m_effectType->insertItem( i18n("Circular Waves 2") );
    m_effectType->insertItem( i18n("Polar Coordinates") );    
    m_effectType->insertItem( i18n("Unpolar Coordinates") );    
    m_effectType->insertItem( i18n("Tile") );    
    m_effectType->insertItem( i18n("Neon") );    
    m_effectType->insertItem( i18n("Find Edges") );    
    QWhatsThis::add( m_effectType, i18n("<p>Select here the effect type to apply on image.<p>"
                                        "<b>Fish Eyes</b>: warps the photograph around a 3D spherical shape to "
                                        "reproduce the common photograph 'Fish Eyes' effect.<p>"
                                        "<b>Twirl</b>: spins the photograph to produce a Twirl pattern.<p>"
                                        "<b>Cylinder Hor.</b>: warps the photograph around a horizontal cylinder.<p>"
                                        "<b>Cylinder Vert.</b>: warps the photograph around a vertical cylinder.<p>"
                                        "<b>Cylinder H/V.</b>: warps the photograph around a 2 cylinders, vertical "
                                        "and horizontal.<p>"
                                        "<b>Caricature</b>: distorts photograph with 'Fish Eyes' effect inverted.<p>"
                                        "<b>Multiple Corners</b>: splits the photograph like a multiple corners pattern.<p>"
                                        "<b>Waves Horizontal</b>: distorts the photograph with horizontal waves.<p>"
                                        "<b>Waves Vertical</b>: distorts the photograph with verticals waves.<p>"
                                        "<b>Block Waves 1</b>: divides the image into cells and makes it look as "
                                        "if it is being viewed through glass blocks.<p>"
                                        "<b>Block Waves 2</b>: like Block Waves 1 but with another version "
                                        "of glass blocks distorsion.<p>"
                                        "<b>Circular Waves 1</b>: distorts the photograph with circular waves.<p>"
                                        "<b>Circular Waves 2</b>: other variation of Circular Waves effect.<p>"
                                        "<b>Polar Coordinates</b>: converts the photograph from rectangular "
                                        "to polar coordinates.<p>"
                                        "<b>Unpolar Coordinates</b>: Polar Coordinate effect inverted.<p>"
                                        "<b>Tile</b>: splits the photograph into square blocks and move "
                                        "them randomly inside the image.<p>"
                                        "<b>Neon</b>: sub-coloring the edges in a photograph to reproduce a "
                                        "neon highlightment.<p>"
                                        "<b>Find Edges</b>: detects the edges in a photograph and their strength."
                                        ));
    gridBox2->addMultiCellWidget(m_effectTypeLabel, 0, 0, 0, 0);
    gridBox2->addMultiCellWidget(m_effectType, 0, 0, 1, 1);
                                                  
    m_levelLabel = new QLabel(i18n("Level:"), gbox2);
    m_levelInput = new KIntNumInput(gbox2);
    m_levelInput->setRange(0, 100, 1, true);
    QWhatsThis::add( m_levelInput, i18n("<p>Set here the level of the effect."));
    
    gridBox2->addMultiCellWidget(m_levelLabel, 1, 1, 0, 0);
    gridBox2->addMultiCellWidget(m_levelInput, 1, 1, 1, 1);
        
    m_iterationLabel = new QLabel(i18n("Iteration:"), gbox2);
    m_iterationInput = new KIntNumInput(gbox2);
    m_iterationInput->setRange(0, 100, 1, true);
    whatsThis = i18n("<p>This value controls the iterations to use for Waves, Tile, and Neon effects.");
    QWhatsThis::add( m_iterationInput, i18n("<p>This value controls the iterations to use for Waves, "
                                            "Tile, and Neon effects."));
    
    gridBox2->addMultiCellWidget(m_iterationLabel, 2, 2, 0, 0);
    gridBox2->addMultiCellWidget(m_iterationInput, 2, 2, 1, 1);
    
    m_progressBar = new KProgress(100, gbox2, "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    gridBox2->addMultiCellWidget(m_progressBar, 3, 3, 0, 1);

    topLayout->addMultiCellWidget(gbox2, 1, 1, 1, 1);

    // -------------------------------------------------------------
    
    adjustSize();
    disableResize();  
    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));
    
    connect(m_levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));            
            
    connect(m_iterationInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));            
}

ImageEffect_DistortionFX::~ImageEffect_DistortionFX()
{
    if (m_timer)
       delete m_timer;
}

void ImageEffect_DistortionFX::slotHelp()
{
    KApplication::kApplication()->invokeHelp("distortionfx",
                                             "digikamimageplugins");
}

void ImageEffect_DistortionFX::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_DistortionFX::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_DistortionFX::slotUser1()
{
    if (m_dirty)
       {
       m_cancel = true;
       }
    else
       {
       m_effectType->setCurrentItem(0);
       slotEffectTypeChanged(0);
       }
} 

void ImageEffect_DistortionFX::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }
    
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
}

void ImageEffect_DistortionFX::slotEffectTypeChanged(int type)
{
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);
    
    m_levelInput->blockSignals(true);
    m_iterationInput->blockSignals(true);
    m_levelInput->setRange(0, 100, 1, true);
    m_levelInput->setValue(25);
          
    switch (type)
       {
       case 1:  // Twirl.
          m_levelInput->setRange(-50, 50, 1, true);
          m_levelInput->setValue(10);
          break;

       case 0:  // Fish Eye.
       case 2:  // Cilindrical Hor.
       case 3:  // Cilindrical Vert.
       case 4:  // Cilindrical H/V.
       case 5:  // Caricature.
          m_levelInput->setRange(0, 200, 1, true);
          m_levelInput->setValue(50);
          break;

       case 6:  // Multiple Corners.
          m_levelInput->setRange(1, 10, 1, true);
          m_levelInput->setValue(4);
          break;
                                                  
       case 7:  // Waves Horizontal.
       case 8:  // Waves Vertical.
       case 9: // Block Waves 1.
       case 10: // Block Waves 2.
       case 11: // Circular Waves 1.
       case 12: // Circular Waves 2.
       case 15: // Tile.
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          m_iterationInput->setRange(0, 200, 1, true);
          m_iterationInput->setValue(10);
          break;

       case 13: // Polar Coordinates.
       case 14: // Unpolar Coordinates.
          m_levelInput->setEnabled(false);
          m_levelLabel->setEnabled(false);
          break;
                 
       case 16: // Neon.
       case 17: // Find Edges.
          m_levelInput->setRange(0, 5, 1, true);
          m_levelInput->setValue(3);
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          m_iterationInput->setRange(0, 5, 1, true);
          m_iterationInput->setValue(2);
          break;          
       }

    m_levelInput->blockSignals(false);
    m_iterationInput->blockSignals(false);
       
    slotEffect();
}

void ImageEffect_DistortionFX::slotEffect()
{
    m_dirty = true;
    m_parent->setCursor( KCursor::waitCursor() );
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
        
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);

    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    // Preview image size.
    int wp      = iface->previewWidth();
    int hp      = iface->previewHeight();
    
    // All data from the image
    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    
    int l       = m_levelInput->value();
    int f       = m_iterationInput->value();

    m_progressBar->setValue(0); 

    switch (m_effectType->currentItem())
       {
       case 0: // Fish Eye.
          fisheye(data, w, h, (double)(l/5.0), false);
          break;
       
       case 1: // Twirl.
          twirl(data, w, h, l, false);
          break;

       case 2: // Cilindrical Hor.
          cilindrical(data, w, h, (double)l, true, false, false);
          break;

       case 3: // Cilindrical Vert.
          cilindrical(data, w, h, (double)l, false, true, false);
          break;
                    
       case 4: // Cilindrical H/V.
          cilindrical(data, w, h, (double)l, true, true, false);
          break;
       
       case 5: // Caricature.
          fisheye(data, w, h, (double)(-l/5.0), false);
          break;
          
       case 6: // Multiple Corners.          
          multipleCorners(data, w, h, l, false);
          break;
       
       case 7: // Waves Horizontal.
          waves(data, w, h, l, f, true, false);
          break;
       
       case 8: // Waves Vertical.
          waves(data, w, h, l, f, true, true);
          break;
       
       case 9: // Block Waves 1.
          blockWaves(data, w, h, l, f, false);
          break;
       
       case 10: // Block Waves 2.
          blockWaves(data, w, h, l, f, true);
          break;
       
       case 11: // Circular Waves 1.
          circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 0.0, false, false);
          break;
       
       case 12: // Circular Waves 2.
          circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 25.0, true, false);
          break;
       
       case 13: // Polar Coordinates.
          polarCoordinates(data, w, h, true, false);
          break;

       case 14: // Unpolar Coordinates.
          polarCoordinates(data, w, h, false, false);
          break;
                    
       case 15:  // Tile.
          tile(data, w, h, 200-f, 200-f, l);
          break;

       case 16: // Neon.
          neon(data, w, h, l, f);
          break;
          
       case 17: // Find Edges.
          findEdges(data, w, h, l, f);
          break;
       }
    
    if ( !m_cancel ) 
       {
       QImage newImg((uchar*)data, w, h, 32, 0, 0, QImage::IgnoreEndian);
       QImage destImg = newImg.smoothScale(wp, hp);
       iface->putPreviewData((uint*)destImg.bits());
       }
                  
    delete [] data;
    m_progressBar->setValue(0); 
    m_previewWidget->update();

    m_effectTypeLabel->setEnabled(true);
    m_effectType->setEnabled(true);
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);
    
    switch (m_effectType->currentItem())
       {
       case 0:  // Fish Eye.
       case 1:  // Twirl.
       case 2:  // Cilindrical Hor.
       case 3:  // Cilindrical Vert.
       case 4:  // Cilindrical H/V.
       case 5:  // Caricature.
       case 6:  // Multiple Corners.          
          break;
       
       case 13: // Polar Coordinates.
       case 14: // Unpolar Coordinates.
          m_levelInput->setEnabled(false);
          m_levelLabel->setEnabled(false);
          break;

       case 7:  // Waves Horizontal.
       case 8:  // Waves Vertical.
       case 9:  // Block Waves 1.
       case 10: // Block Waves 2.
       case 11: // Circular Waves 1.
       case 12: // Circular Waves 2.
       case 15: // Tile.
       case 16: // Neon.
       case 17: // Find Edges.
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          break;
       }
    
    m_cancel = false;
    m_dirty = false;
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    enableButton(Ok, true);
    m_parent->setCursor( KCursor::arrowCursor() );
}

void ImageEffect_DistortionFX::slotOk()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    int l       = m_levelInput->value();
    int f       = m_iterationInput->value();

    m_progressBar->setValue(0); 
        
    if (data) 
       {
       switch (m_effectType->currentItem())
          {
          case 0: // Fish Eye.
             fisheye(data, w, h, (double)(l/5.0));
             break;
       
          case 1: // Twirl.
             twirl(data, w, h, l);
             break;

          case 2: // Cilindrical Hor.
             cilindrical(data, w, h, (double)l, true, false);
             break;
          
          case 3: // Cilindrical Vert.
             cilindrical(data, w, h, (double)l, false, true);
             break;
          
          case 4: // Cilindrical H/V.
             cilindrical(data, w, h, (double)l, false, true);
             break;
          
          case 5: // Caricature.
             fisheye(data, w, h, (double)(-l/5.0));
             break;
          
          case 6: // Multiple Corners.          
             multipleCorners(data, w, h, l);
             break;
          
          case 7: // Waves Horizontal.
             waves(data, w, h, l, f, true, false);
             break;
       
          case 8: // Waves Vertical.
             waves(data, w, h, l, f, true, true);
             break;

          case 9: // Block Waves 1.
             blockWaves(data, w, h, l, f, false);
             break;
          
          case 10: // Block Waves 2.
             blockWaves(data, w, h, l, f, true);
             break;
       
          case 11: // Circular Waves 1.
             circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 0.0, false);
             break;
          
          case 12: // Circular Waves 2.
             circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 25.0, true);
             break;
          
          case 13: // Polar Coordinates.
             polarCoordinates(data, w, h, true);
             break;

          case 14: // Unpolar Coordinates.
             polarCoordinates(data, w, h, false);
             break;
                          
          case 15:  // Tile.
             tile(data, w, h, 200-f, 200-f, l);
             break;

          case 16: // Neon.
             neon(data, w, h, l, f);
             break;
          
          case 17: // Find Edges.
             findEdges(data, w, h, l, f);
             break;
          }
       
       if ( !m_cancel ) iface->putOriginalData(i18n("Distortion Effects"), data);
       }
    
    delete [] data;    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

/* Function to apply the fisheye effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.    
 * Coeff            => Distortion effect coeff. Positive value render 'Fish Eyes' effect, 
 *                     and negative values render 'Caricature' effect.
 * Antialias        => Smart bluring result.                       
 *                                                                                  
 * Theory           => This is a great effect if you take employee photos
 *                     Its pure trigonometry. I think if you study hard the code you
 *                     understand very well.
 */
void ImageEffect_DistortionFX::fisheye(uint *data, int Width, int Height, double Coeff, bool AntiAlias)
{
    if (Coeff == 0.0) return;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];

    register int h, w, th, tw, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height;
    int nStride = GetStride(Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfRadius, lfRadMax, lfAngle, lfCoeff, lfCoeffStep = Coeff / 1000.0;

    if (nWidth > nHeight)
        lfYScale = (double)nWidth / (double)nHeight;
    else if (nHeight > nWidth)
        lfXScale = (double)nHeight / (double)nWidth;

    lfRadMax = (double)QMAX(nHeight, nWidth) / 2.0;
    lfCoeff = lfRadMax / log (fabs (lfCoeffStep) * lfRadMax + 1.0);

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        th = (int)(lfYScale * (double)h);
        
        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++)
            {
            tw = (int)(lfXScale * (double)w);

            // we find the distance from the center
            lfRadius = sqrt (th * th + tw * tw);

            if (lfRadius < lfRadMax)
                {
                lfAngle = atan2 (th, tw);

                if (Coeff > 0.0)
                    lfRadius = (exp (lfRadius / lfCoeff) - 1.0) / lfCoeffStep;
                else
                    lfRadius = lfCoeff * log (1.0 + (-1.0 * lfCoeffStep) * lfRadius);

                nw = (double)nHalfW + (lfRadius / lfXScale) * cos (lfAngle);
                nh = (double)nHalfH + (lfRadius / lfYScale) * sin (lfAngle);

                if (AntiAlias)
                    {
                    AntiAliasing(data, Width, Height, nw, nh, &pResBits[i], &pResBits[i+1], &pResBits[i+2]);
                    i += 4;
                    }
                else
                    {
                    // we get the position adjusted
                    j = SetPositionAdjusted (Width, Height, (int)nw, (int)nh);

                    // now we set the pixel
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    }
                }
            else
                {
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                }
            }

        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / (nHeight - nHalfH)));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;
}

/* Function to apply the twirl effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Twirl            => Distance value.
 * Antialias        => Smart bluring result.    
 *                                                                                  
 * Theory           => Take spiral studies, you will understand better, I'm studying
 *                     hard on this effect, because it's not too fast.
 */
void ImageEffect_DistortionFX::twirl(uint *data, int Width, int Height, int Twirl, bool AntiAlias)
{
    // if twirl value is zero, we do nothing
    
    if (Twirl == 0)
        return;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];
    
    register int h, w, i = 0, j;
    register double tw, th, nh, nw;
    int nWidth = Width, nHeight = Height;
    int nStride = GetStride (Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfNewAngle, lfAngleStep, lfAngleSum, lfCurrentRadius, lfRadMax;
    
    if (nWidth > nHeight)
        lfYScale = (double)nWidth / (double)nHeight;
    else if (nHeight > nWidth)
        lfXScale = (double)nHeight / (double)nWidth;

    // the angle step is twirl divided by 10000
    lfAngleStep = Twirl / 10000.0;
    // now, we get the minimum radius
    lfRadMax = (double)QMAX(nWidth, nHeight) / 2.0;

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        th = (lfYScale * (double)h);

        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++)
            {
            tw = (lfXScale * (double)w);

            // now, we get the distance
            lfCurrentRadius = sqrt (th * th + tw * tw);
            
            // if distance is less than maximum radius...
            if (lfCurrentRadius < lfRadMax)
                {
                // we find the angle from the center
                lfAngle = atan2 (th, tw);
                // we get the accumuled angle
                lfAngleSum = lfAngleStep * (-1.0 * (lfCurrentRadius - lfRadMax));
                // ok, we sum angle with accumuled to find a new angle
                lfNewAngle = lfAngle + lfAngleSum;

                // now we find the exact position's x and y
                nw = (double)nHalfW + cos (lfNewAngle) * (lfCurrentRadius / lfXScale);
                nh = (double)nHalfH + sin (lfNewAngle) * (lfCurrentRadius / lfYScale);

                if (AntiAlias)
                    {
                    AntiAliasing(data, Width, Height, nw, nh, &pResBits[i], &pResBits[i+1], &pResBits[i+2]);
                    i += 4;
                    }
                else
                    {
                    // we get the position adjusted
                    j = SetPositionAdjusted(Width, Height, (int)nw, (int)nh);

                    // now we set the pixel
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    }
                }
            else
                {
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                }
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / (nHeight - nHalfH)));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;    
}

/* Function to apply the Cilindrical effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Coeff            => Cilindrical value.
 * Horizontal       => Apply horizontally.
 * Vertical         => Apply vertically.
 * Antialias        => Smart bluring result. 
 *                                                                                  
 * Theory           => This is a great effect, similar to Spherize (Photoshop).    
 *                     If you understand FishEye, you will understand Cilindrical    
 *                     FishEye apply a logarithm function using a sphere radius,     
 *                     Spherize use the same function but in a rectangular        
 *                     enviroment.
 */
void ImageEffect_DistortionFX::cilindrical(uint *data, int Width, int Height, double Coeff, 
                                           bool Horizontal, bool Vertical, bool AntiAlias)

{
    if ((Coeff == 0.0) || (! (Horizontal || Vertical)))
        return;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];
    memcpy (pResBits, data, BitCount);     

    register int h, w, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height;
    int nStride = GetStride (Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfCoeffX, lfCoeffY, lfCoeffStep = Coeff / 1000.0;

    if (Horizontal)
        lfCoeffX = (double)nHalfW / log (fabs (lfCoeffStep) * nHalfW + 1.0);
    if (Vertical)
        lfCoeffY = (double)nHalfH / log (fabs (lfCoeffStep) * nHalfH + 1.0);

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++)
            {
            // we find the distance from the center
            nh = fabs ((double)h);
            nw = fabs ((double)w);

            if (Horizontal)
                {
                if (Coeff > 0.0)
                    nw = (exp (nw / lfCoeffX) - 1.0) / lfCoeffStep;
                else
                    nw = lfCoeffX * log (1.0 + (-1.0 * lfCoeffStep) * nw);
                }

            if (Vertical)
                {
                if (Coeff > 0.0)
                    nh = (exp (nh / lfCoeffY) - 1.0) / lfCoeffStep;
                else
                    nh = lfCoeffY * log (1.0 + (-1.0 * lfCoeffStep) * nh);
                }

            nw = (double)nHalfW + ((w >= 0) ? nw : -nw);
            nh = (double)nHalfH + ((h >= 0) ? nh : -nh);

            if (AntiAlias)
                {
                AntiAliasing(data, Width, Height, nw, nh, &pResBits[i], &pResBits[i+1], &pResBits[i+2]);
                i += 4;
                }
            else
                {
                // we get the position adjusted
                j = SetPositionAdjusted(Width, Height, (int)nw, (int)nh);

                // now we set the pixel
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                }
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / (nHeight - nHalfH)));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;        
}

/* Function to apply the Multiple Corners effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.       
 * Factor           => nb corners.
 * Antialias        => Smart bluring result.                      
 *                                                                                  
 * Theory           => This is an amazing function, you've never seen this before. 
 *                     I was testing some trigonometric functions, and I saw that if  
 *                     I multiply the angle by 2, the result is an image like this   
 *                     If we multiply by 3, we can create the SixCorners effect. 
 */
void ImageEffect_DistortionFX::multipleCorners(uint *data, int Width, int Height, int Factor, bool AntiAlias)
{
    if (Factor == 0) return;
    
    register int h, w, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height;
    int nStride = GetStride(Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfAngle, lfNewRadius, lfCurrentRadius, lfRadMax;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];

    lfRadMax = sqrt (nHeight * nHeight + nWidth * nWidth) / 2.0;

    // main loop
    
    for (h = 0; !m_cancel && (h < nHeight); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < nWidth); w++)
            {
            // we find the distance from the center
            nh = nHalfH - h;
            nw = nHalfW - w;

            // now, we get the distance
            lfCurrentRadius = sqrt (nh * nh + nw * nw);
            // we find the angle from the center
            lfAngle = atan2 (nh, nw) * (double)Factor;
            
            // ok, we sum angle with accumuled to find a new angle
            lfNewRadius = lfCurrentRadius * lfCurrentRadius / lfRadMax;

            // now we find the exact position's x and y
            nw = (double)nHalfW - (cos (lfAngle) * lfNewRadius);
            nh = (double)nHalfH - (sin (lfAngle) * lfNewRadius);

            if (AntiAlias)
               {
                AntiAliasing(data, Width, Height, nw, nh, &pResBits[i], &pResBits[i+1], &pResBits[i+2]);
                i += 4;
                }
            else
                {
                // we get the position adjusted
                j = SetPositionAdjusted(Width, Height, (int)nw, (int)nh);

                // now we set the pixel
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                }
            }            
            
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / nHeight));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits; 
}

/* Function to apply the Polar Coordinates effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.
 * Type             => if true Polar Coordinate to Polar else inverse.
 * Antialias        => Smart bluring result.                      
 *                                                                                  
 * Theory           => Similar to PolarCoordinates from Photoshop. We apply the polar   
 *                     transformation in a proportional (Height and Width) radius.
 */
void ImageEffect_DistortionFX::polarCoordinates(uint *data, int Width, int Height, bool Type, bool AntiAlias)
{
    register int h, w, i = 0, j;
    register double nh, nw, th, tw;
    int nWidth = Width, nHeight = Height;
    int nStride = GetStride(Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfRadius, lfRadMax;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];
        
    if (nWidth > nHeight)
        lfYScale = (double)nWidth / (double)nHeight;
    else if (nHeight > nWidth)
        lfXScale = (double)nHeight / (double)nWidth;

    lfRadMax = (double)QMAX(nHeight, nWidth) / 2.0;

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        th = lfYScale * (double)h;

        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++, i += 4)
            {
            tw = lfXScale * (double)w;

            if (Type)
                {
                // now, we get the distance
                lfRadius = sqrt (th * th + tw * tw);
                // we find the angle from the center
                lfAngle = atan2 (tw, th);
            
                // now we find the exact position's x and y
                nh = lfRadius * (double)nHeight / lfRadMax;
                nw =  lfAngle * (double) nWidth / (2 * M_PI);

                nw = (double)nHalfW + nw;
                }
            else
                {
                lfRadius = (double)(h + nHalfH) * lfRadMax / (double)nHeight;
                lfAngle  = (double)(w + nHalfW) * (2 * M_PI) / (double) nWidth;

                nw = (double)nHalfW - (lfRadius / lfXScale) * sin (lfAngle);
                nh = (double)nHalfH - (lfRadius / lfYScale) * cos (lfAngle);
                }

            if (AntiAlias)
                {
                AntiAliasing(data, Width, Height, nw, nh, &pResBits[i], &pResBits[i+1], &pResBits[i+2]);
                } 
            else
                {
                // we get the position adjusted
                j = SetPositionAdjusted(Width, Height, (int)nw, (int)nh);

                // now we set the pixel
                pResBits[ i ] = pBits[j++];
                pResBits[i+1] = pBits[j++];
                pResBits[i+2] = pBits[j++];
                }
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) ((double)h * 100.0) / (nHeight - nHalfH));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits; 
}

/* Function to apply the circular waves effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.     
 * X, Y             => Position of circle center on the image.                       
 * Amplitude        => Sinoidal maximum height                                        
 * Frequency        => Frequency value.
 * Phase            => Phase value.
 * WavesType        => If true  the amplitude is proportional to radius.
 * Antialias        => Smart bluring result.                      
 *                                                                                  
 * Theory           => Similar to Waves effect, but here I apply a senoidal function
 *                     with the angle point.                                                      
 */
void ImageEffect_DistortionFX::circularWaves(uint *data, int Width, int Height, int X, int Y, double Amplitude, 
                                             double Frequency, double Phase, bool WavesType, bool AntiAlias)
{
    if (Amplitude < 0.0) Amplitude = 0.0;
    if (Frequency < 0.0) Frequency = 0.0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    register int h, w, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height;
    int nStride = GetStride(Width);
    double lfRadius, lfRadMax, lfNewAmp = Amplitude;
    double lfFreqAngle = Frequency * ANGLE_RATIO;
    
    int BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];

    Phase *= ANGLE_RATIO;

    lfRadMax = sqrt (nHeight * nHeight + nWidth * nWidth);

    for (h = 0; !m_cancel && (h < nHeight); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < nWidth); w++)
            {
            nw = X - w;
            nh = Y - h;

            lfRadius = sqrt (nw * nw + nh * nh);

            if (WavesType)
                lfNewAmp = Amplitude * lfRadius / lfRadMax;

            nw = (double)w + lfNewAmp * sin(lfFreqAngle * lfRadius + Phase);
            nh = (double)h + lfNewAmp * cos(lfFreqAngle * lfRadius + Phase);

            if (AntiAlias)
                {
                AntiAliasing( data, Width, Height, nw, nh, &pResBits[i], &pResBits[i+1], &pResBits[i+2]);
                i += 4;
                }
            else
                {
                j = SetPositionAdjusted(Width, Height, (int)nw, (int)nh);

                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
               }
            }
                        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / nHeight));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;
}

/* Function to apply the waves effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Amplitude        => Sinoidal maximum height.                                        
 * Frequency        => Frequency value.                                                
 * FillSides        => Like a boolean variable.                                        
 * Direction        => Vertical or horizontal flag.                                    
 *                                                                                    
 * Theory           => This is an amazing effect, very funny, and very simple to    
 *                     understand. You just need understand how sin and cos works.   
 */
void ImageEffect_DistortionFX::waves(uint *data, int Width, int Height,
                                     int Amplitude, int Frequency, 
                                     bool FillSides, bool Direction)
{
    if (Amplitude < 0) Amplitude = 0;
    if (Frequency < 0) Frequency = 0;

    QImage PicSrcDC((uchar*)data, Width, Height, 32, 0, 0, QImage::IgnoreEndian);
    QImage PicDestDC(Width, Height, 32);

    register int h, w;
    
    if (Direction)        // Horizontal
        {
        int tx;
        
        for (h = 0; !m_cancel && (h < Height); h++)
            {
            tx = (int)(Amplitude * sin ((Frequency * 2) * h * (M_PI / 180)));
            bitBlt(&PicDestDC, tx, h, &PicSrcDC, 0, h, Width, 1);
            
            if (FillSides)
                {
                bitBlt(&PicDestDC, 0, h, &PicSrcDC, Width - tx, h, tx, 1);
                bitBlt(&PicDestDC, Width + tx, h, &PicSrcDC, 0, h, Width - (Width - 2 * Amplitude + tx), 1);
                }
            
            // Update the progress bar in dialog.
            m_progressBar->setValue((int) (((double)h * 100.0) / Height));
            kapp->processEvents();             
            }
        }
    else
        {
        int ty;
        
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            ty = (int)(Amplitude * sin ((Frequency * 2) * w * (M_PI / 180)));
            bitBlt(&PicDestDC, w, ty, &PicSrcDC, w, 0, 1, Height);
            
            if (FillSides)
                {
                bitBlt(&PicDestDC, w, 0, &PicSrcDC, w, Height - ty, 1, ty);
                bitBlt(&PicDestDC, w, Height + ty, &PicSrcDC, w, 0, 1, Height - (Height - 2 * Amplitude + ty));
                }
            
            // Update the progress bar in dialog.
            m_progressBar->setValue((int) (((double)w * 100.0) / Width));
            kapp->processEvents();             
            }
        }

    if (!m_cancel) 
       memcpy (data, PicDestDC.bits(), PicDestDC.numBytes());        
}

/* Function to apply the block waves effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Amplitude        => Sinoidal maximum height                                        
 * Frequency        => Frequency value                                                
 * Mode             => The mode to be applied.                                       
 *                                                                                  
 * Theory           => This is an amazing effect, very funny when amplitude and     
 *                     frequency are small values.                                  
 */
void ImageEffect_DistortionFX::blockWaves(uint *data, int Width, int Height,
                                          int Amplitude, int Frequency, bool Mode)
{
    if (Amplitude < 0) Amplitude = 0;
    if (Frequency < 0) Frequency = 0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar*    Bits = (uchar*)data;
    uchar* NewBits = new uchar[BitCount];

    int i = 0, j = 0, nw, nh;
    double Radius;
        
    for (int w = 0; !m_cancel && (w < Width); w++)
        {
        for (int h = 0; !m_cancel && (h < Height); h++)
            {
            i = h * LineWidth + 4 * w;
            nw = Width / 2 - w;
            nh = Height / 2 - h;
            Radius = sqrt (nw * nw + nh * nh);
                
            if (Mode)
                {
                nw = (int)(w + Amplitude * sin (Frequency * nw * (M_PI / 180)));
                nh = (int)(h + Amplitude * cos (Frequency * nh * (M_PI / 180)));
                }
            else
                {
                nw = (int)(w + Amplitude * sin (Frequency * w * (M_PI / 180)));
                nh = (int)(h + Amplitude * cos (Frequency * h * (M_PI / 180)));
                }

            nw = (nw < 0) ? 0 : ((nw >= Width) ? Width - 1 : nw);
            nh = (nh < 0) ? 0 : ((nh >= Height) ? Height - 1 : nh);
            j = nh * LineWidth + 4 * nw;
            NewBits[i+2] = Bits[j+2];
            NewBits[i+1] = Bits[j+1];
            NewBits[ i ] = Bits[ j ];
            }
            
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)w * 100.0) / Width));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, NewBits, BitCount);        
                
    delete [] NewBits;
}

/* Function to apply the tile effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * WSize            => Tile Width                                                        
 * HSize            => Tile Height                                                      
 * Random           => Maximum random value                                        
 *                                                                                    
 * Theory           => Similar to Tile effect from Photoshop and very easy to        
 *                     understand. We get a rectangular area using WSize and HSize and    
 *                     replace in a position with a random distance from the original    
 *                     position.                                                    
 */
void ImageEffect_DistortionFX::tile(uint *data, int Width, int Height, 
                                    int WSize, int HSize, int Random)
{
    if (WSize < 1)  WSize = 1;
    if (HSize < 1)  HSize = 1;
    if (Random < 1) Random = 1;
        
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    QImage PicSrcDC((uchar*)data, Width, Height, 32, 0, 0, QImage::IgnoreEndian);
    QImage PicDestDC(Width, Height, 32);
    
    int tx, ty, h, w;
    
    for (h = 0; !m_cancel && (h < Height); h += HSize)
        {
        for (w = 0; !m_cancel && (w < Width); w += WSize)
            {
            tx = (int)(rand() % Random) - (Random / 2);
            ty = (int)(rand() % Random) - (Random / 2);
            bitBlt (&PicDestDC, w + tx, h + ty, &PicSrcDC, w, h, WSize, HSize);
            }
            
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, PicDestDC.bits(), PicDestDC.numBytes());      
}

/* Function to apply the Neon effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.  
 * Intensity        => Intensity value                                                
 * BW               => Border Width                            
 *                                                                                  
 * Theory           => Wow, this is a great effect, you've never seen a Neon effect   
 *                     like this on PSC. Is very similar to Growing Edges (photoshop)  
 *                     Some pictures will be very interesting   
 */
void ImageEffect_DistortionFX::neon(uint *data, int Width, int Height, int Intensity, int BW)
{
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    Intensity = (Intensity < 0) ? 0 : (Intensity > 5) ? 5 : Intensity;
    BW = (BW < 1) ? 1 : (BW > 5) ? 5 : BW;
    
    uchar*    Bits = (uchar*)data;
    int i = 0, j = 0, color_1, color_2;
        
    for (int h = 0; h < Height; h++)
        {
        for (int w = 0; w < Width; w++)
            {
            for (int k = 0; k <= 2; k++)
                {
                i = h * LineWidth + 4 * w;
                j = h * LineWidth + 4 * (w + Lim_Max (w, BW, Width));
                color_1 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                j = (h + Lim_Max (h, BW, Height)) * LineWidth + 4 * w;
                color_2 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                Bits[i+k] = LimitValues ((int)(sqrt ((color_1 + color_2) << Intensity)));
                }
            }
            
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();                   
        }
}

/* Function to apply the Find Edges effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.  
 * Intensity        => Intensity value                                                
 * BW               => Border Width                            
 *                                                                                  
 * Theory           => Wow, another Photoshop filter (FindEdges). Do you understand  
 *                     Neon effect ? This is the same engine, but is inversed with   
 *                     255 - color.  
 */
void ImageEffect_DistortionFX::findEdges(uint *data, int Width, int Height, int Intensity, int BW)
{
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    Intensity = (Intensity < 0) ? 0 : (Intensity > 5) ? 5 : Intensity;
    BW = (BW < 1) ? 1 : (BW > 5) ? 5 : BW;

    uchar*    Bits = (uchar*)data;
    int i = 0, j = 0, color_1, color_2;
    
    for (int h = 0; h < Height; h++)
        {
        for (int w = 0; w < Width; w++)
            {
            for (int k = 0; k <= 2; k++)
                {
                i = h * LineWidth + 4 * w;
                j = h * LineWidth + 4 * (w + Lim_Max (w, BW, Width));
                color_1 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                j = (h + Lim_Max (h, BW, Height)) * LineWidth + 4 * w;
                color_2 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                Bits[i+k] = 255 - LimitValues ((int)(sqrt ((color_1 + color_2) << Intensity)));
                }
            }
                
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();                   
        }
}

/* Function to return the maximum radius with a determined angle                    
 *                                                                                  
 * Height           => Height of the image                                         
 * Width            => Width of the image                                           
 * Angle            => Angle to analize the maximum radius                          
 *                                                                                  
 * Theory           => This function calcule the maximum radius to that angle      
 *                     so, we can build an oval circunference                        
 */                                                                                   
double ImageEffect_DistortionFX::MaximumRadius(int Height, int Width, double Angle)
{
    double MaxRad, MinRad;
    double Radius, DegAngle = fabs (Angle * 57.295);    // Rads -> Degrees

    MinRad = QMIN (Height, Width) / 2.0;                // Gets the minor radius
    MaxRad = QMAX (Height, Width) / 2.0;                // Gets the major radius

    // Find the quadrant between -PI/2 and PI/2
    if (DegAngle > 90.0)
        Radius = ProportionalValue (MinRad, MaxRad, (DegAngle * (255.0 / 90.0)));
    else
        Radius = ProportionalValue (MaxRad, MinRad, ((DegAngle - 90.0) * (255.0 / 90.0)));
    return (Radius);
}

void ImageEffect_DistortionFX::AntiAliasing (uint *data, int Width, int Height, 
                                             double X, double Y, uchar *R, uchar *G, uchar *B)
{
    int nX, nY, j;
    double lfWeightX[2], lfWeightY[2], lfWeight;
    double lfTotalR = 0.0, lfTotalG = 0.0, lfTotalB = 0.0;
    uchar* pBits = (uchar*)data;

    nX = (int)X;
    nY = (int)Y;

    if (Y >= 0.0)
        lfWeightY[0] = 1.0 - (lfWeightY[1] = Y - (double)nY);
    else
        lfWeightY[1] = 1.0 - (lfWeightY[0] = -(Y - (double)nY));

    if (X >= 0.0)
        lfWeightX[0] = 1.0 - (lfWeightX[1] = X - (double)nX);
    else
        lfWeightX[1] = 1.0 - (lfWeightX[0] = -(X - (double)nX));

    for (int loopx = 0; loopx <= 1; loopx++)
        {
        for (int loopy = 0; loopy <= 1; loopy++)
            {
            lfWeight = lfWeightX[loopx] * lfWeightY[loopy];
            j = SetPositionAdjusted (Width, Height, nX + loopx, nY + loopy);

            lfTotalR += ((double)pBits[j++] * lfWeight);
            lfTotalG += ((double)pBits[j++] * lfWeight);
            lfTotalB += ((double)pBits[j++] * lfWeight);
            }
        }

    *B = LimitValues ((int)lfTotalB);
    *G = LimitValues ((int)lfTotalG);
    *R = LimitValues ((int)lfTotalR);
}

}  // NameSpace DigikamDistortionFXImagesPlugin

#include "imageeffect_distortionfx.moc"
