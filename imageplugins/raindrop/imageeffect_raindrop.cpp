/* ============================================================
 * File  : imageeffect_raindrop.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : a Digikam image plugin for to add
 *               raindrops on an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original RainDrop algorithm copyrighted 2004 by 
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
    m_timerDrop   = 0;
    m_timerAmount = 0;
    m_timerCoeff  = 0;
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
            this, SLOT(slotTimerDrop()));  
    
    connect(m_amountInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimerAmount()));  
    
    connect(m_coeffInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimerCoeff()));  
}

ImageEffect_RainDrop::~ImageEffect_RainDrop()
{
    if (m_timerDrop)
       delete m_timerDrop;
    
    if (m_timerAmount)
       delete m_timerAmount;
    
    if (m_timerCoeff)
       delete m_timerCoeff;
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

void ImageEffect_RainDrop::slotTimerDrop()
{
    if (m_timerDrop)
       {
       m_timerDrop->stop();
       delete m_timerDrop;
       }
    
    m_timerDrop = new QTimer( this );
    connect( m_timerDrop, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timerDrop->start(500, true);
}

void ImageEffect_RainDrop::slotTimerAmount()
{
    if (m_timerAmount)
       {
       m_timerAmount->stop();
       delete m_timerAmount;
       }
    
    m_timerAmount = new QTimer( this );
    connect( m_timerAmount, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timerAmount->start(500, true);
}

void ImageEffect_RainDrop::slotTimerCoeff()
{
    if (m_timerCoeff)
       {
       m_timerCoeff->stop();
       delete m_timerCoeff;
       }
    
    m_timerCoeff = new QTimer( this );
    connect( m_timerCoeff, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timerCoeff->start(500, true);
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
       // Get a copy of the selected image region for restoring at end...
       QImage orgImg, selectedImg;
       orgImg.create( w, h, 32 );
       memcpy(orgImg.bits(), data, orgImg.numBytes());
       selectedImg = orgImg.copy(selectedX, selectedY, selectedW, selectedH);
    
       rainDrops(data, w, h, d, a, c);
    
       QImage newImg, targetImg;
       newImg.create( w, h, 32 );
       memcpy(newImg.bits(), data, newImg.numBytes());
        
       bitBlt( &newImg, selectedX, selectedY, 
               &selectedImg, 0, 0, selectedImg.width(), selectedImg.height());
    
       QImage destImg = newImg.smoothScale(wp, hp);
       iface->putPreviewData((uint*)destImg.bits());
       }
    else 
       {
       rainDrops(data, w, h, d, a, c);
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
          // Get a copy of the selected image region for restoring at end...
          QImage orgImg, selectedImg;
          orgImg.create( w, h, 32 );
          memcpy(orgImg.bits(), data, orgImg.numBytes());
          selectedImg = orgImg.copy(selectedX, selectedY, selectedW, selectedH);
    
          rainDrops(data, w, h, d, a, c);
    
          QImage newImg, targetImg;
          newImg.create( w, h, 32 );
          memcpy(newImg.bits(), data, newImg.numBytes());

          bitBlt( &newImg, selectedX, selectedY, 
                  &selectedImg, 0, 0, selectedImg.width(), selectedImg.height());
               
          if ( !m_cancel ) iface->putOriginalData((uint*)newImg.bits());
          }
       else 
          {
          rainDrops(data, w, h, d, a, c);
          if ( !m_cancel ) iface->putOriginalData(data);
          }
       }
    
    delete [] data;    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the RainDrops effect (inspired from Jason Waltman code)          
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * DropSize         => Raindrop size                                              
 * Amount           => Maximum number of raindrops                                  
 * Coeff            => FishEye coefficient                                           
 *                                                                                   
 * Theory           => This functions does several math's functions and the engine   
 *                     is simple to undestand, but a little hard to implement. A       
 *                     control will indicate if there is or not a raindrop in that       
 *                     area, if not, a fisheye effect with a random size (max=DropSize)
 *                     will be applied, after this, a shadow will be applied too.       
 *                     and after this, a blur function will finish the effect.            
 */
