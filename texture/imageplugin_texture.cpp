/* ============================================================
 * File  : imageplugin_texture.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-03-10
 * Description : 
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

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>

// Local includes.

#include "bannerwidget.h"
#include "imageeffect_texture.h"
#include "imageplugin_texture.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_texture,
                            KGenericFactory<ImagePlugin_Texture>("digikamimageplugin_texture"));

ImagePlugin_Texture::ImagePlugin_Texture(QObject *parent, const char*, const QStringList &)
                   : Digikam::ImagePlugin(parent, "ImagePlugin_Texture")
{
    m_textureAction = new KAction(i18n("Apply Texture..."), "texture", 0, 
                          this, SLOT(slotTexture()),
                          actionCollection(), "imageplugin_texture");
                
    setXMLFile( "digikamimageplugin_texture_ui.rc" );                                
    
    DDebug() << "ImagePlugin_Texture plugin loaded" << endl;
}

ImagePlugin_Texture::~ImagePlugin_Texture()
{
}

void ImagePlugin_Texture::setEnabledActions(bool enable)
{
    m_textureAction->setEnabled(enable);
}

void ImagePlugin_Texture::slotTexture()
{
    QString title = i18n("Apply Texture");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamTextureImagesPlugin::ImageEffect_Texture dlg(parentWidget(),
            title, headerFrame);
    dlg.exec();
    delete headerFrame;
}

#include "imageplugin_texture.moc"
