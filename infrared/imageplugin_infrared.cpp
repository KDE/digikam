/* ============================================================
 * File  : imageplugin_infrared.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-22
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

#include "imageeffect_infrared.h"
#include "imageplugin_infrared.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_infrared,
                            KGenericFactory<ImagePlugin_Infrared>("digikamimageplugin_infrared"));

ImagePlugin_Infrared::ImagePlugin_Infrared(QObject *parent, const char*, const QStringList &)
                     : Digikam::ImagePlugin(parent, "ImagePlugin_Infrared")
{
    m_infraredAction = new KAction(i18n("Infrared Film..."), "infrared", 0, 
                           this, SLOT(slotInfrared()),
                           actionCollection(), "imageplugin_infrared");
                
    setXMLFile( "digikamimageplugin_infrared_ui.rc" );                                
    
    kdDebug() << "ImagePlugin_Infrared plugin loaded" << endl;
}

ImagePlugin_Infrared::~ImagePlugin_Infrared()
{
}

void ImagePlugin_Infrared::setEnabledActions(bool enable)
{
    m_infraredAction->setEnabled(enable);
}

void ImagePlugin_Infrared::slotInfrared()
{
    DigikamInfraredImagesPlugin::ImageEffect_Infrared dlg(parentWidget());
    dlg.exec();
}

#include "imageplugin_infrared.moc"
