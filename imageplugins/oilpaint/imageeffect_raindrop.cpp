/* ============================================================
 * File  : imageeffect_raindrop.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : a Digikam image plugin for to simulate 
 *               a rain droppping on an image.
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * Original RainDrop algorithm copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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

// Digikam includes.

#include <digikam/imageiface.h>
#include <digikam/imagewidget.h>

// Local includes.

#include "version.h"
#include "imageeffect_raindrop.h"


namespace DigikamRainDropImagesPlugin
{

ImageEffect_RainDrop::ImageEffect_RainDrop(QWidget* parent)
                    : KDialogBase(Plain, i18n("Rain Dropping Image"),
                                  Help|User1|Ok|Cancel, Ok,
                                  parent, 0, true, true, i18n("&Reset values")),
                      m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Rain Dropping"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A rain dropping image effect plugin for Digikam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://digikam.sourceforge.net");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Rain dropping algorithm"), 
                     "pieter_voloshyn at ame.com.br"); 
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Rain Dropping handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Rain Dropping Image"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left", "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Rain Dropping"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320,frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);
    
    // -------------------------------------------------------------
                                                  
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Drop size:"), plainPage());
    
    m_dropSlider = new QSlider(1, 200, 1, 1, Qt::Horizontal, plainPage(), "m_dropSlider");
    m_dropSlider->setTickmarks(QSlider::Below);
    m_dropSlider->setTickInterval(20);
    m_dropSlider->setTracking ( false );
    
    m_dropSpinBox = new QSpinBox(1, 200, 1, plainPage(), "m_dropSpinBox");
    
    whatsThis = i18n("<p>Set here the raindrop size.");
    QWhatsThis::add( m_dropSpinBox, whatsThis);
    QWhatsThis::add( m_dropSlider, whatsThis);
    
    hlay2->addWidget(label1, 1);
    hlay2->addWidget(m_dropSlider, 3);
    hlay2->addWidget(m_dropSpinBox, 1);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Amount:"), plainPage());
    
    m_amountSlider = new QSlider(1, 500, 1, 1, Qt::Horizontal, plainPage(), "m_amountSlider");
    m_amountSlider->setTickmarks(QSlider::Below);
    m_amountSlider->setTickInterval(50);
    m_amountSlider->setTracking ( false );  
    
    m_amountSpinBox = new QSpinBox(1, 500, 1, plainPage(), "m_amountSpinBox");
    
    whatsThis = i18n("<p>This value controls the maximum number of raindrops.");
    QWhatsThis::add( m_amountSpinBox, whatsThis);
    QWhatsThis::add( m_amountSlider, whatsThis);                     
    
    hlay3->addWidget(label2, 1);
    hlay3->addWidget(m_amountSlider, 3);
    hlay3->addWidget(m_amountSpinBox, 1);
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay4 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Fish eyes:"), plainPage());
    
    m_coeffSlider = new QSlider(1, 100, 1, 1, Qt::Horizontal, plainPage(), "m_coeffSlider");
    m_coeffSlider->setTickmarks(QSlider::Below);
    m_coeffSlider->setTickInterval(10);
    m_coeffSlider->setTracking ( false );  
    
    m_coeffSpinBox = new QSpinBox(1, 100, 1, plainPage(), "m_coeffSpinBox");
    
    whatsThis = i18n("<p>This value is the fish eyes effect optical distortion coefficient.");
    QWhatsThis::add( m_coeffSpinBox, whatsThis);
    QWhatsThis::add( m_coeffSlider, whatsThis);                     
    
    hlay4->addWidget(label3, 1);
    hlay4->addWidget(m_coeffSlider, 3);
    hlay4->addWidget(m_coeffSpinBox, 1);
    
    // -------------------------------------------------------------
        
    QHBoxLayout *hlay6 = new QHBoxLayout(topLayout);
    m_progressBar = new KProgress(100, plainPage(), "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    hlay6->addWidget(m_progressBar, 1);

    // -------------------------------------------------------------
    
    adjustSize();
    slotUser1();    // Reset all parameters to the default values.        
    
    connect(m_dropSlider, SIGNAL(valueChanged(int)),
            m_dropSpinBox, SLOT(setValue(int)));
    connect(m_dropSpinBox, SIGNAL(valueChanged(int)),
            m_dropSlider, SLOT(setValue(int)));            
    connect(m_dropSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));            
            
    connect(m_amountSlider, SIGNAL(valueChanged(int)),
            m_amountSpinBox, SLOT(setValue(int)));
    connect(m_amountSpinBox, SIGNAL(valueChanged(int)),
            m_amountSlider, SLOT(setValue(int)));   
    connect(m_amountSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     

    connect(m_coeffSlider, SIGNAL(valueChanged(int)),
            m_coeffSpinBox, SLOT(setValue(int)));
    connect(m_coeffSpinBox, SIGNAL(valueChanged(int)),
            m_coeffSlider, SLOT(setValue(int)));   
    connect(m_coeffSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     
}

ImageEffect_RainDrop::~ImageEffect_RainDrop()
{
}

void ImageEffect_RainDrop::slotHelp()
{
    KApplication::kApplication()->invokeHelp("raindrop",
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
    blockSignals(true);
    m_dropSlider->setValue(50);
    m_dropSpinBox->setValue(50);
    
    m_amountSlider->setValue(50);
    m_amountSpinBox->setValue(50);
    
    m_coeffSlider->setValue(30);
    m_coeffSpinBox->setValue(30);
    slotEffect();
    blockSignals(false);
} 

void ImageEffect_RainDrop::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    int wp      = iface->previewWidth();
    int hp      = iface->previewHeight();
    
    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    int d       = m_dropSpinBox->value();
    int a       = m_amountSpinBox->value();
    int c       = m_coeffSpinBox->value();

    m_progressBar->setValue(0); 
    rainDrops(data, w, h, d, a, c);
    
    QImage image;
    image.create( w, h, 32 );
    image.setAlphaBuffer(true) ;
    memcpy(image.bits(), data, image.numBytes());
    QImage newImage = image.scale(wp, hp);
    
    iface->putPreviewData((uint*)newImage.bits());

    delete [] data;

    m_previewWidget->update();
}

void ImageEffect_RainDrop::slotOk()
{
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    int d       = m_dropSpinBox->value();
    int a       = m_amountSpinBox->value();
    int c       = m_coeffSpinBox->value();

    if (data) 
       {
       m_progressBar->setValue(0); 
       rainDrops(data, w, h, d, a, c);    
       
       if ( !m_cancel )
          iface->putOriginalData(data);
        
       delete [] data;
       }
    
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

    int LineWidth = Width * 4;                     // ??
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

    srand ((uint) 100000);
    
        for (i = 0 ; !m_cancel && (i < Width) ; ++i)
            for (j = 0 ; !m_cancel && (j < Height) ; ++j)
            {
                p = j * LineWidth + 4 * i;            // ??
                NewBits[p+2] = Bits[p+2];
                NewBits[p+1] = Bits[p+1];
                NewBits[ p ] = Bits[ p ];
                BoolMatrix[i][j] = false;
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
            } while (!m_cancel && (FindAnother && (Counter < 10000)) );

            if (Counter >= 10000)
            {
                NumBlurs = Amount;
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
                                p = k * LineWidth + 4 * l;        // ??
                                q = m * LineWidth + 4 * n;        // ??
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
                                    p = m * LineWidth + 4 * n;           // ??
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
                            p = m * LineWidth + 4 * n;                  // ??
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
