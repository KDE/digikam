/* ============================================================
 * File  : imageplugin_blurfx.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-02-09
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
#include "imageeffect_blurfx.h"
#include "imageplugin_blurfx.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_blurfx,
                            KGenericFactory<ImagePlugin_BlurFX>("digikamimageplugin_blurfx"));

ImagePlugin_BlurFX::ImagePlugin_BlurFX(QObject *parent, const char*, const QStringList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_BlurFX")
{
    m_blurfxAction = new KAction(i18n("Blur Effects..."), "blurfx", 0, 
                         this, SLOT(slotBlurFX()),
                         actionCollection(), "imageplugin_blurfx");
                
    setXMLFile( "digikamimageplugin_blurfx_ui.rc" );    
        
    DDebug() << "ImagePlugin_BlurFX plugin loaded" << endl;
}

ImagePlugin_BlurFX::~ImagePlugin_BlurFX()
{
}

void ImagePlugin_BlurFX::setEnabledActions(bool enable)
{
    m_blurfxAction->setEnabled(enable);
}

void ImagePlugin_BlurFX::slotBlurFX()
{
    QString title = i18n("Apply Blurring Special Effect to Photograph");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamBlurFXImagesPlugin::ImageEffect_BlurFX dlg(parentWidget(),
            title, headerFrame);
    dlg.exec();
    delete headerFrame;
}


#include "imageplugin_blurfx.moc"
