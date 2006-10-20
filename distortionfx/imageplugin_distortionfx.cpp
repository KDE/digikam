/* ============================================================
 * File  : imageplugin_distortionfx.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-02-11
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
#include <kdebug.h>

// Local includes.

#include "bannerwidget.h"
#include "imageeffect_distortionfx.h"
#include "imageplugin_distortionfx.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_distortionfx,
                            KGenericFactory<ImagePlugin_DistortionFX>("digikamimageplugin_distortionfx"));

ImagePlugin_DistortionFX::ImagePlugin_DistortionFX(QObject *parent, const char*, const QStringList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_DistortionFX")
{
    m_distortionfxAction = new KAction(i18n("Distortion Effects..."), "distortionfx", 0, 
                               this, SLOT(slotDistortionFX()),
                               actionCollection(), "imageplugin_distortionfx");
                
    setXMLFile( "digikamimageplugin_distortionfx_ui.rc" );    
        
    kdDebug() << "ImagePlugin_DistortionFX plugin loaded" << endl;
}

ImagePlugin_DistortionFX::~ImagePlugin_DistortionFX()
{
}

void ImagePlugin_DistortionFX::setEnabledActions(bool enable)
{
    m_distortionfxAction->setEnabled(enable);
}

void ImagePlugin_DistortionFX::slotDistortionFX()
{
    QString title = i18n("Distortion Effects");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamDistortionFXImagesPlugin::ImageEffect_DistortionFX dlg(parentWidget(),
            title, headerFrame);
    dlg.exec();
    delete headerFrame;
}


#include "imageplugin_distortionfx.moc"
