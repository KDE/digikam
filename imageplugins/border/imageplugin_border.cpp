/* ============================================================
 * File  : imageplugin_border.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-20
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

#include "imageeffect_border.h"
#include "imageplugin_border.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_border,
                            KGenericFactory<ImagePlugin_Border>("digikamimageplugin_border"));

ImagePlugin_Border::ImagePlugin_Border(QObject *parent, const char*, const QStringList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_Border")
{
    new KAction(i18n("Add Border..."), 0, 
                this, SLOT(slotBorder()),
                actionCollection(), "imageplugin_border");
                
    
    kdDebug() << "ImagePlugin_Border plugin loaded" << endl;
}

ImagePlugin_Border::~ImagePlugin_Border()
{
}

QStringList ImagePlugin_Border::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/&Image/Image/Action/imageplugin_border/ ");
    return guiDef;
}

void ImagePlugin_Border::slotBorder()
{
    DigikamBorderImagesPlugin::ImageEffect_Border dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_border.moc"
