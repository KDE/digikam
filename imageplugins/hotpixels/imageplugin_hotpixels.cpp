/* ============================================================
 * Authors: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date   : 2005-03-27
 * Description : 
 * 
 * Copyright 2005-2007 by Unai Garro
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
#include "imageeffect_hotpixels.h"
#include "imageplugin_hotpixels.h"
#include "imageplugin_hotpixels.moc"

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_hotpixels,
                           KGenericFactory<ImagePlugin_HotPixels>("digikamimageplugin_hotpixels"));

ImagePlugin_HotPixels::ImagePlugin_HotPixels(QObject *parent, const char*, const QStringList &)
                     : Digikam::ImagePlugin(parent, "ImagePlugin_HotPixels")
{
    m_hotpixelsAction = new KAction(i18n("Hot Pixels..."), "hotpixels", 0, 
                            this, SLOT(slotHotPixels()),
                            actionCollection(), "imageplugin_hotpixels");
    
    setXMLFile("digikamimageplugin_hotpixels_ui.rc");            
        
    DDebug() << "ImagePlugin_HotPixels plugin loaded" << endl;
}

ImagePlugin_HotPixels::~ImagePlugin_HotPixels()
{
}

void ImagePlugin_HotPixels::setEnabledActions(bool enable)
{
    m_hotpixelsAction->setEnabled(enable);
}

void ImagePlugin_HotPixels::slotHotPixels()
{
    DigikamHotPixelsImagesPlugin::ImageEffect_HotPixels dlg(parentWidget());
    dlg.exec();
}
