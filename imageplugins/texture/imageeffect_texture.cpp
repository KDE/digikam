/* ============================================================
 * File  : imageeffect_texture.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-10
 * Description : a digiKam image editor plugin for apply 
 *               texture on image.
 * 
 * Copyright 2005 by Gilles Caulier
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

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8)) 
 
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
#include <qtimer.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_texture.h"

namespace DigikamTextureImagesPlugin
{

ImageEffect_Texture::ImageEffect_Texture(QWidget* parent)
                   : KDialogBase(Plain, i18n("Apply Texture"),
                                 Help|User1|Ok|Cancel, Ok,
                                 parent, 0, true, true,
                                 i18n("&Reset Values")),
                     m_parent(parent)
{
    m_timer = 0;
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Apply Texture"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply a decorative texture to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Apply Texture Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
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
    QLabel *labelTitle = new QLabel( i18n("Apply Decorative Texture to Image"), headerFrame, "labelTitle" );
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
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, i18n("Preview"), plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
        
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Type:"), plainPage());
    
    m_textureType = new QComboBox( false, plainPage() );
    m_textureType->insertItem( i18n("Paper") );
    m_textureType->insertItem( i18n("Paper 2") );
    m_textureType->insertItem( i18n("Fabric") );
    m_textureType->insertItem( i18n("Burlap") );
    m_textureType->insertItem( i18n("Bricks") );
    m_textureType->insertItem( i18n("Bricks 2") );
    m_textureType->insertItem( i18n("Canvas") );
    m_textureType->insertItem( i18n("Marble") );
    m_textureType->insertItem( i18n("Marble 2") );
    m_textureType->insertItem( i18n("Blue Jean") );
    m_textureType->insertItem( i18n("Cell Wood") );
    m_textureType->insertItem( i18n("Metal Wire") );
    m_textureType->insertItem( i18n("Modern") );
    m_textureType->insertItem( i18n("Wall") );
    m_textureType->insertItem( i18n("Moss") );
    m_textureType->insertItem( i18n("Stone") );
    QWhatsThis::add( m_textureType, i18n("<p>Set here the texture type to apply on image."));
    
    hlay->addWidget(label1, 3);
    hlay->addWidget(m_textureType, 1);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Relief:"), plainPage());
    
    m_blendGain = new KIntNumInput(plainPage());
    m_blendGain->setRange(1, 255, 1, true);  
    m_blendGain->setValue(200);
    QWhatsThis::add( m_blendGain, i18n("<p>Set here the relief gain used to merge texture and image."));

    hlay2->addWidget(label2, 1);
    hlay2->addWidget(m_blendGain, 4);
    
    // -------------------------------------------------------------
        
    adjustSize();
    disableResize(); 
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_textureType, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));
            
    connect(m_blendGain, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
}

ImageEffect_Texture::~ImageEffect_Texture()
{
    if (m_timer)
       delete m_timer;
}

void ImageEffect_Texture::slotUser1()
{
    blockSignals(true);

    m_textureType->setCurrentItem(PaperTexture);    // Solid.
    m_blendGain->setValue(200);
    
    blockSignals(false);
    slotEffect();    
} 

void ImageEffect_Texture::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_Texture::slotHelp()
{
    KApplication::kApplication()->invokeHelp("texture", "digikamimageplugins");
}

void ImageEffect_Texture::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_Texture::slotTimer()
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

void ImageEffect_Texture::slotEffect()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setEnable(false);
    
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    uint* data   = (uint *)image.bits();
    int   w      = image.width();
    int   h      = image.height();
    int   b      = 255 - m_blendGain->value();
    int   t      = m_textureType->currentItem();

    m_imagePreviewWidget->setProgress(0);
    texture(data, w, h, b, t);
    
    if (m_cancel) return;
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageData(image);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_textureType->setEnabled(true);
    m_blendGain->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);
}

void ImageEffect_Texture::slotOk()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
        
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    int b      = 255 - m_blendGain->value();
    int t      = m_textureType->currentItem();
    
    m_imagePreviewWidget->setProgress(0);
    texture(data, w, h, b, t);

    if ( !m_cancel )
       iface.putOriginalData(i18n("Apply Texture"), data);
       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

// This method is based on the Simulate Texture Film tutorial from GimpGuru.org web site 
// available at this url : http://www.gimpguru.org/Tutorials/SimulatedTexture/

void ImageEffect_Texture::texture(uint* data, int width, int height, int blendGain, int texture)
{
    QString pattern;
    
    switch (texture)
       {
       case PaperTexture: 
          pattern = "paper-texture";
          break;
          
       case Paper2Texture: 
          pattern = "paper2-texture";
          break;

       case FabricTexture: 
          pattern = "fabric-texture";
          break;
       
       case BurlapTexture:
          pattern = "burlap-texture";
          break;

       case BricksTexture:
          pattern = "bricks-texture";
          break;

       case Bricks2Texture:
          pattern = "bricks2-texture";
          break;
                           
       case CanvasTexture:
          pattern = "canvas-texture";
          break;
                           
       case MarbleTexture:
          pattern = "marble-texture";
          break;
                           
       case Marble2Texture:
          pattern = "marble2-texture";
          break;

       case BlueJeanTexture:
          pattern = "bluejean-texture";
          break;
                                     
       case CellWoodTexture:
          pattern = "cellwood-texture";
          break;
       
       case MetalWireTexture:
          pattern = "metalwire-texture";
          break;
       
       case ModernTexture:
          pattern = "modern-texture";
          break;
       
       case WallTexture:
          pattern = "wall-texture";
          break;

       case MossTexture:
          pattern = "moss-texture";
          break;
                    
       case StoneTexture:
          pattern = "stone-texture";
          break;
       }

    QPixmap texturePixmap(width, height);
    
    KGlobal::dirs()->addResourceType(pattern.ascii(), KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    QString path = KGlobal::dirs()->findResourceDir(pattern.ascii(), pattern + ".png");
    
    // Texture tile.
    QPainter p(&texturePixmap);
    p.fillRect( 0, 0, texturePixmap.width(), texturePixmap.height(),
                QBrush::QBrush(Qt::black,
                QPixmap::QPixmap(path + pattern + ".png")) );
    p.end();

    QImage textureImg = texturePixmap.convertToImage();
    
    int   nStride = GetStride(width);    
    int LineWidth = width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar* pInData = (uchar*)data;
    uchar* pTeData = textureImg.bits();
        
    int    BitCount = LineWidth * height;
    uchar* pTransparent = new uchar[BitCount];    
    memset(pTransparent, 128, BitCount);

    register int i = 0, h, w;
    
    
    for (h = 0; !m_cancel && (h < height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < width); w++)
            {     
            // Make textured transparent layout.
            
            pTeData[i++] = (pTeData[i] * (255 - blendGain) + pTransparent[i] * blendGain) >> 8;    // Blue.
            pTeData[i++] = (pTeData[i] * (255 - blendGain) + pTransparent[i] * blendGain) >> 8;    // Green.
            pTeData[i++] = (pTeData[i] * (255 - blendGain) + pTransparent[i] * blendGain) >> 8;    // Red.
            i++;                                                                                   // Alpha.
            }
        
        // Update progress bar in dialog.
        m_imagePreviewWidget->setProgress((int) (((double)h * 50.0) / height));
        kapp->processEvents(); 
        }
            
    uint   tmp, tmpM;
    i = 0;

    for (h = 0; !m_cancel && (h < height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < width); w++)
            {     
            // Merge layout and image using overlay method.
            
            pInData[i++] = INT_MULT(pInData[i], pInData[i] + INT_MULT(2 * pTeData[i], 255 - pInData[i], tmpM), tmp);  // Blue.
            pInData[i++] = INT_MULT(pInData[i], pInData[i] + INT_MULT(2 * pTeData[i], 255 - pInData[i], tmpM), tmp);  // Green.
            pInData[i++] = INT_MULT(pInData[i], pInData[i] + INT_MULT(2 * pTeData[i], 255 - pInData[i], tmpM), tmp);  // Red.
            i++;                                                                                                      // Alpha.
            }
        
        // Update progress bar in dialog.
        m_imagePreviewWidget->setProgress((int) (50.0 + ((double)h * 50.0) / height));
        kapp->processEvents(); 
        }
        
    delete [] pTransparent;
}

}  // NameSpace DigikamTextureImagesPlugin

#include "imageeffect_texture.moc"
