/* ============================================================
 * File  : imageeffect_raindrop.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : a Digikam image plugin for to add
 *               raindrops on an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original RainDrop algorithm copyrighted 2004-2005 by 
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
#include <kstandarddirs.h>
#include <kprogress.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_raindrop.h"


namespace DigikamRainDropImagesPlugin
{

ImageEffect_RainDrop::ImageEffect_RainDrop(QWidget* parent)
                    : KDialogBase(Plain, i18n("Raindrops"),
                                  Help|User1|Ok|Cancel, Ok,
                                  parent, 0, true, true, i18n("&Reset Values")),
                      m_parent(parent)
{
    m_timer = 0;
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );
    m_cancel = false;
    m_dirty = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Raindrops"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to add raindrops to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Raindrops algorithm"), 
                     "pieter_voloshyn at ame.com.br"); 
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Raindrops Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------
        
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Add Raindrops to Image"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the Raindrop effect."
                                           "<p>Note: if you have previously selected an image part on Digikam editor, "
                                           "this part will be unused by the filter. You can use this way for disable "
                                           "the Raindrops effect on a human face for example.") );
    topLayout->addWidget(gbox);
    
    // -------------------------------------------------------------
                                                  
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Drop size:"), plainPage());
    
    m_dropInput = new KIntNumInput(plainPage());
    m_dropInput->setRange(0, 200, 1, true);
    m_dropInput->setValue(80);
    QWhatsThis::add( m_dropInput, i18n("<p>Set here the raindrops' size."));
    
    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_dropInput, 3);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Number:"), plainPage());
    
    m_amountInput = new KIntNumInput(plainPage());
    m_amountInput->setRange(1, 500, 1, true);
    m_amountInput->setValue(150);
    QWhatsThis::add( m_amountInput, i18n("<p>This value controls the maximum number of raindrops."));                     
    
    hlay3->addWidget(label2, 1);
    hlay3->addWidget(m_amountInput, 3);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay4 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Fish eyes:"), plainPage());
    
    m_coeffInput = new KIntNumInput(plainPage());
    m_coeffInput->setRange(1, 100, 1, true);
    m_coeffInput->setValue(30);
    QWhatsThis::add( m_coeffInput, i18n("<p>This value is the fish-eye-effect optical distortion coefficient."));                     
    
    hlay4->addWidget(label3, 1);
    hlay4->addWidget(m_coeffInput, 3);
    
    // -------------------------------------------------------------
        
    QHBoxLayout *hlay6 = new QHBoxLayout(topLayout);
    m_progressBar = new KProgress(100, plainPage(), "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    hlay6->addWidget(m_progressBar, 1);

    adjustSize();
    disableResize();  
    
    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_dropInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
    
    connect(m_amountInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
    
    connect(m_coeffInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));  
}

ImageEffect_RainDrop::~ImageEffect_RainDrop()
{
    if (m_timer)
       delete m_timer;
}

void ImageEffect_RainDrop::slotHelp()
{
    KApplication::kApplication()->invokeHelp("raindrops",
                                             "digikamimageplugins");
}

void ImageEffect_RainDrop::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_RainDrop::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_RainDrop::slotUser1()
{
    if (m_dirty)
       {
       m_cancel = true;
       }
    else
       {    
       m_dropInput->blockSignals(true);
       m_amountInput->blockSignals(true);
       m_coeffInput->blockSignals(true);
      
       m_dropInput->setValue(80);
       m_amountInput->setValue(150);
       m_coeffInput->setValue(30);

       m_dropInput->blockSignals(false);
       m_amountInput->blockSignals(false);
       m_coeffInput->blockSignals(false);
       slotEffect();
       }
} 

void ImageEffect_RainDrop::slotTimer()
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

void ImageEffect_RainDrop::slotEffect()
{
    m_dirty = true;
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    // Preview image size.
    int wp      = iface->previewWidth();
    int hp      = iface->previewHeight();
    
    // All data from the image
    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    int d       = m_dropInput->value();
    int a       = m_amountInput->value();
    int c       = m_coeffInput->value();

    // Selected data from the image
    int selectedX = iface->selectedXOrg();
    int selectedY = iface->selectedYOrg();
    int selectedW = iface->selectedWidth();
    int selectedH = iface->selectedHeight();

    m_progressBar->setValue(0); 

    // If we have a region selection in image, use it for ignore the filter modification on,
    // else, applied the filter on the full image.
    
    if (selectedW && selectedH)     
       {
       QImage orgImg, zone1, zone2, zone3, zone4, selectedImg;
       orgImg.create( w, h, 32 );
       memcpy(orgImg.bits(), data, orgImg.numBytes());
       selectedImg = orgImg.copy(selectedX, selectedY, selectedW, selectedH);
       
       // Cut the original image in 4 area without clipping region.       
       
       zone1 = orgImg.copy(0, 0, selectedX, w);
       zone2 = orgImg.copy(selectedX, 0, selectedX + selectedW, selectedY);
       zone3 = orgImg.copy(selectedX, selectedY + selectedH, selectedX + selectedW, h);
       zone4 = orgImg.copy(selectedX + selectedW, 0, w, h);
    
       // Apply effect on each area.
       
       rainDrops((uint*)zone1.bits(), zone1.width(), zone1.height(), 0, d, a, c, true, 0, 25);
       rainDrops((uint*)zone2.bits(), zone2.width(), zone2.height(), 0, d, a, c, true, 25, 50);
       rainDrops((uint*)zone3.bits(), zone3.width(), zone3.height(), 0, d, a, c, true, 50, 75);
       rainDrops((uint*)zone4.bits(), zone4.width(), zone4.height(), 0, d, a, c, true, 75, 100);
    
       // Build the target image.
       
       QImage newImg;
       newImg.create( w, h, 32 );
        
       bitBlt( &newImg, 0, 0, &zone1, 0, 0, selectedX, w );
       bitBlt( &newImg, selectedX, 0, &zone2, 0, 0, selectedX + selectedW, selectedY );
       bitBlt( &newImg, selectedX, selectedY + selectedH, &zone3, 0, 0, selectedX + selectedW, h );
       bitBlt( &newImg, selectedX + selectedW, 0, &zone4, 0, 0, w, h );
       bitBlt( &newImg, selectedX, selectedY, &selectedImg, 0, 0, selectedImg.width(), selectedImg.height());
       
       QImage destImg = newImg.smoothScale(wp, hp);
       iface->putPreviewData((uint*)destImg.bits());
       }
    else 
       {
       rainDrops(data, w, h, 0, d, a, c, true);
       QImage newImg;
       newImg.create( w, h, 32 );
       memcpy(newImg.bits(), data, newImg.numBytes());
    
       QImage destImg = newImg.smoothScale(wp, hp);
       iface->putPreviewData((uint*)destImg.bits());
       }
           
    delete [] data;
    m_progressBar->setValue(0); 
    m_previewWidget->update();

    m_dropInput->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_coeffInput->setEnabled(true);
        
    m_cancel = false;
    m_dirty = false;
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    enableButton(Ok, true);    
}

void ImageEffect_RainDrop::slotOk()
{
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    int d       = m_dropInput->value();
    int a       = m_amountInput->value();
    int c       = m_coeffInput->value();

    // Selected data from the image
    int selectedX = iface->selectedXOrg();
    int selectedY = iface->selectedYOrg();
    int selectedW = iface->selectedWidth();
    int selectedH = iface->selectedHeight();

    m_progressBar->setValue(0); 
        
    if (data) 
       {
       // If we have a region selection in image, use it for ignore the filter modification on,
       // else, applied the filter on the full image.
           
       if (selectedW && selectedH)     
          {
          QImage orgImg, zone1, zone2, zone3, zone4, selectedImg;
          orgImg.create( w, h, 32 );
          memcpy(orgImg.bits(), data, orgImg.numBytes());
          selectedImg = orgImg.copy(selectedX, selectedY, selectedW, selectedH);
       
          // Cut the original image in 4 area without clipping region.       
       
          zone1 = orgImg.copy(0, 0, selectedX, w);
          zone2 = orgImg.copy(selectedX, 0, selectedX + selectedW, selectedY);
          zone3 = orgImg.copy(selectedX, selectedY + selectedH, selectedX + selectedW, h);
          zone4 = orgImg.copy(selectedX + selectedW, 0, w, h);
    
          // Apply effect on each area.
       
          rainDrops((uint*)zone1.bits(), zone1.width(), zone1.height(), 0, d, a, c, true, 0, 25);
          rainDrops((uint*)zone2.bits(), zone2.width(), zone2.height(), 0, d, a, c, true, 25, 50);
          rainDrops((uint*)zone3.bits(), zone3.width(), zone3.height(), 0, d, a, c, true, 50, 75);
          rainDrops((uint*)zone4.bits(), zone4.width(), zone4.height(), 0, d, a, c, true, 75, 100);
    
          // Build the target image.
       
          QImage newImg;
          newImg.create( w, h, 32 );
        
          bitBlt( &newImg, 0, 0, &zone1, 0, 0, selectedX, w );
          bitBlt( &newImg, selectedX, 0, &zone2, 0, 0, selectedX + selectedW, selectedY );
          bitBlt( &newImg, selectedX, selectedY + selectedH, &zone3, 0, 0, selectedX + selectedW, h );
          bitBlt( &newImg, selectedX + selectedW, 0, &zone4, 0, 0, w, h );
          bitBlt( &newImg, selectedX, selectedY, &selectedImg, 0, 0, selectedImg.width(), selectedImg.height());
       
          if ( !m_cancel ) iface->putOriginalData(i18n("Raindrops"), (uint*)newImg.bits());
          }
       else 
          {
          rainDrops(data, w, h, 0, d, a, c, true);
          
          if ( !m_cancel ) iface->putOriginalData(i18n("Raindrops"), data);
          }
       }
    
    delete [] data;    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

/* Function to apply the RainDrops effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * MinDropSize      => It's the minimum random size for rain drop.
 * MaxDropSize      => It's the minimum random size for rain drop.
 * Amount           => It's the maximum number for rain drops inside the image.
 * Coeff            => It's the fisheye's coefficient.
 * bLimitRange      => If true, the drop will not be cut.
 * progressMin      => Min. value for progress bar (can be different if using clipping area).
 * progressMax      => Max. value for progress bar (can be different if using clipping area).
 *                                                                                   
 * Theory           => This functions does several math's functions and the engine   
 *                     is simple to undestand, but a little hard to implement. A       
 *                     control will indicate if there is or not a raindrop in that       
 *                     area, if not, a fisheye effect with a random size (max=MaxDropSize)
 *                     will be applied, after this, a shadow will be applied too.       
 *                     and after this, a blur function will finish the effect.            
 */
