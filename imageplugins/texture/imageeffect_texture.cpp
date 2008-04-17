/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : a plugin to apply texture over an image
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QLabel>
#include <QComboBox>
#include <QImage>
#include <QGridLayout>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "texture.h"
#include "imageeffect_texture.h"
#include "imageeffect_texture.moc"

namespace DigikamTextureImagesPlugin
{

ImageEffect_Texture::ImageEffect_Texture(QWidget* parent)
                   : Digikam::CtrlPanelDlg(parent, i18n("Apply Texture"), 
                                           "texture", false, false, true, 
                                           Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Apply Texture"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin to apply a decorative "
                                       "texture to an image."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005, Gilles Caulier\n"
                                       "(c) 2006-2007, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       "http://www.digikam.org");

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);

    QLabel *label1 = new QLabel(i18n("Type:"), gboxSettings);

    m_textureType = new QComboBox( gboxSettings );
    m_textureType->addItem( i18n("Paper") );
    m_textureType->addItem( i18n("Paper 2") );
    m_textureType->addItem( i18n("Fabric") );
    m_textureType->addItem( i18n("Burlap") );
    m_textureType->addItem( i18n("Bricks") );
    m_textureType->addItem( i18n("Bricks 2") );
    m_textureType->addItem( i18n("Canvas") );
    m_textureType->addItem( i18n("Marble") );
    m_textureType->addItem( i18n("Marble 2") );
    m_textureType->addItem( i18n("Blue Jean") );
    m_textureType->addItem( i18n("Cell Wood") );
    m_textureType->addItem( i18n("Metal Wire") );
    m_textureType->addItem( i18n("Modern") );
    m_textureType->addItem( i18n("Wall") );
    m_textureType->addItem( i18n("Moss") );
    m_textureType->addItem( i18n("Stone") );
    m_textureType->setWhatsThis( i18n("<p>Set here the texture type to apply to image."));
    
    // -------------------------------------------------------------
    
    QLabel *label2 = new QLabel(i18n("Relief:"), gboxSettings);
    
    m_blendGain    = new KIntNumInput(gboxSettings);
    m_blendGain->setRange(1, 255, 1);  
    m_blendGain->setSliderEnabled(true);
    m_blendGain->setValue(200);
    m_blendGain->setWhatsThis( i18n("<p>Set here the relief gain used to merge "
                                    "texture and image."));

    // -------------------------------------------------------------

    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());
    gridSettings->addWidget(label1, 0, 0, 1, 1);
    gridSettings->addWidget(m_textureType, 0, 1, 1, 1);
    gridSettings->addWidget(label2, 1, 0, 1, 2 );
    gridSettings->addWidget(m_blendGain, 2, 0, 1, 2 );
    
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("texture Tool Dialog");
    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);
    m_textureType->setCurrentIndex(group.readEntry("TextureType", (int)PaperTexture));
    m_blendGain->setValue(group.readEntry("BlendGain", 200));
    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);
}

void ImageEffect_Texture::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("texture Tool Dialog");
    group.writeEntry("TextureType", m_textureType->currentIndex());
    group.writeEntry("BlendGain", m_blendGain->value());
    group.sync();
}

void ImageEffect_Texture::resetValues()
{
    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);
    m_textureType->setCurrentIndex(PaperTexture);    
    m_blendGain->setValue(200);
    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);
} 

void ImageEffect_Texture::prepareEffect()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);

    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();
    QString texture = getTexturePath( m_textureType->currentIndex() );

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
    QString texture = getTexturePath( m_textureType->currentIndex() );

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Texture(iface.getOriginalImg(), this, b, texture));
}

void ImageEffect_Texture::putPreviewData()
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_Texture::putFinalData()
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
    
    return (KStandardDirs::locate("data", QString("digikam/data/") + pattern + QString(".png")));
}
    
}  // NameSpace DigikamTextureImagesPlugin
