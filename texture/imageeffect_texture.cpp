/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-03-10
 * Description : a digiKam image editor plugin to apply 
 *               texture on image.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

// Qt includes.

#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qimage.h>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>

// Local includes.

#include "version.h"
#include "texture.h"
#include "imageeffect_texture.h"
#include "imageeffect_texture.moc"

namespace DigikamTextureImagesPlugin
{

ImageEffect_Texture::ImageEffect_Texture(QWidget* parent, QString title, QFrame* banner)
                   : Digikam::CtrlPanelDlg(parent, title, "texture", false, false, true, 
                                           Digikam::ImagePannelWidget::SeparateViewAll, banner)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Apply Texture"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply a decorative "
                                       "texture to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier\n"
                                       "(c) 2006-2007, Gilles Caulier and Marcel Wiesweg",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 2, 1, 0, spacingHint());
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

    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 1);
    gridSettings->addMultiCellWidget(m_blendGain, 2, 2, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------
        
    connect(m_textureType, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));
            
    connect(m_blendGain, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
}

ImageEffect_Texture::~ImageEffect_Texture()
{
}

void ImageEffect_Texture::renderingFinished()
{
    m_textureType->setEnabled(true);
    m_blendGain->setEnabled(true);
}

void ImageEffect_Texture::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("texture Tool Dialog");
    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);
    m_textureType->setCurrentItem(config->readNumEntry("TextureType", PaperTexture));
    m_blendGain->setValue(config->readNumEntry("BlendGain", 200));
    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);
}

void ImageEffect_Texture::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("texture Tool Dialog");
    config->writeEntry("TextureType", m_textureType->currentItem());
    config->writeEntry("BlendGain", m_blendGain->value());
    config->sync();
}

void ImageEffect_Texture::resetValues()
{
    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);
    m_textureType->setCurrentItem(PaperTexture);    
    m_blendGain->setValue(200);
    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);
} 

void ImageEffect_Texture::prepareEffect()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);

    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();
    QString texture = getTexturePath( m_textureType->currentItem() );

    int b = 255 - m_blendGain->value();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Texture(&image, this, b, texture));
}

void ImageEffect_Texture::prepareFinal()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);

    int b = 255 - m_blendGain->value();

    Digikam::ImageIface iface(0, 0);
    QString texture = getTexturePath( m_textureType->currentItem() );

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Texture(iface.getOriginalImg(), this, b, texture));
}

void ImageEffect_Texture::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_Texture::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Texture"), m_threadedFilter->getTargetImage().bits());
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