void ImageEffect_RainDrop::rainDrops(uint *data, int Width, int Height, int DropSize, int Amount, int Coeff)
{
    int BitCount = 0;

    if (Coeff <= 0) Coeff = 1;
    
    if (Coeff > 100) Coeff = 100;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    BitCount = LineWidth * Height;
    uchar*    Bits = (uchar*)data;
    uchar* NewBits = new uchar[BitCount];
    bool** BoolMatrix = CreateBoolArray (Width, Height);

    int       i, j, k, l, m, n;                 // loop variables
    int       p, q;                             // positions
    int       Bright;                           // Bright value for shadows and highlights
    int       x, y;                             // center coordinates
    int       Counter = 0;                      // Counter (duh !)
    int       NewSize;                          // Size of current raindrop
    int       halfSize;                         // Half of the current raindrop
    int       Radius;                           // Maximum radius for raindrop
    int       BlurRadius;                       // Blur Radius
    int       BlurPixels;
    
    double    r, a;                             // polar coordinates
    double    OldRadius;                        // Radius before processing
    double    NewCoeff = (double)Coeff * 0.01;  // FishEye Coefficients
    double    s;
    double    R, G, B;

    bool      FindAnother = false;              // To search for good coordinates
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    
    srand ((uint) dt.secsTo(Y2000));
    
    // Init booleen Matrix.
    
    for (i = 0 ; !m_cancel && (i < Width) ; ++i)
       {
       for (j = 0 ; !m_cancel && (j < Height) ; ++j)
          {
          p = j * LineWidth + 4 * i;         
          NewBits[p+2] = Bits[p+2];
          NewBits[p+1] = Bits[p+1];
          NewBits[ p ] = Bits[ p ];
          BoolMatrix[i][j] = false;
          }
       }

    for (int NumBlurs = 0 ; !m_cancel && (NumBlurs <= Amount) ; ++NumBlurs)
        {
        NewSize = (int)(rand() * ((double)(DropSize - 5) / RAND_MAX) + 5);
        halfSize = NewSize / 2;
        Radius = halfSize;
        s = Radius / log (NewCoeff * Radius + 1);

        Counter = 0;
        
        do
            {
            FindAnother = false;
            y = (int)(rand() * ((double)( Width - 1) / RAND_MAX));
            x = (int)(rand() * ((double)(Height - 1) / RAND_MAX));

            if (BoolMatrix[y][x])
                FindAnother = true;
            else
                for (i = x - halfSize ; !m_cancel && (i <= x + halfSize) ; i++)
                    for (j = y - halfSize ; !m_cancel && (j <= y + halfSize) ; j++)
                        if ((i >= 0) && (i < Height) && (j >= 0) && (j < Width))
                            if (BoolMatrix[j][i])
                                FindAnother = true;

            Counter++;
            } 
        while (!m_cancel && (FindAnother && (Counter < 10000)) );

        if (Counter >= 10000)
            {
            NumBlurs = Amount;
            
            // Update the progress bar in dialog.
            m_progressBar->setValue(100);
            kapp->processEvents(); 
            
            break;
            }

        for (i = -1 * halfSize ; !m_cancel && (i < NewSize - halfSize) ; i++)
            {
            for (j = -1 * halfSize ; !m_cancel && (j < NewSize - halfSize) ; j++)
                {
                r = sqrt (i * i + j * j);
                a = atan2 (i, j);

                if (r <= Radius)
                    {
                    OldRadius = r;
                    r = (exp (r / s) - 1) / NewCoeff;

                    k = x + (int)(r * sin (a));
                    l = y + (int)(r * cos (a));

                    m = x + i;
                    n = y + j;

                    if ((k >= 0) && (k < Height) && (l >= 0) && (l < Width))
                        {
                        if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width))
                            {
                            p = k * LineWidth + 4 * l;        
                            q = m * LineWidth + 4 * n;        
                            NewBits[q+2] = Bits[p+2];
                            NewBits[q+1] = Bits[p+1];
                            NewBits[ q ] = Bits[ p ];
                            BoolMatrix[n][m] = true;
                            Bright = 0;
                                
                            if (OldRadius >= 0.9 * Radius)
                                {
                                if ((a <= 0) && (a > -2.25))
                                   Bright = -80;
                                else if ((a <= -2.25) && (a > -2.5))
                                   Bright = -40;
                                else if ((a <= 0.25) && (a > 0))
                                   Bright = -40;
                                }

                            else if (OldRadius >= 0.8 * Radius)
                                {
                                if ((a <= -0.75) && (a > -1.50))
                                    Bright = -40;
                                else if ((a <= 0.10) && (a > -0.75))
                                    Bright = -30;
                                else if ((a <= -1.50) && (a > -2.35))
                                    Bright = -30;
                                }

                            else if (OldRadius >= 0.7 * Radius)
                                {
                                if ((a <= -0.10) && (a > -2.0))
                                    Bright = -20;
                                else if ((a <= 2.50) && (a > 1.90))
                                    Bright = 60;
                                }
                                
                            else if (OldRadius >= 0.6 * Radius)
                                {
                                if ((a <= -0.50) && (a > -1.75))
                                    Bright = -20;
                                else if ((a <= 0) && (a > -0.25))
                                    Bright = 20;
                                else if ((a <= -2.0) && (a > -2.25))
                                    Bright = 20;
                                }

                            else if (OldRadius >= 0.5 * Radius)
                                {
                                if ((a <= -0.25) && (a > -0.50))
                                    Bright = 30;
                                else if ((a <= -1.75 ) && (a > -2.0))
                                    Bright = 30;
                                }

                            else if (OldRadius >= 0.4 * Radius)
                                {
                                if ((a <= -0.5) && (a > -1.75))
                                    Bright = 40;
                                }

                            else if (OldRadius >= 0.3 * Radius)
                                {
                                if ((a <= 0) && (a > -2.25))
                                    Bright = 30;
                                }

                            else if (OldRadius >= 0.2 * Radius)
                                {
                                if ((a <= -0.5) && (a > -1.75))
                                    Bright = 20;
                                }

                            NewBits[q+2] = LimitValues (NewBits[q+2] + Bright);
                            NewBits[q+1] = LimitValues (NewBits[q+1] + Bright);
                            NewBits[ q ] = LimitValues (NewBits[ q ] + Bright);
                            }
                        }
                    }
                }
            }

        BlurRadius = NewSize / 25 + 1;
            
        for (i = -1 * halfSize - BlurRadius ; !m_cancel && (i < NewSize - halfSize + BlurRadius) ; i++)
            {
            for (j = -1 * halfSize - BlurRadius ; !m_cancel && (j < NewSize - halfSize + BlurRadius) ; j++)
                {
                r = sqrt (i * i + j * j);
                
                if (r <= Radius * 1.1)
                    {
                    R = G = B = 0;
                    BlurPixels = 0;
                        
                    for (k = -1 * BlurRadius; k < BlurRadius + 1; k++)
                        for (l = -1 * BlurRadius; l < BlurRadius + 1; l++)
                            {
                            m = x + i + k;
                            n = y + j + l;
                            
                            if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width))
                                {
                                p = m * LineWidth + 4 * n;           
                                R += NewBits[p+2];
                                G += NewBits[p+1];
                                B += NewBits[ p ];
                                BlurPixels++;
                                }
                            }

                    m = x + i;
                    n = y + j;
                        
                    if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width))
                        {
                        p = m * LineWidth + 4 * n;                  
                        NewBits[p+2] = (uchar)(R / BlurPixels);
                        NewBits[p+1] = (uchar)(G / BlurPixels);
                        NewBits[ p ] = (uchar)(B / BlurPixels);
                        }
                    }
                }
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)NumBlurs * 100.0) / Amount));
        kapp->processEvents(); 
        }
    
    if (!m_cancel) 
       memcpy (data, NewBits, BitCount);        
        
    delete [] NewBits;

    FreeBoolArray (BoolMatrix, Width);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to free a dinamic boolean array                                            
 *                                                               
 * lpbArray          => Dynamic boolean array                                      
 * Columns           => The array bidimension value                                 
 *                                                                                  
 * Theory            => An easy to undestand 'for' statement                          
 */