void ImageEffect_RainDrop::rainDrops(uint *data, int Width, int Height, int MinDropSize, int MaxDropSize, 
                                     int Amount, int Coeff, bool bLimitRange, int progressMin, int progressMax)
{
    int    nRandSize, i;
    int    nCounter = 0;
    int    nRandX, nRandY;
    int    nWidth = Width;
    int    nHeight = Height;
    bool   bResp;

    if (Amount <= 0)
        return;

    if (MinDropSize >= MaxDropSize)
        MaxDropSize = MinDropSize + 1;

    if (MaxDropSize <= 0)
        return;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar* pResBits = new uchar[BitCount];
    memcpy (pResBits, data, BitCount);     

    uchar *pStatusBits = new uchar[nHeight * nWidth];
    memset(pStatusBits, 0, sizeof(nHeight * nWidth));
    
    // Randomize.
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    for (i = 0; !m_cancel && (i < Amount); i++)
        {
        nCounter = 0;
        
        do 
            {
            nRandX = (int)(rand() * ((double)( nWidth - 1) / RAND_MAX));
            nRandY = (int)(rand() * ((double)(nHeight - 1) / RAND_MAX));

            nRandSize = (rand() % (MaxDropSize - MinDropSize)) + MinDropSize;

            bResp = CreateRainDrop (data, Width, Height, pResBits, pStatusBits, nRandX, nRandY, nRandSize, Coeff, bLimitRange);

            nCounter++;
            }
        while ((bResp == false) && (nCounter < 10000) && !m_cancel);

        if (nCounter >= 10000)
            {
            i = Amount;
            
            // Update the progress bar in dialog.
            m_progressBar->setValue(progressMax);
            kapp->processEvents(); 
            break;
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue( (int)(progressMin + ((double)(i) * (double)(progressMax-progressMin)) / (double)Amount) );
        kapp->processEvents(); 
        }

    delete [] pStatusBits;
    
    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;
}

bool ImageEffect_RainDrop::CreateRainDrop(uint *data, int Width, int Height, uchar *dest, uchar* pStatusBits,
                                          int X, int Y, int DropSize, double Coeff, bool bLimitRange)
{
    register int w, h, nw1, nh1, nw2, nh2;
    int          nHalfSize = DropSize / 2;
    int          pos1, pos2, nBright;
    double       lfRadius, lfOldRadius, lfAngle, lfDiv;
    
    uchar *pBits    = (uchar*)data; 
    uchar *pResBits = (uchar*)dest;

    int nTotalR, nTotalG, nTotalB, nBlurPixels, nBlurRadius;

    if (CanBeDropped(Width, Height, pStatusBits, X, Y, DropSize, bLimitRange))
        {
        Coeff *= 0.01; 
        lfDiv = (double)nHalfSize / log (Coeff * (double)nHalfSize + 1.0);

        for (h = -nHalfSize; !m_cancel && (h <= nHalfSize); h++)
            {
            for (w = -nHalfSize; !m_cancel && (w <= nHalfSize); w++)
                {
                lfRadius = sqrt (h * h + w * w);
                lfAngle = atan2 (h, w);

                if (lfRadius <= (double)nHalfSize)
                    {
                    lfOldRadius = lfRadius;
                    lfRadius = (exp (lfRadius / lfDiv) - 1.0) / Coeff;
                    
                    nw1 = (int)((double)X + lfRadius * cos (lfAngle));
                    nh1 = (int)((double)Y + lfRadius * sin (lfAngle));

                    nw2 = X + w;
                    nh2 = Y + h;

                    if (IsInside(Width, Height, nw1, nh1))
                        {
                        if (IsInside(Width, Height, nw2, nh2))
                            {
                            pos1 = SetPosition(Width, nw1, nh1);
                            pos2 = SetPosition(Width, nw2, nh2);

                            nBright = 0;
                            
                            if (lfOldRadius >= 0.9 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.0) && (lfAngle < 2.25))
                                    nBright = -80;
                                else if ((lfAngle >= 2.25) && (lfAngle < 2.5))
                                    nBright = -40;
                                else if ((lfAngle >= -0.25) && (lfAngle < 0.0))
                                    nBright = -40;
                                }

                            else if (lfOldRadius >= 0.8 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.75) && (lfAngle < 1.50))
                                    nBright = -40;
                                else if ((lfAngle >= -0.10) && (lfAngle < 0.75))
                                    nBright = -30;
                                else if ((lfAngle >= 1.50) && (lfAngle < 2.35))
                                    nBright = -30;
                                }

                            else if (lfOldRadius >= 0.7 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.10) && (lfAngle < 2.0))
                                    nBright = -20;
                                else if ((lfAngle >= -2.50) && (lfAngle < -1.90))
                                    nBright = 60;
                                }
                            
                            else if (lfOldRadius >= 0.6 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.50) && (lfAngle < 1.75))
                                    nBright = -20;
                                else if ((lfAngle >= 0.0) && (lfAngle < 0.25))
                                    nBright = 20;
                                else if ((lfAngle >= 2.0) && (lfAngle < 2.25))
                                    nBright = 20;
                                }

                            else if (lfOldRadius >= 0.5 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.25) && (lfAngle < 0.50))
                                    nBright = 30;
                                else if ((lfAngle >= 1.75 ) && (lfAngle < 2.0))
                                    nBright = 30;
                                } 

                            else if (lfOldRadius >= 0.4 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.5) && (lfAngle < 1.75))
                                    nBright = 40;
                                }

                            else if (lfOldRadius >= 0.3 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.0) && (lfAngle < 2.25))
                                    nBright = 30;
                                }

                            else if (lfOldRadius >= 0.2 * (double)nHalfSize)
                                {
                                if ((lfAngle >= 0.5) && (lfAngle < 1.75))
                                    nBright = 20;
                                }

                            pResBits[pos2++] = LimitValues (pBits[pos1++] + nBright);
                            pResBits[pos2++] = LimitValues (pBits[pos1++] + nBright);
                            pResBits[pos2++] = LimitValues (pBits[pos1++] + nBright);
                            }
                        }
                    }
                }
            }

        nBlurRadius = DropSize / 25 + 1;

        for (h = -nHalfSize - nBlurRadius; !m_cancel && (h <= nHalfSize + nBlurRadius); h++)
            {
            for (w = -nHalfSize - nBlurRadius; !m_cancel && (w <= nHalfSize + nBlurRadius); w++)
                {
                lfRadius = sqrt (h * h + w * w);

                if (lfRadius <= (double)nHalfSize * 1.1)
                    {
                    nTotalR = nTotalG = nTotalB = 0;
                    nBlurPixels = 0;

                    for (nh1 = -nBlurRadius; !m_cancel && (nh1 <= nBlurRadius); nh1++)
                        {
                        for (nw1 = -nBlurRadius; !m_cancel && (nw1 <= nBlurRadius); nw1++)
                            {
                            nw2 = X + w + nw1;
                            nh2 = Y + h + nh1;

                            if (IsInside (Width, Height, nw2, nh2))
                                {
                                pos1 = SetPosition (Width, nw2, nh2);

                                nTotalB += pResBits[pos1++];
                                nTotalG += pResBits[pos1++];
                                nTotalR += pResBits[pos1++];
                                nBlurPixels++;
                                }
                            }
                        }

                    nw1 = X + w;
                    nh1 = Y + h;

                    if (IsInside (Width, Height, nw1, nh1))
                        {
                        pos1 = SetPosition (Width, nw1, nh1);

                        pResBits[pos1++] = nTotalB / nBlurPixels;
                        pResBits[pos1++] = nTotalG / nBlurPixels;
                        pResBits[pos1++] = nTotalR / nBlurPixels;
                        }
                    }
                }
            }

        SetDropStatusBits (Width, Height, pStatusBits, X, Y, DropSize);
        }
    else
        return (false);

    return (true);
}


