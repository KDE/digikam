/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-08-26
 * Description : 
 * 
 * Copyright 2004-2007 by Gilles Caulier
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
#include "imageeffect_emboss.h"
#include "imageplugin_emboss.h"
#include "imageplugin_emboss.moc"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_emboss,
                            KGenericFactory<ImagePlugin_Emboss>("digikamimageplugin_emboss"));

ImagePlugin_Emboss::ImagePlugin_Emboss(QObject *parent, const char*,
                                       const QStringList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_Emboss")
{
    m_embossAction = new KAction(i18n("Emboss..."), "embosstool", 0, 
                         this, SLOT(slotEmboss()),
                         actionCollection(), "imageplugin_emboss");
                
    setXMLFile( "digikamimageplugin_emboss_ui.rc" );                
    
    DDebug() << "ImagePlugin_Emboss plugin loaded" << endl;
}

ImagePlugin_Emboss::~ImagePlugin_Emboss()
{
}

void ImagePlugin_Emboss::setEnabledActions(bool enable)
{
    m_embossAction->setEnabled(enable);
}

void ImagePlugin_Emboss::slotEmboss()
{
    QString title = i18n("Emboss Image");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamEmbossImagesPlugin::ImageEffect_Emboss dlg(parentWidget(),
            title, headerFrame);
    dlg.exec();
    delete headerFrame;
}
