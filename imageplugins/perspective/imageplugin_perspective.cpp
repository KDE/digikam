/* ============================================================
 * File  : imageplugin_perspective.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-17
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

#include "imageeffect_perspective.h"
#include "imageplugin_perspective.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_perspective,
                            KGenericFactory<ImagePlugin_Perspective>("digikamimageplugin_perspective"));

ImagePlugin_Perspective::ImagePlugin_Perspective(QObject *parent, const char*, const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_Perspective")
{
    new KAction(i18n("Perspective Adjustment..."), 0, 
                this, SLOT(slotPerspective()),
                actionCollection(), "imageplugin_perspective");
                
    kdDebug() << "ImagePlugin_Perspective plugin loaded" << endl;
}

ImagePlugin_Perspective::~ImagePlugin_Perspective()
{
}

QStringList ImagePlugin_Perspective::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/&Transform/Transform/Action/imageplugin_perspective/ ");
    return guiDef;
}

void ImagePlugin_Perspective::slotPerspective()
{
    DigikamPerspectiveImagesPlugin::ImageEffect_Perspective dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_perspective.moc"
