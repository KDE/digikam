/* ============================================================
 * File  : imageeffect_texture.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-10
 * Description : a digiKam image editor plugin to apply 
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
#include <qimage.h>

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
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "texture.h"
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
    m_currentRenderingMode = NoneRendering;
    m_textureFilter        = 0L;
    m_timer                = 0;
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    resize(configDialogSize("Texture Tool Dialog")); 
        
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Apply Texture"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply a decorative "
                                       "texture to an image."),
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
    QLabel *labelTitle = new QLabel( i18n("Apply Decorative Texture on Photograph"), headerFrame, "labelTitle" );
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
    
    m_imagePreviewWidget = new Digikam::ImagePannelWidget(240, 160, plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
        
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 2, marginHint(), spacingHint());
    QLabel *label1 = new QLabel(i18n("Type:"), gboxSettings);
    
    m_textureType = new QComboBox( false, gboxSettings );
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
    
    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_textureType, 0, 0, 1, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Relief:"), gboxSettings);
    
    m_blendGain = new KIntNumInput(gboxSettings);
    m_blendGain->setRange(1, 255, 1, true);  
    m_blendGain->setValue(200);
    QWhatsThis::add( m_blendGain, i18n("<p>Set here the relief gain used to merge texture and image."));

    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_blendGain, 1, 1, 1, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
        
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotFocusChanged()));
    
    connect(m_textureType, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));
            
    connect(m_blendGain, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
}

ImageEffect_Texture::~ImageEffect_Texture()
{
    saveDialogSize("Texture Tool Dialog"); 

    if (m_textureFilter)
       delete m_textureFilter;       
    
    if (m_timer)
       delete m_timer;
}


void ImageEffect_Texture::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_textureType->setEnabled(true);
    m_blendGain->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
}

void ImageEffect_Texture::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_textureFilter->stopComputation();
       }
    else
       {
       blockSignals(true);
       m_textureType->setCurrentItem(PaperTexture);    
       m_blendGain->setValue(200);
       blockSignals(false);
       slotEffect();    
       }
} 

void ImageEffect_Texture::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_textureFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void ImageEffect_Texture::slotHelp()
{
    KApplication::kApplication()->invokeHelp("texture", "digikamimageplugins");
}

void ImageEffect_Texture::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_textureFilter->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_Texture::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_textureFilter->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
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
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;     
    
    m_currentRenderingMode = PreviewRendering;
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
            
    QImage image   = m_imagePreviewWidget->getOriginalClipImage();
    QString texture = getTexturePath( m_textureType->currentItem() );
    
    int b = 255 - m_blendGain->value();
    
    m_imagePreviewWidget->setProgress(0);
    
    if (m_textureFilter)
       delete m_textureFilter;
        
    m_textureFilter = new Texture(&image, this, b, texture);
}

void ImageEffect_Texture::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    int b = 255 - m_blendGain->value();
    
    if (m_textureFilter)
       delete m_textureFilter;
       
    Digikam::ImageIface iface(0, 0);
    QImage orgImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( orgImage.bits(), data, orgImage.numBytes() );
    QString texture = getTexturePath( m_textureType->currentItem() );
    
    m_textureFilter = new Texture(&orgImage, this, b, texture);
           
    delete [] data;
}

void ImageEffect_Texture::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Texture::EventData *d = (Texture::EventData*) event->data();

    if (!d) return;
    
    if (d->starting)           // Computation in progress !
        {
        m_imagePreviewWidget->setProgress(d->progress);
        }  
    else 
        {
        if (d->success)        // Computation Completed !
            {
            switch (m_currentRenderingMode)
              {
              case PreviewRendering:
                 {
                 kdDebug() << "Preview Texture completed..." << endl;
                 
                 QImage imDest = m_textureFilter->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
    
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Texture completed..." << endl;
                 
                 Digikam::ImageIface iface(0, 0);
  
                 iface.putOriginalData(i18n("Texture"), 
                                       (uint*)m_textureFilter->getTargetImage().bits());
                    
                 kapp->restoreOverrideCursor();
                 accept();
                 break;
                 }
              }
            }
        else                   // Computation Failed !
            {
            switch (m_currentRenderingMode)
                {
                case PreviewRendering:
                    {
                    kdDebug() << "Preview Texture failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                    abortPreview();
                    break;
                    }
                
                case FinalRendering:
                    break;
                }
            }
        }
    
    delete d;        
}

QString ImageEffect_Texture::getTexturePath(int texture)
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
    
    KGlobal::dirs()->addResourceType(pattern.ascii(), KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    return (KGlobal::dirs()->findResourceDir(pattern.ascii(), pattern + ".png") + pattern + ".png" );
}
    
}  // NameSpace DigikamTextureImagesPlugin

#include "imageeffect_texture.moc"
