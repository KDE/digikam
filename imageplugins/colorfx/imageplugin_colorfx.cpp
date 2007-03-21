/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com> 
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

#include "ddebug.h"
#include "imageeffect_colorfx.h"
#include "imageplugin_colorfx.h"
#include "imageplugin_colorfx.moc"

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_colorfx,
                           KGenericFactory<ImagePlugin_ColorFX>("digikamimageplugin_colorfx"));

ImagePlugin_ColorFX::ImagePlugin_ColorFX(QObject *parent, const char*, const QStringList &)
                   : Digikam::ImagePlugin(parent, "ImagePlugin_ColorFX")
{
    m_solarizeAction = new KAction(i18n("Color Effects..."), "colorfx", 0, 
                           this, SLOT(slotColorFX()),
                           actionCollection(), "imageplugin_colorfx");
                
    setXMLFile( "digikamimageplugin_colorfx_ui.rc" );    
        
    DDebug() << "ImagePlugin_ColorFX plugin loaded" << endl;
}

ImagePlugin_ColorFX::~ImagePlugin_ColorFX()
{
}

void ImagePlugin_ColorFX::setEnabledActions(bool enable)
{
    m_solarizeAction->setEnabled(enable);
}

void ImagePlugin_ColorFX::slotColorFX()
{
    DigikamColorFXImagesPlugin::ImageEffect_ColorFX dlg(parentWidget());
    dlg.exec();
}

