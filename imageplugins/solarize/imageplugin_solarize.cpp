/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2004-02-14
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006-2007 by Gilles Caulier
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
#include "imageeffect_solarize.h"
#include "imageplugin_solarize.h"
#include "imageplugin_solarize.moc"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_solarize,
                            KGenericFactory<ImagePlugin_Solarize>("digikamimageplugin_solarize"));

ImagePlugin_Solarize::ImagePlugin_Solarize(QObject *parent, const char*,
                                                   const QStringList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_Solarize")
{
    m_solarizeAction = new KAction(i18n("Solarize Image..."), "solarizetool", 0, 
                           this, SLOT(slotSolarize()),
                           actionCollection(), "imageplugin_solarize");
                
    setXMLFile( "digikamimageplugin_solarize_ui.rc" );    
        
    DDebug() << "ImagePlugin_Solarize plugin loaded" << endl;
}

ImagePlugin_Solarize::~ImagePlugin_Solarize()
{
}

void ImagePlugin_Solarize::setEnabledActions(bool enable)
{
    m_solarizeAction->setEnabled(enable);
}

void ImagePlugin_Solarize::slotSolarize()
{
    QString title = i18n("Solarize Photograph");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamSolarizeImagesPlugin::ImageEffect_Solarize dlg(parentWidget(),
                                 title, headerFrame);
    dlg.exec();
    delete headerFrame;
}

