/* ============================================================
 * File  : imageplugin_raindrop.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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

#include "imageeffect_raindrop.h"
#include "imageplugin_raindrop.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_raindrop,
                            KGenericFactory<ImagePlugin_RainDrop>("digikamimageplugin_raindrop"));

ImagePlugin_RainDrop::ImagePlugin_RainDrop(QObject *parent, const char*, const QStringList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_RainDrop")
{
    new KAction(i18n("Rain dropping..."), 0, 
                this, SLOT(slotRainDrop()),
                actionCollection(), "imageplugin_raindrop");
                
    
    kdDebug() << "ImagePlugin_RainDrop plugin loaded" << endl;
}

ImagePlugin_RainDrop::~ImagePlugin_RainDrop()
{
}

QStringList ImagePlugin_RainDrop::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/imageplugin_raindrop/ ");
    return guiDef;
}

void ImagePlugin_RainDrop::slotRainDrop()
{
    DigikamRainDropImagesPlugin::ImageEffect_RainDrop dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_raindrop.moc"