void ImageEffect_RainDrop::FreeBoolArray (bool** lpbArray, uint Columns)
{
    for (uint i = 0; i < Columns; ++i)
        free (lpbArray[i]);
        
    free (lpbArray);
}

/* Function to create a bidimentional dinamic boolean array                  
 *                                                                                
 * Columns           => Number of columns                                          
 * Rows              => Number of rows                                              
 *                                                                                  
 * Theory            => Using 'for' statement, we can alloc multiple dinamic arrays   
 *                      To create more dimentions, just add some 'for's, ok?           
 */
bool** ImageEffect_RainDrop::CreateBoolArray (uint Columns, uint Rows)
{
    bool** lpbArray = NULL;
    lpbArray = (bool**) malloc (Columns * sizeof (bool*));

    if (lpbArray == NULL)
        return (NULL);

    for (uint i = 0; i < Columns; ++i)
    {
        lpbArray[i] = (bool*) malloc (Rows * sizeof (bool));
        if (lpbArray[i] == NULL)
        {
            FreeBoolArray (lpbArray, Columns);
            return (NULL);
        }
    }

    return (lpbArray);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.  
 
/* This function limits the RGB values                        
 *                                                                         
 * ColorValue        => Here, is an RGB value to be analized                   
 *                                                                             
 * Theory            => A color is represented in RGB value (e.g. 0xFFFFFF is     
 *                      white color). But R, G and B values has 256 values to be used   
 *                      so, this function analize the value and limits to this range   
 */   
                     
uchar ImageEffect_RainDrop::LimitValues (int ColorValue)
{
    if (ColorValue > 255)        // MAX = 255
        ColorValue = 255;        
    if (ColorValue < 0)          // MIN = 0
        ColorValue = 0;
    return ((uchar) ColorValue);
}

}  // NameSpace DigikamRainDropImagesPlugin

#include "imageeffect_raindrop.moc"
