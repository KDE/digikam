/* ============================================================
 * File  : imageplugin_restoration.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-26
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

#include "imageeffect_restoration.h"
#include "imageplugin_restoration.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_restoration,
                            KGenericFactory<ImagePlugin_Restoration>("digikamimageplugin_restoration"));

ImagePlugin_Restoration::ImagePlugin_Restoration(QObject *parent, const char*, const QStringList &)
                       : Digikam::ImagePlugin(parent, "ImagePlugin_Restoration")
{
    m_restorationAction = new KAction(i18n("Restoration..."), "restoration", 0, 
                              this, SLOT(slotRestoration()),
                              actionCollection(), "imageplugin_restoration");
                
    setXMLFile( "digikamimageplugin_restoration_ui.rc" );                                
    
    kdDebug() << "ImagePlugin_Restoration plugin loaded" << endl;
}

ImagePlugin_Restoration::~ImagePlugin_Restoration()
{
}

void ImagePlugin_Restoration::setEnabledActions(bool enable)
{
    m_restorationAction->setEnabled(enable);
}

void ImagePlugin_Restoration::slotRestoration()
{
    DigikamRestorationImagesPlugin::ImageEffect_Restoration dlg(parentWidget());
    dlg.exec();
}

#include "imageplugin_restoration.moc"