bool ImageEffect_RainDrop::CanBeDropped(int Width, int Height, uchar *pStatusBits, int X, int Y, 
                                        int DropSize, bool bLimitRange)
{
    register int w, h, i = 0;
    int nHalfSize = DropSize / 2;

    if (pStatusBits == NULL)
        return (true);
    
    for (h = Y - nHalfSize; h <= Y + nHalfSize; h++)
        {
        for (w = X - nHalfSize; w <= X + nHalfSize; w++)
            {
            if (IsInside (Width, Height, w, h))
                {
                i = h * Width + w;
                if (pStatusBits[i])
                    return (false);
                }
            else
                {
                if (bLimitRange)
                    return (false);
                }
            }
        }

    return (true);
}

bool ImageEffect_RainDrop::SetDropStatusBits (int Width, int Height, uchar *pStatusBits, int X, int Y, int DropSize)
{
    register int w, h, i = 0;
    int nHalfSize = DropSize / 2;

    if (pStatusBits == NULL)
        return (false);

    for (h = Y - nHalfSize; h <= Y + nHalfSize; h++)
        {
        for (w = X - nHalfSize; w <= X + nHalfSize; w++)
            {
            if (IsInside (Width, Height, w, h))
                {
                i = h * Width + w;
                pStatusBits[i] = 255;
                }
            }
        }

    return (true);
}

}  // NameSpace DigikamRainDropImagesPlugin

#include "imageeffect_raindrop.moc"
